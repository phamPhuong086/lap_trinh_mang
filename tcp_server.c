#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int port = atoi(argv[1]);
    char *file_chao = argv[2];
    char *file_ghi = argv[3];

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    listen(listener, 5);
    printf("Dang doi ket noi o cong %d...\n", port);

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Chấp nhận kết nối
    int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client < 0) {
        perror("accept() failed");
        exit(EXIT_FAILURE);
    }
    //printf("Da ket noi toi client: %s\n", inet_ntoa(client_addr.sin_addr));

    //Gửi câu chào tới client
    FILE *f = fopen(file_chao, "r");
    if (f != NULL) {
        char msg[1024];
        if (fgets(msg, sizeof(msg), f) != NULL) {
            send(client, msg, strlen(msg), 0);
            printf("Da gui cau chao tu file: %s\n", file_chao);
        }
        fclose(f); // Đóng ngay sau khi dùng xong
    } else {
        printf("Khong mo duoc file chao!\n");
    }

    FILE *f_ghi = fopen(file_ghi, "wb");
    if (f_ghi != NULL) {
        char buf[256];
        while(1) {
            int n = recv(client, buf, sizeof(buf), 0);
            if (n <= 0) break; // Client đóng kết nối hoặc lỗi
            
            fwrite(buf, 1, n, f_ghi);
            fflush(f_ghi);
            printf("Da ghi %d bytes vao file %s\n", n, file_ghi);
        }
        fclose(f_ghi);
    }
    printf("Da dong ket noi\n");
    close(client);
    close(listener);
    
    return 0;
}