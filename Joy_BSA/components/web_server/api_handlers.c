#include "api_handlers.h"
#include "ext_flash.h"
#include "config_mgr.h"
#include "audio_player.h"
#include "led_ctrl.h"
#include "wifi_mgr.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "cJSON.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

static const char *TAG = "api_handlers";

#define MAX_REQ_BODY 2048

static char *read_request_body(httpd_req_t *req)
{
    int content_len = req->content_len;
    if (content_len <= 0 || content_len > MAX_REQ_BODY) {
        return NULL;
    }
    char *buf = malloc(content_len + 1);
    if (buf == NULL) return NULL;

    int received = httpd_req_recv(req, buf, content_len);
    if (received <= 0) {
        free(buf);
        return NULL;
    }
    buf[received] = '\0';
    return buf;
}

static void send_json_ok(httpd_req_t *req, const char *msg)
{
    httpd_resp_set_type(req, "application/json");
    char resp[128];
    snprintf(resp, sizeof(resp), "{\"status\":\"ok\",\"message\":\"%s\"}", msg);
    httpd_resp_sendstr(req, resp);
}

static void send_json_error(httpd_req_t *req, int code, const char *msg)
{
    httpd_resp_set_status(req, (code == 404) ? "404 Not Found" : "400 Bad Request");
    httpd_resp_set_type(req, "application/json");
    char resp[128];
    snprintf(resp, sizeof(resp), "{\"status\":\"error\",\"message\":\"%s\"}", msg);
    httpd_resp_sendstr(req, resp);
}

// GET /api/status
static esp_err_t status_handler(httpd_req_t *req)
{
    cJSON *root = cJSON_CreateObject();

    // WiFi status
    char ip[16] = "0.0.0.0";
    wifi_mgr_get_ip(ip, sizeof(ip));
    cJSON_AddBoolToObject(root, "wifi_connected", wifi_mgr_is_connected());
    cJSON_AddStringToObject(root, "ip", ip);

    // Flash status
    cJSON_AddNumberToObject(root, "flash_free", ext_flash_get_free_space());
    cJSON_AddNumberToObject(root, "flash_total", ext_flash_get_total_space());

    // Audio status
    cJSON_AddBoolToObject(root, "audio_playing", audio_player_is_playing());
    char *sel = config_mgr_get_str("audio_file");
    cJSON_AddStringToObject(root, "selected_audio", sel ? sel : "");
    free(sel);

    // Volume
    int32_t vol = config_mgr_get_i32("volume", 80);
    cJSON_AddNumberToObject(root, "volume", vol);

    char *json = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json);
    free(json);
    cJSON_Delete(root);
    return ESP_OK;
}

// GET /api/wifi/scan
static esp_err_t wifi_scan_handler(httpd_req_t *req)
{
    uint16_t num = 20;
    wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * num);
    if (ap_records == NULL) {
        send_json_error(req, 500, "Memory allocation failed");
        return ESP_OK;
    }

    wifi_scan_config_t scan_cfg = { .show_hidden = false };
    esp_wifi_scan_start(&scan_cfg, true);
    esp_wifi_scan_get_ap_num(&num);
    if (num > 20) num = 20;
    esp_wifi_scan_get_ap_records(&num, ap_records);

    cJSON *root = cJSON_CreateArray();
    for (int i = 0; i < num; i++) {
        cJSON *ap = cJSON_CreateObject();
        cJSON_AddStringToObject(ap, "ssid", (char *)ap_records[i].ssid);
        cJSON_AddNumberToObject(ap, "rssi", ap_records[i].rssi);
        cJSON_AddNumberToObject(ap, "auth", ap_records[i].authmode);
        cJSON_AddItemToArray(root, ap);
    }
    free(ap_records);

    char *json = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json);
    free(json);
    cJSON_Delete(root);
    return ESP_OK;
}

// POST /api/wifi/config
static esp_err_t wifi_config_handler(httpd_req_t *req)
{
    char *body = read_request_body(req);
    if (body == NULL) {
        send_json_error(req, 400, "Invalid request body");
        return ESP_OK;
    }

    cJSON *root = cJSON_Parse(body);
    free(body);
    if (root == NULL) {
        send_json_error(req, 400, "Invalid JSON");
        return ESP_OK;
    }

    cJSON *ssid = cJSON_GetObjectItem(root, "ssid");
    cJSON *pass = cJSON_GetObjectItem(root, "password");

    if (!cJSON_IsString(ssid) || strlen(ssid->valuestring) == 0) {
        cJSON_Delete(root);
        send_json_error(req, 400, "SSID is required");
        return ESP_OK;
    }

    config_mgr_set_str("wifi_ssid", ssid->valuestring);
    config_mgr_set_str("wifi_pass", cJSON_IsString(pass) ? pass->valuestring : "");

    cJSON_Delete(root);
    send_json_ok(req, "WiFi credentials saved. Restarting WiFi...");

    // Reconnect with new credentials
    char *new_ssid = config_mgr_get_str("wifi_ssid");
    char *new_pass = config_mgr_get_str("wifi_pass");
    if (new_ssid) {
        wifi_mgr_start_sta(new_ssid, new_pass ? new_pass : "");
        free(new_ssid);
        free(new_pass);
    }

    return ESP_OK;
}

