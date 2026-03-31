#include "api_client.h"
#include "config_mgr.h"

#include "esp_log.h"
#include "esp_http_client.h"

#include <string.h>
#include <stdlib.h>

static const char *TAG = "api_client";

esp_err_t api_client_init(void)
{
    ESP_LOGI(TAG, "API client initialized");
    return ESP_OK;
}

static void parse_and_set_headers(esp_http_client_handle_t client, const char *headers)
{
    if (headers == NULL || strlen(headers) == 0) return;

    char *copy = strdup(headers);
    if (copy == NULL) return;

    char *line = strtok(copy, "\n");
    while (line != NULL) {
        char *colon = strchr(line, ':');
        if (colon != NULL) {
            *colon = '\0';
            char *key = line;
            char *value = colon + 1;
            // Trim leading space from value
            while (*value == ' ') value++;
            // Trim trailing \r
            char *cr = strchr(value, '\r');
            if (cr) *cr = '\0';
            esp_http_client_set_header(client, key, value);
        }
        line = strtok(NULL, "\n");
    }
    free(copy);
}

esp_err_t api_client_send(const api_request_t *request)
{
    if (request == NULL || request->url == NULL || strlen(request->url) == 0) {
        ESP_LOGW(TAG, "No API URL configured, skipping");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Sending %s request to: %s",
             request->method ? request->method : "GET", request->url);

    esp_http_client_config_t config = {
        .url = request->url,
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to init HTTP client");
        return ESP_FAIL;
    }

    // Set method
    if (request->method != NULL && strcmp(request->method, "POST") == 0) {
        esp_http_client_set_method(client, HTTP_METHOD_POST);
    } else {
        esp_http_client_set_method(client, HTTP_METHOD_GET);
    }

    // Set custom headers
    parse_and_set_headers(client, request->headers);

    // Set POST body
    if (request->body != NULL && strlen(request->body) > 0) {
        esp_http_client_set_post_field(client, request->body, strlen(request->body));
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        int content_len = esp_http_client_get_content_length(client);
        ESP_LOGI(TAG, "API response: status=%d, content_length=%d", status, content_len);
    } else {
        ESP_LOGE(TAG, "API request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}
