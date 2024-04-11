#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/times.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "fragmenta.h"

#define N_DESCRIPTOR 10 // Habrá 2 jugadores y lo demas serán observadores
#define LBUFFER 2000
#define BACKLOG 5
#define TAMANO_FILA 10
#define TAMANO_COLUMNA 10

//El estado del servidor
int FLOTAS ;
int descriptores[N_DESCRIPTOR];
int numclientes;
FILE *fd_sock;
FILE *f_sock[N_DESCRIPTOR];
int contJugador1, contJugador2;
int matrizJugador1[TAMANO_FILA][TAMANO_COLUMNA];
int matrizJugador2[TAMANO_FILA][TAMANO_COLUMNA];

char nombreJugador1[20];
char nombreJugador2[20];

void aceptar_nuevo_cliente(int sock);

void crearID(char id2[]);

char enteroACaracter(int numero);
    
bool comprobarId(char id[]);

int main (int argc, char* argv[]){ // servidor puerto 
    srand(time(0));
    int sock;
    int puerto, i;
    struct sockaddr_in servidor;
    char buf[LBUFFER];
    char **fragmentacion;
    FILE *usuarios;
    char id[12];
    char **resp;
    int a, c;
    FLOTAS = rand()%5 + 4;
     //maximo 10 flotas
    int flotas[FLOTAS] ;
    for (i = 0; i < FLOTAS; i++){
        flotas[i] = rand()%4 + 2;
    }
    
    
    for (i = 0; i<= N_DESCRIPTOR; i++)descriptores[i] = -1; //Inicializamos array de ficheros
    
    if (argc <= 1)puerto = 1234;
    else puerto = atoi(argv[1]);
    
    
    //Inicializamos socket
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(puerto);
    servidor.sin_addr.s_addr = INADDR_ANY;
    bzero(&(servidor.sin_zero), 8);
    
    //sock = socket (PF_INET, SOCK_STREAM, 0);
    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error Socket()");
        exit(EXIT_FAILURE);
    }
    
    if (bind (sock, (struct sockaddr *)&servidor, sizeof(servidor)) == -1){
        printf("Error: no puedo coger el puerto \n");
        exit(-1);
    }
    
    if (listen(sock, BACKLOG) == -1){
        printf("Error en listen() \n");
        exit(EXIT_FAILURE);
    }
    
    numclientes = 0;
    
    
    while (1){
        fd_set paraleer; // Conjunto de descriptores
        int fd_maximo;
        FD_ZERO(&paraleer);
        FD_SET(sock, &paraleer);
        fd_maximo = sock;
        
        for (i = 0; i < N_DESCRIPTOR; i++){
            if (descriptores[i] != -1){ //Si hay algo escrito en el descriptor entonces ->
                FD_SET(descriptores[i], &paraleer); //añadimos al conjunto de descriptores
                fd_maximo = fd_maximo>descriptores[i]?fd_maximo:descriptores[i]; // Cosas raras
            }
        }
        fd_maximo++;
        if (select (fd_maximo, &paraleer, NULL, NULL, NULL)){ //Espera a que alguno de los descriptores tenga algo para escribir
            if (FD_ISSET(sock, &paraleer)){ //si entra un cliente se escribe en el sock, por lo que así sabemos si ha entrado un cliente nuevo
                aceptar_nuevo_cliente(sock);
            }
        }
            for (i = 0;i < N_DESCRIPTOR; i++){
                if ((descriptores[i] != -1) && (FD_ISSET(descriptores[i], &paraleer))){
                        memset(buf, '\0', LBUFFER);
                        
                        if (fgets(buf, LBUFFER, f_sock[i]) != NULL){
                            printf("%s", buf);
                            fragmentacion = fragmentar(buf);
                            
                            if (strcmp(strtok(fragmentacion[1], "\n"), "REGISTRAR") == 0){
                                id[12];
                                **resp;
                                printf("Relizando peticion de registro. \n");
                                memset(buf, '\0', LBUFFER);
                                a = rand()%11 + 30;
                                c = rand()%11 + 10;
                                printf("Estableciendo prueba %d + %d.\n", a, c);
                                memset(buf, '\0', LBUFFER);
                                sprintf(buf, "S RESUELVE %d %d = \n", a, c);

                                fprintf(f_sock[i], "%s", buf);
                                
                                memset(buf, '\0', LBUFFER);
                            }
                            else if(strcmp(strtok(fragmentacion[1], "\n"), "RESPUESTA") == 0){
                                resp = fragmentar(buf);
                                resp[2] = strtok(resp[2], "\n");
                
                                if (atoi(resp[2]) == (a + c)){
                                    printf("Respuesta %s.Prueba superada\n", strtok(resp[2], "\n"));
                                    crearID(id);

                                    printf("Asignando id %s .\n", id);
                                    fprintf (f_sock[i], "S REGISTRADO OK %s\n", id);

                                    sprintf(buf, "%s %d Invitado\n",  strtok(id, "\n"), (a + c));
                                    usuarios = fopen("servidor.txt", "a");
                                    fputs(buf, usuarios);
                                    fclose(usuarios);
                                }
                                else{
                                    fprintf(f_sock[i], "S REGISTRADO ERROR \n");
                                }
                                free(resp);
                                sleep(5);
                            }
                            else if (strcmp(strtok(fragmentacion[1], "\n"), "LOGIN") == 0){
                                printf("Sesion inciada con id: %s \n", fragmentacion[2]);
                                memset(buf, '\0', LBUFFER);
                                
                                usuarios = fopen("servidor.txt", "r");
                                bool b = false;
                                
                                char **usuario;
                                while ((fgets(buf, LBUFFER, usuarios) != NULL) && !(b)){
                                    usuario = fragmentar(buf);
                                    strcat(buf, "\0");
                                    if ((strcmp(strtok(fragmentacion[2], "\n"), strtok(usuario[0], "\n")) == 0)&&(strcmp(strtok(fragmentacion[3], "\n"), strtok(usuario[1], "\n")) == 0)){
                                        b = true;
                                    }
                                }
                                if (b){
                                    printf("Usuario con id %s aceptado. \n", strtok(fragmentacion[2], "\n"));
                                    sleep(1);
                                    fprintf(f_sock[i], "S LOGIN OK .\n");
                                }
                                else{
                                    printf("Usuario con id %s rechazado. \n", strtok(fragmentacion[2], "\n"));
                                    fprintf(f_sock[i], "S LOGIN ERROR .\n");
                                }
                            }
                            else if (strcmp(strtok(fragmentacion[1], "\n"), "NOMBRE") == 0){
                                if (comprobarId(fragmentacion[2])){
                                    printf("Jugador %d %s\n", (i + 1), fragmentacion[2]);  
                                    if (i == 1 || i == 0){
                                        fprintf(f_sock[i], "S NOMBRE OK\n");
                                        if (i == 0){
                                            strcpy(nombreJugador1, fragmentacion[2]);
                                            printf("Nombre: %s\n", nombreJugador1);
                                            fprintf(f_sock[i], "S BARCO %d\n", flotas[contJugador1]);
                                        }
                                        else if (i == 1){
                                            strcpy(nombreJugador2, fragmentacion[2]);
                                            printf("Nombre: %s\n", nombreJugador2);
                                            fprintf(f_sock[i], "S BARCO %d\n", flotas[contJugador2]);
                                        }
                                    }
                                    else{
                                        fprintf(f_sock[i], "S OBSERVADOR\n");
                                    }
                                }
                                else{
                                    fprintf(f_sock[i], "S NOMBRE ERROR\n");
                                }
                            }
                            else if (strcmp(strtok(fragmentacion[1], "\n"), "BARCO") == 0){
                                int columna = atoi(fragmentacion[2]);
                                int fila = atoi(fragmentacion[3]);
                                bool b = true;
                                bool tocandoFlota;
                                if (columna > (TAMANO_COLUMNA-1)){
                                    fprintf(f_sock[i], "S BARCO ERROR\n");
                                    b = false;
                                }
                                else if (columna < 0){
                                    fprintf(f_sock[i], "S BARCO ERROR\n");
                                    b = false;
                                }
                                else if (fila > (TAMANO_FILA - 1)){
                                    fprintf(f_sock[i], "S BARCO ERROR\n");
                                    b = false;
                                }
                                else if (fila < 0){
                                    fprintf(f_sock[i], "S BARCO ERROR\n");
                                    b = false;
                                }
                                
                                if (strcmp("v", strtok(fragmentacion[4], "\n")) == 0){
                                    if (i == 0){
                                        if ((flotas[contJugador1 - 1] + fila) > (TAMANO_FILA)){
                                            fprintf(f_sock[i], "S BARCO ERROR\n");
                                            b = false;
                                        }
                                        else{
                                            for (int j = fila;j < (flotas[contJugador1 - 1] + fila); j++){
                                                if (j == fila){
                                                    if (j != 0 && columna != 0 && j!= (TAMANO_FILA-1) && columna!= (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador1[j+1][columna] != 1)&&(matrizJugador1[j][columna+1] != 1)&&(matrizJugador1[j+1][columna+1] != 1)&&(matrizJugador1[j-1][columna-1] != 1)&&(matrizJugador1[j-1][columna] != 1)&&(matrizJugador1[j][columna-1] != 1)&&(matrizJugador1[j+1][columna-1] != 1)&&(matrizJugador1[j-1][columna+1] != 1));
                                                    }
                                                    else if (j == 0 && columna == 0){
                                                        tocandoFlota = ((matrizJugador1[j+1][columna+1] != 1)&&(matrizJugador1[j][columna+1] != 1)&&(matrizJugador1[j+1][columna] != 1));
                                                    }
                                                    else if (j == 0 && columna == (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador1[j][columna-1] != 1)&&(matrizJugador1[j+1][columna-1] != 1)&&(matrizJugador1[j+1][columna] != 1));
                                                    }
                                                    else if (j== (TAMANO_FILA-1) && columna== 0){
                                                        tocandoFlota = ((matrizJugador1[j-1][columna+1] != 1)&&(matrizJugador1[j-1][columna] != 1)&&(matrizJugador1[j][columna+1] != 1));
                                                    }
                                                    else if (j== (TAMANO_FILA-1) && columna == (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador1[j-1][columna-1] != 1)&&(matrizJugador1[j][columna-1] != 1)&&(matrizJugador1[j-1][columna] != 1));
                                                    }
                                                    else if (j == 0){
                                                        tocandoFlota = ((matrizJugador1[j+1][columna] != 1)&&(matrizJugador1[j][columna+1] != 1)&&(matrizJugador1[j+1][columna+1] != 1)&&(matrizJugador1[j][columna-1] != 1)&&(matrizJugador1[j+1][columna-1] != 1));
                                                    }
                                                    else if (columna == 0){
                                                        tocandoFlota = ((matrizJugador1[j+1][columna] != 1)&&(matrizJugador1[j][columna+1] != 1)&&(matrizJugador1[j+1][columna+1] != 1)&&(matrizJugador1[j-1][columna] != 1)&&(matrizJugador1[j-1][columna+1] != 1));
                                                    }
                                                    
                                                    else if (j== (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador1[j][columna+1] != 1)&&(matrizJugador1[j-1][columna-1] != 1)&&(matrizJugador1[j-1][columna] != 1)&&(matrizJugador1[j][columna-1] != 1)&&(matrizJugador1[j-1][columna+1] != 1));
                                                    }
                                                    else if (columna== (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador1[j+1][columna] != 1)&&(matrizJugador1[j-1][columna-1] != 1)&&(matrizJugador1[j-1][columna] != 1)&&(matrizJugador1[j][columna-1] != 1)&&(matrizJugador1[j+1][columna-1] != 1));
                                                    }
                                                }
                                                else{
                                                    if (j != 0 && columna != 0 && j!= (TAMANO_FILA-1) && columna!= (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador1[j+1][columna] != 1)&&(matrizJugador1[j][columna+1] != 1)&&(matrizJugador1[j+1][columna+1] != 1)&&(matrizJugador1[j-1][columna-1] != 1)&&(matrizJugador1[j][columna-1] != 1)&&(matrizJugador1[j+1][columna-1] != 1)&&(matrizJugador1[j-1][columna+1] != 1));
                                                    }
                                                    else if (j == 0 && columna == 0){
                                                        tocandoFlota = ((matrizJugador1[j+1][columna+1] != 1)&&(matrizJugador1[j][columna+1] != 1)&&(matrizJugador1[j+1][columna] != 1));
                                                    }
                                                    else if (j == 0 && columna == (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador1[j][columna-1] != 1)&&(matrizJugador1[j+1][columna-1] != 1)&&(matrizJugador1[j+1][columna] != 1));
                                                    }
                                                    else if (j== (TAMANO_FILA-1) && columna== 0){
                                                        tocandoFlota = ((matrizJugador1[j-1][columna+1] != 1)&&(matrizJugador1[j][columna+1] != 1));
                                                    }
                                                    else if (j== (TAMANO_FILA-1) && columna == (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador1[j-1][columna-1] != 1)&&(matrizJugador1[j][columna-1] != 1));
                                                    }
                                                    else if (j == 0 ){
                                                        tocandoFlota = ((matrizJugador1[j+1][columna] != 1)&&(matrizJugador1[j][columna+1] != 1)&&(matrizJugador1[j+1][columna+1] != 1)&&(matrizJugador1[j][columna-1] != 1)&&(matrizJugador1[j+1][columna-1] != 1));
                                                    }
                                                    else if ( columna == 0){
                                                        tocandoFlota = ((matrizJugador1[j+1][columna] != 1)&&(matrizJugador1[j][columna+1] != 1)&&(matrizJugador1[j+1][columna+1] != 1)&&(matrizJugador1[j-1][columna+1] != 1));
                                                    }
                                                    else if (j== (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador1[j][columna+1] != 1)&&(matrizJugador1[j-1][columna-1] != 1)&&(matrizJugador1[j][columna-1] != 1)&&(matrizJugador1[j-1][columna+1] != 1));
                                                    }
                                                    else if (columna== (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador1[j+1][columna] != 1)&&(matrizJugador1[j-1][columna-1] != 1)&&(matrizJugador1[j][columna-1] != 1)&&(matrizJugador1[j+1][columna-1] != 1));
                                                    }
                                                }
                                                if ((matrizJugador1[j][columna] == 0) && (tocandoFlota)){
                                                    matrizJugador1[j][columna] = 1;
                                                }
                                                else if (matrizJugador1[j][columna] != 0){
                                                    fprintf(f_sock[i], "S BARCO ERROR\n");
                                                    b = false;
                                                    for (int k = fila;k < j; k++){
                                                        matrizJugador1[k][columna] = 0;
                                                    }
                                                    break;
                                                }
                                                else if (!tocandoFlota){
                                                    fprintf(f_sock[i], "S BARCO ERROR\n");
                                                    b = false;
                                                    for (int k = fila;k < j; k++){
                                                        matrizJugador1[k][columna] = 0;
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    else if (i == 1){
                                        if ((flotas[contJugador2 - 1] + fila) > (TAMANO_FILA)){
                                            fprintf(f_sock[i], "S BARCO ERROR\n");
                                            b = false;
                                        }
                                        else{
                                            for (int j = fila;j < (flotas[contJugador2 - 1] + fila); j++){
                                                if (j == fila){
                                                    if (j != 0 && columna != 0 && j!= (TAMANO_FILA-1) && columna!= (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador2[j+1][columna] != 1)&&(matrizJugador2[j][columna+1] != 1)&&(matrizJugador2[j+1][columna+1] != 1)&&(matrizJugador2[j-1][columna-1] != 1)&&(matrizJugador2[j-1][columna] != 1)&&(matrizJugador2[j][columna-1] != 1)&&(matrizJugador2[j+1][columna-1] != 1)&&(matrizJugador2[j-1][columna+1] != 1));
                                                    }
                                                    else if (j == 0 && columna == 0){
                                                        tocandoFlota = ((matrizJugador2[j+1][columna+1] != 1)&&(matrizJugador2[j][columna+1] != 1)&&(matrizJugador2[j+1][columna] != 1));
                                                    }
                                                    else if (j == 0 && columna == (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador2[j][columna-1] != 1)&&(matrizJugador2[j+1][columna-1] != 1)&&(matrizJugador2[j+1][columna] != 1));
                                                    }
                                                    else if (j== (TAMANO_FILA-1) && columna== 0){
                                                        tocandoFlota = ((matrizJugador2[j-1][columna+1] != 1)&&(matrizJugador2[j-1][columna] != 1)&&(matrizJugador2[j][columna+1] != 1));
                                                    }
                                                    else if (j== (TAMANO_FILA-1) && columna == (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador2[j-1][columna-1] != 1)&&(matrizJugador2[j][columna-1] != 1)&&(matrizJugador2[j-1][columna] != 1));
                                                    }
                                                    else if (j == 0 ){
                                                        tocandoFlota = ((matrizJugador2[j+1][columna] != 1)&&(matrizJugador2[j][columna+1] != 1)&&(matrizJugador2[j+1][columna+1] != 1)&&(matrizJugador2[j][columna-1] != 1)&&(matrizJugador2[j+1][columna-1] != 1));
                                                    }
                                                    else if (columna == 0){
                                                        tocandoFlota = ((matrizJugador2[j+1][columna] != 1)&&(matrizJugador2[j][columna+1] != 1)&&(matrizJugador2[j+1][columna+1] != 1)&&(matrizJugador2[j-1][columna] != 1)&&(matrizJugador2[j-1][columna+1] != 1));
                                                    }
                                                    
                                                    else if (j== (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador2[j][columna+1] != 1)&&(matrizJugador2[j-1][columna-1] != 1)&&(matrizJugador2[j-1][columna] != 1)&&(matrizJugador2[j][columna-1] != 1)&&(matrizJugador2[j-1][columna+1] != 1));
                                                    }
                                                    else if (columna== (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador2[j+1][columna] != 1)&&(matrizJugador2[j-1][columna-1] != 1)&&(matrizJugador2[j-1][columna] != 1)&&(matrizJugador2[j][columna-1] != 1)&&(matrizJugador2[j+1][columna-1] != 1));
                                                    }
                                                }
                                                else{
                                                    if (j != 0 && columna != 0 && j!= (TAMANO_FILA-1) && columna!= (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador2[j+1][columna] != 1)&&(matrizJugador2[j][columna+1] != 1)&&(matrizJugador2[j+1][columna+1] != 1)&&(matrizJugador2[j-1][columna-1] != 1)&&(matrizJugador2[j][columna-1] != 1)&&(matrizJugador2[j+1][columna-1] != 1)&&(matrizJugador2[j-1][columna+1] != 1));
                                                    }
                                                    else if (j == 0 && columna == 0){
                                                        tocandoFlota = ((matrizJugador2[j+1][columna+1] != 1)&&(matrizJugador2[j][columna+1] != 1)&&(matrizJugador2[j+1][columna] != 1));
                                                    }
                                                    else if (j == 0 && columna == (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador2[j][columna-1] != 1)&&(matrizJugador2[j+1][columna-1] != 1)&&(matrizJugador2[j+1][columna] != 1));
                                                    }
                                                    else if (j== (TAMANO_FILA-1) && columna== 0){
                                                        tocandoFlota = ((matrizJugador2[j-1][columna+1] != 1)&&(matrizJugador2[j][columna+1] != 1));
                                                    }
                                                    else if (j== (TAMANO_FILA-1) && columna == (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador2[j-1][columna-1] != 1)&&(matrizJugador2[j][columna-1] != 1));
                                                    }
                                                    else if (j == 0){
                                                        tocandoFlota = ((matrizJugador2[j+1][columna] != 1)&&(matrizJugador2[j][columna+1] != 1)&&(matrizJugador2[j+1][columna+1] != 1)&&(matrizJugador2[j][columna-1] != 1)&&(matrizJugador2[j+1][columna-1] != 1));
                                                    }
                                                    else if (columna == 0){
                                                        tocandoFlota = ((matrizJugador2[j+1][columna] != 1)&&(matrizJugador2[j][columna+1] != 1)&&(matrizJugador2[j+1][columna+1] != 1)&&(matrizJugador2[j-1][columna+1] != 1));
                                                    }
                                                    else if (j== (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador2[j][columna+1] != 1)&&(matrizJugador2[j-1][columna-1] != 1)&&(matrizJugador2[j][columna-1] != 1)&&(matrizJugador2[j-1][columna+1] != 1));
                                                    }
                                                    else if (columna== (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador2[j+1][columna] != 1)&&(matrizJugador2[j-1][columna-1] != 1)&&(matrizJugador2[j][columna-1] != 1)&&(matrizJugador2[j+1][columna-1] != 1));
                                                    }
                                                }
                                                if ((matrizJugador2[j][columna] == 0) && (tocandoFlota)){
                                                    matrizJugador2[j][columna] = 1;
                                                }
                                                else if (matrizJugador2[j][columna] != 0){
                                                    fprintf(f_sock[i], "S BARCO ERROR\n");
                                                    b = false;
                                                    for (int k = fila;k < j; k++){
                                                        matrizJugador2[k][columna] = 0;
                                                    }
                                                    break;
                                                }
                                                else if (!tocandoFlota){
                                                    fprintf(f_sock[i], "S BARCO ERROR\n");
                                                    b = false;
                                                    for (int k = fila;k < j; k++){
                                                        matrizJugador2[k][columna] = 0;
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                else if (strcmp("h", strtok(fragmentacion[4], "\n")) == 0){
                                    if (i == 0){
                                        if ((flotas[contJugador1 - 1] + columna) > (TAMANO_COLUMNA)){
                                            fprintf(f_sock[i], "S BARCO ERROR\n");
                                            b = false;
                                        }
                                        else{
                                            for (int j = columna;j < (flotas[contJugador1 - 1] + columna); j++){
                                                if (columna == j){
                                                    if (j != 0 && fila != 0 && j!= (TAMANO_COLUMNA-1) && fila!= (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador1[fila+1][j] != 1)&&(matrizJugador1[fila][j+1] != 1)&&(matrizJugador1[fila+1][j+1] != 1)&&(matrizJugador1[fila-1][j-1] != 1)&&(matrizJugador1[fila-1][j] != 1)&&(matrizJugador1[fila][j-1] != 1)&&(matrizJugador1[fila+1][j-1] != 1)&&(matrizJugador1[fila-1][j+1] != 1));
                                                    }
                                                    else if (j == (TAMANO_COLUMNA-1) && fila == 0){
                                                        tocandoFlota = ((matrizJugador1[fila][j-1] != 1)&&(matrizJugador1[fila+1][j-1] != 1)&&(matrizJugador1[fila+1][j] != 1));
                                                    }
                                                    else if (j== 0 && fila == (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador1[fila-1][j+1] != 1)&&(matrizJugador1[fila-1][j] != 1)&&(matrizJugador1[fila][j+1] != 1));
                                                    }
                                                    else if (j == 0 && fila == 0){
                                                        tocandoFlota = ((matrizJugador1[fila+1][j+1] != 1)&&(matrizJugador1[fila][j+1] != 1)&&(matrizJugador1[fila+1][j] != 1));
                                                    }
                                                    else if (j== (TAMANO_COLUMNA-1) && fila == (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador1[fila-1][j-1] != 1)&&(matrizJugador1[fila][j-1] != 1)&&(matrizJugador1[fila-1][j] != 1));
                                                    }
                                                    else if (fila == 0 ){
                                                        tocandoFlota = ((matrizJugador1[fila+1][j] != 1)&&(matrizJugador1[fila][j+1] != 1)&&(matrizJugador1[fila+1][j+1] != 1)&&(matrizJugador1[fila][j-1] != 1)&&(matrizJugador1[fila+1][j-1] != 1));
                                                    }
                                                    else if (j == 0){
                                                        tocandoFlota = ((matrizJugador1[fila+1][j] != 1)&&(matrizJugador1[fila][j+1] != 1)&&(matrizJugador1[fila+1][j+1] != 1)&&(matrizJugador1[fila-1][j] != 1)&&(matrizJugador1[fila-1][j+1] != 1));
                                                    }
                                                    
                                                    else if (fila== (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador1[fila][j+1] != 1)&&(matrizJugador1[fila-1][j-1] != 1)&&(matrizJugador1[fila-1][j] != 1)&&(matrizJugador1[fila][j-1] != 1)&&(matrizJugador1[fila-1][j+1] != 1));
                                                    }
                                                    else if (j== (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador1[fila+1][j] != 1)&&(matrizJugador1[fila-1][j-1] != 1)&&(matrizJugador1[fila-1][j] != 1)&&(matrizJugador1[fila][j-1] != 1)&&(matrizJugador1[fila+1][j-1] != 1));
                                                    }
                                                }
                                                else{
                                                    if (j != 0 && fila != 0 && j!= (TAMANO_COLUMNA-1) && fila!= (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador1[fila+1][j] != 1)&&(matrizJugador1[fila][j+1] != 1)&&(matrizJugador1[fila+1][j+1] != 1)&&(matrizJugador1[fila-1][j-1] != 1)&&(matrizJugador1[fila-1][j] != 1)&&(matrizJugador1[fila+1][j-1] != 1)&&(matrizJugador1[fila-1][j+1] != 1));
                                                    }
                                                    else if (j == (TAMANO_COLUMNA-1) && fila == 0){
                                                        tocandoFlota = ((matrizJugador1[fila+1][j-1] != 1)&&(matrizJugador1[fila+1][j] != 1));
                                                    }
                                                    else if (j== 0 && fila== (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador1[fila-1][j+1] != 1)&&(matrizJugador1[fila-1][j] != 1)&&(matrizJugador1[fila][j+1] != 1));
                                                    }
                                                    else if (j == 0 && fila == 0){
                                                        tocandoFlota = ((matrizJugador1[fila+1][j+1] != 1)&&(matrizJugador1[fila][j+1] != 1)&&(matrizJugador1[fila+1][j] != 1));
                                                    }
                                                    else if (j== (TAMANO_COLUMNA-1) && fila == (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador1[fila-1][j-1] != 1)&&(matrizJugador1[fila-1][j] != 1));
                                                    }
                                                    else if (j != 0 && fila == 0 ){
                                                        tocandoFlota = ((matrizJugador1[fila+1][j] != 1)&&(matrizJugador1[fila][j+1] != 1)&&(matrizJugador1[fila+1][j+1] != 1)&&(matrizJugador1[fila+1][j-1] != 1));
                                                    }
                                                    else if (j == 0){
                                                        tocandoFlota = ((matrizJugador1[fila+1][j] != 1)&&(matrizJugador1[fila][j+1] != 1)&&(matrizJugador1[fila+1][j+1] != 1)&&(matrizJugador1[fila-1][j] != 1)&&(matrizJugador1[fila-1][j+1] != 1));
                                                    }
                                                    
                                                    else if (fila == (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador1[fila][j+1] != 1)&&(matrizJugador1[fila-1][j-1] != 1)&&(matrizJugador1[fila-1][j] != 1)&&(matrizJugador1[fila-1][j+1] != 1));
                                                    }
                                                    else if (j == (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador1[fila+1][j] != 1)&&(matrizJugador1[fila-1][j-1] != 1)&&(matrizJugador1[fila-1][j] != 1)&&(matrizJugador1[fila+1][j-1] != 1));
                                                    }
                                                    
                                                }
                                                if ((matrizJugador1[fila][j] == 0) && (tocandoFlota)){
                                                    matrizJugador1[fila][j] = 1;
                                                }
                                                else if ((matrizJugador1[fila][j] != 0)){
                                                    fprintf(f_sock[i], "S BARCO ERROR\n");
                                                    b = false;
                                                    for (int k = columna;k < j; k++){
                                                        matrizJugador1[fila][k] = 0;
                                                    }
                                                    break;
                                                }
                                                else if (!tocandoFlota){
                                                    fprintf(f_sock[i], "S BARCO ERROR\n");
                                                    b = false;
                                                    for (int k = columna;k < j; k++){
                                                        matrizJugador1[fila][k] = 0;
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    else if (i == 1){
                                        if ((flotas[contJugador2 - 1] + columna) > (TAMANO_COLUMNA)){
                                            fprintf(f_sock[i], "S BARCO ERROR\n");
                                            b = false;
                                        }
                                        else{
                                            for (int j = columna;j < (flotas[contJugador2 - 1] + columna); j++){
                                                if (columna == j){
                                                    if (j != 0 && fila != 0 && j!= (TAMANO_COLUMNA-1) && fila!= (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador2[fila+1][j] != 1)&&(matrizJugador2[fila][j+1] != 1)&&(matrizJugador2[fila+1][j+1] != 1)&&(matrizJugador2[fila-1][j-1] != 1)&&(matrizJugador2[fila-1][j] != 1)&&(matrizJugador2[fila][j-1] != 1)&&(matrizJugador2[fila+1][j-1] != 1)&&(matrizJugador2[fila-1][j+1] != 1));
                                                    }
                                                    else if (j == (TAMANO_COLUMNA-1) && fila == 0){
                                                        tocandoFlota = ((matrizJugador2[fila][j-1] != 1)&&(matrizJugador2[fila+1][j-1] != 1)&&(matrizJugador2[fila+1][j] != 1));
                                                    }
                                                    else if (j== 0 && fila == (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador2[fila-1][j+1] != 1)&&(matrizJugador2[fila-1][j] != 1)&&(matrizJugador2[fila][j+1] != 1));
                                                    }
                                                    else if (j == 0 && fila == 0){
                                                        tocandoFlota = ((matrizJugador2[fila+1][j+1] != 1)&&(matrizJugador2[fila][j+1] != 1)&&(matrizJugador2[fila+1][j] != 1));
                                                    }
                                                    else if (j== (TAMANO_COLUMNA-1) && fila == (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador2[fila-1][j-1] != 1)&&(matrizJugador2[fila][j-1] != 1)&&(matrizJugador2[fila-1][j] != 1));
                                                    }
                                                    else if (fila == 0 ){
                                                        tocandoFlota = ((matrizJugador2[fila+1][j] != 1)&&(matrizJugador2[fila][j+1] != 1)&&(matrizJugador2[fila+1][j+1] != 1)&&(matrizJugador2[fila][j-1] != 1)&&(matrizJugador2[fila+1][j-1] != 1));
                                                    }
                                                    else if (j == 0){
                                                        tocandoFlota = ((matrizJugador2[fila+1][j] != 1)&&(matrizJugador2[fila][j+1] != 1)&&(matrizJugador2[fila+1][j+1] != 1)&&(matrizJugador2[fila-1][j] != 1)&&(matrizJugador2[fila-1][j+1] != 1));
                                                    }
                                                    
                                                    else if (fila== (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador2[fila][j+1] != 1)&&(matrizJugador2[fila-1][j-1] != 1)&&(matrizJugador2[fila-1][j] != 1)&&(matrizJugador2[fila][j-1] != 1)&&(matrizJugador2[fila-1][j+1] != 1));
                                                    }
                                                    else if (j == (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador2[fila+1][j] != 1)&&(matrizJugador2[fila-1][j-1] != 1)&&(matrizJugador2[fila-1][j] != 1)&&(matrizJugador2[fila][j-1] != 1)&&(matrizJugador2[fila+1][j-1] != 1));
                                                    }
                                                }
                                                else{
                                                    if (j != 0 && fila != 0 && j!= (TAMANO_COLUMNA-1) && fila!= (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador2[fila+1][j] != 1)&&(matrizJugador2[fila][j+1] != 1)&&(matrizJugador2[fila+1][j+1] != 1)&&(matrizJugador2[fila-1][j-1] != 1)&&(matrizJugador2[fila-1][j] != 1)&&(matrizJugador2[fila+1][j-1] != 1)&&(matrizJugador2[fila-1][j+1] != 1));
                                                    }
                                                    else if (j == (TAMANO_COLUMNA-1) && fila == 0){
                                                        tocandoFlota = ((matrizJugador2[fila+1][j-1] != 1)&&(matrizJugador2[fila+1][j] != 1));
                                                    }
                                                    else if (j== 0 && fila== (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador2[fila-1][j+1] != 1)&&(matrizJugador2[fila-1][j] != 1)&&(matrizJugador2[fila][j+1] != 1));
                                                    }
                                                     else if (j == 0 && fila == 0){
                                                        tocandoFlota = ((matrizJugador2[fila+1][j+1] != 1)&&(matrizJugador2[fila][j+1] != 1)&&(matrizJugador2[fila+1][j] != 1));
                                                    }
                                                    else if (j== (TAMANO_COLUMNA-1) && fila == (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador2[fila-1][j-1] != 1)&&(matrizJugador2[fila-1][j] != 1));
                                                    }
                                                    else if (j == 0){
                                                        tocandoFlota = ((matrizJugador2[fila+1][j] != 1)&&(matrizJugador2[fila][j+1] != 1)&&(matrizJugador2[fila+1][j+1] != 1)&&(matrizJugador2[fila-1][j] != 1)&&(matrizJugador2[fila-1][j+1] != 1));
                                                    }
                                                    else if ( fila == 0 ){
                                                        tocandoFlota = ((matrizJugador2[fila+1][j] != 1)&&(matrizJugador2[fila][j+1] != 1)&&(matrizJugador2[fila+1][j+1] != 1)&&(matrizJugador2[fila+1][j-1] != 1));
                                                    }
                                                    
                                                    else if (fila== (TAMANO_COLUMNA-1)){
                                                        tocandoFlota = ((matrizJugador2[fila][j+1] != 1)&&(matrizJugador2[fila-1][j-1] != 1)&&(matrizJugador2[fila-1][j] != 1)&&(matrizJugador2[fila-1][j+1] != 1));
                                                    }
                                                    else if (j == (TAMANO_FILA-1)){
                                                        tocandoFlota = ((matrizJugador2[fila+1][j] != 1)&&(matrizJugador2[fila-1][j-1] != 1)&&(matrizJugador2[fila-1][j] != 1)&&(matrizJugador2[fila+1][j-1] != 1));
                                                    }
                                                    
                                                }
                                                if ((matrizJugador2[fila][j] == 0) && (tocandoFlota)){
                                                    matrizJugador2[fila][j] = 1;
                                                }
                                                else if ((matrizJugador2[fila][j] != 0)){
                                                    fprintf(f_sock[i], "S BARCO ERROR\n");
                                                    b = false;
                                                    for (int k = columna;k < j; k++){
                                                        matrizJugador2[fila][k] = 0;
                                                    }
                                                    break;
                                                }
                                                else if (!tocandoFlota){
                                                    fprintf(f_sock[i], "S BARCO ERROR\n");
                                                    b = false;
                                                    for (int k = columna;k < j; k++){
                                                        matrizJugador2[fila][k] = 0;
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                else{
                                    fprintf(f_sock[i], "S BARCO ERROR\n");
                                    b = false;
                                }
                                if (b){
                                    if ((contJugador1 >= FLOTAS) && (i == 0)){
                                        fprintf(f_sock[i], "S ESPERA\n");
                                        contJugador1 = contJugador1 + 1;
                                    }
                                    else if ((contJugador2 >= FLOTAS) && (i == 1)){
                                        fprintf(f_sock[i], "S ESPERA\n");
                                        contJugador2 =  contJugador2 + 1;
                                    }
                                    else if (i == 0){
                                        fprintf(f_sock[i], "S BARCO %d\n", flotas[contJugador1]);
                                        contJugador1 = contJugador1 + 1;
                                    }
                                    else if (i == 1){
                                        fprintf(f_sock[i], "S BARCO %d\n", flotas[contJugador2]);
                                        contJugador2 =  contJugador2 + 1;
                                    }
                                    
                                    if ((contJugador2 >= (FLOTAS + 1))&&(contJugador1 >= (FLOTAS + 1))){
                                        for (int j = 0;j < N_DESCRIPTOR; j++){
                                            if (descriptores[j] != -1){
                                                fprintf(f_sock[j], "S INICIO %s %s \n", nombreJugador1, nombreJugador2);
                                            }
                                        }
                                        fprintf(f_sock[0], "S TUTURNO\n");
                                        for (int i = 0; i < TAMANO_FILA; i++){
                                            for (int j = 0; j < TAMANO_COLUMNA; j++){
                                                printf("%d ", matrizJugador1[i][j]);
                                            }
                                            printf("\n");
                                        }
                                        for (int i = 0; i < TAMANO_FILA; i++){
                                            for (int j = 0; j < TAMANO_COLUMNA; j++){
                                                printf("%d ", matrizJugador2[i][j]);
                                            }
                                            printf("\n");
                                        }
                                    }
                                }
                            }
                            else if (strcmp(strtok(fragmentacion[1], "\n"), "DISPARA") == 0){
                                int columna = atoi(fragmentacion[2]);
                                int fila = atoi(fragmentacion[3]);
                                bool tocandoFlota = false;
                                if (columna < 0 || columna > (TAMANO_COLUMNA-1) || fila < 0 || fila > (TAMANO_FILA-1)){
                                    fprintf(f_sock[i], "S NOVALE\n");
                                }
                                else if (i == 1){
                                    if (matrizJugador1[fila][columna] == 2){
                                        fprintf(f_sock[i], "S NOVALE\n");
                                    }
                                    else if (matrizJugador1[fila][columna] == 0){
                                        matrizJugador1[fila][columna] = 2;
                                        fprintf(f_sock[i], "S AGUA\n");
                                        for (int j = 0;j < N_DESCRIPTOR; j++){
                                            if (j != i && descriptores[j] != -1){
                                                fprintf(f_sock[j], "S INFO AGUA %s %d %d \n", nombreJugador2, columna, fila);
                                            }
                                        }
                                        fprintf(f_sock[i-1], "S TUTURNO\n");
                                    }
                                    else if (matrizJugador1[fila][columna] == 1){
                                        matrizJugador1[fila][columna] = 2;
                                        if (columna != 0 && fila != 0 && columna!= (TAMANO_COLUMNA-1) && fila!= (TAMANO_FILA-1)){
                                            tocandoFlota = ((matrizJugador1[fila+1][columna] != 1)&&(matrizJugador1[fila][columna+1] != 1)&&(matrizJugador1[fila+1][columna+1] != 1)&&(matrizJugador1[fila-1][columna-1] != 1)&&(matrizJugador1[fila-1][columna] != 1)&&(matrizJugador1[fila][columna-1] != 1)&&(matrizJugador1[fila+1][columna-1] != 1)&&(matrizJugador1[fila-1][columna+1] != 1));
                                        }
                                        else if (columna == (TAMANO_COLUMNA-1) && fila == 0){
                                            tocandoFlota = ((matrizJugador1[fila][columna-1] != 1)&&(matrizJugador1[fila+1][columna-1] != 1)&&(matrizJugador1[fila+1][columna] != 1));
                                                    }
                                        else if (columna== 0 && fila == (TAMANO_FILA-1)){
                                            tocandoFlota = ((matrizJugador1[fila-1][columna+1] != 1)&&(matrizJugador1[fila-1][columna] != 1)&&(matrizJugador1[fila][columna+1] != 1));
                                        }
                                        else if (columna == 0 && fila == 0){
                                            tocandoFlota = ((matrizJugador1[fila+1][columna+1] != 1)&&(matrizJugador1[fila][columna+1] != 1)&&(matrizJugador1[fila+1][columna] != 1));
                                        }
                                        else if (columna == (TAMANO_COLUMNA-1) && fila == (TAMANO_FILA-1)){
                                            tocandoFlota = ((matrizJugador1[fila-1][columna-1] != 1)&&(matrizJugador1[fila][columna-1] != 1)&&(matrizJugador1[fila-1][columna] != 1));
                                        }
                                        else if (fila == 0 ){
                                            tocandoFlota = ((matrizJugador1[fila+1][columna] != 1)&&(matrizJugador1[fila][columna+1] != 1)&&(matrizJugador1[fila+1][columna+1] != 1)&&(matrizJugador1[fila][columna-1] != 1)&&(matrizJugador1[fila+1][columna-1] != 1));
                                        }
                                        else if (columna == 0){
                                            tocandoFlota = ((matrizJugador1[fila+1][columna] != 1)&&(matrizJugador1[fila][columna+1] != 1)&&(matrizJugador1[fila+1][columna+1] != 1)&&(matrizJugador1[fila-1][columna] != 1)&&(matrizJugador1[fila-1][columna+1] != 1));
                                        }
                                        else if (fila== (TAMANO_COLUMNA-1)){
                                            tocandoFlota = ((matrizJugador1[fila][columna+1] != 1)&&(matrizJugador1[fila-1][columna-1] != 1)&&(matrizJugador1[fila-1][columna] != 1)&&(matrizJugador1[fila][columna-1] != 1)&&(matrizJugador1[fila-1][columna+1] != 1));
                                        }
                                        else if (columna == (TAMANO_FILA-1)){
                                            tocandoFlota = ((matrizJugador1[fila+1][columna] != 1)&&(matrizJugador1[fila-1][columna-1] != 1)&&(matrizJugador1[fila-1][columna] != 1)&&(matrizJugador1[fila][columna-1] != 1)&&(matrizJugador1[fila+1][columna-1] != 1));
                                        }
                                        if (tocandoFlota){
                                            matrizJugador1[fila][columna] = 2;
                                            fprintf(f_sock[i], "S HUNDIDO\n");
                                            for (int j = 0;j < N_DESCRIPTOR; j++){
                                                if ((j != i) && (descriptores[j] != -1)){
                                                    fprintf(f_sock[j], "S INFO HUNDIDO %s %d %d \n", nombreJugador2, columna, fila);
                                                }
                                            }
                                        }
                                        else{
                                            matrizJugador1[fila][columna] = 2;
                                            fprintf(f_sock[i], "S TOCADO\n");
                                            for (int j = 0;j < N_DESCRIPTOR; j++){
                                                if (j != i && (descriptores[j] != -1 )){
                                                    fprintf(f_sock[j], "S INFO TOCADO %s %d %d \n", nombreJugador2, columna, fila);
                                                }
                                            }
                                        }
                                        bool premio = true;
                                        for (int j = 0; j < TAMANO_FILA; j++){
                                            for (int k = 0; k < TAMANO_COLUMNA; k++){
                                                if (matrizJugador1[j][k] == 1){
                                                    premio = false;
                                                }
                                            }
                                        }
                                        if (premio){
                                            fprintf(f_sock[i], "S PREMIO\n");
                                            fprintf(f_sock[i-1], "S SIGUE JUGANDO\n");
                                            for (int j = 0;j < N_DESCRIPTOR; j++){
                                                if ((j!= i) && (j != i - 1) && (descriptores[j] != -1)){
                                                    fprintf(f_sock[j], "S GANA %s\n", nombreJugador2);
                                                }
                                            }
                                        }
                                        else{
                                            fprintf(f_sock[i], "S TUTURNO\n");
                                        }
                                    }
                                }
                                else if (i == 0){
                                    if (matrizJugador2[fila][columna] == 2){
                                        fprintf(f_sock[i], "S NOVALE\n");
                                    }
                                    else if (matrizJugador2[fila][columna] == 0){
                                        matrizJugador2[fila][columna] = 2;
                                        fprintf(f_sock[i], "S AGUA\n");
                                        for (int j = 0;j < N_DESCRIPTOR; j++){
                                            if ((j != i) && (descriptores[j] != -1)){
                                                fprintf(f_sock[j], "S INFO AGUA %s %d %d\n", nombreJugador1, columna, fila);
                                            }
                                        }
                                        fprintf(f_sock[i+1], "S TUTURNO\n");
                                    }
                                    else if (matrizJugador2[fila][columna] == 1){
                                        matrizJugador2[fila][columna] = 2;
                                        if (columna != 0 && fila != 0 && columna!= (TAMANO_COLUMNA-1) && fila!= (TAMANO_FILA-1)){
                                            tocandoFlota = ((matrizJugador2[fila+1][columna] != 1)&&(matrizJugador2[fila][columna+1] != 1)&&(matrizJugador2[fila+1][columna+1] != 1)&&(matrizJugador2[fila-1][columna-1] != 1)&&(matrizJugador2[fila-1][columna] != 1)&&(matrizJugador2[fila][columna-1] != 1)&&(matrizJugador2[fila+1][columna-1] != 1)&&(matrizJugador2[fila-1][columna+1] != 1));
                                        }
                                        else if (columna == (TAMANO_COLUMNA-1) && fila == 0){
                                            tocandoFlota = ((matrizJugador2[fila][columna-1] != 1)&&(matrizJugador2[fila+1][columna-1] != 1)&&(matrizJugador2[fila+1][columna] != 1));
                                                    }
                                        else if (columna== 0 && fila == (TAMANO_FILA-1)){
                                            tocandoFlota = ((matrizJugador2[fila-1][columna+1] != 1)&&(matrizJugador2[fila-1][columna] != 1)&&(matrizJugador2[fila][columna+1] != 1));
                                        }
                                        else if (columna == 0 && fila == 0){
                                            tocandoFlota = ((matrizJugador2[fila+1][columna+1] != 1)&&(matrizJugador2[fila][columna+1] != 1)&&(matrizJugador2[fila+1][columna] != 1));
                                        }
                                        else if (columna == (TAMANO_COLUMNA-1) && fila == (TAMANO_FILA-1)){
                                            tocandoFlota = ((matrizJugador2[fila-1][columna-1] != 1)&&(matrizJugador2[fila][columna-1] != 1)&&(matrizJugador2[fila-1][columna] != 1));
                                        }
                                        else if (fila == 0 ){
                                            tocandoFlota = ((matrizJugador2[fila+1][columna] != 1)&&(matrizJugador2[fila][columna+1] != 1)&&(matrizJugador2[fila+1][columna+1] != 1)&&(matrizJugador2[fila][columna-1] != 1)&&(matrizJugador2[fila+1][columna-1] != 1));
                                        }
                                        else if (columna == 0){
                                            tocandoFlota = ((matrizJugador2[fila+1][columna] != 1)&&(matrizJugador2[fila][columna+1] != 1)&&(matrizJugador2[fila+1][columna+1] != 1)&&(matrizJugador2[fila-1][columna] != 1)&&(matrizJugador2[fila-1][columna+1] != 1));
                                        }
                                        else if (fila== (TAMANO_COLUMNA-1)){
                                            tocandoFlota = ((matrizJugador2[fila][columna+1] != 1)&&(matrizJugador2[fila-1][columna-1] != 1)&&(matrizJugador2[fila-1][columna] != 1)&&(matrizJugador2[fila][columna-1] != 1)&&(matrizJugador2[fila-1][columna+1] != 1));
                                        }
                                        else if (columna == (TAMANO_FILA-1)){
                                            tocandoFlota = ((matrizJugador2[fila+1][columna] != 1)&&(matrizJugador2[fila-1][columna-1] != 1)&&(matrizJugador2[fila-1][columna] != 1)&&(matrizJugador2[fila][columna-1] != 1)&&(matrizJugador2[fila+1][columna-1] != 1));
                                        }
                                        if (tocandoFlota){
                                            matrizJugador2[fila][columna] = 2;
                                            fprintf(f_sock[i], "S HUNDIDO\n");
                                            for (int j = 0;j < N_DESCRIPTOR; j++){
                                                if (j != i && descriptores[j] != -1){
                                                    fprintf(f_sock[j], "S INFO HUNDIDO %s %d %d\n", nombreJugador1, columna, fila);
                                                }
                                            }
                                        }
                                        else{
                                            matrizJugador2[fila][columna] = 2;
                                            fprintf(f_sock[i], "S TOCADO\n");
                                            for (int j = 0;j < N_DESCRIPTOR; j++){
                                                if (j != i && descriptores[j]!=-1){
                                                    fprintf(f_sock[j], "S INFO TOCADO %s %d %d\n", nombreJugador1, columna, fila);
                                                }
                                            }
                                        }
                                        bool premio = true;
                                        for (int j = 0; j < TAMANO_FILA; j++){
                                            for (int k = 0; k < TAMANO_COLUMNA; k++){
                                                if (matrizJugador2[j][k] == 1){
                                                    premio = false;
                                                }
                                            }
                                        }
                                        if (premio){
                                            fprintf(f_sock[i], "S PREMIO\n");
                                            fprintf(f_sock[i+1], "S SIGUE JUGANDO\n");
                                            for (int j = 0;j < N_DESCRIPTOR; j++){
                                                if ((j != i) && (j != (i + 1)) && descriptores[j] != -1){
                                                    fprintf(f_sock[j], "S GANA %s\n", nombreJugador1);
                                                }
                                            }
                                        }
                                        else{
                                            fprintf(f_sock[i], "S TUTURNO\n");
                                        }
                                    }
                                }
                            }
                        }
                        else{
                            descriptores[i] = -1;
                            numclientes--;
                            printf("Ahora hay %d clientes \n", numclientes);
                        }
                    

                    memset(buf, '\0', LBUFFER);
                }
                
            }
        
    }
    fclose(fd_sock);
    close (sock);
}

