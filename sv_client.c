#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

struct sinhvien {
    int mssv;
    char hoten[60];
    char ngaySinh[12];
    float diemTB;
};

int main(int argc, char *argv[]) {
    //tham so dong lenh
    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    //tao socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(server_ip);
    addr.sin_port = htons(server_port);

    //ket noi toi server
    if(connect(client, (struct sockaddr *)&addr, sizeof(addr))){
        perror("connect() failed");
        exit(EXIT_FAILURE);
    }

    struct sinhvien sv;

    while(1) {
        printf("Enter mssv: ");
        scanf("%d", &sv.mssv);
        //while (getchar() != '\n'); 
        getchar();

        printf("Enter hoten: ");
        fgets(sv.hoten, sizeof(sv.hoten), stdin);
        sv.hoten[strcspn(sv.hoten, "\n")] = 0;

        printf("Enter ngaysinh: ");
        scanf("%s", sv.ngaySinh);
        //while (getchar() != '\n');
        getchar();

        printf("Enter diem: ");
        scanf("%f", &sv.diemTB);
        //while (getchar() != '\n');
        getchar();

        send(client, &sv, sizeof(sv), 0);
    }

    close(client);
}