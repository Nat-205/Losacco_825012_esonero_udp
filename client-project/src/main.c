/* client main */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "protocol.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock.h>
    void clearwinsock() { WSACleanup(); }
    int winstartup() {
        WSADATA windata;
        if (WSAStartup(MAKEWORD(2,2), &windata) != 0) {
            puts("Errore in WSAStartup()");
            return 0;
        }
        return 1;
    }
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #define closesocket close
#endif

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

#if defined(_WIN32) || defined(_WIN64)
    if (!winstartup()) return -1;
#endif

    int port = SERVER_PORT;
    char server_str[BUFFER_SIZE];
    memset(server_str, 0, BUFFER_SIZE);
    strcpy(server_str, "127.0.0.1"); // Default

    weather_request_t request;
    memset(&request, 0, sizeof(request));

    int opt;
    struct hostent *host = NULL;

    while ((opt = getopt(argc, argv, "r:p:c:t:s:")) != -1) {
        switch (opt) {
            case 'r':
                sscanf(optarg, " %c %63[^\n]", &request.type, request.city);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 't':
                request.type = optarg[0];
                break;
            case 'c':
                strncpy(request.city, optarg, sizeof(request.city) - 1);
                break;
            case 's':
                strncpy(server_str, optarg, sizeof(server_str) - 1);
                server_str[sizeof(server_str) - 1] = '\0';
                break;
        }
    }

    if (request.type == 0 || strlen(request.city) == 0) {
        return -1;
    }

    int connysocks = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (connysocks < 0) {
		puts("errore di generazione della socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_aton(server_str, &server_addr.sin_addr) != 0) {
        host = gethostbyaddr((char*)&server_addr.sin_addr, sizeof(server_addr.sin_addr), AF_INET);
    }
    else {

        host = gethostbyname(server_str);
        if (host == NULL) {
        	puts("non e' possibile collegarsi con l'host");
            closesocket(connysocks);
            return -1;
        }

        server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    }


    if (sendto(connysocks, (char*)&request, sizeof(request), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    	puts("errore di invio della richiesta ");
        closesocket(connysocks);
        return -1;
    }

    char resp[sizeof(uint32_t) + sizeof(char) + sizeof(float)];
    int server_size = sizeof(server_addr);

    if (recvfrom(connysocks, resp, sizeof(resp), 0, (struct sockaddr *)&server_addr, &server_size) < 0) {
    	puts("errore nella ricezione della risposta da parte del server ");
        closesocket(connysocks);
        return -1;
    }

    weather_response_t response;
    int pos = 0;

    // DESERIALIZZAZIONE
    // 1. Status
    uint32_t stat;
    memcpy(&stat, resp + pos, sizeof(uint32_t));
    response.status = ntohl(stat);
    pos += sizeof(uint32_t);

    // 2. Type
    memcpy(&response.type, resp + pos, sizeof(char));
    pos += sizeof(char);

    // 3. Value
    uint32_t flo;
    memcpy(&flo, resp + pos, sizeof(uint32_t));
    flo = ntohl(flo);
    memcpy(&response.value, &flo, sizeof(float));
    pos += sizeof(float);

    char *nome_server = (host != NULL) ? host->h_name : "Sconosciuto";
    printf("Ricevuto risultato dal server %s (IP %s).  ", nome_server, inet_ntoa(server_addr.sin_addr));

    request.city[0] = toupper(request.city[0]);

    switch (response.status) {
        case STATUS_SUCCESS:
            switch (response.type) {
                case TYPE_TEMPERATURE:
                    printf("%s: Temperatura = %.2f C\n", request.city, response.value);
                    break;
                case TYPE_HUMIDITY:
                    printf("%s: Umidita' = %.1f %%\n", request.city, response.value);
                    break;
                case TYPE_WIND:
                    printf("%s: Vento = %.1f km/h\n", request.city, response.value);
                    break;
                case TYPE_PRESSURE:
                    printf("%s: Pressione = %.1f hPa\n", request.city, response.value);
                    break;
            }
            break;
        case STATUS_CITY_UNAVAILABLE:
            printf("Citta' non disponibile\n");
            break;
        case STATUS_INVALID_REQUEST:
            printf("Richiesta non valida\n");
            break;
    }

    closesocket(connysocks);
#if defined(_WIN32) || defined(_WIN64)
    clearwinsock();
#endif
    return 0;
}
