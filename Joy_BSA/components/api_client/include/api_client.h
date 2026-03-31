#pragma once

#include "esp_err.h"

typedef struct {
    const char *url;
    const char *method;     // "GET" or "POST"
    const char *headers;    // newline-delimited "Key: Value" pairs
    const char *body;       // POST body (NULL for GET)
} api_request_t;

esp_err_t api_client_init(void);
esp_err_t api_client_send(const api_request_t *request);
