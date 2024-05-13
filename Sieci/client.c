#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 200

char* sending(char* client_name, char option) {
    static char message[15];
    message[0] = '@';

    switch(option) {
        case '1': 
            strncpy(&message[1], client_name, sizeof(message) - 1);
            message[9] = '0';
            message[10] = '!';
            message[11] = 'N';
            message[12] = ':';
            message[13] = '0';
            message[14] = '#';
            break;
        default:
            message[1] = 'E';
            break;
    }

    return message;
}
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <client name> <Ip addr> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    //Utworzenie tablicy charów z nazwą klienta
    char *client_name = argv[1];
    //Sprawdzenie ile znaków ma nazwa klienta
    int client_name_len = strlen(client_name);
    
    //printf("Client name length: %d\n", client_name_len);
    //Jeżeli nazwa klienta ma mniej niż 8 znaków to uzupełnij resztę _ do 8 znaków
    char new_client_name[8];
    if (client_name_len < 8) {
        
        for (int i = 0; i < 8; i++) {
            if (i < client_name_len) {
                new_client_name[i] = client_name[i];
            } else {
                new_client_name[i] = '_';
            }
        }
    }else if(client_name_len>8){
        printf("Client name is too long, it should be maximum 8 characters\n");
        exit(EXIT_FAILURE);
    }else{
        for(int i=0;i<8;i++){
            new_client_name[i]=client_name[i];
        }
    }


    //Utworzenie struktury sockaddr_in dla serwera
    int portno;
    struct sockaddr_in server_addr;
    //Przypisanie ip adresu serwera do zmiennej ip_addr
    char *ip_addr = argv[2];

    portno = atoi(argv[3]);
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);

    if (inet_pton(AF_INET, ip_addr, &server_addr.sin_addr) <= 0) {
        perror("inet_pton() ERROR");
        exit(EXIT_FAILURE);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket() ERROR");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect() error\n");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server. Enter message to send (or 'exit' to quit): \n");


    fd_set readfds;
    FD_ZERO(&readfds);

    FD_SET(STDIN_FILENO , &readfds); // stdin do zbiotu odczytu
    FD_SET(sock, &readfds);         // sock do zbioru do odczytu

    while (1) {
        fflush(stdout);

        int max_fd = (sock > STDIN_FILENO) ? sock : STDIN_FILENO;
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select() error");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char input_buffer[BUFFER_SIZE];
            fgets(input_buffer, BUFFER_SIZE, stdin);

            if (strncmp(input_buffer, "exit", 4) == 0) {
                printf("Exiting...\n");
                break;
            }

            if (send(sock, sending(new_client_name,'1'), strlen(sending(new_client_name,'1')), 0) < 0) {
                perror("send() error");
                exit(EXIT_FAILURE);
            }
            printf("Client name: %s\n", new_client_name);
        }


        if (FD_ISSET(sock, &readfds)) {
            char buffer[BUFFER_SIZE];
            ssize_t n = recv(sock, buffer, sizeof(buffer), 0);
            if (n < 0) {
                perror("recv() error");
                exit(EXIT_FAILURE);
            } else if (n == 0) {
                printf("Server closed the connection.\n");
                break;
            } else {
                printf("\nReceived message from server: %.*s\n\n", (int)n, buffer);
            }
        }

        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock, &readfds);
    }

    close(sock);
    return 0;
}