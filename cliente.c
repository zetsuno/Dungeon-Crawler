
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#define MAX 100

int NSEGUNDOS = 0;

void conta(int s){
	NSEGUNDOS++;
	alarm(1);
}


int main(int argc, char *argv[]){

char str_usr[MAX], str_pwd[MAX], str[MAX];
int res;

signal(14, conta);
alarm(1);
system("clear");

printf("|-- Dungeon Crawler v1.0.8b - Client --|");
printf("\n >> Username:  ");
scanf("%s", str_usr);
//envia o username ao servidor, caso não exista, dá erro e sai fora do cliente
printf("\n[ERRO]: Utilizador inexistente! ");
//exit(0);
//caso exista:
printf("\n >> Password:  ");
scanf("%s", str_pwd);
//envia a password ao servidor e verifica no ficheiro de texto se é coorespondente ao username.


//caso o servidor retorne FALSO (-1, por exemplo)

printf("[ERRO]: Password de utilizador não reconhecida!");
//exit(0);

//caso o servidor retorne verdadeiro (1, por exemplo)

printf("\nIntroduza o username que pretende usar in-game: ");
scanf("%s", str_usr);
//ações de verificação se o username ainda nao está em uso c/ fifos
//caso esteje a ser usado:
//while(res == -1){
printf("[ERRO]: O username desejado já está em uso por outro jogador!\n");
printf("\nIntroduza o username que pretende usar in-game: ");
scanf("%s", str_usr);
//  }
printf("\nIntroduza a password desejada: ");
scanf("%s", str_pwd);

// envia a info ao server, e começa no ciclo inicial de opções


do{
printf(">>>");
scanf(" %[^\n]", str);

//operações de fifo para mandar p/ servidor
//aguardar resposta do servidor

	if(res == 1){
	printf("[ERRO]: Comando nao reconhecido!\n");
	}
	if(res == 0){
	printf("[ERRO]: O tempo de se juntar excedeu o limite!\n");
	}
	if(res == -1){
	printf("[ERRO]: Sintaxe do comando inválida!\n");
    }


}while(strcmp(str,"fim") !=0);

}
