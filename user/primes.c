#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void PRIMARDIUM (int pipe_izq){

    int primo;
    if (read(pipe_izq, &primo, sizeof(int)) == 0) { //Agarra el primer número y revisa si es el último en el pipe.
        return;
    }
    //Todos los nodos hijos dicen su primo correspondiente.
    printf("prime %d\n", primo);

    int fd_nuevo[2];
    pipe(fd_nuevo);

    //Si es que estamos en el hijo hacemos llamada recursiva y pasamos lo que tenemos en la parte de lectura del pipe.
    if(fork() == 0){ 
        close(fd_nuevo[1]);
        PRIMARDIUM(fd_nuevo[0]);
    }

    //Agarramos los valores entrantes del padre y los filtramos con nuestro primo, si pasa los escribimos en el pipe nuevo.
    else{
        close(fd_nuevo[0]);
        int x;
        while(read(pipe_izq, &x, sizeof(int)) > 0){
            if(x % primo != 0){
                write(fd_nuevo[1], &x, sizeof(int));
            }
        }
        //Cerramos todo y esperamos al hijo.
        close(fd_nuevo[1]);
        close(pipe_izq);
        wait(0);
        return;
    }
}


int main(){
    int fd_init[2];
    pipe(fd_init);

    if (fork()==0){
        close(fd_init[1]);
        PRIMARDIUM(fd_init[0]);
    }
    //Primer proceso crea los números y los pasa todos a su hijo.
    else{
        close(fd_init[0]);
        for(int i=2; i<=35; i++){
            write(fd_init[1], &i, sizeof(int));
        }
        close(fd_init[1]);
        wait(0);
    }
    exit(0);
    
}