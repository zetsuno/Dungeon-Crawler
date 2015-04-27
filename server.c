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




typedef struct dados_players
{
    int saude; //start = 20, Max = 30
    int peso; // Max = 20
    int atk;
    int def;
    char inventory[20][20];
    int coin_count = 0; //contador de moedas
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

monstros tabela_monstros[5]
{
 [0] = {"morcego", 1, 4, 3, 4, 4, 5, 10, 1, 1,"NULL" },
 [1] = {"escorpiao", 1, 7, 5, 7, 7, 9, 10, 1, 0, "moeda"},
 [2] = {"lobisomem", 5, 7, 5, 7, 7, 9, 4, 1, 1, "faca"},
 [3] = {"pedobear", 8, 10, 10, 12, 10, 2, 0, 0, "faca moeda"},  //quando é mais que 1 item, podemos usar o strtok pra dividir isto bem, e mais facilmente
 [4] = {"b0ss", 10, 12, 15, 15, 15, 15, 1, 0, 0, "moeda moeda moeda moeda moeda"};
}

items tabela_items[8]
{
 [0] = {"sandes", 0.5, 10, 0, 0, 1, 3, NULL},
 [1] = {"aspirina", 0.1, 20, 0, 0, 1, 1, NULL},
 [2] = {"xarope", 1.0, 4, 0, 0, 1, 4, NULL},
 [3] = {"faca", 2.0, 5, 5, 0, NULL, NULL, NULL},
 [4] = {"espada", 8.0, 3, 8, 2, NULL, NULL, 2},
 [5] = {"granada", 1.0, 2, 30, 0, 1, -5, NULL},
 [6] = {"escudo", 4.0, 4, 5, 5, NULL, NULL, 5},
 [7] = {"moeda", 0.1, 5, 0, 0, NULL, NULL, NULL};
}

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





void main(){




}


