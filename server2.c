// server version for file serving over the baseline working server

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 8080

// Read entire file into memory
char *read_file(const char *path, int *out_size)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return NULL;

    struct stat st;
    if (stat(path, &st) < 0)
    {
        close(fd);
        return NULL;
    }

    *out_size = st.st_size;
    char *buffer = malloc(*out_size);
    if (!buffer)
    {
        close(fd);
        return NULL;
    }

    read(fd, buffer, *out_size);
    close(fd);
    return buffer;
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    //socket is created
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    // bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // server listen for connections
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1)
    {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0)
        {
            perror("accept");
            continue;
        }

        char request[1024] = {0};
        read(new_socket, request, sizeof(request));
        printf("Request:\n%s\n", request);

        // naive request check
        if (strncmp(request, "GET / ", 6) == 0)
        {
            int file_size;
            char *file = read_file("www/index.html", &file_size);

            if (file)
            {
                char header[256];
                sprintf(header,
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n"
                        "\r\n",
                        file_size);

                send(new_socket, header, strlen(header), 0);
                send(new_socket, file, file_size, 0);
                free(file);
            }
            else
            {
                char *not_found =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n"
                    "\r\n";
                send(new_socket, not_found, strlen(not_found), 0);
            }
        }
        else
        {
            char *bad_request =
                "HTTP/1.1 400 Bad Request\r\n"
                "Content-Length: 0\r\n"
                "Connection: close\r\n"
                "\r\n";
            send(new_socket, bad_request, strlen(bad_request), 0);
        }

        close(new_socket);
    }

    return 0;
}