void aceptar_nuevo_cliente(int sock){
    int c_sock; //nuevo socket cliente 
    struct sockaddr_in cliente; 
    int dirlen = sizeof(cliente);
    int i;
    printf("Esperando cliente.... \n");
    c_sock = accept(sock, (struct sockaddr*)&cliente, &dirlen);
    if (numclientes  < N_DESCRIPTOR){
        numclientes++; 
        for (i = 0; i< N_DESCRIPTOR; i++){
            if (descriptores[i] == -1){
                descriptores[i] = c_sock;
                f_sock[i] = fdopen(descriptores[i], "r+");
                setbuf(f_sock[i], NULL);
                fprintf(f_sock[i], "S HOLA\n");
                if (i == 0){
                    contJugador1 = 1;
                    for (int j = 0; j < TAMANO_FILA; j++){
                        for (int k = 0; k < TAMANO_COLUMNA; k++){
                            matrizJugador1[j][k] = 0;
                        }
                    }
                }
                else if (i == 1){
                    contJugador2 = 1;
                    for (int j = 0; j < TAMANO_FILA; j++){
                        for (int k = 0; k < TAMANO_COLUMNA; k++){
                            matrizJugador2[j][k] = 0;
                        }
                    }
                }
                break;
            }
        }
    }
    else{
        char cadena[100];
        FILE *fichero;
        fichero = fdopen(c_sock, "r+");
        setbuf(fichero, NULL);
        fprintf(fichero, "S LLENO\n");
        printf("Servidor lleno \n");
    }
}

void crearID(char id2[]){
    int tamano;
    char car;
    int random;
    tamano = rand () % (7) + 6; //numero random entre 6 y 12
    char id[(int)tamano];
    memset(id, '\0', tamano*sizeof (char));
    for (int i = 0; i <tamano; i++){
        random = (rand()%2);
        if (random == 0){
            car = 'a' + rand() % (('z' - 'a') + 1);//caracter random entre a y z
            id[i]=car;
        }
        else if (random == 1){
            int a;
            a = rand() % (10);
            car = enteroACaracter(a);
            id[i]=car;
        }
    }
    id[tamano] = '\0';
    strcpy(id2, id);
}

char enteroACaracter(int numero){
    return numero + '0';
}

bool comprobarId(char id[]){
    bool b = true;
    if (strlen(id) <= 0){
        b = false;
    }
    for (int i = 0; i < strlen(id); i++){
        if (((id[i] >= 'A') && (id[i] <= 'Z')) || ((id[i] >= 'a') && (id[i] <= 'z')) || ((id[i] >= '0') && (id[i] <= '9'))){
            continue;
        }
        else{
            b = false;
        }
    }
    return b;
}