// GET /api/audio/list
static esp_err_t audio_list_handler(httpd_req_t *req)
{
    DIR *dir = opendir(EXT_FLASH_AUDIO_DIR);
    cJSON *root = cJSON_CreateArray();

    if (dir != NULL) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                cJSON *file = cJSON_CreateObject();
                cJSON_AddStringToObject(file, "name", entry->d_name);

                char filepath[160];
                snprintf(filepath, sizeof(filepath), "%s/%s", EXT_FLASH_AUDIO_DIR, entry->d_name);
                struct stat st;
                if (stat(filepath, &st) == 0) {
                    cJSON_AddNumberToObject(file, "size", st.st_size);
                }
                cJSON_AddItemToArray(root, file);
            }
        }
        closedir(dir);
    }

    char *json = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json);
    free(json);
    cJSON_Delete(root);
    return ESP_OK;
}

// POST /api/audio/upload - handled in file_upload.c
extern esp_err_t file_upload_handler(httpd_req_t *req);

// DELETE /api/audio/delete
static esp_err_t audio_delete_handler(httpd_req_t *req)
{
    char *body = read_request_body(req);
    if (body == NULL) {
        send_json_error(req, 400, "Invalid body");
        return ESP_OK;
    }

    cJSON *root = cJSON_Parse(body);
    free(body);
    if (root == NULL) {
        send_json_error(req, 400, "Invalid JSON");
        return ESP_OK;
    }

    cJSON *filename = cJSON_GetObjectItem(root, "filename");
    if (!cJSON_IsString(filename)) {
        cJSON_Delete(root);
        send_json_error(req, 400, "filename required");
        return ESP_OK;
    }

    // Prevent path traversal
    if (strchr(filename->valuestring, '/') || strchr(filename->valuestring, '\\') || strstr(filename->valuestring, "..")) {
        cJSON_Delete(root);
        send_json_error(req, 400, "Invalid filename");
        return ESP_OK;
    }

    char filepath[160];
    snprintf(filepath, sizeof(filepath), "%s/%s", EXT_FLASH_AUDIO_DIR, filename->valuestring);

    if (remove(filepath) == 0) {
        ESP_LOGI(TAG, "Deleted: %s", filepath);
        send_json_ok(req, "File deleted");
    } else {
        send_json_error(req, 404, "File not found");
    }

    cJSON_Delete(root);
    return ESP_OK;
}

// POST /api/audio/select
static esp_err_t audio_select_handler(httpd_req_t *req)
{
    char *body = read_request_body(req);
    if (body == NULL) {
        send_json_error(req, 400, "Invalid body");
        return ESP_OK;
    }

    cJSON *root = cJSON_Parse(body);
    free(body);
    if (root == NULL) {
        send_json_error(req, 400, "Invalid JSON");
        return ESP_OK;
    }

    cJSON *filename = cJSON_GetObjectItem(root, "filename");
    if (cJSON_IsString(filename)) {
        config_mgr_set_str("audio_file", filename->valuestring);
        ESP_LOGI(TAG, "Selected audio: %s", filename->valuestring);
        send_json_ok(req, "Audio file selected");
    } else {
        send_json_error(req, 400, "filename required");
    }

    cJSON_Delete(root);
    return ESP_OK;
}

// POST /api/audio/play
static esp_err_t audio_play_handler(httpd_req_t *req)
{
    char *body = read_request_body(req);
    cJSON *root = body ? cJSON_Parse(body) : NULL;
    free(body);

    const char *filename = NULL;
    if (root) {
        cJSON *fn = cJSON_GetObjectItem(root, "filename");
        if (cJSON_IsString(fn)) filename = fn->valuestring;
    }

    // Use selected file if no filename provided
    char *sel = NULL;
    if (filename == NULL) {
        sel = config_mgr_get_str("audio_file");
        filename = sel;
    }

    if (filename == NULL || strlen(filename) == 0) {
        cJSON_Delete(root);
        free(sel);
        send_json_error(req, 400, "No audio file specified");
        return ESP_OK;
    }

    char filepath[160];
    snprintf(filepath, sizeof(filepath), "%s/%s", EXT_FLASH_AUDIO_DIR, filename);

    esp_err_t ret = audio_player_play(filepath);
    if (ret == ESP_OK) {
        send_json_ok(req, "Playing");
    } else {
        send_json_error(req, 500, "Playback failed");
    }

    cJSON_Delete(root);
    free(sel);
    return ESP_OK;
}

