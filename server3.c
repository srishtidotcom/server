//server version for parsing file path from the http request over serving static file server version

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 8080
#define BUF_SIZE 4096

// ---------------- MIME TYPE ----------------
const char* get_mime_type(const char* path)
{
    const char* ext = strrchr(path, '.');
    if (!ext) return "text/plain";
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0)  return "text/css";
    if (strcmp(ext, ".js") == 0)   return "application/javascript";
    if (strcmp(ext, ".png") == 0)  return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
        return "image/jpeg";
    return "application/octet-stream";
}

// ---------------- READ FILE ----------------
char* read_file(const char* path, int* size)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) return NULL;

    struct stat st;
    if (stat(path, &st) < 0)
    {
        close(fd);
        return NULL;
    }

    *size = st.st_size;
    char* buffer = malloc(*size);
    if (!buffer)
    {
        close(fd);
        return NULL;
    }

    read(fd, buffer, *size);
    close(fd);
    return buffer;
}

// ---------------- PARSE PATH ----------------
int parse_path(const char* request, char* out_path)
{
    char method[8];
    char path[256];

    if (sscanf(request, "%7s %255s", method, path) != 2)
        return -1;

    if (strcmp(method, "GET") != 0)
        return -1;

    // prevent directory traversal
    if (strstr(path, ".."))
        return -1;

    if (strcmp(path, "/") == 0)
        strcpy(out_path, "www/index.html");
    else
        snprintf(out_path, 512, "www%s", path);

    return 0;
}

// ---------------- MAIN ----------------
int main()
{
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        exit(1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    memset(address.sin_zero, 0, sizeof address.sin_zero);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        exit(1);
    }

    printf("Server listening on port %d\n", PORT);

    while (1)
    {
        client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        char request[BUF_SIZE] = {0};
        read(client_fd, request, sizeof(request));
        printf("Request:\n%s\n", request);

        char file_path[512];

        if (parse_path(request, file_path) == 0)
        {
            int file_size;
            char* file = read_file(file_path, &file_size);

            if (file)
            {
                char header[512];
                sprintf(header,
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n"
                        "\r\n",
                        get_mime_type(file_path),
                        file_size);

                send(client_fd, header, strlen(header), 0);
                send(client_fd, file, file_size, 0);
                free(file);
            }
            else
            {
                char* not_found =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n"
                    "\r\n";
                send(client_fd, not_found, strlen(not_found), 0);
            }
        }
        else
        {
            char* forbidden =
                "HTTP/1.1 403 Forbidden\r\n"
                "Content-Length: 0\r\n"
                "Connection: close\r\n"
                "\r\n";
            send(client_fd, forbidden, strlen(forbidden), 0);
        }

        close(client_fd);
    }

    return 0;
}
