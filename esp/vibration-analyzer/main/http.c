#include "esp_http_client.h"
#include <stdbool.h>


#define MAX_HTTP_OUTPUT_BUFFER 2048
char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};

esp_err_t http_event_handler(esp_http_client_event_t *event)
{
    switch(event->event_id) {
        case HTTP_EVENT_ON_DATA:
            break;
    }
    return ESP_OK;
}

esp_http_client_handle_t http_config(void)
{
    esp_http_client_config_t config = {
        .host = "192.168.1.103",
        .path = "/get",
        .query = "esp",
        .event_handler = http_event_handler,
        .user_data = local_response_buffer,
        .disable_auto_redirect = true,
    };
    return esp_http_client_init(&config);
}

void http_get(esp_http_client_handle_t client)
{
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        esp_http_client_get_status_code(client);
        esp_http_client_get_content_length(client);
    } else {
        esp_err_to_name(err);
    }
    // local_response_buffer
}

esp_err_t http_post(esp_http_client_handle_t client)
{
    const char *post_data = "{\"field1\":\"value1\"}";

    esp_http_client_set_url(client, "http://192.168.1.103/post");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    return esp_http_client_perform(client);
}