#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 4096

// Hàm gửi lệnh FTP và nhận phản hồi từ kênh Control Connection
void send_ftp_cmd(int ctrl_sock, const char* cmd, char* out_res) {
    if (cmd != NULL && strlen(cmd) > 0) {
        send(ctrl_sock, cmd, strlen(cmd), 0);
        printf(">> %s", cmd);
    }
    
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(ctrl_sock, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("<< %s", buffer);
        if (out_res != NULL) {
            strcpy(out_res, buffer);
        }
    } else if (out_res != NULL) {
        out_res[0] = '\0';
    }
}

// Hàm kết nối Socket tới một Hostname/IP và Port cụ thể
int connect_server(const char* host, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    struct hostent* he = gethostbyname(host);
    if (he == NULL) return -1;

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr*)he->h_addr_list[0]);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}

// Hàm xử lý lệnh PASV để lấy IP và Port của kênh Data Connection
int setup_data_connection(int ctrl_sock) {
    char res[BUFFER_SIZE];
    send_ftp_cmd(ctrl_sock, "PASV\r\n", res);
    
    char* start = strchr(res, '(');
    char* end = strchr(res, ')');
    if (start == NULL || end == NULL) return -1;

    *end = '\0'; 
    start++;     

    int h1, h2, h3, h4, p1, p2;

    if (sscanf(start, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2) != 6) {
        return -1;
    }

    char data_ip[32];
    sprintf(data_ip, "%d.%d.%d.%d", h1, h2, h3, h4);
    int data_port = p1 * 256 + p2;

    return connect_server(data_ip, data_port);
}

// Hàm đảo ngược chuỗi ký tự
void reverse_string(char* str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

int main() {
   
    char ftp_server[] = "lebavui.io.vn"; 
    char username[] = "user_20235404";  // Gõ thẳng tên user của bạn vào đây
    char password[] = "540418";

    printf("--- BẮT ĐẦU CHẠY FTP CLIENT (C LANGUAGE) ---\n");

    int ctrl_sock = connect_server(ftp_server, 21);
    if (ctrl_sock < 0) {
        perror("LỖI: Không thể kết nối tới FTP Server!");
        return 1;
    }
    
    send_ftp_cmd(ctrl_sock, "", NULL); // Nhận phản hồi chào mừng 220
    
    char user_cmd[100], pass_cmd[100];
    sprintf(user_cmd, "USER %s\r\n", username);
    sprintf(pass_cmd, "PASS %s\r\n", password);
    
    send_ftp_cmd(ctrl_sock, user_cmd, NULL);
    send_ftp_cmd(ctrl_sock, pass_cmd, NULL);

    int data_sock = setup_data_connection(ctrl_sock);
    if (data_sock < 0) {
        fprintf(stderr, "LỖI: Không kết nối được kênh Data!\n");
        close(ctrl_sock);
        return 1;
    }
    send_ftp_cmd(ctrl_sock, "NLST\r\n", NULL);

    char data_buf[BUFFER_SIZE];
    char file_list[BUFFER_SIZE * 2];
    memset(file_list, 0, sizeof(file_list));
    int bytes;

    while ((bytes = recv(data_sock, data_buf, BUFFER_SIZE - 1, 0)) > 0) {
        data_buf[bytes] = '\0';
        strcat(file_list, data_buf);
    }
    close(data_sock);
    send_ftp_cmd(ctrl_sock, "", NULL);

    // Tìm tên file có định dạng question_xxxxxx.txt từ danh sách trả về
    char question_file[256] = "";
    char* token = strtok(file_list, "\r\n ");
    while (token != NULL) {
        if (strncmp(token, "question_", 9) == 0 && strstr(token, ".txt") != NULL) {
            strcpy(question_file, token);
            break;
        }
        token = strtok(NULL, "\r\n ");
    }

    if (strlen(question_file) == 0) {
        fprintf(stderr, "LỖI: Không tìm thấy file câu hỏi nào phù hợp!\n");
        close(ctrl_sock);
        return 1;
    }
    printf("==> Tìm thấy file câu hỏi trên Server: %s\n", question_file);

    data_sock = setup_data_connection(ctrl_sock);
    char retr_cmd[300];
    sprintf(retr_cmd, "RETR %s\r\n", question_file);
    send_ftp_cmd(ctrl_sock, retr_cmd, NULL);

    char question_content[BUFFER_SIZE] = "";
    while ((bytes = recv(data_sock, data_buf, BUFFER_SIZE - 1, 0)) > 0) {
        data_buf[bytes] = '\0';
        strcat(question_content, data_buf);
    }
    close(data_sock);
    send_ftp_cmd(ctrl_sock, "", NULL); 
    printf("==> Nội dung câu hỏi nhận được: %s\n", question_content);

    char answer_content[BUFFER_SIZE];
    strcpy(answer_content, question_content);
    reverse_string(answer_content); 

    char answer_file[256];
    sprintf(answer_file, "answer%s", question_file + 8); 
    
    printf("==> Chuỗi sau khi đảo ngược: %s\n", answer_content);
    printf("==> Sẽ thực hiện upload file: %s\n", answer_file);

    data_sock = setup_data_connection(ctrl_sock);
    char stor_cmd[300];
    sprintf(stor_cmd, "STOR %s\r\n", answer_file);
    send_ftp_cmd(ctrl_sock, stor_cmd, NULL);

    send(data_sock, answer_content, strlen(answer_content), 0);
    close(data_sock); 
    send_ftp_cmd(ctrl_sock, "", NULL); 

    send_ftp_cmd(ctrl_sock, "QUIT\r\n", NULL);
    close(ctrl_sock);

    return 0;
}