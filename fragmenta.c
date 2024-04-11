#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fragmenta.h"

char **fragmentar(char *cadena){
    int palabras,i;
    palabras = 1;
    i = 0;
    while (cadena[i] != '\0'){
        if (cadena[i] == ' '){
            palabras++;
            while (cadena[i] == ' '){
                i++;
            }
        }
        else{
            i++;
        }
    }
    char **comando = (char **)malloc((palabras + 1) * sizeof(char*));
    i = 0;
    int z = 0;
    int log = 0;
    int j = 0;
    while (cadena[i] != '\0'){
        z = 0;
        log = 0;
        while ((cadena[i] != ' ') && (cadena[i] != '\0')){
            log++;
            i++;
        }
        i = i - log;
        if (cadena[i] == ' ') {
            while (cadena[i] == ' '){
                i++;
            }
        }
        else{
	    comando[j] = NULL;
            comando[j] = (char *)malloc(log + 1);
            while ((cadena[i] != ' ') && (cadena[i] != '\0')){
                comando[j][z] = cadena[i];
                z++;
                i++;
            }
            int y;
            for (y = z; y <= (log + 1); y++){
                comando[j][y] = '\0';
            }
            j++;
        }
        
    }
    comando[j] = NULL;
    return (comando);
    
}

void borrarg(char **comando){
    int i = 0;
    while (comando[i] != NULL){
        comando[i] = NULL;
        free(comando[i]);
        i++;
    }
}
