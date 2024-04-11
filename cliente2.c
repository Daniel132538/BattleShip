#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <stdbool.h>
#include <time.h>
#include "fragmenta.h"

#define LBUFFER 2000
#define BACKLOG 5

int main (int argc, char *argv[]){// cliente direccion puerto
    if (argc > 2){
        //Primer paso, definir variables
        srand(time(0));
        char *ip;
        int fd, numbytes, puerto;
        char buf[LBUFFER];
        puerto = atoi (argv[2]);
        ip = argv[1];
        FILE *usuario;
        FILE *fd_sock;
        char **fragmentacion;
        
        struct hostent *he;
        //Estructura que recibira informacion sobre el nodo remoto
        struct sockaddr_in server;
        //Informacion sobre la direccion del servidor
        
        if ((he = gethostbyname(ip)) == NULL){
            //llamada a gethostbyname()
            printf("gethostbyname error. \n");
            exit(EXIT_FAILURE);
        }
        //Paso 2,, definicon de socket
        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
            //llamada al socket()
            printf("socket() error \n");
            exit(-1);
        }
        //Datos del servidor
        server.sin_family = AF_INET; 
        server.sin_port = htons(puerto); //Puerto
        server.sin_addr.s_addr = inet_addr(ip); //Cualquier cliente puede conectarse
        bzero(&(server.sin_zero), 8); //Funcion que rellena con 0's
        
        if (connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1){
            //llamada a connect()
            printf("Error connect() \n");
            exit(EXIT_FAILURE);
        }
        
        fd_sock = fdopen(fd, "r+");
        setbuf(fd_sock, NULL);
        
        printf ("Conectado correctamente\n ");
        memset(buf, '\0', LBUFFER);
        fgets(buf, LBUFFER, fd_sock);
        fragmentacion = fragmentar(buf);
        
        if (strcmp(strtok(fragmentacion[1], "\n"), "HOLA") == 0){
            if (usuario = fopen("cliente.txt", "r")){
                memset(buf, '\0', LBUFFER);
                if (fgets(buf, LBUFFER, usuario) != NULL){
                    fragmentacion = fragmentar(buf);
                    printf("Hay datos para el usuario %s, probamos autentificacion.\n", fragmentacion[0]);
                    fprintf(fd_sock, "P LOGIN %s %s\n", fragmentacion[0], fragmentacion[1]);
                    memset(buf, '\0', LBUFFER);
                    fgets(buf, LBUFFER, fd_sock);
                    fragmentacion = fragmentar(buf);
                    memset(buf, '\0', LBUFFER);
                }
                else{
                    printf("No hemos encontrado datos. \n");
                    memset(buf, '\0', LBUFFER);
                    fgets(buf, LBUFFER, fd_sock);
                }
            }
            else{
                fprintf(fd_sock, "P REGISTRAR\n");
                printf("No existe id almacenado, realizando peticion de registro.\n");
                
                fgets(buf, LBUFFER, fd_sock);
                fragmentacion = fragmentar(buf);
            
                fragmentacion[2] = strtok(fragmentacion[2], "\n");
        
                fragmentacion[3] = strtok(fragmentacion[3], "\n");
                
                
                int sol = atoi(fragmentacion[2]) + atoi(fragmentacion[3]);
                
                printf("Resuelve  %s + %s = %d \n", fragmentacion[2], fragmentacion[3], sol);
            
                char resp[LBUFFER] = "RESPUESTA ";
                fprintf(fd_sock, "P %s %d\n", resp, sol);
                memset(buf, '\0', LBUFFER);
            
               
                fgets(buf, LBUFFER, fd_sock);
                
                fragmentacion = fragmentar(buf);
            
                if (strcmp("OK", strtok(fragmentacion[2], "\n")) == 0){
                    usuario = fopen("cliente.txt", "w");
                    setbuf(usuario, NULL);
                    sprintf(buf, "%s %d Invitado", strtok(fragmentacion[3], "\n"), sol);
                    fputs(buf, usuario);
                    fclose(usuario);
                    memset(buf, '\0', LBUFFER);
                }
            }
        }
        else if (strcmp(buf, "S LLENO\n") == 0){
            printf("Servidor lleno \n");
            fclose(fd_sock);
            close (fd);
            exit(-1);
        }
        else{
            printf("PROTOCOLO ERROR\n");
        }
        
        printf("%s", buf);
        
        
        if (strcmp("OK", strtok(fragmentacion[2], "\n")) == 0){
            char nombre[20];
            printf("Conseguido. Logeado correctamente.\n");
            printf("\n");
            printf("¿Como te llamarás? : ");
            scanf("%s",nombre);
            fprintf(fd_sock, "P NOMBRE %s \n", nombre);
            memset(buf, '\0', LBUFFER);
            fgets(buf, LBUFFER, fd_sock);
            fragmentacion = fragmentar(buf);
            if (strcmp(buf, "S OBSERVADOR\n") != 0){
                while  (strcmp("ERROR", strtok(fragmentacion[1], "\n")) == 0){
                    printf("Escribe nombre que contenga solo mayusculas, minusculas y numeros de un tamaño de 20 carácteres. \n");
                    printf("¿Como te llamarás? : ");
                    scanf("%s",nombre);
                    fprintf(fd_sock, "P NOMBRE %s \n", nombre);
                    memset(buf, '\0', LBUFFER);
                    fgets(buf, LBUFFER, fd_sock);
                    fragmentacion = fragmentar(buf);
                }
            }
            if (strcmp(buf, "S OBSERVADOR\n") == 0){
                printf("Solo puedes observar la partida\n");
                while ((strcmp(strtok(fragmentacion[1], "\n"), "GANA") != 0) && (fgets(buf, LBUFFER, fd_sock)!= NULL)){
                    fragmentacion = fragmentar(buf);
                    if (strcmp(strtok(fragmentacion[2], "\n"), "TOCADO") == 0){
                        printf("El jugador %s ha TOCADO una flota en [%s] [%s] . \n", strtok(fragmentacion[3], "\n"), strtok(fragmentacion[4], "\n"), strtok(fragmentacion[5], "\n"));
                        printf("\n");
                    }
                    else if (strcmp(strtok(fragmentacion[2], "\n"), "HUNDIDO") == 0){
                        printf("El jugador %s ha HUNDIDO una flota una flota en [%s] [%s] . \n", strtok(fragmentacion[3], "\n"), strtok(fragmentacion[4], "\n"), strtok(fragmentacion[5], "\n"));
                        printf("\n");
                    }
                    else if (strcmp(strtok(fragmentacion[2], "\n"), "AGUA") == 0){
                        printf("El jugador %s ha hecho AGUA  una flota en [%s] [%s] .\n", strtok(fragmentacion[3], "\n"), strtok(fragmentacion[4], "\n"), strtok(fragmentacion[5], "\n"));
                        printf("\n");
                    }
                    else if (strcmp(strtok(fragmentacion[1], "\n"), "INICIO") == 0){
                        printf("Partida entre %s y %s \n", strtok(fragmentacion[2], "\n"), strtok(fragmentacion[3], "\n"));
                    }
                }
                printf("El ganador es %s \n", strtok(fragmentacion[2], "\n"));
            }
            else if (strcmp("OK", strtok(fragmentacion[2], "\n")) == 0){
                printf("Nombre correcto \n");
                printf("Ahora comenzará a colocar las diferentes flotas donde le apetezca en un rango de 10x10\n");
                memset(buf, '\0', LBUFFER);
                fgets(buf, LBUFFER, fd_sock);
                fragmentacion = fragmentar(buf);
                int columna;
                int fila;
                char direccion[5];
                int flota;
                while (strcmp("BARCO", strtok(fragmentacion[1], "\n")) == 0){
                    if (strcmp("ERROR", strtok(fragmentacion[2], "\n")) == 0){
                        printf("Ha ocurrido un error vuelva a introducir donde quiere colocar la flota de %d casillas\n", flota);
                        printf("Escribe la columna del 0 al 9: ");
                        scanf("%d", &columna);
                        printf("Escribe la fila del 0 al 9: ");
                        scanf("%d", &fila);
                        printf("Escribe la direccion de la flota vertical(v) u horizontal (h): ");
                        scanf("%s", direccion);
                        fprintf(fd_sock, "P BARCO %d %d %s\n", columna, fila, direccion);
                    }
                    else{
                        flota = atoi(fragmentacion[2]);
                        printf("Donde desea colocar el barco de %s casillas\n", strtok(fragmentacion[2], "\n"));
                        printf("Escribe la columna del 0 al 9: ");
                        scanf("%d", &columna);
                        printf("Escribe la fila del 0 al 9: ");
                        scanf("%d", &fila);
                        printf("Escribe la direccion de la flota vertical(v) u horizontal (h): ");
                        scanf("%s", direccion);
                        fprintf(fd_sock, "P BARCO %d %d %s\n", columna, fila, direccion);
                    }
                    memset(buf, '\0', LBUFFER);
                    fgets(buf, LBUFFER, fd_sock);
                    fragmentacion = fragmentar(buf);
                }
                if (strcmp("S ESPERA\n", buf) == 0){
                    printf("Esperando a que el otro jugador este listo \n");
                    fgets(buf, LBUFFER, fd_sock);
                    fragmentacion = fragmentar(buf);
                    if (strcmp("INICIO", strtok(fragmentacion[1], "\n")) == 0){
                        printf("Partida entre %s y %s \n", strtok(fragmentacion[2], "\n"), strtok(fragmentacion[3], "\n"));
                    }
                    int columna;
                    int fila;
                    while ((fgets(buf, LBUFFER, fd_sock)!= NULL) && (strcmp(buf, "S PREMIO\n") != 0) && (strcmp(buf, "S SIGUE JUGANDO\n")) != 0){
                        fragmentacion = fragmentar(buf);
                        if (strcmp(buf, "S TUTURNO\n") == 0){
                            printf("A continuación realizará su disparo. \n");
                            printf("Elige la columna a la que desea disparar del 0 al 9: ");
                            scanf("%d", &columna);
                            printf("Elige la fila a la que desea disparar del 0 al 9: ");
                            scanf("%d", &fila);
                            fprintf(fd_sock, "P DISPARA %d %d \n", columna, fila);
                        }
                        else if (strcmp(buf, "S NOVALE\n") == 0){
                            printf("Ha este lugar ya has disparado o esta fuera de rango. Prueba otra vez \n");
                            printf("Elige la columna a la que desea disparar del 0 al 9: ");
                            scanf("%d", &columna);
                            printf("Elige la fila a la que desea disparar del 0 al 9: ");
                            scanf("%d", &fila);
                            printf("\n");
                            fprintf(fd_sock, "P DISPARA %d %d \n", columna, fila);
                        }
                        else if (strcmp(buf, "S TOCADO\n") == 0){
                            printf("Has TOCADO una flota. \n");
                            printf("\n");
                        }
                        else if (strcmp(buf, "S HUNDIDO\n") == 0){
                            printf("Has HUNDIDO una flota\n");
                            printf("\n");
                        }
                        else if (strcmp(buf, "S AGUA\n") == 0){
                            printf("AGUA\n");
                            printf("\n");
                        }
                        else if (strcmp(strtok(fragmentacion[2], "\n"), "TOCADO") == 0){
                            printf("El rival %s ha TOCADO una flota en [%s] [%s] . \n", strtok(fragmentacion[3], "\n"), strtok(fragmentacion[4], "\n"), strtok(fragmentacion[5], "\n"));
                            printf("\n");
                        }
                        else if (strcmp(strtok(fragmentacion[2], "\n"), "HUNDIDO") == 0){
                            printf("El rival %s ha HUNDIDO una flota una flota en [%s] [%s] . \n", strtok(fragmentacion[3], "\n"), strtok(fragmentacion[4], "\n"), strtok(fragmentacion[5], "\n"));
                            printf("\n");
                        }
                        else if (strcmp(strtok(fragmentacion[2], "\n"), "AGUA") == 0){
                            printf("El rival %s ha hecho AGUA  una flota en [%s] [%s] .\n", strtok(fragmentacion[3], "\n"), strtok(fragmentacion[4], "\n"), strtok(fragmentacion[5], "\n"));
                            printf("\n");
                        }
                    }
                    if (strcmp(buf, "S PREMIO\n") == 0){
                        printf("Enhourabuena! Has ganado! \n");
                    }
                    else if (strcmp(buf, "S SIGUE JUGANDO\n") == 0){
                        printf("El rival te ha ganado. Sigue jugando \n");
                    }
                    else{
                        printf("PROTOCOLO ERROR \n");
                    }
                }
            }
            else{
                printf("PROTOCOLO ERROR\n");
            }
        }
        else{
            printf("No se ha conseguido registrar o logear\n");
        }
        
        fclose(fd_sock);
        close (fd);
    }
    else{
        printf("No se ingreso el ip y el puerto por parametro \n");
    }
}
