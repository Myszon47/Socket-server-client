#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>

#define BUFSIZE 8192
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 10

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void send_file(FILE *fp, int sockfd);
void * handle_connection(int);
int check(int exp, const char *msg);
int accept_new_connection(int server_socket);
int setup_server(short port, int backlog);

int main(void) {
    short port;
    printf("Na ktorym porcie mam sluchac? : ");
    scanf("%hu", &port);
    int server_socket = setup_server(port, SERVER_BACKLOG);

    fd_set current_sockets, ready_sockets;
    FD_ZERO(&current_sockets);
    FD_SET(server_socket, &current_sockets);

    while (true) {
        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0) {
            perror("select error");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &ready_sockets)) {
                if (i == server_socket) {
                    int client_socket = accept_new_connection(server_socket);
                    FD_SET(client_socket, &current_sockets);
                } else {
                    handle_connection(i);
                    FD_CLR(i, &current_sockets);
                }
            }
        }
    }
    return EXIT_SUCCESS;
}

int setup_server(short port, int backlog){
    int server_socket;
    SA_IN server_addr;

    check((server_socket = socket(AF_INET, SOCK_STREAM,0))
            , "Failed to create socket");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    check(bind(server_socket, (SA*)&server_addr, sizeof(server_addr))
            , "Bind failed");
    check(listen(server_socket, backlog)
            , "Listen Failed");
    return server_socket;
}

int accept_new_connection(int server_socket){
    int addr_size = sizeof(SA_IN);
    int client_socket;
    SA_IN client_addr;
    check(client_socket = accept(server_socket,
                                 (SA*)&client_addr,
                                 (socklen_t*)&addr_size),
          "accept failed");
    return client_socket;
}

int check(int exp, const char  *msg){
    if(exp == SOCKETERROR){
        perror(msg);
        exit(1);
    }
    return exp;
}

void send_file(FILE *fp, int sockfd)
{
    char sendBuffer[BUFSIZE];
    int nb = fread(sendBuffer, 1, sizeof(sendBuffer), fp);
    while(!feof(fp)) {
        write(sockfd, sendBuffer, nb);
        nb = fread(sendBuffer, 1, sizeof(sendBuffer), fp);
    }
}

void * handle_connection(int client_socket){
    char buffer[BUFSIZE];
    memset(buffer, 0, 8192);
    recv(client_socket, buffer, 8192, 0);
    char* file_path = buffer;
    char* ext = "";
    char good_path[strlen(file_path)+strlen(ext)+1];
    snprintf(good_path,sizeof(good_path),"%s%s", file_path, ext);
    memset(buffer, 0, 8192);

    FILE *fp;
    fp = fopen(good_path, "r");
    if(fp == NULL)
    {
        perror("Error in reading file.");
        exit(1);
    }
    send_file(fp,client_socket);
    printf("File data send successfully. \n");
    close(client_socket);
    fclose(fp);
    printf("closing connection\n");
    return NULL;

}
