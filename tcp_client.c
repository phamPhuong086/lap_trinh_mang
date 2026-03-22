#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    //tham so dong lenh
    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    //tao socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(server_ip);
    addr.sin_port = htons(server_port);
    
    //ket noi toi server
    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("connect() failed");
        exit(EXIT_FAILURE);
    }
    printf("Da ket noi toi server %s: %d\n", server_ip, server_port);

    //gui du lieu
    char buf[1024];
    while(1) {
        //doc du lieu tu ban phim
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            break;
        }
        buf[strcspn(buf, "\n")] = 0; //xoa ki tu cuoi

        //exit -> thoat vong lap
        if (strcmp(buf, "exit") == 0) break;

        //gui du lieu toi server
        int sent = send(client, buf, strlen(buf), 0);
        if (sent < 0) {
            perror("send() failed");
            break;
        }
    }

    close(client);

    return 0;
}