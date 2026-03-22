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
    int port = atoi(argv[1]);
    char *file_chao = argv[2];
    char *file_ghi = argv[3];

    //khoi tao socket server
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    if (listen(listener, 5) < 0) {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    printf("Dang doi ket noi o cong %d\n", port);

    //chap nhan ket noi
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);
    printf("Client da ket noi");

    //doc cau chao tu tep tin va gui cho client
    FILE *f = fopen(file_chao, "r");
    if (f != NULL) {
        char msg[1024];
        //doc noi dung file chao
        fgets(msg, sizeof(msg), f);
        send(client, msg, strlen(msg), 0);
    }

    //Nhan noi dung tu client va ghi vao tep tin khac
    FILE *f_ghi = fopen(file_ghi, "wb");
    char buf[256];
    while(1) {
        int n = recv(client, buf, sizeof(buf), 0);
        if (n <= 0) {
            break;
        }
        fwrite(buf, 1, n, f_ghi);
        printf("Da ghi %d bytes nhan duoc vao file.\n", n);
    }

    close(listener);
    close(client);
    fclose(f);
    fclose(f_ghi);
    
    return 0;
}