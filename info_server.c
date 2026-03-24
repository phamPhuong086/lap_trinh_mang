#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listener, (struct sockaddr*)&addr, sizeof(addr));
    listen(listener, 5);

    printf("Server dang cho ket noi...\n");

    int client = accept(listener, NULL, NULL);
    printf("Client da ket noi!\n");

    char path[256];
    char filename[256];
    int filesize;

    //Nhận đường dẫn
    recv(client, path, 256, 0);
    printf("%s\n", path);

    //Nhận danh sách file
    while (recv(client, filename, 256, 0) > 0) {
        //Nhận kích thước
        if (recv(client, &filesize, sizeof(filesize), 0) <= 0) break;
        
        if (strlen(filename) > 0) {
            printf("%s - %d bytes\n", filename, filesize);
        }
    }

    close(client);
    close(listener);
    return 0;
}