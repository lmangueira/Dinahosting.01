#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "4673"

#define LOG_FILEPATH "C:/ServidorDH/log.txt"
#define LOG_DEFAULT_NAME "log.txt"
#define COMMAND_REBOOT "reboot"

int log_connection(SOCKET client_socket, char* command);
char* get_current_datetime();
int execute_command(char* command);

int main() {
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Inicializa Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup ha fallado con el ID de error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    do{
        // Resuelve la dirección y puerto del Server
        iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
        if (iResult != 0) {
            printf("getaddrinfo ha fallado con el ID de error: %d\n", iResult);
            WSACleanup();
            return 1;
        }

        // Crea el SOCKET para el Server
        ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (ListenSocket == INVALID_SOCKET) {
            printf("socket ha fallado. Error: %ld\n", WSAGetLastError());
            freeaddrinfo(result);
            WSACleanup();
            return 1;
        }

        // Socket TCP - Escuchando
        iResult = bind(ListenSocket, result->ai_addr, (int) result->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            printf("bind fallido. Error: %d\n", WSAGetLastError());
            freeaddrinfo(result);
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        freeaddrinfo(result);

        iResult = listen(ListenSocket, SOMAXCONN);
        if (iResult == SOCKET_ERROR) {
            printf("Método listen fallido. Error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        // Acepta los sockets de los Clientes
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept fallido. Error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        // No longer need server socket
        closesocket(ListenSocket);;

        // Recibimos datos
        do {
            iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (iResult > 0) {
                log_connection(ClientSocket, recvbuf);
                char *responseBuffer = NULL;
                if (strcmp(recvbuf, COMMAND_REBOOT) == 0) {
                    responseBuffer = "ERROR";
                } else {
                    execute_command(recvbuf);
                    responseBuffer = "OK";
                }

                // Enviamos respuesta al Client
                iSendResult = send(ClientSocket, responseBuffer, sizeof(responseBuffer), 0);
                if (iSendResult == SOCKET_ERROR) {
                    printf("Envío de respuesta fallido. Error: %d\n", WSAGetLastError());
                    closesocket(ClientSocket);
                    WSACleanup();
                    return 1;
                };
            } else if (iResult != 0)
            {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }

        } while (iResult > 0);
    } while(1);
}

/*
 * Logs each connection to current server
 * */
int log_connection(SOCKET client_socket, char* command){
    char* current_datetime = get_current_datetime();
    FILE *log = NULL;

    log = fopen(LOG_FILEPATH, "a");
    if (log == NULL) {
        log = fopen(LOG_DEFAULT_NAME, "a");
        if (log == NULL)
            return -1;
    }
    // Get current Socket IP Address
    struct sockaddr_in* clientAddress = (struct sockaddr_in*)&client_socket;
    char *ip = inet_ntoa(clientAddress->sin_addr);

    fprintf(log, "[%s] Client: %s - Command: %s\n", current_datetime, ip, command);
    fclose(log);
}

/*
 * Returns a formatted string for current datetime
 * */
char* get_current_datetime(){
    time_t now;
    time(&now);

    int hours, minutes, seconds, day, month, year;
    struct tm *local = localtime(&now);
    hours = local->tm_hour;
    minutes = local->tm_min;
    seconds = local->tm_sec;
    day = local->tm_mday;
    month = local->tm_mon +1;
    year = local->tm_year + 1900;

    char* formatted_datetime = (char*)malloc(20 * sizeof(char));
    sprintf(formatted_datetime, "%04d/%02d/%02d %02d:%02d:%02d", year, month, day, hours, minutes, seconds);
    return formatted_datetime;
}

/*
 * Executes a command
 * */
int execute_command(char* command){
    FILE *fp;
    char path[2048];
    fp = popen(command, "r");
    if (fp == NULL){
        printf("Failed to run command\n");
        return 1;
    }

    while (fgets(path, sizeof(path), fp) != NULL){
        printf("%s", path);
    }

    pclose(fp);;
    return 0;
}

