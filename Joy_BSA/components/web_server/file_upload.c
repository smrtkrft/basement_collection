#include "esp_http_server.h"
#include "esp_log.h"
#include "ext_flash.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static const char *TAG = "file_upload";

#define UPLOAD_BUF_SIZE  4096

/*
 * Simple file upload handler.
 * Expects the filename in the "X-Filename" header.
 * Body is raw file data (not multipart).
 * Frontend sends the file with XMLHttpRequest.
 */
esp_err_t file_upload_handler(httpd_req_t *req)
{
    // Get filename from header
    char filename[64] = {0};
    if (httpd_req_get_hdr_value_str(req, "X-Filename", filename, sizeof(filename)) != ESP_OK) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_sendstr(req, "{\"status\":\"error\",\"message\":\"X-Filename header required\"}");
        return ESP_OK;
    }

    // Validate filename - prevent path traversal
    if (strchr(filename, '/') || strchr(filename, '\\') || strstr(filename, "..") || strlen(filename) == 0) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_sendstr(req, "{\"status\":\"error\",\"message\":\"Invalid filename\"}");
        return ESP_OK;
    }

    // Check content length
    if (req->content_len <= 0) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_sendstr(req, "{\"status\":\"error\",\"message\":\"Empty file\"}");
        return ESP_OK;
    }

    // Check free space
    size_t free_space = ext_flash_get_free_space();
    if ((size_t)req->content_len > free_space) {
        httpd_resp_set_status(req, "507 Insufficient Storage");
        httpd_resp_sendstr(req, "{\"status\":\"error\",\"message\":\"Not enough space\"}");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Uploading: %s (%d bytes)", filename, req->content_len);

    // Open file for writing
    char filepath[160];
    snprintf(filepath, sizeof(filepath), "%s/%s", EXT_FLASH_AUDIO_DIR, filename);

    FILE *f = fopen(filepath, "wb");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing: %s", filepath);
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_sendstr(req, "{\"status\":\"error\",\"message\":\"Failed to create file\"}");
        return ESP_OK;
    }

    // Read and write in chunks
    char *buf = malloc(UPLOAD_BUF_SIZE);
    if (buf == NULL) {
        fclose(f);
        remove(filepath);
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_sendstr(req, "{\"status\":\"error\",\"message\":\"Memory allocation failed\"}");
        return ESP_OK;
    }

    int remaining = req->content_len;
    int total_written = 0;
    bool error = false;

    while (remaining > 0) {
        int to_read = (remaining < UPLOAD_BUF_SIZE) ? remaining : UPLOAD_BUF_SIZE;
        int received = httpd_req_recv(req, buf, to_read);
        if (received <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                continue; // Retry on timeout
            }
            ESP_LOGE(TAG, "Upload receive error");
            error = true;
            break;
        }

        size_t written = fwrite(buf, 1, received, f);
        if (written != (size_t)received) {
            ESP_LOGE(TAG, "Write error");
            error = true;
            break;
        }

        remaining -= received;
        total_written += received;
    }

    free(buf);
    fclose(f);

    if (error) {
        remove(filepath);
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_sendstr(req, "{\"status\":\"error\",\"message\":\"Upload failed\"}");
    } else {
        ESP_LOGI(TAG, "Upload complete: %s (%d bytes)", filename, total_written);
        httpd_resp_set_type(req, "application/json");
        char resp[128];
        snprintf(resp, sizeof(resp), "{\"status\":\"ok\",\"message\":\"Uploaded\",\"size\":%d}", total_written);
        httpd_resp_sendstr(req, resp);
    }

    return ESP_OK;
}
