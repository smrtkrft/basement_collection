#include "web_server.h"
#include "api_handlers.h"

#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"

#include <string.h>
#include <stdio.h>

static const char *TAG = "web_server";
static httpd_handle_t s_server = NULL;

#define WEB_MOUNT_POINT "/www"

static esp_err_t init_spiffs(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = WEB_MOUNT_POINT,
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true,
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount SPIFFS");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "SPIFFS partition not found");
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "SPIFFS: total=%zu, used=%zu", total, used);
    }

    return ESP_OK;
}

// Serve logo
static esp_err_t logo_handler(httpd_req_t *req)
{
    extern const uint8_t logo_start[] asm("_binary_B_LOGO_T_png_start");
    extern const uint8_t logo_end[] asm("_binary_B_LOGO_T_png_end");
    size_t len = logo_end - logo_start;

    httpd_resp_set_type(req, "image/png");
    httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=86400");
    httpd_resp_send(req, (const char *)logo_start, len);
    return ESP_OK;
}

// Serve index.html with embedded frontend
static esp_err_t index_handler(httpd_req_t *req)
{
    // Try to serve from SPIFFS first
    FILE *f = fopen(WEB_MOUNT_POINT "/index.html", "r");
    if (f != NULL) {
        httpd_resp_set_type(req, "text/html");
        char buf[512];
        size_t read;
        while ((read = fread(buf, 1, sizeof(buf), f)) > 0) {
            httpd_resp_send_chunk(req, buf, read);
        }
        httpd_resp_send_chunk(req, NULL, 0);
        fclose(f);
        return ESP_OK;
    }

    // Fallback: embedded minimal UI
    extern const char index_html_start[] asm("_binary_index_html_start");
    extern const char index_html_end[] asm("_binary_index_html_end");
    size_t len = index_html_end - index_html_start;

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, index_html_start, len);
    return ESP_OK;
}

esp_err_t web_server_init(void)
{
    init_spiffs();

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 24;
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.stack_size = 16384;
    config.recv_wait_timeout = 60;
    config.send_wait_timeout = 30;

    esp_err_t ret = httpd_start(&s_server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(ret));
        return ret;
    }

    // Static file routes
    httpd_uri_t index_uri = { .uri = "/", .method = HTTP_GET, .handler = index_handler };
    httpd_register_uri_handler(s_server, &index_uri);

    httpd_uri_t logo_uri = { .uri = "/logo.png", .method = HTTP_GET, .handler = logo_handler };
    httpd_register_uri_handler(s_server, &logo_uri);

    // Register API handlers
    api_handlers_register(s_server);

    ESP_LOGI(TAG, "Web server started");
    return ESP_OK;
}

esp_err_t web_server_stop(void)
{
    if (s_server) {
        httpd_stop(s_server);
        s_server = NULL;
        ESP_LOGI(TAG, "Web server stopped");
    }
    return ESP_OK;
}
