/////////////////////
//Parte do servidor//
/////////////////////

#include <stdio.h>
#include <time.h>   // dá jeito pro rand()
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#define DIM_LAB 10
#define MAX 100
#define MAX_JOGADORES 10

int NSEGUNDOS = 0;
int njogadores = 0;
int game_running_flag;

typedef struct dados_players
{   char nome[25];
    int id;     //jogador mestre tem ID = 1
    int pos_x; //abcissa da posição do jogador na matriz
    int pos_y;  //ordenada da posição do jogador na matriz
    float saude; //start = 20, Max = 30
    float peso; // Max = 20
    int atk;
    int def;
    char *inventory[10];
    int pid; //pid
    int id_sala; //id da sala onde se encontra
    int flag_ingame;  // se o jogador está ingame ou não
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
    int forca_atk;
    int forca_atk_min;
    int forca_atk_max;
    int forca_def;
    int forca_def_min;
    int forca_def_max;
    float saude;
    float saude_min;
    float saude_max;
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
    monstros monstros_room[3];     // min 0, max 2
    items items_room[5];
    char descricao[80];
    jogador jogadores_room[10];

}sala;

sala labirinto[DIM_LAB][DIM_LAB];

monstros tabela_monstros[5] =
{
 [0] = {"morcego", 0, 1, 4, 0, 3, 4, 0, 4, 5, 10, 1, 1,"NULL" },
 [1] = {"escorpiao", 0, 1, 7, 0, 5, 7, 0, 7, 9, 10, 1, 0, "moeda"},
 [2] = {"lobisomem", 0, 5, 7, 0, 5, 7, 0, 7, 9, 4, 1, 1, "faca"},
 [3] = {"pedobear", 0, 8, 10, 0, 10, 12, 0,  10, 10, 2, 0, 0, "faca moeda"},  //quando é mais que 1 item, podemos usar o strtok pra dividir isto bem, e mais facilmente
 [4] = {"b0ss", 0, 10, 12, 0, 15, 15, 0, 15, 15, 1, 0, 0, "moeda moeda moeda moeda moeda"}
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

jogador lista_jogadores[10]={"NULL"};


float RandomFloat(float a, float b) {  //genera valor aleatorio float
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}
void clear_struct(int j, int k, int n){

    int i;

    labirinto[j][k].jogadores_room[n].nome[0] = '\0';
    labirinto[j][k].jogadores_room[n].id = 0;

    for(i=0; i<10; i++){
        labirinto[j][k].jogadores_room[n].inventory[i] = "";
    }
    labirinto[j][k].jogadores_room[n].pid = 0;
    labirinto[j][k].jogadores_room[n].coin_count = 0;
    labirinto[j][k].jogadores_room[n].flag_ingame = 0;

}
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

float porrada(int atk_attacker, int def_defender){  //PUNHADA

    int aux_rand, aux_rand2;
    float dano;

    aux_rand = RandomFloat(0, (float)atk_attacker);
    aux_rand2 = RandomFloat(0.0,0.5);

    dano = abs(aux_rand - (aux_rand2 * def_defender));

    return dano;
}

void player_dies(char *ign, int j, int k, int i){

    int l = 0, m = 0, n, b = 0;

    for(n=0;n<10;n++){
        if(strcmp(labirinto[j][k].jogadores_room[n].nome, ign) == 0) //procura o jogador em questao
            break;
            }

            do{

                    while(strcmp(labirinto[j][k].jogadores_room[n].inventory[b], tabela_items[l].nome) !=0){  //procura o item na lista
                        l++;
                        if(l == 10)
                         goto kill_player;
                    }


                    while(strcmp(labirinto[j][k].items_room[m].nome, "") != 0){   //procura o primeiro espaco em branco nos slots de items da sala
                        m++;
                        if(m == 5)
                         goto kill_player;
                        
                    }
                    labirinto[j][k].items_room[m] = tabela_items[l];  //mete o item na room


            m=0, l=0;
            b++;
            }while(m<5);
            
kill_player:
            for(b=0;b<10;b++){                   //limpa o user da lista de jogadores
            if(strcmp(lista_jogadores[b].nome,ign) == 0){
                lista_jogadores[b].nome[0] = '\0';
                lista_jogadores[b].id = 0;
                lista_jogadores[b].pid = 0;
                lista_jogadores[b].coin_count = 0;
		lista_jogadores[b].flag_ingame = 0;
                for(l=0;l<10;l++){
                lista_jogadores[b].inventory[l] = "";  //limpa o inventário
                }
            }
            }
            clear_struct(j,k,n);       //limpa os valores do user do sitio onde estava no mapa
}

int avalia_frase(char **palavra, int aux) //char *ign
{
    int i = 0,j, k, l, n , m,  timeout_aux, aux_pos_x, aux_pos_y;
    char *str_aux;
    float dano;
    FILE *f;

    if(strcmp(palavra[i],"novo") == 0){
        for(i=0;i<njogadores;i++){
            if(strcmp(ign, lista_jogadores[i].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        if(lista_jogadores[i].id != 1){
            printf("\n[ERRO]: Só o jogador mestre (ID = 1) pode começar jogos!\n");
            return;
        }
        else{
            if(game_running_flag == 1){
                printf("\n[ERRO]: Já existe um jogo em execução! \n");
            }
        i++;
        if(i == aux){
            return -1;
        }
        timeout_aux = atoi(palavra[i]);
        if(timeout_aux<0){
            printf("\n[ERRO]: Timeout Invalido!\n");
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
            printf("\n[ERRO]: Erro ao abrir o ficheiro '%s'\n", str_aux);
            return;

            }
        }
        }
    }
    if(strcmp(palavra[i], "jogar") == 0){
        if(NSEGUNDOS < 0){
            printf("\n[ERRO]: O tempo de se juntar ao jogo excedeu o limite!\n");
            return;
            }
        if(njogadores >= MAX_JOGADORES){
            printf("\n[ERRO]: O jogo já está na sua capacidade máxima de jogadores!\n");
            }
            else{
                system("kill -10 /* pids dos outros jogadores */");
                printf("\nUm jogo acabou de ser lançado!\n");
                join_game();
                system("clear");
                game();
            }
    }
    if(strcmp(palavra[i], "sair") == 0 && lista_jogadores[i].id_sala == 1){
        sair();
        for(i=0;i<njogadores;i++){
            if(strcmp(ign, lista_jogadores[i].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }

        system("kill -11 /* pid filhos */");   // avisa os filhos que o jogador x saiu do jogo
        //operações de retirar o jogador do labirinto
    }
    if(strcmp(palavra[i],"terminar") == 0){
        for(i=0;i<njogadores;i++){
            if(strcmp(ign, lista_jogadores[i].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        if(lista_jogadores[i].id == 1){

        system("kill -11 /*pid filhos*/");  //avisa os filhos que o jogo acabou
        showgameresult();     //mostra o resultado do jogo: moedas dos users, e quem tem mais, ou seja, quem ganhou.
        return;
        }
        else{
            printf("\n[ERRO]: Você não é o jogador mestre! Não tem permissão para acabar com o jogo!\n");
        }
    }
    if(strcmp(palavra[i],"desistir") == 0){
        for(i=0;i<njogadores;i++){
            if(strcmp(ign, lista_jogadores[i].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
                break;
        }
        if(lista_jogadores[i].flag_ingame == 0){
        printf("\n[ERRO]: O jogador não se encontra em jogo. \n");
        }
        if(lista_jogadores[i].id == 1){
            getmasterplayer();  //eleger outro jogador mestre
        }
        //desistir: remover jogador da lista, droppar os items
        njogadores--;
        if(njogadores == 0){
            showgameresult();
            end_game();
        }



    }
    if(strcmp(palavra[i],"logout") == 0){
        for(i=0;i<njogadores;i++){
            if(strcmp(ign, lista_jogadores[i].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        if(lista_jogadores[i].flag_ingame == 1){
            printf("\n[ERRO]: Ainda está em jogo! Use 'sair' para sair do jogo e depois execute este comando!\n");
            return -1;
        }
        else{
            system("kill -8 /*pid do filho a matar*/");
        }
    }
    if(strcmp(palavra[i],"info") == 0){
        for(i=0;i<njogadores;i++){
            if(strcmp(ign, lista_jogadores[i].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        if(lista_jogadores[i].saude > 15.0){
            printf("Feelin' good!\n");
        }
        if(lista_jogadores[i].saude > 10.0 && lista_jogadores[i].saude < 15.0){
            printf("A few cuts n' bruises...\n");
        }
        if(lista_jogadores[i].saude < 10.0){
            printf("Need something to cure the pain... and fast...\n");
        }
        if(lista_jogadores[i].peso > 15.0){
            printf("I feel heavy...\n");
        }
        if(lista_jogadores[i].peso > 10.0 && lista_jogadores[i].peso < 15.0){
            printf("I'm okay weight-wise.\n");
        }
        if(lista_jogadores[i].peso < 10.0){
            printf("Light as a feather!\n");
        }
        printf("\nTenho %d moedas!", lista_jogadores[i].coin_count);
        printf("\nNo meu inventário tenho:\t");
        for(j=0;j<10;j++){
            printf("%s\t", lista_jogadores[i].inventory[j]);
            j++;
            printf("%s\t", lista_jogadores[i].inventory[j]);
            j++;
            printf("%s\n", lista_jogadores[i].inventory[j]);

        }

    }
    if(strcmp(palavra[i],"norte") == 0){
         for(i=0;i<njogadores;i++){
            if(strcmp(ign, lista_jogadores[i].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        aux_pos_x=lista_jogadores[i].pos_x;
        aux_pos_y=lista_jogadores[i].pos_y;

        for(j=0;j<DIM_LAB;j++){   //percorrer o labirinto
            for(k=0;k<DIM_LAB;k++){
                if(j == aux_pos_y && k == aux_pos_x){ //encontrar a posicao do jogador
                    for(l=0;l<3;l++){
                        if(labirinto[j][k].monstros_room[l].agressividade  == 1){    //verificar se ha monstros agressivos na sala
                            dano = porrada(labirinto[j][k].monstros_room[l].forca_atk, lista_jogadores[i].def);  //calcular o dano entre monstro/jogador
                            lista_jogadores[i].saude -= dano;
                            if(lista_jogadores[i].saude <= 0){

                                player_dies(ign, j, k, i);
                            }
                            for(n=0;n<10;n++){
                                if(strcmp(ign, labirinto[j][k].jogadores_room[n].nome) == 0){
                                    labirinto[j][k].jogadores_room[n].saude = lista_jogadores[i].saude;  //fazer as alteracoes da hp em lista_jogadores e em labirinto[i][j].jogadores_room[n]
                                }
                            }
                            printf("[INFO]: Um monstro agressivo atacou-o antes que conseguisse sair da sala! Perdeu %.1f de vida e o seu movimento não foi aceite!", dano);
                            return;
                        }
                    }
                    if(labirinto[j][k].p_norte == 1){
                    for(m=0;m<10;m++){
                        if(strcmp(labirinto[j-1][k].jogadores_room[m].nome, "") == 0)
                            break;
                    }
                    labirinto[j-1][k].jogadores_room[m] = labirinto[j][k].jogadores_room[n];
                    clear_struct(j,k,n);
                    }
                    else{
                        printf("[INFO]: Não existe uma porta aberta na direção especificada!\n");
                        return;
                    }
                }
            }
        }
    }
     if(strcmp(palavra[i],"sul") == 0){
         for(i=0;i<njogadores;i++){
            if(strcmp(ign, lista_jogadores[i].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        aux_pos_x=lista_jogadores[i].pos_x;
        aux_pos_y=lista_jogadores[i].pos_y;

        for(j=0;j<DIM_LAB;j++){   //percorrer o labirinto
            for(k=0;k<DIM_LAB;k++){
                if(j == aux_pos_y && k == aux_pos_x){ //encontrar a posicao do jogador
                    for(l=0;l<3;l++){
                        if(labirinto[j][k].monstros_room[l].agressividade  == 1){    //verificar se ha monstros agressivos na sala
                            dano = porrada(labirinto[j][k].monstros_room[l].forca_atk, lista_jogadores[i].def);  //calcular o dano entre monstro/jogador
                            lista_jogadores[i].saude -= dano;
                            if(lista_jogadores[i].saude <= 0){

                                player_dies(ign, j, k, i);
                            }
                            for(n=0;n<10;n++){
                                if(strcmp(ign, labirinto[j][k].jogadores_room[n].nome) == 0){
                                    labirinto[j][k].jogadores_room[n].saude = lista_jogadores[i].saude;  //fazer as alteracoes da hp em lista_jogadores e em labirinto[i][j].jogadores_room[n]
                                }
                            }
                            printf("[INFO]: Um monstro agressivo atacou-o antes que conseguisse sair da sala! Perdeu %.1f de vida e o seu movimento não foi aceite!", dano);
                            return;
                        }
                    }
                    if(labirinto[j][k].p_sul == 1){
                    for(m=0;m<10;m++){
                        if(strcmp(labirinto[j+1][k].jogadores_room[m].nome, "") == 0)
                            break;
                    }
                    labirinto[j+1][k].jogadores_room[m] = labirinto[j][k].jogadores_room[n];
                    clear_struct(j,k,n);
                    }
                    else{
                        printf("[INFO]: Não existe uma porta aberta na direção especificada!\n");
                        return;
                    }
                }
            }
        }
    }
     if(strcmp(palavra[i],"este") == 0){
         for(i=0;i<njogadores;i++){
            if(strcmp(ign, lista_jogadores[i].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        aux_pos_x=lista_jogadores[i].pos_x;
        aux_pos_y=lista_jogadores[i].pos_y;

        for(j=0;j<DIM_LAB;j++){   //percorrer o labirinto
            for(k=0;k<DIM_LAB;k++){
                if(j == aux_pos_y && k == aux_pos_x){ //encontrar a posicao do jogador
                    for(l=0;l<3;l++){
                        if(labirinto[j][k].monstros_room[l].agressividade  == 1){    //verificar se ha monstros agressivos na sala
                            dano = porrada(labirinto[j][k].monstros_room[l].forca_atk, lista_jogadores[i].def);  //calcular o dano entre monstro/jogador
                            lista_jogadores[i].saude -= dano;
                            if(lista_jogadores[i].saude <= 0){

                                player_dies(ign, j, k, i);
                            }
                            for(n=0;n<10;n++){
                                if(strcmp(ign, labirinto[j][k].jogadores_room[n].nome) == 0){
                                    labirinto[j][k].jogadores_room[n].saude = lista_jogadores[i].saude;  //fazer as alteracoes da hp em lista_jogadores e em labirinto[i][j].jogadores_room[n]
                                }
                            }
                            printf("[INFO]: Um monstro agressivo atacou-o antes que conseguisse sair da sala! Perdeu %.1f de vida e o seu movimento não foi aceite!", dano);
                            return;
                        }
                    }
                    if(labirinto[j][k].p_este == 1){
                    for(m=0;m<10;m++){
                        if(strcmp(labirinto[j][k+1].jogadores_room[m].nome, "") == 0)
                            break;
                    }
                    labirinto[j][k+1].jogadores_room[m] = labirinto[j][k].jogadores_room[n];
                    clear_struct(j,k,n);
                    }
                    else{
                        printf("[INFO]: Não existe uma porta aberta na direção especificada!\n");
                        return;
                    }
                }
            }
        }
    }
     if(strcmp(palavra[i],"oeste") == 0){
         for(i=0;i<njogadores;i++){
            if(strcmp(ign, lista_jogadores[i].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        aux_pos_x=lista_jogadores[i].pos_x;
        aux_pos_y=lista_jogadores[i].pos_y;

        for(j=0;j<DIM_LAB;j++){   //percorrer o labirinto
            for(k=0;k<DIM_LAB;k++){
                if(j == aux_pos_y && k == aux_pos_x){ //encontrar a posicao do jogador
                    for(l=0;l<3;l++){
                        if(labirinto[j][k].monstros_room[l].agressividade  == 1){    //verificar se ha monstros agressivos na sala
                            dano = porrada(labirinto[j][k].monstros_room[l].forca_atk, lista_jogadores[i].def);  //calcular o dano entre monstro/jogador
                            lista_jogadores[i].saude -= dano;
                            if(lista_jogadores[i].saude <= 0){

                                player_dies(ign, j, k, i);
                            }
                            for(n=0;n<10;n++){
                                if(strcmp(ign, labirinto[j][k].jogadores_room[n].nome) == 0){
                                    labirinto[j][k].jogadores_room[n].saude = lista_jogadores[i].saude;  //fazer as alteracoes da hp em lista_jogadores e em labirinto[i][j].jogadores_room[n]
                                }
                            }
                            printf("[INFO]: Um monstro agressivo atacou-o antes que conseguisse sair da sala! Perdeu %.1f de vida e o seu movimento não foi aceite!", dano);
                            return;
                        }
                    }
                    if(labirinto[j][k].p_oeste == 1){
                    for(m=0;m<10;m++){
                        if(strcmp(labirinto[j][k-1].jogadores_room[m].nome, "") == 0)
                            break;
                    }
                    labirinto[j][k-1].jogadores_room[m] = labirinto[j][k].jogadores_room[n];
                    clear_struct(j,k,n);
                    }
                    else{
                        printf("[INFO]: Não existe uma porta aberta na direção especificada!\n");
                        return;
                    }
                }
            }
        }
    }
    return 7;

}

void initialize_game(){
    int i, j, k, id_aux1, id_aux2, id_aux3, aux_rand, aux_rand2, aux_rand3, aux_rarity,
    bat_count = 0, scorp_count = 0, werewolf_count = 0, bear_count = 0, boss_count = 0;

    //Inicializaçao do labirinto
    for(i=0;i<DIM_LAB;i++){
        for(j=0;j<DIM_LAB;j++){
            //Exepções de cantos
            if(i ==0 && j == 0){
                labirinto[i][j].p_norte = 0;
                labirinto[i][j].p_oeste = 0;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_este = aux_rand;
                aux_rand = random_number(0, 1);
                labirinto[i][j].p_sul = aux_rand;
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

    for(i=0;i<DIM_LAB;i++){
        for(j=0;j<DIM_LAB;j++){
            if(i == id_aux1 && j == id_aux2){
            labirinto[i][j].id = 1;
            }

        }
    }

    //colocação aleatória de items

    for(i=0;i<DIM_LAB;i++){
        for(j=0;j<DIM_LAB;j++){
            aux_rand = random_number(0, 5);
            for(k=0;k<aux_rand;k++){
                aux_rand2 = random_number(0, 7);
                aux_rarity = tabela_items[aux_rand2].raridade;
                aux_rand3 = random_number(0, 100);
                if(aux_rand3 < aux_rarity){
                    labirinto[i][j].items_room[aux_rand] = tabela_items[aux_rand2];
                }
            }
        }
    }

    //Inicialização do inventário e stats do utilizador
    id_aux3 = 1;
    for(i=0;i<njogadores;i++){
        if(strcmp(lista_jogadores[i].nome, "NULL") != 0){
            lista_jogadores[i].inventory[0] = "faca";
            lista_jogadores[i].inventory[1] = "aspirina";
            lista_jogadores[i].peso = 2.1;
            lista_jogadores[i].id = id_aux3;
            lista_jogadores[i].saude = 20;
            lista_jogadores[i].atk = 5;
            lista_jogadores[i].def = 0;
            lista_jogadores[i].id_sala = 1;
            lista_jogadores[i].flag_ingame = 1;
            lista_jogadores[i].coin_count = 0;
            lista_jogadores[i].pos_x = id_aux1;
            lista_jogadores[i].pos_y = id_aux2;
            id_aux1++;
        }

    }
        //inicializaão dos monstros no labirinto, consoante os pre-requisitos numéricos

    for(i=0;i<DIM_LAB;i++){
        for(j=0;j<DIM_LAB;j++){
            aux_rand = random_number(0,2);
            for(k=0;k<aux_rand;k++){
                aux_rand2 = random_number(0, 4);
                if(bat_count == tabela_monstros[0].num_inicial && scorp_count == tabela_monstros[1].num_inicial && werewolf_count == tabela_monstros[2].num_inicial && bear_count == tabela_monstros[3].num_inicial && boss_count == tabela_monstros[4].num_inicial)
                    goto end_monsters;              //Salta lá pra bauxo
                while(aux_rand2 == 0  && bat_count == tabela_monstros[aux_rand2].num_inicial){
                    aux_rand2 = random_number(0,4);
                }
                while(aux_rand2 == 1 && scorp_count == tabela_monstros[aux_rand2].num_inicial){
                    aux_rand2 = random_number(0,4);
                }
                while(aux_rand2 == 2 && werewolf_count == tabela_monstros[aux_rand2].num_inicial){
                    aux_rand2 = random_number(0,4);
                }
                while(aux_rand2 == 3 && bear_count == tabela_monstros[aux_rand2].num_inicial){
                    aux_rand2 = random_number(0,4);
                }
                while(aux_rand2 == 4 && boss_count == tabela_monstros[aux_rand2].num_inicial){
                    aux_rand2 = random_number(0,4);
                }
                if(aux_rand2 == 0)
                    bat_count++;
                if(aux_rand2 == 1)
                    scorp_count++;
                if(aux_rand2 == 2)
                    werewolf_count++;
                if(aux_rand2 == 3)
                    bear_count++;
                if(aux_rand2 == 4)
                    boss_count++;

                labirinto[i][j].monstros_room[k] = tabela_monstros[aux_rand2];
                labirinto[i][j].monstros_room[k].forca_atk = random_number(labirinto[i][j].monstros_room[k].forca_atk_min, labirinto[i][j].monstros_room[k].forca_atk_max);
                labirinto[i][j].monstros_room[k].forca_def = random_number(labirinto[i][j].monstros_room[k].forca_def_min, labirinto[i][j].monstros_room[k].forca_def_max);
                labirinto[i][j].monstros_room[k].saude = RandomFloat(labirinto[i][j].monstros_room[k].saude_min, labirinto[i][j].monstros_room[k].saude_max);




            }
        }
    }
    end_monsters: /*inserir o que falta aqui*/;   //Goto = JMP em assembly
}
void help(){
system("clear");
printf("----- Help -----\n\n");
printf(" >  novo {valor-timeout} {nome-ficheiro}  -- Comecar um jogo novo a partir de um ficheiro \n");
printf(" >  novo {valor-timeout} {valor-dificuldade} -- Comecar o jogo com distribuição aleatória \n");
printf(" >  jogar -- Associa o jogador a um jogo já existente \n");
printf(" >  sair -- Faz com que o utilizador saia do jogo. \n");
printf(" >  terminar -- Faz com que o jogo termine para todos os utilizadores (Requer que o jogador esteje na sala inicial!)\n");
printf(" >  logout -- Termina a aplicação 'cliente' para o utilizador apenas, assumindo que não se encontra em jogo!\n");
printf(" >  info -- Uma pequena descrição do estado do utilizador.\n");
printf(" ----------------\n\n");
}
int main(){
/////////////Client.c UI - AQUI PARA TESTES APENAS //////////////
// server.c --> int njogadores = 0; //incrementa ou decrementa consoante o nº de jogadores no jogo
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



