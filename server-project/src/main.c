
// main del server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>
#include "protocol.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    // Su Windows strcasecmp si chiama _stricmp
    #define strcasecmp _stricmp
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #define closesocket close
#endif

// Lista città supportate
static const char *SUPPORTED_CITIES[] = {
    "Bari", "Roma", "Milano", "Napoli", "Torino",
    "Palermo", "Genova", "Bologna", "Firenze", "Venezia"
};

void errorhandler(char *errorMessage) { printf("%s\n", errorMessage); }

void clearwinsock() {
#if defined(_WIN32) || defined(_WIN64)
    WSACleanup();
#endif
}

float random_float(float min, float max) {
    float scale = rand() / (float) RAND_MAX;
    return min + scale * (max - min);
}

void valida(weather_request_t *req, weather_response_t *resp) {

    // 1. Controllo Tipo
    if(req->type != TYPE_TEMPERATURE && req->type != TYPE_HUMIDITY &&
       req->type != TYPE_WIND && req->type != TYPE_PRESSURE) {
        resp->status = STATUS_INVALID_REQUEST;
        return;
    }

    // 2. Controllo del nome città
    for (int i = 0; req->city[i] != '\0'; i++) {
        if (!isalpha((unsigned char)req->city[i])) {
            resp->status = STATUS_INVALID_REQUEST;
            return;
        }
    }

    int found = 0;
    for (int i = 0; i < 10; i++) {
        if(strcasecmp(req->city, SUPPORTED_CITIES[i]) == 0) {
            found = 1;
            break;
        }
    }

    if (found) {
        resp->status = STATUS_SUCCESS;
    } else {
        resp->status = STATUS_CITY_UNAVAILABLE;
    }
}


float get_temperature(void) { return random_float(-10.0, 40.0); }
float get_humidity(void)    { return random_float(20.0, 100.0); }
float get_wind(void)        { return random_float(0.0, 100.0); }
float get_pressure(void)    { return random_float(950.0, 1050.0); }

//inizio main server
int main(int argc, char *argv[]) {
#if defined(_WIN32) || defined(_WIN64)
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2,2), &wsa_data) != 0) {
        printf("Errore in WSAStartup()\n");
        return 0;
    }
#endif

    int port = SERVER_PORT;
    if (argc > 2 && strcmp(argv[1], "-p") == 0) {
        port = atoi(argv[2]);
    }

    int socks = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socks < 0) {
        errorhandler("Creazione socket fallita.\n");
        clearwinsock();
        return -1;
    }

    struct sockaddr_in sad;  /* server address */
    struct sockaddr_in cad;  /* client address */
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = htonl(INADDR_ANY);
    sad.sin_port = htons(port);

    if (bind(socks, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
        errorhandler("Bind fallito.\n");
        closesocket(socks);
        clearwinsock();
        return -1;
    }

    srand(time(NULL));

    puts("connessione con l'host effettuata");
    while (1) {

        int cad_len = sizeof(cad);
        weather_request_t request;
        memset(&request, 0, sizeof(request));


        int n_recv = recvfrom(socks, (char*)&request, sizeof(request), 0, (struct sockaddr *)&cad, &cad_len);

        if (n_recv < 0) {
            errorhandler("richiesta non ricevuta correttamente! ");
            continue;
        }

        weather_response_t response;
        memset(&response, 0, sizeof(response));


        valida(&request, &response);


        if(response.status == STATUS_SUCCESS) {
            response.type = request.type;
            switch (request.type) {
                case TYPE_TEMPERATURE: response.value = get_temperature(); break;
                case TYPE_HUMIDITY:    response.value = get_humidity(); break;
                case TYPE_WIND:        response.value = get_wind(); break;
                case TYPE_PRESSURE:    response.value = get_pressure(); break;
            }
        } else {
            response.type = request.type;
            response.value = 0.0;
        }


        //serializzazione della struct
        char info[sizeof(uint32_t) + sizeof(char) + sizeof(float)];
        int serialize = 0;

        // A. Status
        uint32_t nstatus = htonl(response.status);
        memcpy(info + serialize, &nstatus, sizeof(uint32_t));
        serialize += sizeof(uint32_t);

        // B. Type
        memcpy(info + serialize, &response.type, sizeof(char));
        serialize += sizeof(char);

        // C. Value (Float)
        uint32_t flo;
        memcpy(&flo, &response.value, sizeof(float));
        flo = htonl(flo);
        memcpy(info + serialize, &flo, sizeof(float));
        serialize += sizeof(float);

        if (sendto(socks, info, serialize, 0, (struct sockaddr *)&cad, cad_len) < 0) {
            errorhandler("Errore in sendto");
        }

    }

    closesocket(socks);
    clearwinsock();
    return 0;
}

