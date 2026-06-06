#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8888
#define BUFFER_SIZE 4096

void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    
    int bytes_received = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        close(client_sock);
        return;
    }

    char method[10], url[255], version[10];
    sscanf(buffer, "%s %s %s", method, url, version);

    char params[512] = {0};

    if (strcmp(method, "GET") == 0) {
        char *query = strchr(url, '?');
        if (query) {
            strcpy(params, query + 1);
        }
    } 

    else if (strcmp(method, "POST") == 0) {
        char *body = strstr(buffer, "\r\n\r\n");
        if (body) {
            strcpy(params, body + 4);
        }
    }

    double a = 0, b = 0;
    char op[10] = {0};
    char result_str[512] = "Không nhận được tham số hợp lệ!";

    char *token = strtok(params, "&");
    while (token != NULL) {
        if (strncmp(token, "a=", 2) == 0) a = atof(token + 2);
        else if (strncmp(token, "b=", 2) == 0) b = atof(token + 2);
        else if (strncmp(token, "op=", 3) == 0) strcpy(op, token + 3);
        token = strtok(NULL, "&");
    }

    if (strlen(op) > 0) {
        if (strcmp(op, "add") == 0) sprintf(result_str, "%.2f + %.2f = %.2f", a, b, a + b);
        else if (strcmp(op, "sub") == 0) sprintf(result_str, "%.2f - %.2f = %.2f", a, b, a - b);
        else if (strcmp(op, "mul") == 0) sprintf(result_str, "%.2f * %.2f = %.2f", a, b, a * b);
        else if (strcmp(op, "div") == 0) {
            if (b != 0) sprintf(result_str, "%.2f / %.2f = %.2f", a, b, a / b);
            else strcpy(result_str, "Lỗi: Không thể chia cho 0!");
        } else sprintf(result_str, "Toán tử '%s' không hợp lệ (Dùng: add, sub, mul, div)", op);
    }

    char response[1024];
    sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n"
                      "<html><head><title>Calculator Server</title></head>"
                      "<body><h2>Kết quả phép tính:</h2>"
                      "<h3>%s</h3>"
                      "<hr><a href='/'>Quay lại</a></body></html>", result_str);
    
    send(client_sock, response, strlen(response), 0);
    close(client_sock);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    printf("Calculator Server đang chạy tại port %d...\n", PORT);

    while (1) {
        int client_sock = accept(server_fd, NULL, NULL);
        handle_client(client_sock);
    }
    close(server_fd);
    return 0;
}