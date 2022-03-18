#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define PORT 6666
#define NB_TENTATIVE 60
#define DELAI 1


typedef struct sockaddr_in_s{
    short               sin_family;
    unsigned short      sin_port;
    struct in_addr      sin_addr;
    char                sin_zero[8];
}sockaddr_in_t;

typedef struct serverStruct_s{
  int valSocket; /** \brief valSocket est un entier permettant de verifier et garder en memoire le socket du client */
  int joueurTrouve; /** \brief joueurTrouve est un entier permettant de verifier si on a trouve un joueur */
}serverStruct_t;

void* rechercheJoueur(void* infos);
int connectC();