// POST /api/audio/stop
static esp_err_t audio_stop_handler(httpd_req_t *req)
{
    audio_player_stop();
    send_json_ok(req, "Stopped");
    return ESP_OK;
}

// GET /api/config
static esp_err_t config_get_handler(httpd_req_t *req)
{
    cJSON *root = cJSON_CreateObject();

    // API config
    cJSON *api = cJSON_CreateObject();
    char *url = config_mgr_get_str("api_url");
    char *method = config_mgr_get_str("api_method");
    char *headers = config_mgr_get_str("api_headers");
    char *body = config_mgr_get_str("api_body");
    cJSON_AddStringToObject(api, "url", url ? url : "");
    cJSON_AddStringToObject(api, "method", method ? method : "GET");
    cJSON_AddStringToObject(api, "headers", headers ? headers : "");
    cJSON_AddStringToObject(api, "body", body ? body : "");
    cJSON_AddItemToObject(root, "api", api);
    free(url); free(method); free(headers); free(body);

    // LED config
    cJSON *led = cJSON_CreateObject();
    cJSON_AddNumberToObject(led, "effect", config_mgr_get_i32("led_effect", 0));
    cJSON_AddNumberToObject(led, "brightness", config_mgr_get_i32("led_bright", 255));
    cJSON_AddNumberToObject(led, "speed", config_mgr_get_i32("led_speed", 50));
    cJSON_AddItemToObject(root, "led", led);

    // Button config
    cJSON *btn = cJSON_CreateObject();
    cJSON_AddNumberToObject(btn, "mode", config_mgr_get_i32("btn_mode", 0));
    cJSON_AddItemToObject(root, "button", btn);

    // Volume
    cJSON_AddNumberToObject(root, "volume", config_mgr_get_i32("volume", 80));

    char *json = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json);
    free(json);
    cJSON_Delete(root);
    return ESP_OK;
}

// POST /api/config/api
static esp_err_t config_api_handler(httpd_req_t *req)
{
    char *body = read_request_body(req);
    if (body == NULL) { send_json_error(req, 400, "Invalid body"); return ESP_OK; }

    cJSON *root = cJSON_Parse(body);
    free(body);
    if (root == NULL) { send_json_error(req, 400, "Invalid JSON"); return ESP_OK; }

    cJSON *url = cJSON_GetObjectItem(root, "url");
    cJSON *method = cJSON_GetObjectItem(root, "method");
    cJSON *headers = cJSON_GetObjectItem(root, "headers");
    cJSON *req_body = cJSON_GetObjectItem(root, "body");

    if (cJSON_IsString(url)) config_mgr_set_str("api_url", url->valuestring);
    if (cJSON_IsString(method)) config_mgr_set_str("api_method", method->valuestring);
    if (cJSON_IsString(headers)) config_mgr_set_str("api_headers", headers->valuestring);
    if (cJSON_IsString(req_body)) config_mgr_set_str("api_body", req_body->valuestring);

    cJSON_Delete(root);
    send_json_ok(req, "API config saved");
    return ESP_OK;
}

// POST /api/config/led
static esp_err_t config_led_handler(httpd_req_t *req)
{
    char *body = read_request_body(req);
    if (body == NULL) { send_json_error(req, 400, "Invalid body"); return ESP_OK; }

    cJSON *root = cJSON_Parse(body);
    free(body);
    if (root == NULL) { send_json_error(req, 400, "Invalid JSON"); return ESP_OK; }

    cJSON *effect = cJSON_GetObjectItem(root, "effect");
    cJSON *brightness = cJSON_GetObjectItem(root, "brightness");
    cJSON *speed = cJSON_GetObjectItem(root, "speed");

    if (cJSON_IsNumber(effect)) {
        config_mgr_set_i32("led_effect", effect->valueint);
        led_ctrl_set_effect((led_effect_t)effect->valueint);
    }
    if (cJSON_IsNumber(brightness)) {
        config_mgr_set_i32("led_bright", brightness->valueint);
        led_ctrl_set_brightness((uint8_t)brightness->valueint);
    }
    if (cJSON_IsNumber(speed)) {
        config_mgr_set_i32("led_speed", speed->valueint);
        led_ctrl_set_speed((uint8_t)speed->valueint);
    }

    cJSON_Delete(root);
    send_json_ok(req, "LED config saved");
    return ESP_OK;
}

