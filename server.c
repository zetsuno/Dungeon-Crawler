/////////////////////
//Parte do servidor//
/////////////////////

#include <stdio.h>
#include <time.h>   // dá jeito pro rand()
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#define MAX 100
#define MAX_JOGADORES 100

int NSEGUNDOS = 0;
int njogadores = 0;
int game_running_flag;

typedef struct dados_players
{
    int id;     //jogador mestre tem ID = 1
    int saude; //start = 20, Max = 30
    int peso; // Max = 20
    int atk;
    int def;
    char inventory[20][20];
    int id_sala; //id da sala onde se encontra
    int flag_ingame  // se o jogador está ingame ou não
    int coin_count; //contador de moedas
}jogador;


typedef struct dados_items
{
    char nome[15];
    float peso;
    int raridade;
    int forca_atk;
    int forca_def;
    int max_usos;
    int use_effect;   //bonus de saude
    int carry_effect; //bonus de defesa
}items;

typedef struct dados_monstros
{
    char nome[10];
    int forca_atk_min;
    int forca_atk_max;
    int forca_def_min;
    int forca_def_max;
    int saude_min;
    int saude_max;
    int num_inicial;
    int agressividade;      //Em vez de andarmos sempre co strcmp, fazemos 1 = agressivo, 0 = passivo
    int movimento;         // Mesmo que em cima, 1 = Movimenta-se, 0 = fica quieto;
    char drops[35];
}monstros;


typedef struct dados_room
{
    int id;         // 1 = ID da sala inicial
    int p_norte;    // 1 = porta aberta, 0 = porta fechada
    int p_sul;
    int p_este;
    int p_oeste;
    char nmonstros[25][25];     // min 0, max 2
    char items[25][25];
    char descricao[80];
    char njogadores[25][25];

}sala;

sala labirinto[10][10];

monstros tabela_monstros[5] =
{
 [0] = {"morcego", 1, 4, 3, 4, 4, 5, 10, 1, 1,"NULL" },
 [1] = {"escorpiao", 1, 7, 5, 7, 7, 9, 10, 1, 0, "moeda"},
 [2] = {"lobisomem", 5, 7, 5, 7, 7, 9, 4, 1, 1, "faca"},
 [3] = {"pedobear", 8, 10, 10, 12, 10, 10, 2, 0, 0, "faca moeda"},  //quando é mais que 1 item, podemos usar o strtok pra dividir isto bem, e mais facilmente
 [4] = {"b0ss", 10, 12, 15, 15, 15, 15, 1, 0, 0, "moeda moeda moeda moeda moeda"}
};

items tabela_items[8] =
{
 [0] = {"sandes", 0.5, 10, 0, 0, 1, 3, -99},   //Assumir que -99 = NULL
 [1] = {"aspirina", 0.1, 20, 0, 0, 1, 1, -99},
 [2] = {"xarope", 1.0, 4, 0, 0, 1, 4,-99},
 [3] = {"faca", 2.0, 5, 5, 0, -99, -99, -99},
 [4] = {"espada", 8.0, 3, 8, 2, -99, -99, 2},
 [5] = {"granada", 1.0, 2, 30, 0, 1, -5, -99},
 [6] = {"escudo", 4.0, 4, 5, 5, -99, -99, 5},
 [7] = {"moeda", 0.1, 5, 0, 0, -99, -99, -99}
};

jogador lista_jogadores[10];

int random_number(int min_num, int max_num)   //genera valor aleatorio entre min_num e max_num
{
            int result=0,low_num=0,hi_num=0;
            if(min_num<max_num)
            {
                low_num=min_num;
                hi_num=max_num+1;
            }else{
                low_num=max_num+1;
                hi_num=min_num;
            }
            srand(time(NULL));
            result = (rand()%(hi_num-low_num))+low_num;
            return result;
}

void start_timer(int s){
	NSEGUNDOS++;
	alarm(1);
}


