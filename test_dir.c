#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    /* Variables */
 DIR *dirp;
 struct dirent *direntp;
 
 /* Comprobamos los argumentos */
 if (argc != 2){
 printf("Uso: %s directorio\n", argv[0]);
 exit(1);
 }
 
 /* Abrimos el directorio */
 dirp = opendir(argv[1]);
 if (dirp == NULL){
 printf("Error: No se puede abrir el directorio\n");
 exit(2);
 }
 
 /* Leemos las entradas del directorio */
 printf("i-nodo\toffset\t\tlong\tnombre\n");
 while ((direntp = readdir(dirp)) != NULL) {
 printf("%d\t%d\t%d\t%s\n", direntp->d_ino, direntp->d_off, direntp->d_reclen, direntp->d_name);
 }
 
 /* Cerramos el directorio */
 closedir(dirp);
}