// POST /api/config/button
static esp_err_t config_button_handler(httpd_req_t *req)
{
    char *body = read_request_body(req);
    if (body == NULL) { send_json_error(req, 400, "Invalid body"); return ESP_OK; }

    cJSON *root = cJSON_Parse(body);
    free(body);
    if (root == NULL) { send_json_error(req, 400, "Invalid JSON"); return ESP_OK; }

    cJSON *mode = cJSON_GetObjectItem(root, "mode");
    if (cJSON_IsNumber(mode)) {
        config_mgr_set_i32("btn_mode", mode->valueint);
    }

    cJSON_Delete(root);
    send_json_ok(req, "Button config saved");
    return ESP_OK;
}

// POST /api/config/volume
static esp_err_t config_volume_handler(httpd_req_t *req)
{
    char *body = read_request_body(req);
    if (body == NULL) { send_json_error(req, 400, "Invalid body"); return ESP_OK; }

    cJSON *root = cJSON_Parse(body);
    free(body);
    if (root == NULL) { send_json_error(req, 400, "Invalid JSON"); return ESP_OK; }

    cJSON *vol = cJSON_GetObjectItem(root, "volume");
    if (cJSON_IsNumber(vol)) {
        int v = vol->valueint;
        if (v < 0) v = 0;
        if (v > 100) v = 100;
        config_mgr_set_i32("volume", v);
        audio_player_set_volume((uint8_t)v);
    }

    cJSON_Delete(root);
    send_json_ok(req, "Volume saved");
    return ESP_OK;
}

// POST /api/format
static esp_err_t format_handler(httpd_req_t *req)
{
    audio_player_stop();
    esp_err_t ret = ext_flash_format();
    if (ret == ESP_OK) {
        send_json_ok(req, "Flash formatted");
    } else {
        send_json_error(req, 500, "Format failed");
    }
    return ESP_OK;
}

esp_err_t api_handlers_register(httpd_handle_t server)
{
    // Status
    httpd_uri_t uri;

    uri = (httpd_uri_t){ .uri = "/api/status", .method = HTTP_GET, .handler = status_handler };
    httpd_register_uri_handler(server, &uri);

    // WiFi
    uri = (httpd_uri_t){ .uri = "/api/wifi/scan", .method = HTTP_GET, .handler = wifi_scan_handler };
    httpd_register_uri_handler(server, &uri);

    uri = (httpd_uri_t){ .uri = "/api/wifi/config", .method = HTTP_POST, .handler = wifi_config_handler };
    httpd_register_uri_handler(server, &uri);

    // Audio
    uri = (httpd_uri_t){ .uri = "/api/audio/list", .method = HTTP_GET, .handler = audio_list_handler };
    httpd_register_uri_handler(server, &uri);

    uri = (httpd_uri_t){ .uri = "/api/audio/upload", .method = HTTP_POST, .handler = file_upload_handler };
    httpd_register_uri_handler(server, &uri);

    uri = (httpd_uri_t){ .uri = "/api/audio/delete", .method = HTTP_POST, .handler = audio_delete_handler };
    httpd_register_uri_handler(server, &uri);

    uri = (httpd_uri_t){ .uri = "/api/audio/select", .method = HTTP_POST, .handler = audio_select_handler };
    httpd_register_uri_handler(server, &uri);

    uri = (httpd_uri_t){ .uri = "/api/audio/play", .method = HTTP_POST, .handler = audio_play_handler };
    httpd_register_uri_handler(server, &uri);

    uri = (httpd_uri_t){ .uri = "/api/audio/stop", .method = HTTP_POST, .handler = audio_stop_handler };
    httpd_register_uri_handler(server, &uri);

    // Config
    uri = (httpd_uri_t){ .uri = "/api/config", .method = HTTP_GET, .handler = config_get_handler };
    httpd_register_uri_handler(server, &uri);

    uri = (httpd_uri_t){ .uri = "/api/config/api", .method = HTTP_POST, .handler = config_api_handler };
    httpd_register_uri_handler(server, &uri);

    uri = (httpd_uri_t){ .uri = "/api/config/led", .method = HTTP_POST, .handler = config_led_handler };
    httpd_register_uri_handler(server, &uri);

    uri = (httpd_uri_t){ .uri = "/api/config/button", .method = HTTP_POST, .handler = config_button_handler };
    httpd_register_uri_handler(server, &uri);

    uri = (httpd_uri_t){ .uri = "/api/config/volume", .method = HTTP_POST, .handler = config_volume_handler };
    httpd_register_uri_handler(server, &uri);

    // Utilities
    uri = (httpd_uri_t){ .uri = "/api/format", .method = HTTP_POST, .handler = format_handler };
    httpd_register_uri_handler(server, &uri);

    ESP_LOGI(TAG, "API handlers registered (%d endpoints)", 14);
    return ESP_OK;
}
