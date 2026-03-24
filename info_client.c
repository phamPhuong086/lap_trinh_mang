#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define BUF_SIZE 4069

int main() {

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    int ret = connect(client, (struct sockaddr*)&addr, sizeof(addr));

    // Gửi đường dẫn trước
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    send(client, cwd, 256, 0);

    DIR *dir = opendir(".");
    struct dirent *entry;

    // Duyệt file
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            struct stat st;
            stat(entry->d_name, &st);

            // Gửi tên file
            char filename[256] = {0};
            strncpy(filename, entry->d_name, 255);
            send(client, filename, 256, 0);

            // Gửi kích thước file
            int size = st.st_size;
            send(client, &size, sizeof(size), 0);
        }
    }

    closedir(dir);
    close(client);
    return 0;
}