/* protocol.h - Header file del  client*/
#ifndef PROTOCOL_H_
#define PROTOCOL_H_

/* Parametri */
#define QUEUE_SIZE 5
#define BUFFER_SIZE 512
#define SERVER_PORT 56700
#define DEFAULT_IP "127.0.0.1"

/* Codici di stato della risposta */
#define STATUS_SUCCESS 0
#define STATUS_CITY_UNAVAILABLE 1
#define STATUS_INVALID_REQUEST 2

/* Tipi di richiesta meteo */
#define TYPE_TEMPERATURE 't'
#define TYPE_HUMIDITY    'h'
#define TYPE_WIND        'w'
#define TYPE_PRESSURE    'p'


/*strutture */

typedef struct{
char type;
char city[64];
}weather_request_t;

typedef struct {
unsigned int status;
char type;
float value;
} weather_response_t;

/* Prototipi funzioni Windows */
#if defined(_WIN32) || defined(_WIN64)
int winstartup(void);
void clearwinsock(void);
#endif

#endif /* PROTOCOL_H_ */


/* protocol.h - Header file del server*/
#ifndef PROTOCOL_H_
#define PROTOCOL_H_

/* Parametri */
#define QUEUE_SIZE 5
#define BUFFER_SIZE 512
#define SERVER_PORT 56700
#define DEFAULT_IP "127.0.0.1"

/* Codici di stato della risposta */
#define STATUS_SUCCESS 0
#define STATUS_CITY_UNAVAILABLE 1
#define STATUS_INVALID_REQUEST 2

/* Tipi di richiesta meteo */
#define TYPE_TEMPERATURE 't'
#define TYPE_HUMIDITY    'h'
#define TYPE_WIND        'w'
#define TYPE_PRESSURE    'p'


/*strutture */

typedef struct{
char type;
char city[64];
}weather_request_t;

typedef struct {
unsigned int status;
char type;
float value;
} weather_response_t;

/* Prototipi funzioni Windows */
#if defined(_WIN32) || defined(_WIN64)
int winstartup(void);
void clearwinsock(void);
#endif

#endif /* PROTOCOL_H_ */
