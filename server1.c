//server side c program to show socket programming

//server side

#include<stdio.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>

#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    //char *hello = "Hello from server";

    char *response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 17\r\n"
    "Connection: close\r\n"
    "\r\n"
    "Hello from server";

    //socket file descriptor is created
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0))==0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("in bind");
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, 3)<0)
    {
        perror("in listen");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("in accept");
            exit(EXIT_FAILURE);
        }

        char buffer[1024] = {0};
        valread = read(new_socket, buffer, 1024);
        printf("%s\n", buffer);

        //send(new_socket, hello, strlen(hello), 0);

        send(new_socket, response, strlen(response), 0);
        printf("Hello message sent\n");
        close(new_socket);
    }

    return 0;

}

 


/*
#include <sys/socket.h>

int server_fd = socket(domain, type, protocol);

if ( ( server_fd = socket(AF_INET, SOCK_STREAM , 0) ) < 0 ) {
    perror("cannot create socket");
    return 0;
}

struct sockaddr_in address;
const int PORT = 8080;

memset((char*)&address, 0, sizeoof(address));
address.sin_family = AF_INET;
address.sin_addr.s_addr = htonl(INADDR_ANY);
address.sin_port = htons(PORT);

if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    return 0;
}

struct sockaddr_in
{
    _uint8_t         sin_len;
    sa_family_t     sin_family;
    in_port_t       sin_port;
    struct  in_addr sin_addr;
    char            sin_zero[8];
}

int bind(int socket, const struct sockaddr *address, socklen_t address_len);

int listen(int socket, int backlog);

int accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);

if (listen(server_fd, 3) < 0) {
    perror("in listen");
    exit(EXIT_FAILURE);
}

if ((new_socket = accept(server_fd (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
    perror("in accept");
    exit(EXIT_FAILURE);
}

char buffer[1024] = {0};

int valread = read(new_socket, buffer, 1024);
printf("%s\n", buffer);
if(valread < 0) {
    printf("no bytes are there to read");
}

char *hello = "Hello from server";
write(new_socket , hello , strlen(hello));

close(new_socket);

*/