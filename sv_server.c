#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

struct sinhvien {
    int mssv;
    char hoten[60];
    char ngaySinh[12];
    float diemTB;
};

int main(int argc, char *argv[]) {
    int port = atoi(argv[1]);
    char *log_file = argv[2];

    //tao socket
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

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
        
    int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client < 0) {
        perror("connect() failed");
        exit(EXIT_FAILURE);
    }

    struct sinhvien sv;

    while(1) {
        int ret = recv(client, &sv, sizeof(sv), 0);
        if (ret > 0) {
            //lay dia chi IP client
            char *client_ip = inet_ntoa(client_addr.sin_addr);

            //lay time hien tai cua he thong
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

            //In ra man hinh
            printf("Sinh viên: %s\n", sv.hoten);
            printf("MSSV: %d\n", sv.mssv);
            printf("Ngày sinh: %s\n", sv.ngaySinh);
            printf("Điểm TB: %.2f\n", sv.diemTB);
            //printf("%s %s %d %s %s %.2f\n", client_ip, time_str, sv.mssv, sv.hoten, sv.ngaySinh, sv.diemTB);

            //ghi vao file log
            FILE *f = fopen(log_file, "a");
            if (f != NULL) {
                fprintf(f, "%s %s %d %s %s %.2f\n", client_ip, time_str, sv.mssv, sv.hoten, sv.ngaySinh, sv.diemTB);
                fclose(f);
            }
        }
    }

    close(client);
    close(listener);

    return 0;
}