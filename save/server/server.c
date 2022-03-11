#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>



#define PORT 6666
#define CLIENT_MAX 32

typedef enum estLibre{OUI,NON}estLibre_t;

typedef struct sockaddr_in_s{
    short               sin_family;
    unsigned short      sin_port;
    struct in_addr      sin_addr;
    char                sin_zero[8];
}sockaddr_in_t;

typedef struct client_s{
  int num;
  estLibre_t libre;
  int numSock;
  sockaddr_in_t sin; //Socket address IN
  socklen_t taille;
}client_t;

typedef struct lien_s{
  client_t client1;
  client_t client2;
}lien_t;

//HORLOGE
struct tm* horloge;

//DEFINITION DU CLIENT
client_t listeClient[CLIENT_MAX];
int nb_client_attente=0;

//DEFINITION DES THREADS SAH QUEL PLAISIR
pthread_t th_attente[CLIENT_MAX];
pthread_t th_connectes[CLIENT_MAX];



struct tm* recupererTemps(){
  time_t temp;

  time(&temp);
  return localtime(&temp);
}

void tableauPropre(){
  int i;
  for(i=0;i<CLIENT_MAX;i++)
    listeClient[i].num=-1;
    listeClient[i].libre=NON;
    listeClient[i].numSock=-1;
}

void* connectes(void* oldJoueurs){
  lien_t* joueurs = (lien_t*) oldJoueurs; //On triche, le  thread forcant un param void*
  client_t joueur1 = joueurs->client1;
  client_t joueur2 = joueurs->client2;
  char temp[10];
  char texteJoueur1[50] = "Tu viens d'etre connecté avec le joueur: "; sprintf(temp, "%i", joueur2.numSock); strcat(texteJoueur1,temp);
  char texteJoueur2[50] = "Tu viens d'etre connecté avec le joueur: "; sprintf(temp, "%i", joueur1.numSock); strcat(texteJoueur2,temp);
  printf("j'envoie à [%i]:\n            la chaine: %s\n", joueur1.numSock, texteJoueur1);
  send(joueur1.numSock, texteJoueur1, 64, 0);
  printf("j'envoie à [%i]:\n            la chaine: %s\n", joueur2.numSock, texteJoueur2);
  send(joueur2.numSock, texteJoueur2, 64, 0);
  shutdown(joueur1.numSock, 2);
  shutdown(joueur2.numSock, 2);
  free(joueurs);
}

void* attente(void* informations){
  client_t* client = (client_t*) informations; //On triche, le  thread forcant un param void*
  void* pasContent;
  ssize_t verif = -1;
  char* buffer = malloc(sizeof(char)*32+1);
  while(verif!=0){
    printf("Client[%i]: j'attends des informations...\n",client->num);
    verif = read(client->numSock,buffer,32);
    if(verif==0){
      printf("Client[%i]: s'est deconnecté (pas cool)\n",client->num);
      client->num=-1;
      pthread_exit(pasContent);
      return NULL;
    }
    else{
      printf("Client[%i]: envoie: chaine[%s]\n",client->num,buffer);
      if(client->num==1){
        send(listeClient[0].numSock, buffer, 32, 0);
      }
      else{
        send(listeClient[1].numSock, buffer, 32, 0);
      }
    }
  }
}

int main(){
  //DEFINITION DU SERVEUR
  sockaddr_in_t server_Sin;                               //Socket address IN
  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);     //serverSocket
  socklen_t s_Taille = sizeof(server_Sin);


  struct tm* date = recupererTemps(); //stucture date, contenant le temps actuel de l'ordinateur

  if(serverSocket==-1){ //On vérifie la création socket de notre serveur
    printf("Sortie à cause d'un bug de création Socket\n");
    return 1;
  }
  printf("La socket %d est maintenant ouverte en mode TCP/IP\n", serverSocket);

  //Configuration du serveur
  server_Sin.sin_addr.s_addr = htonl(INADDR_ANY);  //htonl donne une ip automatique
  server_Sin.sin_family = AF_INET;                 //Protocole ici (IP)
  server_Sin.sin_port = htons(PORT);               //Port


  //DECLARATION *test afin de se souvenir de la valeur des tests
  int* test = malloc(sizeof(int));

  *test = bind(serverSocket, (struct sockaddr *)&server_Sin, sizeof(server_Sin));
  if(*test==SO_ERROR){
    printf("Sortie à cause d'un bug de bind Socket\n");
    return 1;
  }



  *test= listen(serverSocket, 50);
  if(*test==SO_ERROR){
    printf("Sortie à cause d'un bug de listen Socket\n");
    return 1;
  }
  free(test);
  printf("Utilisation et écoute du port %d...\n\n", PORT);



  //horloge=localtime((time_t*)time(NULL));
  int i,j;
  int connect = -1;
  tableauPropre();
  while(1){
    date=recupererTemps();
    printf("TEMPS: %i:%i:%i\n", date->tm_hour,date->tm_min,date->tm_sec);

    if(nb_client_attente!=0 && nb_client_attente%2==0){ //deux clients dispos pour jouer
      for(j=0;j<CLIENT_MAX;j++){
        if(listeClient[j].libre == OUI){
          listeClient[i].libre=NON;
          listeClient[j].libre=NON;
          lien_t* nouvelleConnexion = malloc(sizeof(lien_t));
          nouvelleConnexion->client1=listeClient[i];
          nouvelleConnexion->client2=listeClient[j];
          pthread_create(&th_attente[i], NULL, connectes, (void*)nouvelleConnexion);
          nb_client_attente-=2;
          break;
        }

      }
      if(j>=CLIENT_MAX) printf("Désolé, j'ai fait nimp\n");
    }


    printf("On attente de la connexion d'un client ...\n");

    for(i=0;i<CLIENT_MAX;i++){
      if(listeClient[i].num==-1){ //Si libre
        listeClient[i].numSock = accept(serverSocket, (struct sockaddr*)&listeClient[i].sin, &listeClient[i].taille);
        listeClient[i].num=i;
        break; //On break afin de garder l'indice de i;
      }
    }
    if(i<CLIENT_MAX){ //On a trouvé une place
      printf("Client[%i]: Nouvelle connexion\n",i);
      //pthread_create(&th_attente[i], NULL, attente, (void*)&listeClient[i]);
      listeClient[i].libre=OUI;
      nb_client_attente++;
    }
    else{
    printf("Utilisateur à essayer de se connecter, mais on a plus de place\n\n");
    shutdown(connect, 2);
    }

  }
  free(date);
}