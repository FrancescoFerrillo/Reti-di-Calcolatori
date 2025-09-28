
/* =====================================================================================================
Il progetto consiste nello sviluppo di un'applicazione Client-Server su Linux, con lo scopo
di simulare un sistema di gestione di abbonamenti annuali per un cinema multisala. Il sistema permette
agli utenti (client) di effettuare la registrazione a una delle tre tipologie di tessera
disponibili oppure di richiederne la disdetta, mentre il server gestisce in modo concorrente le
richieste e tiene traccia delle disponibilità residue per ciascun tipo di tessera
(100 per ogni tipo: ingressi con festivi esclusi, festivi inclusi o tutti i giorni incluse le anteprime).
=======================================================================================================*/


#include <stdio.h>      // printf, perror
#include <stdlib.h>     // malloc, free, exit
#include <string.h>     //strcpy, strlen

// Librerie per la programmazione concorrente e socket di rete
#include <pthread.h>    // thread, mutex: gestione della concorrenza (server multithread)
#include <arpa/inet.h>  // socket, sockaddr_in, htons, inet_ntoa: gestione indirizzi e porte
#include <unistd.h>     // close, pthread_exit



#define PORT 8080            // Porta su cui il server resta in ascolto (scelta arbitraria >= 1024)
#define MAX_TESSERE 100       // Numero max per ogni tipo di tessera (tessera1, tessera2, tessera3)

// Mutex per gestire accesso concorrente ai contatori delle tessere (protezione in ambienti multithread)
pthread_mutex_t lock;

// Contatori per il numero di tessere attivate per ciascun tipo
int count_t1 = 0, count_t2 = 0, count_t3 = 0;



void *gestisci_client(void *arg);  // Gestione per ogni client in thread separato
int inizializza_socket_server();   // Crea, configura e avvia il socket di ascolto
void accetta_client(int server_socket); // Accetta nuova connessione client
void ricevi_stringa(int conn_client_socket, char *buffer, size_t size); // Riceve stringa (nome/cognome)
int ricevi_intero(int conn_client_socket); // Riceve intero (tipo abbonamento)
void invia_risposta(int conn_client_socket, int stato); // Invia stato (successo, errore)
void stampa_riepilogo(const char *nome, const char *cognome, int tipo, int numero, struct sockaddr_in *client_info); // Stampa info lato server
void gestisci_abbonamento(int tipo, int conn_client_socket, const char *nome, const char *cognome, struct sockaddr_in *client_info); // Logica di attivazione



int main() {
    int server_socket = inizializza_socket_server(); // Inizializzazione socket server TCP
    pthread_mutex_init(&lock, NULL); // Inizializzazione mutex per sezioni critiche

    printf("Server Cinema in ascolto...\n");
    while (1) {
        accetta_client(server_socket); //Loop per accettare e gestire nuovi client
    }

    close(server_socket);
    pthread_mutex_destroy(&lock);
    return 0;
}



//CREAZIONE E CONFIGURAZIONE SOCKET SERVER
int inizializza_socket_server() {
    int server_socket;
    struct sockaddr_in server_addr; // Struttura per indirizzo del server (IP e porta)

    // Creazione del socket: dominio IPv4 (AF_INET), per connesioni TCP (SOCK_STREAM)
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Errore creazione socket");
        exit(1);
    }

    // inizializzazione campi struct da passare a bind
    server_addr.sin_family = AF_INET;               // Protocollo IPv4
    server_addr.sin_port = htons(PORT);             // Converte la porta in formato big-endian (ordine di byte di rete)
    server_addr.sin_addr.s_addr = INADDR_ANY;       // costante che permette al server di ascoltare su tutte le interfacce di rete disponibili sulla macchina

    // Assegna l'indirizzo e porta al socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Errore nel binding");
        close(server_socket);
        exit(1);
    }

    // Imposta il socket in modalità ascolto (accetta massimo 5 client in coda)
    if (listen(server_socket, 5) < 0) {
        perror("Errore nella ricezione del Client");
        close(server_socket);
        exit(1);
    }

    return server_socket;
}


void accetta_client(int server_socket) {
    int conn_client_socket;
    struct sockaddr_in client_addr; // Struct contenente IP e porta del client
    socklen_t client_len = sizeof(client_addr);

    // Accept: accetta una connessione entrante, restituisce socket connesso al client
    conn_client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
    if (conn_client_socket < 0) {
        perror("Errore nella accept");
        return;
    }
    printf("\nNuova connessione stabilita\n");

    // Alloca memoria per socket da passare al thread
    int *client_sock = malloc(sizeof(int));
    *client_sock = conn_client_socket;

    // Creazione thread separato per client: gestione server concorrente (multithreading)
    pthread_t thread;
    if (pthread_create(&thread, NULL, gestisci_client, client_sock) != 0) {
        perror("Errore nella creazione del thread");
        free(client_sock);
        close(conn_client_socket);
        return;
    }
    pthread_detach(thread); // Il thread verrà eliminato automaticamente alla fine
}

