#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <mysql/mysql.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "4673"

#define MYSQL_HOST "82.98.134.234"
#define MYSQL_DB "seleccion_2020"
#define MYSQL_USER "u_seleccion"
#define MYSQL_PASSWORD "51j33oqU"

bool is_authorized_user(char *user, char *password);

int send_command(char *command, char *server);

int main(int argc, char* argv[]) {

    // Comprobación de la entrada por consola
    if (argc < 5){
        printf("Se deben introducir 4 argumentos: usuario, clave, servidor y comando a ejectuar");
        return 1;
    }

    // Asignación y visualización de los datos a usar
    char *user = argv[1];
    char *password = argv[2];
    char *server = argv[3];
    char *command = argv[4];
    printf("- Usuario: %s\n", user);
    printf("- Clave: %s\n", password);
    printf("- Servidor: %s\n", server);
    printf("- Comando a ejecutar: %s\n", command);

    // Comprobación de si el usuario está autorizado
    if (is_authorized_user(user, password))
        printf("El usuario está en la BB.DD\n");
    else {
        printf("Vaya... no está en BB.DD...\n");
        exit(1);
    }

    // Envío del comando
    send_command(command, server);
    return 0;
}

/*
 * Autenticación del usuario en la BB.DD.
 * */
bool is_authorized_user(char *user, char *password) {
    // MySQL
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    // Conexión MySQL
    conn = mysql_init(NULL);

    if (mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DB, 0, NULL, 0) == NULL)
        return false;

    // Creación de la consulta SQL
    char query[256];
    sprintf(query,"SELECT * FROM usuarios WHERE usuario = '%s' AND clave = '%s'", user, password);

    // Consulta a BB.DD.
    int query_result = mysql_query(conn, query);
    if (query_result)
        return false;

    // Comprobación del resultado de la consulta
    res = mysql_use_result(conn);
    if (res == NULL)
        return false;

    // Finalmente, se comprueba si el usuario y clave se corresponden.
    // Para ello se debe devolver, al menos, un registro
    row = mysql_fetch_row(res);

    // Almacenamos el resultato de la autenticación antes de liberar los recursos
    bool is_authenticated = row != NULL;

    // Liberamos los recursos
    mysql_free_result(res);
    mysql_close(conn);

    // Devolvemos el valor de la consulta
    return is_authenticated;
}

/*
 * Envía un comando a un socket remoto
 * */
int send_command(char *command, char *remote_server){
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
            *ptr = NULL,
            hints;

    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup ha fallado con el siguiente ID de error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolución de la dirección y puerto del Servidor
    iResult = getaddrinfo(remote_server, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo ha fallado con el siguiente ID de error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Intento de conexión
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
        // Creación del SOCKET para conectarse con el servidor.
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                               ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("El Socket ha fallado con el ID de error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Conexión con el servidor.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("No se ha podido conectar con el servidor. Cerrando conexión...\n");
        WSACleanup();
        return 1;
    }

    // Send del commando
    iResult = send( ConnectSocket, command, (int)strlen(command), 0 );
    if (iResult == SOCKET_ERROR) {
        printf("El envío a fallado con el error : %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Cerramos la conexión de envío ya que no vamos a enviar más datos
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("No hemos podido cerrar la conexión. Error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 ) {
            printf("Respuesta del Servidor: %s\n", recvbuf);
            break;
        }
        else if ( iResult == 0 )
            printf("Conexión cerrada\n");
        else
            printf("recv fallido. Error: %d\n", WSAGetLastError());

    } while( iResult > 0 );

    // Limpieza de sockets
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}