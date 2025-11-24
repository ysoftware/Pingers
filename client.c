#include "stdio.h"
#include "lib/mongoose-7.19/mongoose.h"

#define NOB_IMPLEMENTATION
#include "include/nob.h"

#define FILE_PATH "data"
void client_event_handler(struct mg_connection *connection, int event, void *event_data) {
    if (event == MG_EV_HTTP_MSG) {
        struct mg_http_message *event_message = (struct mg_http_message *) event_data;
    } else if (event == MG_EV_CONNECT) {
        mg_printf(connection, "POST /set HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
    } else if (event == MG_EV_WRITE) {
        connection->is_closing = true;
        printf("Pinged\n");
    } else if (event == MG_EV_ERROR) {
        char *message = (char*) event_data;
        printf("An error occured during download: %s\n", message);
    }
}

#define TIMESTAMP signed long long
#define USEC_PER_SEC 1000000ull

TIMESTAMP get_timestamp_usec(void) {
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    return time.tv_sec * 1000000L + time.tv_nsec / 1000;
}

void print_usage(void) {
    printf("Usage: ./client.app [server host] [interval seconds]\n");
}

int main(int argc, char **argv) {
    if (argc < 3) {
        print_usage();
        return 1;
    }

    nob_shift_args(&argc, &argv);
    char host[100] = {0};
    sprintf(host, "%s:81", *argv);

    nob_shift_args(&argc, &argv);
    int seconds = atoi(*argv);
    if (seconds < 0) {
        print_usage();
        return 1;
    }

    mg_log_set(MG_LL_NONE);
    struct mg_mgr manager;
    mg_mgr_init(&manager);

    printf("Client started. Host: %s, interval: %d sec.\n", host, seconds);

    TIMESTAMP next_ping = 0;
    while (true) {
        TIMESTAMP now = get_timestamp_usec();
        if (now > next_ping) {
            mg_http_connect(&manager, host, client_event_handler, NULL);
            next_ping = now + seconds * USEC_PER_SEC;
        }
        mg_mgr_poll(&manager, 10);
    }
}
