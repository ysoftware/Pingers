#include "stdio.h"
#include "lib/mongoose-7.19/mongoose.h"

#define NOB_IMPLEMENTATION
#include "include/nob.h"

#define FILE_PATH "data"
void server_event_handler(struct mg_connection *connection, int event, void *event_data) {
    if (event == MG_EV_HTTP_MSG) {
        struct mg_http_message *event_message = (struct mg_http_message *) event_data;

        char ip[20] = {0};
        sprintf(ip, "%d.%d.%d.%d\n", MG_IPADDR_PARTS(&connection->rem));

        if (mg_match(event_message->uri, mg_str("/set"), NULL)) {
            printf("[SET] Received data from %s", ip);
            if (nob_write_entire_file(FILE_PATH, ip, strlen(ip))) {
                mg_printf(connection, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");
            } else {
                mg_printf(connection, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
            }
        } else if (mg_match(event_message->uri, mg_str("/get"), NULL)) {
            printf("[GET] Request from %s", ip);
            Nob_String_Builder sb = {0};
            if (nob_read_entire_file(FILE_PATH, &sb)) {
                mg_printf(connection, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s", sb.count, sb.items);
            } else {
                mg_printf(connection, "HTTP/1.1 204 No Content\r\nContent-Length: 0\r\n\r\n");
            }
            nob_sb_free(sb);
        } else {
            connection->is_closing = true;
        }
    }
}

int main(void) {
    mg_log_set(MG_LL_VERBOSE);
    struct mg_mgr manager;
    mg_mgr_init(&manager);

    mg_http_listen(&manager, "http://0.0.0.0:81", server_event_handler, NULL);
    mg_wakeup_init(&manager);

    printf("Server started.\n");
    while (true) mg_mgr_poll(&manager, 1000);

    printf("Server stopped.\n");
}
