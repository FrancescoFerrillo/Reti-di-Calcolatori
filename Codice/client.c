
/* ====================================================================================================
Il progetto consiste nello sviluppo di un'applicazione Client-Server su Linux, con lo scopo
di simulare un sistema di gestione abbonamenti annuali per un cinema multisala. Il sistema permette
agli utenti (client) di effettuare la registrazione a una delle tre tipologie di tessera
disponibili oppure di richiederne la disdetta, mentre il server gestisce in modo concorrente le
richieste e tiene traccia delle disponibilità residue per ciascun tipo di tessera
(100 per ogni tipo: ingressi con festivi esclusi, festivi inclusi o tutti i giorni incluse le anteprime).
=======================================================================================================*/


// Librerie standard per input/output e gestione della memoria
#include <stdio.h>      //printf, perror, fgets
#include <stdlib.h>     // exit, EXIT_FAILURE
#include <string.h>     // strlen

// Librerie per socket e rete
#include <unistd.h>     // close
#include <arpa/inet.h>  //sockaddr_in, socket, connect, htons, inet_addr



#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"  //localhost



int inizializza_client_socket();                      // Crea e connette il socket al server
void invia_stringa(int client_socket, const char *str);
void invia_intero(int client_socket, int val);
void ricevi_risposta(int client_socket, int *stato);
int scegli_tipo_tessera();                           // Interazione utente per scelta tessera
void registrazione_utente(int client_socket);        // Gestisce flusso completo: input → invio → risposta



int main() {
    int client_socket = inizializza_client_socket();  // Creazione e connessione socket al server
    registrazione_utente(client_socket);                   // Invio dati e ricezione risposta
    close(client_socket);                             // Chiusura socket a fine operazione
    return 0;
}



int inizializza_client_socket() {
    int client_socket;
    struct sockaddr_in server_addr;

    //Creazione socket: dominio IPv4 (AF_INET), tipo TCP (SOCK_STREAM)
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Errore creazione client socket");
        exit(EXIT_FAILURE);
    }

    //Inizializzazione struttura per connettersi al server
    server_addr.sin_family = AF_INET;                   //Protocollo IPv4
    server_addr.sin_port = htons(SERVER_PORT);          //Porta del server (conversione in network byte order)
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP); //IP server

    //Connessione al server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Errore di connessione al server");
        close(client_socket); //chiude il socket
        exit(EXIT_FAILURE);  // termina il programma con errore
    }

    return client_socket;
}


void invia_stringa(int client_socket, const char *str) {
    send(client_socket, str, strlen(str) + 1, 0);
}

void invia_intero(int client_socket, int val) {
    send(client_socket, &val, sizeof(val), 0);
}


void ricevi_risposta(int client_socket, int *stato) {
    // Riceve intero come risposta dal server
    int n = recv(client_socket, stato, sizeof(*stato), 0);
    if (n < 0) {
        perror("Errore di ricezione della risposta");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
}


int scegli_tipo_tessera() {
    int scelta;

    do {
        printf("\nScegli il tipo di tessera:\n");
        printf("1 - Tessera 1 (Festivi esclusi, €150)\n");
        printf("2 - Tessera 2 (Festivi inclusi, €200)\n");
        printf("3 - Tessera 3 (Tutti i giorni, €250)\n");
        printf("4 - Disdici abbonamento e richiedi rimborso\n");
        printf("Inserisci scelta (1-4): ");
        scanf("%d", &scelta);
        if (scelta < 1 || scelta > 4)
            printf("Scelta non valida. Riprova.\n");
    } while (scelta < 1 || scelta > 4);
    return scelta;
}


void registrazione_utente(int client_socket) {
    char nome[50], cognome[50];
    int scelta, stato;

    printf("\nInserisci il tuo nome: ");
    fgets(nome, sizeof(nome), stdin);
    nome[strcspn(nome, "\n")] = '\0';
    invia_stringa(client_socket, nome);

    printf("Inserisci il tuo cognome: ");
    fgets(cognome, sizeof(cognome), stdin);
    cognome[strcspn(cognome, "\n")] = '\0';
    invia_stringa(client_socket, cognome);

    // Scelta tipo tessera o disdetta
    scelta = scegli_tipo_tessera();
    invia_intero(client_socket, scelta);

    if (scelta == 4) {
        printf("Inserisci il tipo di tessera da disdire (1-3): ");
        int tipo_disdetta;
        scanf("%d", &tipo_disdetta);
        invia_intero(client_socket, tipo_disdetta);

        //Ricezione risposta dopo aver inviato il tipo da disdire
        ricevi_risposta(client_socket, &stato);

        if (stato == 1) {
            printf("\nDisdetta effettuata con successo! Ora puoi richiedere il rimborso.\n");
        } else {
            printf("\nErrore nel processo di disdetta, non risultano abbonamenti attivi.\n");
        }
    } else {
        ricevi_risposta(client_socket, &stato);

        if (stato == 1) {
            printf("\nTessera di tipo %d registrata con successo!\n", scelta);
        } else if (stato == -1) {
            printf("\nPosti esauriti per la tessera selezionata.\n");
        } else {
            printf("\nErrore sconosciuto nel processo di registrazione.\n");
        }
    }


}
