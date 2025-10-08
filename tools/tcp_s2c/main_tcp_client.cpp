//https://xie.infoq.cn/article/8eae86e25998538a12f11b7b3

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <uv.h>

static uv_loop_t *loop;


static void client_on_close(uv_handle_t* handle) {
    free(handle);
}

static void client_on_shutdown(uv_shutdown_t *sd, int status) {
    if (status < 0) {
        fprintf(stderr, "failed to shutdown connection, err: %s\n", uv_strerror(status));
        return;
    } else {
        uv_close((uv_handle_t *)sd->handle, client_on_close);
    }

    free(sd);
}

void client_read_alloc_callback(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = static_cast<char *>(malloc(suggested_size));
    buf->len = suggested_size;
}

void client_read_callback(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread < 0) {
        fprintf(stderr, "failed to read from server, err: %s\n", uv_strerror(nread));
        free(buf->base);
        exit(1);
        return;
    } else if (nread > 100) {
        buf->base[nread] = '\0';
        printf("recv %s\n", buf->base);

        uv_shutdown_t *sd = static_cast<uv_shutdown_t *>(malloc(sizeof(uv_shutdown_t)));
        uv_shutdown(sd, stream, client_on_shutdown);
    } else if (nread > 0) {
        buf->base[nread] = '\0';
        printf("recv %s\n", buf->base);
    }

    free(buf->base);
}

static void client_on_response_callback(uv_write_t* req, int status) {
    if (status < 0) {
        fprintf(stderr, "failed to write hello to server, err: %s\n", uv_strerror(status));
    } else {
        return;
    }

    fprintf(stderr, "uv_write error: %s\n", uv_strerror(status));

    if (status == UV_ECANCELED)
        return;

    assert(status == UV_EPIPE);
    uv_close((uv_handle_t *)req->handle, client_on_close);
    free(req);
}

void client_on_connected(uv_connect_t *conn, int status) {
    int r;
    uv_write_t wr;

    if (status < 0) {
        fprintf(stderr, "failed to dial to server, err: %s\n", uv_strerror(status));
        return;
    }

    uv_buf_t b[] = {
            {.base = "hello", .len = 5}
    };

    if ((r = uv_write(&wr, conn->handle, b, 1, client_on_response_callback)) != 0) {
        printf("failed to send hello to server, err: %s\n", uv_strerror(r));
        return;
    }

    printf("sent hello\n");

    if ((r = uv_read_start(conn->handle, client_read_alloc_callback, client_read_callback)) != 0) {
        printf("failed to start reading data from server, err: %s\n", uv_strerror(r));
        return ;
    }
}

int main() {
    int r;
    struct sockaddr_in addr;
    uv_tcp_t *client = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));
    uv_connect_t *connect = static_cast<uv_connect_t *>(malloc(sizeof(uv_connect_t)));

    loop = uv_default_loop();

    uv_tcp_init(loop, client);

    if ((r = uv_ip4_addr("192.168.91.232", 10086, &addr)) != 0) {
        printf("failed to init server address, err: %s\n", uv_strerror(r));
        return 1;
    }

    if ((r = uv_tcp_connect(connect, client, (const struct sockaddr *)&addr, client_on_connected)) != 0) {
        printf("failed to connect to server, err: %s\n", uv_strerror(r));
        return 2;
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}