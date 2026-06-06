#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#define PORT 9000
#define BUFFER_SIZE 4096

const char* get_content_type(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot) return "text/plain; charset=UTF-8";
    
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) return "text/html; charset=UTF-8";
    if (strcmp(dot, ".txt") == 0 || strcmp(dot, ".c") == 0 || strcmp(dot, ".h") == 0) {
        return "text/plain; charset=UTF-8";
    }
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(dot, ".png") == 0) return "image/png";
    if (strcmp(dot, ".mp3") == 0) return "audio/mpeg";
    if (strcmp(dot, ".mp4") == 0) return "video/mp4";
    
    return "application/octet-stream"; 
}

void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    
    int bytes_received = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        close(client_sock);
        return;
    }

    char method[10], url[512], version[10];
    method[0] = '\0'; url[0] = '\0';
    sscanf(buffer, "%s %s %s", method, url, version);

    if (strcmp(url, "/favicon.ico") == 0) {
        char *not_found = "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";
        send(client_sock, not_found, strlen(not_found), 0);
        close(client_sock);
        return;
    }

    char path[512] = ".";
    strcat(path, url);

    struct stat st;
    if (stat(path, &st) == -1) {
        char *not_found = "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";
        send(client_sock, not_found, strlen(not_found), 0);
        close(client_sock);
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        DIR *d = opendir(path);
        // Bổ sung Connection: close vào Header
        char response[16384] = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n"
                               "<html><head><title>HTTP File Server</title>"
                               "<style>a { text-decoration: none; font-size: 18px; }</style></head><body>"
                               "<h2>Danh sách tệp tin & thư mục hiện tại:</h2><ul>";
        struct dirent *dir;
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
                    continue;

                char sub_path[1024];
                if (path[strlen(path) - 1] == '/') sprintf(sub_path, "%s%s", path, dir->d_name);
                else sprintf(sub_path, "%s/%s", path, dir->d_name);

                struct stat sub_st;
                int is_directory = 0;

                if (dir->d_type == DT_DIR) {
                    is_directory = 1;
                } else if (dir->d_type == DT_UNKNOWN) {
                    if (stat(sub_path, &sub_st) == 0 && S_ISDIR(sub_st.st_mode)) {
                        is_directory = 1;
                    }
                }

                char link[1024];
                if (url[strlen(url) - 1] == '/') sprintf(link, "%s%s", url, dir->d_name);
                else sprintf(link, "%s/%s", url, dir->d_name);

                if (is_directory) {
                    sprintf(response + strlen(response), "<li><a href=\"%s\"><b>[%s]</b></a></li>", link, dir->d_name);
                } else {
                    sprintf(response + strlen(response), "<li><a href=\"%s\"><i>%s</i></a></li>", link, dir->d_name);
                }
            }
            closedir(d);
        }
        strcat(response, "</ul></body></html>");
        send(client_sock, response, strlen(response), 0);
    } 

    else if (S_ISREG(st.st_mode)) {
        FILE *file = fopen(path, "rb");
        if (file) {
            char header[512];

            sprintf(header, "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", 
                    get_content_type(path), st.st_size);
            send(client_sock, header, strlen(header), 0);

            char file_buf[BUFFER_SIZE];
            int read_bytes;
            while ((read_bytes = fread(file_buf, 1, sizeof(file_buf), file)) > 0) {
                send(client_sock, file_buf, read_bytes, 0);
            }
            fclose(file);
        }
    }
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

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Lỗi bind cổng thất bại");
        exit(EXIT_FAILURE);
    }
    
    listen(server_fd, 10);
    printf("HTTP File Server đang chạy tại port %d...\n", PORT);

    while (1) {
        int client_sock = accept(server_fd, NULL, NULL);
        if (client_sock >= 0) {
            handle_client(client_sock);
        }
    }
    close(server_fd);
    return 0;
}