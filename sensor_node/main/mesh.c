#include <string.h>
#include "mesh.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mesh.h"

static const char* TAG = "bnet";
static const uint8_t MESH_ID[6] = { 0x77, 0x77, 0x77, 0x77, 0x77, 0x77};

// TODO: Extract this into a class
static bool is_running = true;
static bool is_mesh_connected = false;
static mesh_addr_t mesh_parent_addr;
static int mesh_layer = -1;

void mesh_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
	mesh_addr_t id = {0,};
	static uint8_t last_layer = 0;

	 switch (event_id) {
    case MESH_EVENT_STARTED: {
        esp_mesh_get_id(&id);
        ESP_LOGI(TAG, "<MESH_EVENT_MESH_STARTED>ID:"MACSTR"", MAC2STR(id.addr));
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;

		case MESH_EVENT_NETWORK_STATE: {
			mesh_event_network_state_t *network_state = (mesh_event_network_state_t *)event_data;
			ESP_LOGI(TAG, "<MESH_EVENT_NETWORK_STATE>is_rootless:%d",
				network_state->is_rootless);
			}
			break;

			case MESH_EVENT_NO_PARENT_FOUND: {
				mesh_event_no_parent_found_t *no_parent = (mesh_event_no_parent_found_t *)event_data;
				ESP_LOGI(TAG, "<MESH_EVENT_NO_PARENT_FOUND>scan times:%d",
					no_parent->scan_times);
			}
			/* TODO handler for the failure */
			break;

		default:
			ESP_LOGI(TAG, "unknown id:%d", event_id);
			break;
	 }

}

void ip_event_handler(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    ESP_LOGI(TAG, "<IP_EVENT_STA_GOT_IP>IP:%s", ip4addr_ntoa(&event->ip_info.ip));
}

inline static void configure_softap() {
	tcpip_adapter_init();
	ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
	ESP_ERROR_CHECK(tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA));
}

inline static void configure_event_loop() {
	ESP_ERROR_CHECK(esp_event_loop_create_default());
}

inline static void configure_wifi() {
	wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&config));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
	ESP_ERROR_CHECK(esp_wifi_start());
	}

inline static void configure_root() {
}

inline static void set_mesh_configuration() {
	mesh_cfg_t cfg = MESH_INIT_CONFIG_DEFAULT();
	cfg.channel = CONFIG_MESH_CHANNEL;
	cfg.router.ssid_len = strlen(CONFIG_MESH_ROUTER_SSID);
	memcpy((uint8_t *) &cfg.mesh_id, MESH_ID, 6);
	memcpy((uint8_t *) &cfg.router.ssid, CONFIG_MESH_ROUTER_SSID, cfg.router.ssid_len);
	memcpy((uint8_t *) &cfg.router.password, CONFIG_MESH_ROUTER_PASSWORD, strlen(CONFIG_MESH_ROUTER_PASSWORD));
	cfg.mesh_ap.max_connection = CONFIG_MESH_AP_CONNECTIONS;
	ESP_ERROR_CHECK(esp_mesh_set_ap_authmode(CONFIG_MESH_AP_AUTHMODE));
	memcpy((uint8_t *) &cfg.mesh_ap.password, CONFIG_MESH_AP_PASSWD, strlen(CONFIG_MESH_AP_PASSWD));
	ESP_ERROR_CHECK(esp_mesh_set_config(&cfg));
	esp_mesh_set_self_organized(true, true);
}

void configure_mesh() {
	configure_softap();
	configure_event_loop();
	configure_wifi();
	configure_root();

	ESP_ERROR_CHECK(esp_mesh_init());
	ESP_ERROR_CHECK(esp_event_handler_register(MESH_EVENT, ESP_EVENT_ANY_ID, &mesh_event_handler, NULL));
	ESP_ERROR_CHECK(esp_mesh_set_max_layer(2));
	ESP_ERROR_CHECK(esp_mesh_set_vote_percentage(1));
	ESP_ERROR_CHECK(esp_mesh_set_ap_assoc_expire(10));
	set_mesh_configuration();

	ESP_ERROR_CHECK(esp_mesh_start());
	ESP_LOGI(TAG, "mesh starts successfully, heap:%d, %s\n",  esp_get_free_heap_size(),
		esp_mesh_is_root_fixed() ? "root fixed" : "root not fixed");

}

void mesh_event_loop() {
	ESP_LOGI(TAG, "Event loop started");
}