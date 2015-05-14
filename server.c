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

struct dados_players
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
};

typedef struct dados_players jogador;

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
    monstros monstros_room[2];     // min 0, max 2
    items items_room[5];
    jogador jogadores_room[10];

}sala;

sala labirinto[DIM_LAB][DIM_LAB];

monstros tabela_monstros[5] =
{
 [0] = {"morcego", 0, 1, 4, 0, 3, 4, 0, 4, 5, 10, 1, 1,"NULL" },
 [1] = {"escorpiao", 0, 1, 7, 0, 5, 7, 0, 7, 9, 10, 1, 0, "moeda"},
 [2] = {"lobisomem", 0, 5, 7, 0, 5, 7, 0, 7, 9, 4, 1, 1, "faca"},
 [3] = {"pedobear", 0, 8, 10, 0, 10, 12, 0,  10, 10, 2, 0, 0, "faca moeda"},  //quando é mais que 1 item, podemos usar o strtok pra dividir isto bem, e mais facilmente
 [4] = {"b0ss", 0, 10, 12, 0, 15, 15, 0, 15, 15, 1, 0, 0, "5moedas"}
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

jogador lista_jogadores[10]={ {.nome = ""}, {.nome = ""},{.nome = ""}, {.nome = ""} ,{.nome = ""}, {.nome = ""},{.nome = ""}, {.nome = ""} ,{.nome = ""}, {.nome = ""} };

char *descricao[10] = {

    [0] = "Sala pequena e escura, com algumas aranhas na parede.",
    [1] = "Sala húmida e escorregadia. Ouço alguém a gritar.",
    [2] = "Sala iluminada por lamparinas. Ao pé destas lamparinas estão algumas traças.",
    [3] = "Sala com esqueletos no chão. Ouço alguma coisa a fazer ruído ao fundo da sala.",
    [4] = "Sala inundada com água. Espero que nada viva nestas águas...",
    [5] = "Sala com uma pequeno altar. Contém uns símbolos estranhos...",
    [6] = "Sala com um buraco no chão... Espero que não saia lá nada de dentro...",
    [7] = "Sala com alguma vegetação... Será que consigo encontrar comida aqui?",
    [8] = "Sala praticamente desmoronada...Será dificil passar por aqui...",
    [9] = "Sala inicial! Está alí a saída!"
};


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

void getmasterplayer(char *ign){

    int aux_rand, aux_pos_x, aux_pos_y, j, k,n;

    aux_rand = random_number(0,9);

    while(strcmp(lista_jogadores[aux_rand].nome, "") == 0){
        aux_rand = random_number(0,9);
    }

    lista_jogadores[aux_rand].id = 1;   //fazer as alteracoes do id em lista_jogadores

    aux_pos_x=lista_jogadores[aux_rand].pos_x;
    aux_pos_y=lista_jogadores[aux_rand].pos_y;

    for(j=0;j<DIM_LAB;j++){   //percorrer o labirinto
        for(k=0;k<DIM_LAB;k++){
            if(j == aux_pos_y && k == aux_pos_x){
                for(n=0;n<10;n++){
                    if(strcmp(ign, labirinto[j][k].jogadores_room[n].nome) == 0){
                        labirinto[j][k].jogadores_room[n].id = 1;  //fazer as alteracoes do id em lista_jogadores e em labirinto[i][j].jogadores_room[n]
                    }
                }
            }
        }
    }
}


void player_dies(char *ign, int j, int k, int i){

    int l = 0, m = 0, n, b = 0;

    for(n=0;n<10;n++){
        if(strcmp(labirinto[j][k].jogadores_room[n].nome, ign) == 0) //procura o jogador em questao
            break;
            }

            do{

                    while(strcmp(labirinto[j][k].jogadores_room[n].inventory[b], tabela_items[l].nome) !=0){  //procura o item na lista
                        if(l == 10){
                         	goto kill_player;
                        }
                        l++;

                    }


                    while(strcmp(labirinto[j][k].items_room[m].nome, "") != 0){   //procura o primeiro espaco em branco nos slots de items da sala
                       if(m == 5){
                       		goto kill_player;
                       }
                        m++;


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
void showgameresult(){

    int aux=0, aux2, i;
    system("clear");

    printf("---- Resultados ----\n");

    for(i=0;i<10;i++){
        if(strcmp(lista_jogadores[i].nome, "") != 0){
        printf("'%s' > %d Moedas.\n", lista_jogadores[i].nome, lista_jogadores[i].coin_count);
        if(aux < lista_jogadores[i].coin_count)
            aux = lista_jogadores[i].coin_count;
            aux2 = i;
        }
    }
    printf("Vencedor: '%s' com %d Moedas.\n", lista_jogadores[aux2].nome, lista_jogadores[aux2].coin_count);

}

void join_game(char *ign, int pid_usr){

int i, j, k,p;

for(p=0;p<10;p++){
    if(strcmp(lista_jogadores[p].nome, "") == 0){
    strcpy(lista_jogadores[p].nome,ign);
    lista_jogadores[p].pid = pid_usr;
    break;
    }
}

    for(j=0;j<DIM_LAB;j++){
        for(k=0;k<DIM_LAB;k++){
            if(labirinto[j][k].id == 1){
               for(i=0;i<10;i++){
                  if(strcmp(labirinto[j][k].jogadores_room[i].nome, "") == 0)
                     strcpy(labirinto[j][k].jogadores_room[i].nome, ign);
                     labirinto[j][k].jogadores_room[i].pid = pid_usr;
                     goto end_copy_usr;

               }
            }
        }
    }

end_copy_usr:
;


}
int avalia_frase(char **palavra, int aux) //char *ign //int pid
{
    int i = 0,j, k, l, n , m, p, x,y,  timeout_aux, aux_pos_x, aux_pos_y, aux_rand, aux1, aux2, flag_its_a_player, flag_its_a_monster;
    char *str_aux, c1,c2,c3,c4;
    float dano;
    FILE *f;


    if(strcmp(palavra[i],"novo") == 0){
        for(j=0;j<njogadores;j++){
            if(strcmp(ign, lista_jogadores[j].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
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
            for(x = 0; x < DIM_LAB; x++){
                for(y = 0; y < DIM_LAB; y++){
                    fscanf(f,"%c%c%c%c ",&c1,&c2,&c3,&c4);
                    if(c1 == '0' || c1 == '1'){
                        labirinto[x][y].p_norte = (c1);
                    }
                    if(c2 == '0' || c2 == '1'){
                        labirinto[x][y].p_este = (c2);
                    }
                    if(c3 == '0' || c3 == '1'){
                        labirinto[x][y].p_sul = (c3);
                    }
                    if(c4 == '0' || c4 == '1'){
                        labirinto[x][y].p_oeste = (c4);
                    }
                }
            }
             fclose(f);
            }
        }
    }

    if(strcmp(palavra[i], "jogar") == 0){
        for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
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
                join_game(ign, pid);
                system("clear");
                game();
            }
    }
    if(strcmp(palavra[i], "sair") == 0){
        sair();
        for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        aux_pos_x=lista_jogadores[p].pos_x;
        aux_pos_y=lista_jogadores[p].pos_y;

          if(lista_jogadores[p].id_sala == 1){
            for(j=0;j<DIM_LAB;j++){   //percorrer o labirinto  //desistir: remover jogador da lista, droppar os items
                for(k=0;k<DIM_LAB;k++){
                    if(j == aux_pos_y && k == aux_pos_x){
                    player_dies(ign,j,k,p);
                }
            }
        }
    }
    else{
        printf("Você não está na sala inicial! Só pode sair do labirinto na sala inicial.");
    }
            // avisa os filhos que o jogador x saiu do jogo
    }
    if(strcmp(palavra[i],"quem") == 0){

        for(p=0;p<10;p++){
            if(strcmp(lista_jogadores[p].nome, "") != 0){
                if(lista_jogadores[p].flag_ingame == 1){
                    printf("'%s' > Ingame!\n", lista_jogadores[p].nome);
                }
                else
                    printf("'%s' > Out of the game.\n", lista_jogadores[p].nome);
            }
        }
    }
    if(strcmp(palavra[i],"ataca") == 0){
        for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        aux_pos_x=lista_jogadores[p].pos_x;
        aux_pos_y=lista_jogadores[p].pos_y;

        for(j=0;j<2;j++){ //Procura o monstro a atacar na sala
            if(strcmp(palavra[i+1], labirinto[aux_pos_x][aux_pos_y].monstros_room[j].nome) == 0){
                aux1 = 1;
                flag_its_a_monster = 1;
                break;
            }
        }
        for(j=0;j<10;j++){ //Procura o jogador a atacar na sala
            if(strcmp(palavra[i+1], labirinto[aux_pos_x][aux_pos_y].jogadores_room[j].nome) == 0){
                aux1 = 1;
                flag_its_a_player = 1;
                break;
            }
        }

        if(aux1 != 1){
            printf("[ERRO]: Não encontrado o que se pretende atacar!\n");
            return;
        }
        for(k=0;k<10;k++){  //procura a arma no inventório do jogador
            if(strcmp(palavra[i+2], lista_jogadores[p].inventory[k])){
            aux2 = 1;
            break;
            }
        }
        if(aux2 != 1){
            printf("[ERRO]: Não tenho tal arma!");
            return;
        }   //operações de diminuição de vida caso o atacado seja um monstro.
        if(flag_its_a_monster == 1){
            for(l=0;l<8;l++){
                if(strcmp(palavra[i+2], tabela_items[l].nome) == 0){
                    dano = porrada(tabela_items[l].forca_atk, labirinto[aux_pos_x][aux_pos_y].monstros_room[j].forca_def);
                    labirinto[aux_pos_x][aux_pos_y].monstros_room[j].saude -= dano;
                    dano = porrada(labirinto[aux_pos_x][aux_pos_y].monstros_room[j].forca_atk, lista_jogadores[p].def);
                    lista_jogadores[p].saude -= dano;
                    for(m=0;m<10;m++){
                        if(strcmp(ign, labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].nome) == 0){
                            labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].saude -=dano;
                            break;
                        }
                    }
                    if(strcmp(palavra[i+2], "granada") == 0){
                        lista_jogadores[p].saude -= 5.0;
                        labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].saude -= 5.0;
                    } //caso jogador morra;
                    if(lista_jogadores[p].saude < 0){
                        player_dies(ign, aux_pos_x, aux_pos_y, p);
                        printf("[INFO]: U DED SON\n");
                    } //caso monstro morra; (drops)
                    if(labirinto[aux_pos_x][aux_pos_y].monstros_room[j].saude < 0){
                        if(strcmp(labirinto[aux_pos_x][aux_pos_y].monstros_room[j].drops, "5moedas") == 0){
                            labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].coin_count +=5;
                            lista_jogadores[p].coin_count +=5;
                        }
                        if(strcmp(labirinto[aux_pos_x][aux_pos_y].monstros_room[j].drops, "moeda") == 0){
                            labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].coin_count +=1;
                            lista_jogadores[p].coin_count +=1;
                        }
                        if(strcmp(labirinto[aux_pos_x][aux_pos_y].monstros_room[j].drops, "faca moeda") == 0){
                            for(n=0;n<5;n++){
                                if(strcmp(labirinto[aux_pos_x][aux_pos_y].items_room[n].nome, "") == 0){
                                labirinto[aux_pos_x][aux_pos_y].items_room[n] = tabela_items[3];
                                break;
                                }
                            }
                            labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].coin_count +=1;
                            lista_jogadores[p].coin_count +=1;
                        }
                        if(strcmp(labirinto[aux_pos_x][aux_pos_y].monstros_room[j].drops, "faca") == 0){
                            for(n=0;n<5;n++){
                                if(strcmp(labirinto[aux_pos_x][aux_pos_y].items_room[n].nome, "") == 0){
                                labirinto[aux_pos_x][aux_pos_y].items_room[n] = tabela_items[3];
                                break;
                                }
                            }
                        }
                       labirinto[aux_pos_x][aux_pos_y].monstros_room[j].nome[0]='\0';
                    }
                }
            }
        }
        if(flag_its_a_player == 1){ //operações de diminuição de vida caso o atacado seja um jogador
            dano = porrada(labirinto[aux_pos_x][aux_pos_y].jogadores_room[j].atk, lista_jogadores[p].def);
            lista_jogadores[p].saude -= dano;
            for(m=0;m<10;m++){
                if(strcmp(ign, labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].nome) == 0){
                    labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].saude -= dano;
                    break;
                }
                //enviar mensagem ao atacado (labirinto[aux_pos_x][aux_pos_y].jogadores_room[j].pid)
                //foi atacado por 'ign', e recebeu 'dano' de dano!
            }
            if(lista_jogadores[p].saude < 0){
                        player_dies(ign, aux_pos_x, aux_pos_y, p);
            }

            dano = porrada(tabela_items[l].forca_atk, labirinto[aux_pos_x][aux_pos_y].jogadores_room[j].def);
            labirinto[aux_pos_x][aux_pos_y].jogadores_room[j].saude -= dano;
            for(m=0;m<10;m++){
                if(strcmp(palavra[i+1], lista_jogadores[m].nome) == 0){
                    lista_jogadores[m].saude -= dano;
                    break;
                }
            }
            if(lista_jogadores[m].saude < 0){

                player_dies(palavra[i+1], aux_pos_x, aux_pos_y, m);
                //enviar info ao user, a avisar que morreu
            }
        }
    }
    if(strcmp(palavra[i],"apanha") == 0){
        for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        aux_pos_x=lista_jogadores[p].pos_x;
        aux_pos_y=lista_jogadores[p].pos_y;

        //verificar se o item existe na sala ou se existe de todo
        for(m=0;m<5;m++){
            if(strcmp(palavra[i+1],labirinto[aux_pos_x][aux_pos_y].items_room[m].nome) == 0){
            aux1=1;
            break;
            }
        }
        if(aux1 != 1){
            printf("O item não existe na sala ou não existe de todo!\n");
            return;
        }
        //procurar espaço livre no inventario do jogador
        for(n=0;n<10;n++){
            if(strcmp(lista_jogadores[p].inventory[n],"") == 0){
                strcpy(lista_jogadores[p].inventory[n], labirinto[aux_pos_x][aux_pos_y].items_room[m].nome);
                for(j=0;j<10;j++){
                    if(strcmp(ign, labirinto[aux_pos_x][aux_pos_y].jogadores_room[j].nome) == 0){
                        strcpy(labirinto[aux_pos_x][aux_pos_y].jogadores_room[j].inventory[n], labirinto[aux_pos_x][aux_pos_y].items_room[m].nome);
                        }
                    }
                aux2 = 1;
                break;
            }
        }
        if(aux2 != 1){
            printf("Não tenho mais espaço no inventorio!\n");
            return;
        }
        //adicionar os efeitos de carregar o item ao jogador
        if(labirinto[aux_pos_x][aux_pos_y].items_room[m].carry_effect != -99)
            lista_jogadores[p].def += labirinto[aux_pos_x][aux_pos_y].items_room[m].carry_effect;
        for(j=0;j<10;j++){
            if(strcmp(ign, labirinto[aux_pos_x][aux_pos_y].jogadores_room[j].nome) == 0){
                if(labirinto[aux_pos_x][aux_pos_y].items_room[m].carry_effect != -99)
                    lista_jogadores[p].def += labirinto[aux_pos_x][aux_pos_y].items_room[m].carry_effect;
            }
        }
        //eliminar o item da sala

        labirinto[aux_pos_x][aux_pos_y].items_room[m].nome[0] = '\0';



    }
    if(strcmp(palavra[i],"usa") == 0){
        for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        aux_pos_x=lista_jogadores[p].pos_x;
        aux_pos_y=lista_jogadores[p].pos_y;

        //verificar se o user tem o item
        for(m=0;m<10;m++){
            if(strcmp(ign, labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].nome) == 0)
            break;
        }
        for(k=0;k<5;k++){
            if(strcmp(palavra[i+1], labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].inventory[k]) == 0){
            aux1 = 1;
            break;
            }

        }

        if(aux1 != 1){
            printf("Nao tenho tal item!\n");
            return;
        }
        // 'usar' o item
        for(n=0;n<8;n++){
            if(strcmp(palavra[i+1],tabela_items[n].nome) == 0){
                if(tabela_items[n].use_effect != -99){
                    lista_jogadores[p].saude += tabela_items[n].use_effect;
                    labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].saude += tabela_items[n].use_effect;
                }
                //apagar o item caso ele chegue ao limite de usos possíveis e reduzir o peso do item ao utilizador.
                if(tabela_items[n].max_usos != -99){
                    for(l=0;l<10;l++){
                        if(strcmp(palavra[i+1], labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].inventory[l])){

                            labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].peso -= tabela_items[n].peso;
                            labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].inventory[l] = "";
                            lista_jogadores[p].peso -= tabela_items[n].peso;
                            lista_jogadores[p].inventory[l] = "";
                        }
                    }
                }
            }
        }
    }
    if(strcmp(palavra[i],"larga") == 0){

    for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        aux_pos_x=lista_jogadores[p].pos_x;
        aux_pos_y=lista_jogadores[p].pos_y;

        //verificar se o user tem o item
        for(m=0;m<10;m++){
            if(strcmp(ign, labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].nome) == 0)
            break;
        }
        for(k=0;k<5;k++){
            if(strcmp(palavra[i+1], labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].inventory[k]) == 0){
            aux1 = 1;
            break;
            }

        }

        if(aux1 != 1){
            printf("Nao tenho tal item!\n");
            return;
        }
        //colocar o item na sala caso haja espaço
        for(j=0;j<5;j++){
            if(strcmp(labirinto[aux_pos_x][aux_pos_y].items_room[j].nome, "") == 0){
                strcpy(labirinto[aux_pos_x][aux_pos_y].items_room[j].nome, palavra[i+1]);
                aux2 = 1;
                break;

            }
        }
        if(aux2 != 1){
            printf("Não há espaço na sala! O item foi destruido!\n");

        }


        //remover o item do inventorio
        lista_jogadores[p].inventory[k] = "";
        labirinto[aux_pos_x][aux_pos_y].jogadores_room[m].inventory[k] = "";

    }
    if(strcmp(palavra[i],"diz") == 0){
        for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        aux_pos_x=lista_jogadores[p].pos_x;
        aux_pos_y=lista_jogadores[p].pos_y;

        for(j=0;j<10;j++){
            if(strcmp(labirinto[aux_pos_x][aux_pos_y].jogadores_room[j].nome,"") != 0){
             //enviar palavra[] para labirinto[aux_pos_x][aux_pos_y].jogadores_room[j].pid
             //ler palavra no lado do cliente, ex: jogador '%s'(lista_jogadores[p].nome) diz tal tal tal (palavra[])
            }

        }
    }
    if(strcmp(palavra[i],"grita") == 0){
        for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        for(j=0;j<10;j++){
            if(strcmp(lista_jogadores[j].nome, "") != 0){
            //enviar palavra[] para lista_jogadores[j].pid
            //ler palavra no lado do cliente, ex: jogador '%s'(lista_jogadores[p].nome) grita tal tal tal (palavra[])
                }
            }
        }

    if(strcmp(palavra[i],"terminar") == 0){
        for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        if(lista_jogadores[p].id == 1){

        system("kill -11 /*pid filhos*/");  //avisa os filhos que o jogo acabou
        showgameresult();     //mostra o resultado do jogo: moedas dos users, e quem tem mais, ou seja, quem ganhou.
        game_running_flag = 0;
        return;
        }
        else{
            printf("\n[ERRO]: Você não é o jogador mestre! Não tem permissão para acabar com o jogo!\n");
        }
    }
    if(strcmp(palavra[i],"desistir") == 0){
        for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
                break;
        }
        if(lista_jogadores[p].flag_ingame == 0){
        printf("\n[ERRO]: O jogador não se encontra em jogo. \n");
        }
        if(lista_jogadores[p].id == 1){
            getmasterplayer(ign);  //eleger outro jogador mestre
        }
        aux_pos_x=lista_jogadores[p].pos_x;
        aux_pos_y=lista_jogadores[p].pos_y;

        for(j=0;j<DIM_LAB;j++){   //percorrer o labirinto  //desistir: remover jogador da lista, droppar os items
            for(k=0;k<DIM_LAB;k++){
                if(j == aux_pos_y && k == aux_pos_x){
                    player_dies(ign,j,k,p);
                }
            }
        }

        njogadores--;
        if(njogadores == 0){
            showgameresult();
            game_running_flag = 0;
        }



    }
    if(strcmp(palavra[i],"logout") == 0){
        for(j=0;j<njogadores;j++){
            if(strcmp(ign, lista_jogadores[j].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        if(lista_jogadores[j].flag_ingame == 1){
            printf("\n[ERRO]: Ainda está em jogo! Use 'sair' para sair do jogo e depois execute este comando!\n");
            return -1;
        }
        else{
            system("kill -8 /*pid do filho a matar*/");
        }
    }
    if(strcmp(palavra[i],"info") == 0){
        for(j=0;j<njogadores;j++){
            if(strcmp(ign, lista_jogadores[j].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        if(lista_jogadores[j].saude > 15.0){
            printf("Feelin' good!\n");
        }
        if(lista_jogadores[j].saude > 10.0 && lista_jogadores[j].saude < 15.0){
            printf("A few cuts n' bruises...\n");
        }
        if(lista_jogadores[j].saude < 10.0){
            printf("Need something to cure the pain... and fast...\n");
        }
        if(lista_jogadores[j].peso > 15.0){
            printf("I feel heavy...\n");
        }
        if(lista_jogadores[j].peso > 10.0 && lista_jogadores[j].peso < 15.0){
            printf("I'm okay weight-wise.\n");
        }
        if(lista_jogadores[j].peso < 10.0){
            printf("Light as a feather!\n");
        }
        printf("\nTenho %d moedas!", lista_jogadores[j].coin_count);
        printf("\nNo meu inventário tenho:\t");
        for(k=0;k<10;k++){
            printf("%s\t", lista_jogadores[j].inventory[k]);
            k++;
            printf("%s\t", lista_jogadores[j].inventory[k]);
            k++;
            printf("%s\n", lista_jogadores[j].inventory[k]);

        }

    }
    if(strcmp(palavra[i],"norte") == 0){
         for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        aux_pos_x=lista_jogadores[p].pos_x;
        aux_pos_y=lista_jogadores[p].pos_y;

        for(j=0;j<DIM_LAB;j++){   //percorrer o labirinto
            for(k=0;k<DIM_LAB;k++){
                if(j == aux_pos_y && k == aux_pos_x){ //encontrar a posicao do jogador
                    for(l=0;l<2;l++){
                        if(labirinto[j][k].monstros_room[l].agressividade  == 1){    //verificar se ha monstros agressivos na sala
                            dano = porrada(labirinto[j][k].monstros_room[l].forca_atk, lista_jogadores[p].def);  //calcular o dano entre monstro/jogador
                            lista_jogadores[p].saude -= dano;
                            if(lista_jogadores[p].saude <= 0){

                                player_dies(ign, j, k, p);
                            }
                            for(n=0;n<10;n++){
                                if(strcmp(ign, labirinto[j][k].jogadores_room[n].nome) == 0){
                                    labirinto[j][k].jogadores_room[n].saude = lista_jogadores[p].saude;  //fazer as alteracoes da hp em lista_jogadores e em labirinto[i][j].jogadores_room[n]
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
                    labirinto[j-1][k].jogadores_room[m].pos_x = j-1;
                    labirinto[j-1][k].jogadores_room[m].pos_y = k;
                    lista_jogadores[p].pos_x = j-1;
                    lista_jogadores[p].pos_y = k;
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
         for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        aux_pos_x=lista_jogadores[p].pos_x;
        aux_pos_y=lista_jogadores[p].pos_y;

        for(j=0;j<DIM_LAB;j++){   //percorrer o labirinto
            for(k=0;k<DIM_LAB;k++){
                if(j == aux_pos_y && k == aux_pos_x){ //encontrar a posicao do jogador
                    for(l=0;l<2;l++){
                        if(labirinto[j][k].monstros_room[l].agressividade  == 1){    //verificar se ha monstros agressivos na sala
                            dano = porrada(labirinto[j][k].monstros_room[l].forca_atk, lista_jogadores[p].def);  //calcular o dano entre monstro/jogador
                            lista_jogadores[p].saude -= dano;
                            if(lista_jogadores[p].saude <= 0){

                                player_dies(ign, j, k, p);
                            }
                            for(n=0;n<10;n++){
                                if(strcmp(ign, labirinto[j][k].jogadores_room[n].nome) == 0){
                                    labirinto[j][k].jogadores_room[n].saude = lista_jogadores[p].saude;  //fazer as alteracoes da hp em lista_jogadores e em labirinto[i][j].jogadores_room[n]
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
                    labirinto[j+1][k].jogadores_room[m].pos_x = j+1;
                    labirinto[j+1][k].jogadores_room[m].pos_y = k;
                    lista_jogadores[p].pos_x = j+1;
                    lista_jogadores[p].pos_y = k;
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
         for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        aux_pos_x=lista_jogadores[p].pos_x;
        aux_pos_y=lista_jogadores[p].pos_y;

        for(j=0;j<DIM_LAB;j++){   //percorrer o labirinto
            for(k=0;k<DIM_LAB;k++){
                if(j == aux_pos_y && k == aux_pos_x){ //encontrar a posicao do jogador
                    for(l=0;l<2;l++){
                        if(labirinto[j][k].monstros_room[l].agressividade  == 1){    //verificar se ha monstros agressivos na sala
                            dano = porrada(labirinto[j][k].monstros_room[l].forca_atk, lista_jogadores[p].def);  //calcular o dano entre monstro/jogador
                            lista_jogadores[p].saude -= dano;
                            if(lista_jogadores[p].saude <= 0){

                                player_dies(ign, j, k, p);
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
                    labirinto[j][k+1].jogadores_room[m].pos_x = j;
                    labirinto[j][k+1].jogadores_room[m].pos_y = k+1;
                    lista_jogadores[p].pos_x = j;
                    lista_jogadores[p].pos_y = k+1;
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
         for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
        aux_pos_x=lista_jogadores[p].pos_x;
        aux_pos_y=lista_jogadores[p].pos_y;

        for(j=0;j<DIM_LAB;j++){   //percorrer o labirinto
            for(k=0;k<DIM_LAB;k++){
                if(j == aux_pos_y && k == aux_pos_x){ //encontrar a posicao do jogador
                    for(l=0;l<2;l++){
                        if(labirinto[j][k].monstros_room[l].agressividade  == 1){    //verificar se ha monstros agressivos na sala
                            dano = porrada(labirinto[j][k].monstros_room[l].forca_atk, lista_jogadores[p].def);  //calcular o dano entre monstro/jogador
                            lista_jogadores[p].saude -= dano;
                            if(lista_jogadores[p].saude <= 0){

                                player_dies(ign, j, k, p);
                            }
                            for(n=0;n<10;n++){
                                if(strcmp(ign, labirinto[j][k].jogadores_room[n].nome) == 0){
                                    labirinto[j][k].jogadores_room[n].saude = lista_jogadores[p].saude;  //fazer as alteracoes da hp em lista_jogadores e em labirinto[i][j].jogadores_room[n]
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
                    labirinto[j][k-1].jogadores_room[m].pos_x = j;
                    labirinto[j][k-1].jogadores_room[m].pos_y = k-1;
                    lista_jogadores[p].pos_x = j;
                    lista_jogadores[p].pos_y = k-1;
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
         if(strcmp(palavra[i],"ver") == 0){
         for(p=0;p<njogadores;p++){
            if(strcmp(ign, lista_jogadores[p].nome) == 0) // ign é o username INGAME do jogador, é diferente do username utilizado pelo cliente! (possivelmente enviado por fifos)
            break;
        }
         aux_pos_x=lista_jogadores[p].pos_x;
         aux_pos_y=lista_jogadores[p].pos_y;


         if(aux == 1){
             if(lista_jogadores[p].id_sala == 1){

                printf("%s\n", descricao[9]);
             }
             else{

                aux_rand = random_number(0,8);
                printf("%s\n", descricao[aux_rand]);

            }
            for(j=0;j<DIM_LAB;j++){   //percorrer o labirinto
                for(k=0;k<DIM_LAB;k++){
                    if(j == aux_pos_y && k == aux_pos_x){
                     printf("Jogadores na sala: \n");
                     for(l=0;l<10;l++){
                        if(strcmp(labirinto[j][k].jogadores_room[l].nome, "") != 0)
                            printf("%s\n", labirinto[j][k].jogadores_room[l].nome);
                     }
                     printf("Monstros da sala: \n");
                     for(l=0;l<2;l++){
                        if(strcmp(labirinto[j][k].monstros_room[l].nome,"") != 0)
                            printf("%s\n", labirinto[j][k].monstros_room[l].nome);
                     }
                     printf("Items da sala: \n");
                     for(l=0;l<5;l++){
                        if(strcmp(labirinto[j][k].items_room[l].nome,"") != 0)
                            printf("%s\n", labirinto[j][k].items_room[l].nome);
                     }
                     printf("Saídas existentes:  \n");
                     if(labirinto[j][k].p_norte == 1)
                        printf(" > Norte \n");
                     if(labirinto[j][k].p_este == 1)
                        printf(" > Este \n");
                     if(labirinto[j][k].p_sul == 1)
                        printf(" > Sul \n");
                    if(labirinto[j][k].p_oeste == 1)
                        printf(" > Oeste \n");

                    }
                }
            }
         }

         if(aux == 2){
         printf("Acerca de '%s'...\n", palavra[i+1]);
         for(l=0;l<9;l++){
            if(strcmp(palavra[i+1], lista_jogadores[l].nome) == 0){
                printf("Forca de ataque: %d\n", lista_jogadores[l].atk);
                printf("Defesa: %d\n", lista_jogadores[l].def);
                printf("Saúde: %.1f\n", lista_jogadores[l].saude);
            }
         }
         for(j=0;j<DIM_LAB;j++){   //percorrer o labirinto
            for(k=0;k<DIM_LAB;k++){
                if(j == aux_pos_y && k == aux_pos_x){
                    for(m=0;m<5;m++){
                        if(strcmp(palavra[i+1], labirinto[j][k].items_room[m].nome) == 0){
                            printf("Nome: %s\n", labirinto[j][k].items_room[m].nome);
                            printf("Ataque: %d\n", labirinto[j][k].items_room[m].forca_atk);
                            printf("Defesa: %d\n", labirinto[j][k].items_room[m].forca_def);
                            printf("Peso: %.1f\n", labirinto[j][k].items_room[m].peso);
                                    }
                                }
                            }
                        }
                    }
        for(j=0;j<DIM_LAB;j++){   //percorrer o labirinto
            for(k=0;k<DIM_LAB;k++){
                if(j == aux_pos_y && k == aux_pos_x){
                    for(m=0;m<2;m++){
                        if(strcmp(palavra[i+1], labirinto[j][k].monstros_room[m].nome) == 0){
                            printf("Nome: %s\n",labirinto[j][k].monstros_room[m].nome);
                            printf("Forca ataque: %d\n", labirinto[j][k].monstros_room[m].forca_atk);
                            printf("Forca defesa: %d\n", labirinto[j][k].monstros_room[m].forca_def);
                            printf("Saúde: %.1f\n", labirinto[j][k].monstros_room[m].saude);
                        }
                    }
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
        if(strcmp(lista_jogadores[i].nome, "") != 0){
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
            id_aux3++;
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



