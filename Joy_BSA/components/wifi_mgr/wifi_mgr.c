#include "wifi_mgr.h"
#include "config_mgr.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "mdns.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

static const char *TAG = "wifi_mgr";

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define WIFI_MAX_RETRY     5

static EventGroupHandle_t s_wifi_event_group = NULL;
static esp_netif_t *s_sta_netif = NULL;
static esp_netif_t *s_ap_netif = NULL;
static int s_retry_count = 0;
static volatile bool s_connected = false;
static char s_device_id[7] = {0};  // "3EF42T" + null
static char s_ap_ssid[32] = {0};

static void generate_device_id(void)
{
    static const char b36[] = "0123456789ABCDEFGHJKLMNPQRSTUVWXYZ";

    // Compile-time seed from __DATE__ and __TIME__
    const char *d = __DATE__;  // "Apr  5 2026"
    const char *t = __TIME__;  // "14:30:05"
    uint32_t seed = 0;
    for (int i = 0; d[i]; i++) seed = seed * 31 + (uint8_t)d[i];
    for (int i = 0; t[i]; i++) seed = seed * 37 + (uint8_t)t[i];

    // Mix with chip MAC for per-device uniqueness
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    uint32_t val = ((uint32_t)mac[2] << 24) | ((uint32_t)mac[3] << 16) |
                   ((uint32_t)mac[4] << 8)  | (uint32_t)mac[5];
    val ^= seed;

    // Encode as 6-char base-34 (no O/I to avoid confusion)
    for (int i = 5; i >= 0; i--) {
        s_device_id[i] = b36[val % 34];
        val /= 34;
    }
    s_device_id[6] = '\0';

    snprintf(s_ap_ssid, sizeof(s_ap_ssid), "Joy BSA-%s", s_device_id);
    ESP_LOGI(TAG, "Device ID: BSA-%s", s_device_id);
}

const char *wifi_mgr_get_device_id(void)
{
    return s_device_id;
}

// Initialize mDNS so the web UI is reachable at "bsa-<deviceid>.local" both
// in AP mode (clients on the soft-AP can resolve it) and in STA mode (devices
// on the same WiFi can resolve it). mdns_init() binds to all active netifs.
static void setup_mdns(void)
{
    char hostname[24];
    snprintf(hostname, sizeof(hostname), "bsa-%s", s_device_id);
    for (int i = 0; hostname[i]; i++) {
        hostname[i] = (char)tolower((unsigned char)hostname[i]);
    }

    esp_err_t ret = mdns_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "mdns_init failed: %s", esp_err_to_name(ret));
        return;
    }
    mdns_hostname_set(hostname);
    mdns_instance_name_set("Joy BSA");
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
    ESP_LOGI(TAG, "mDNS: http://%s.local", hostname);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            s_connected = false;
            if (s_retry_count < WIFI_MAX_RETRY) {
                esp_wifi_connect();
                s_retry_count++;
                ESP_LOGI(TAG, "Retrying WiFi connection (%d/%d)", s_retry_count, WIFI_MAX_RETRY);
            } else {
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                ESP_LOGW(TAG, "WiFi connection failed, switching to AP mode");
                wifi_mgr_start_ap();
            }
            break;
        case WIFI_EVENT_AP_STACONNECTED: {
            wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
            ESP_LOGI(TAG, "Station connected to AP, AID=%d", event->aid);
            break;
        }
        case WIFI_EVENT_AP_STADISCONNECTED: {
            wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
            ESP_LOGI(TAG, "Station disconnected from AP, AID=%d", event->aid);
            break;
        }
        default:
            break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Connected! IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_count = 0;
        s_connected = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_mgr_init(void)
{
    generate_device_id();
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    s_sta_netif = esp_netif_create_default_wifi_sta();
    s_ap_netif = esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    // mDNS responder for both AP and STA — call once after netif/event setup.
    setup_mdns();

    // Try STA mode if credentials exist
    char *ssid = config_mgr_get_str("wifi_ssid");
    char *pass = config_mgr_get_str("wifi_pass");

    if (ssid != NULL && strlen(ssid) > 0) {
        ESP_LOGI(TAG, "Found saved WiFi credentials, connecting to: %s", ssid);
        wifi_mgr_start_sta(ssid, pass ? pass : "");
        free(ssid);
        free(pass);

        // Wait for connection with timeout
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE, pdFALSE, pdMS_TO_TICKS(15000));

        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi connected in STA mode");
            return ESP_OK;
        }
        // If failed, AP mode was already started by event handler
    } else {
        free(ssid);
        free(pass);
        ESP_LOGI(TAG, "No WiFi credentials found, starting AP mode");
        wifi_mgr_start_ap();
    }

    return ESP_OK;
}

esp_err_t wifi_mgr_start_ap(void)
{
    esp_wifi_stop();

    wifi_config_t ap_cfg = {
        .ap = {
            .channel = 1,
            .max_connection = 4,
            .authmode = WIFI_AUTH_OPEN,
            .pmf_cfg = {
                .required = false,
            },
        },
    };
    strncpy((char *)ap_cfg.ap.ssid, s_ap_ssid, sizeof(ap_cfg.ap.ssid) - 1);
    ap_cfg.ap.ssid_len = strlen(s_ap_ssid);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "AP mode started: SSID=%s, IP=192.168.4.1", s_ap_ssid);
    return ESP_OK;
}

esp_err_t wifi_mgr_start_sta(const char *ssid, const char *password)
{
    esp_wifi_stop();
    s_retry_count = 0;

    wifi_config_t sta_cfg = {0};
    strncpy((char *)sta_cfg.sta.ssid, ssid, sizeof(sta_cfg.sta.ssid) - 1);
    if (password) {
        strncpy((char *)sta_cfg.sta.password, password, sizeof(sta_cfg.sta.password) - 1);
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "STA mode started, connecting to: %s", ssid);
    return ESP_OK;
}

bool wifi_mgr_is_connected(void)
{
    return s_connected;
}

esp_err_t wifi_mgr_get_ip(char *buf, size_t buf_len)
{
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = s_connected ? s_sta_netif : s_ap_netif;
    esp_err_t ret = esp_netif_get_ip_info(netif, &ip_info);
    if (ret == ESP_OK) {
        snprintf(buf, buf_len, IPSTR, IP2STR(&ip_info.ip));
    }
    return ret;
}
