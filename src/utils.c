#include "utils.h"

#define STYLE_BOLD "\033[1m"
#define STYLE_NO_BOLD "\033[22m"

char readFromSocket(FILE* fp) {
    char buf[MAX_LEN];
    do {
        fgets(buf, MAX_LEN - 1, fp);
        printf("%s", buf);
    } while (buf[3] == '-');

    return buf[0];
}

int create_socket(char* ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    // Server address handling
    bzero((char*) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip); /* 32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);          /* server TCP port must be network byte ordered */

    // Open an TCP socket

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(0);
    }

    /* Connect to the server */
    if (connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(0);
    }

    return sockfd;
}

int get_port(char* str) {
    int port;
    str += 27; // Skip useless info

    char* token = strtok(str, ",");

    for (int i = 0; i < 4; i++)
        token = strtok(NULL, ",");

    port = atoi(token) * 256;
    token = strtok(NULL, ",");
    port += atoi(token);

    printf("\nPort : %d\n\n", port);
    return port;
}

int parse_url(char* arguments, struct url* url) {
    strcpy(url->user, "anonymous"); // Assume anon
    strcpy(url->password, "");      // Assume anon

    // Get protocol name

    if (strncmp(arguments, "ftp://", 6)) {
        printf("ERROR: Couldn't parse the protocol name\n");
        return -1;
    }

    arguments += 6; // Skip ftp://
    char* token;

    // Get user
    if (strstr(arguments, "@") == NULL) {
        puts("No username found. Assuming anonymous user.");
        strcpy(url->user, "anonymous"); // Assume anon

        // Get host
        token = strtok(arguments, "/");
    } else {
        if (strstr(arguments, ":") == NULL) { // No password provided
            token = strtok(arguments, "@");
            strcpy(url->user, token);
            strcpy(url->password, "");
        }

        else {
            token = strtok(arguments, ":");
            strcpy(url->user, token);
            token = strtok(NULL, "@");
            strcpy(url->password, token);
        }

        // Get host
        token = strtok(NULL, "/");
    }

    if (token == NULL) {
        printf("ERROR: Couldn't parse the host\n");
        return -1;
    }

    strcpy(url->host, token);

    // Get file path

    token = strtok(NULL, "");

    if (token == NULL) {
        printf("ERROR: Couldn't parse the filepath.\n");
        return -1;
    }

    strcpy(url->filepath, token);

    return 0;
}

int get_file_size(char* response) {
    char* s = strrchr(response, '(') + 1;
    int file_size = atoi(strtok(s, " "));

    return file_size;
}

int download_file(int file_size, int data_socket_fd, char* filepath) {
    char buf[MAX_LEN];
    int bytes;
    float total_bytes = 0, percentage = 0;

    char* filename = strrchr(filepath, '/');

    if (filename == NULL)
        filename = filepath;
    else
        filename += 1;

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("ERROR: Failed to open file.\n");
        exit(1);
    }

    printf("\nStarting to download %s\n\n", filename);
    progress_bar(percentage);
    while ((bytes = read(data_socket_fd, buf, sizeof(buf))) > 0) {
        bytes = fwrite(buf, 1, bytes, file);
        total_bytes += bytes;
        percentage = (float) total_bytes / file_size * 100;
        progress_bar(percentage);
    }
    printf("\nFinished downloading %s\n\n", filename);

    fclose(file);

    return 0;
}

void progress_bar(float percentage) {
    printf("[");

    for (int i = 0; i < 50; i++) {
        if (percentage / 2 < i)
            printf(" ");

        else
            printf("x");
    }

    printf("] %.2f%% complete\n", percentage);
}