int avalia_frase(char **palavra, int aux)
{
    int i = 0, timeout_aux;
    char *str_aux;
    FILE *f;

    if(strcmp(palavra[i],"novo") == 0){
        for(i=0;i<njogadores;i++){
            if(strcmp(ign, lista_jogadores[i]) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        if(lista_jogadores[i].id != 1){
            printf("[ERRO]: Só o jogador mestre (ID = 1) pode começar jogos!\n");
            return;
        }
        else{
            if(game_running_flag == 1){
                printf("[ERRO]: Já existe um jogo em execução! \n")
            }
        i++;
        if(i == aux){
            return -1;
        }
        timeout_aux = atoi(palavra[i]);
        if(timeout_aux<0){
            printf("[ERRO]: Timeout Invalido!\n");
            return -1;
        }
        i++;
        if(i == aux){
            return;
        }
        else{
        str_aux = palavra[i];
        f=fopen(str_aux, "rt");
        if(f == NULL){
            printf("[ERRO]: Erro ao abrir o ficheiro '%s'\n", str_aux);
            return;

            }
        }
        }
    }
    if(strcmp(palavra[i], "jogar") == 0){
        if(NSEGUNDOS < 0){
            printf("[ERRO]: O tempo de se juntar ao jogo excedeu o limite! \n");
            return;
            }
        if(n_jogadores >= MAX_JOGADORES){
            printf("[ERRO]: O jogo já está na sua capacidade máxima de jogadores! \n");
            }
            else{
                system("kill -10 /* pids dos outros jogadores */");
                printf("Um jogo acabou de ser lançado!\n");
                join_game();
                system("clear");
                game();
            }
    }
    if(strcmp(palavra[i], "sair") == 0 && lista_jogadores[i].id_sala == 1){
        sair();
        for(i=0;i<njogadores;i++){
            if(strcmp(ign, lista_jogadores[i]) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }

        system("kill -11 /* pid filhos */");   // avisa os filhos que o jogador x saiu do jogo
        //operações de retirar o jogador do labirinto
    }
    if(strcmp(palavra[i],"terminar") == 0){
        for(i=0;i<njogadores;i++){
            if(strcmp(ign, lista_jogadores[i]) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        if(lista_jogadores[i].id == 1){

        system("kill -11 /*pid filhos*/");  //avisa os filhos que o jogo acabou
        showgameresult();     //mostra o resultado do jogo: moedas dos users, e quem tem mais, ou seja, quem ganhou.
        return;
        }
        else{
            printf("[ERRO]: Você não é o jogador mestre! Não tem permissão para acabar com o jogo!\n");
        }
    }
    if(strcmp(palavra[i],"desistir") == 0){
        for(i=0;i<njogadores;i++){
            if(strcmp(ign, lista_jogadores[i]) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
                break;
        }
        if(lista_jogadores[i].flag_ingame == 0){
        printf("[ERRO]: O jogador não se encontra em jogo. \n");
        }
        if(lista_jogadores[i].id == 1){
            getmasterplayer();  //eleger outro jogador mestre
        }
        //desistir: remover jogador da lista, droppar os items
        njogadores--:
        if(njogadores == 0){
            showgameresult();
            end_game();
        }



    }
    return 7;
}

void initialize_game(){
    int i, j, id_aux1, id_aux2, aux_rand;
	//Inicialização do labirinto
    for(i=0;i<10;i++){
        for(j=0;j<10;j++){
            //Exepções de cantos
            if(i ==0 && j == 0){
                labirinto[i][j].p_norte = 0;
                labirinto[i][j].p_oeste = 0;
                aux_rand = random_number(0, 1);
                p_este = aux_rand;
                aux_rand = random_number(0, 1);
                p_sul = aux_rand;
            }
            else if(i == 0 && j == 9){
                labirinto[i][j].p_este = 0;
                labirinto[i][j].p_norte = 0;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_oeste = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_sul = aux_rand;
            }
            else if(i == 9 && j == 0){
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_norte = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_este = aux_rand;
                labirinto[i][j].p_sul = 0;
                labirinto[i][j].p_oeste = 0;
            }
            else if(i == 9 && j == 9){
                labirinto[i][j].p_sul = 0;
                labirinto[i][j].p_este = 0;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_norte = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_oeste = aux_rand;
            }
            //Exepções de linhas
            else if(i == 0){
                labirinto[i][j].p_norte = 0;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_sul = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_este = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_oeste = aux_rand;
            }
            else if(i == 9){
                labirinto[i][j].p_sul = 0;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_norte = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_este = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_oeste = aux_rand;
            }
            else if(j == 0){
                labirinto[i][j].p_oeste = 0;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_sul = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_este = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_norte = aux_rand;
            }
            else if(j == 9){
                labirinto[i][j].p_este = 0;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_sul = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_norte = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_oeste = aux_rand;
            }
            //Outros no geral
            else{
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_norte = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_sul = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_este = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_oeste = aux_rand;

            }

        }
    }
    //calcular a sala inical
    id_aux1 = random_number(0, 9);
    id_aux2 = random_number(0, 9);

    for(i=0;i<9;i++){
        for(j=0;j<9;j++){
            if(i == id_aux1 && j == id_aux2){
            labirinto[i][j].id = 1;
            }

        }
    }

}
void help(){
system("clear");
printf("----- Help -----\n\n");
printf(" >  novo {valor-timeout} {nome-ficheiro}  -- Comecar um jogo novo a partir de um ficheiro \n");
printf(" >  novo {valor-timeout} {valor-dificuldade} -- Comecar o jogo com distribuição aleatória \n");
printf(" >  jogar -- Associa o jogador a um jogo já existente \n");
printf(" >  sair -- Faz com que o utilizador saia do jogo. \n");
printf(" >  terminar -- Faz com que o jogo termine para todos os utilizadores (Requer que o jogador esteje na sala inicial!)\n");
printf(" ----------------\n\n");
}
int main(){
/////////////Client.c UI - AQUI PARA TESTES APENAS //////////////
// server.c --> int n_jogadores = 0; //incrementa ou decrementa consoante o nº de jogadores no jogo
char str[MAX];
char *palavra[8];
int i=0, res, estado;

do{
printf(">>>");
scanf(" %[^\n]", str);
if(strcmp(str, "help")==0)
{
    help();
}
palavra[i] = strtok(str, " ");
do{
	i++;
	palavra[i] = strtok(NULL," ");
}while(palavra[i] != NULL);

	res = avalia_frase(palavra, i);
	i=0;
	if(res == 7){

	    if(game_running_flag == 0){
            initialize_game();
            //enviar o pid ao cliente.c
            // server.c --> sign4(14, start_timer);
            // server.c --> alarm(1);
            system("kill -10  /*pid*/ ");
            game_running_flag = 1;
	    }
	    else{
            printf("O jogo já está em execução! Junte-se a ele com o comando 'jogar'!\n");
	    }


	}
	if(res == 1){
	printf("[ERRO]: Comando nao reconhecido!\n");
	}
	if(res == -1){
	printf("[ERRO]: Sintaxe do comando inválida!\n");
    }


}while(strcmp(str,"fim") !=0);

return 1;
}