//THREAD PER LA GESTIONE DEL CLIENT
void *gestisci_client(void *arg) {
    int conn_client_socket = *(int *)arg;
    free(arg);

    char nome[50], cognome[50]; // Buffer per i dati ricevuti
    int tipo;                   // Tipo di tessera richiesto
    struct sockaddr_in client_info; // Info client (IP e porta)
    socklen_t len = sizeof(client_info);

    // riempie la struct client_info recuperando le informazioni (IP e porta) del peer remoto (il client) associate a conn_client_socket
    getpeername(conn_client_socket, (struct sockaddr*)&client_info, &len);

    ricevi_stringa(conn_client_socket, nome, sizeof(nome));
    ricevi_stringa(conn_client_socket, cognome, sizeof(cognome));

    tipo = ricevi_intero(conn_client_socket);

    // Gestione disdetta da parte del client
    if (tipo == 4) {
        int tipo_da_disdire = ricevi_intero(conn_client_socket); // Riceve il tipo da disdire
        pthread_mutex_lock(&lock); // solo un thread per volta modifica i contatori
        int disd = 0;
        if (tipo_da_disdire==1 && count_t1 > 0) { count_t1--; disd = 1; }
        else if (tipo_da_disdire==2 && count_t2 > 0) { count_t2--; disd = 1; }
        else if (tipo_da_disdire==3 && count_t3 > 0) { count_t3--; disd = 1; }
        pthread_mutex_unlock(&lock);

        if (disd) {
            printf("\n--------------------------");
            printf("\nRichiesta di disdetta da %s:%d\n",
                   //inet_ntoa: Convertitore di indirizzo IP da formato binario a stringa leggibile
                   inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));
            printf("Nome: %s\nCognome: %s\nTipo tessera: %d\n",
                nome, cognome, tipo_da_disdire);
            printf("--------------------------\n");
            invia_risposta(conn_client_socket, 1); // 1 = disdetta OK
        } else {
            invia_risposta(conn_client_socket, -1); // -1 = disdetta fallita
        }

        close(conn_client_socket);
        pthread_exit(NULL);
    }

    gestisci_abbonamento(tipo, conn_client_socket, nome, cognome, &client_info);
    close(conn_client_socket);
    pthread_exit(NULL);
}


void ricevi_stringa(int conn_client_socket, char *buffer, size_t size) {
    // Riceve nome o cognome dal client
    if (recv(conn_client_socket, buffer, size, 0) < 0) {
        perror("Errore ricezione stringa");
        close(conn_client_socket);
        pthread_exit(NULL);
    }
}

int ricevi_intero(int conn_client_socket) {
    int val;
    if (recv(conn_client_socket, &val, sizeof(val), 0) < 0) {
        perror("Errore ricezione intero");
        close(conn_client_socket);
        pthread_exit(NULL);
    }
    return val;
}


void invia_risposta(int conn_client_socket, int stato) {
    // Invia un singolo intero come risposta al client (1 = successo, -1 = fallimento)
    send(conn_client_socket, &stato, sizeof(stato), 0);
}


// RIEPILOGO LATO SERVER
void stampa_riepilogo(const char *nome, const char *cognome, int tipo, int numero, struct sockaddr_in *client_info) {
    const char *descrizione;
    if (tipo == 1) descrizione = "Tessera1";
    else if (tipo == 2) descrizione = "Tessera2";
    else descrizione = "Tessera3";

    printf("\n--------------------------");
    printf("\nRichiesta di abbonamento da %s:%d\n",
           inet_ntoa(client_info->sin_addr), ntohs(client_info->sin_port));
    printf("Nome: %s\nCognome: %s\nTipo: %s (%d/%d)\n",
           nome, cognome, descrizione, numero, MAX_TESSERE);
    printf("--------------------------\n");
}


// LOGICA DI GESTIONE ABBONAMENTO
void gestisci_abbonamento(int tipo, int conn_client_socket, const char *nome, const char *cognome, struct sockaddr_in *client_info) {
    pthread_mutex_lock(&lock); // solo un thread per volta modifica i contatori
    int abb_accettato = 0;
    int numero = 0;

    // Verifica disponibilità e aggiorna il contatore
    if (tipo == 1 && count_t1 < MAX_TESSERE) {
        numero = ++count_t1;
        abb_accettato = 1;
    } else if (tipo == 2 && count_t2 < MAX_TESSERE) {
        numero = ++count_t2;
        abb_accettato = 1;
    } else if (tipo == 3 && count_t3 < MAX_TESSERE) {
        numero = ++count_t3;
        abb_accettato = 1;
    }

    pthread_mutex_unlock(&lock); // Fine sezione critica

    // Risposta e stampa in base all'esito
    if (abb_accettato) {
        invia_risposta(conn_client_socket, 1); // Successo
        stampa_riepilogo(nome, cognome, tipo, numero, client_info);
    } else {
        invia_risposta(conn_client_socket, -1); // Fallimento (posti esauriti)
        printf("\n--------------------------");
        printf("\nRichiesta di abbonamento rifiutata per %s : %d\n",
               inet_ntoa(client_info->sin_addr), ntohs(client_info->sin_port));
        printf("Nome: %s\nCognome: %s\nTipo Tessera: %d\n",
               nome, cognome, tipo);
        printf("--------------------------\n");
    }
}
