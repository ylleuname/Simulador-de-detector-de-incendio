#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

void *sensor(void *args);
void *fogo_no_parquinho(void *args);
void *central(void *args);
void comunication(int s, int t, int thread_atual);

/*                            VARIÁVEIS GLOBAIS                        */

#define linha 30
#define coluna 30
#define THREADS 100
#define n_t 10
char mapa[linha][coluna];
FILE *arq;

// STRUCT DE MENSAGENS
typedef struct mensagem {
  long id;
  int coordenada_incendio_x;
  int coordenada_incendio_y;
  int hora;
  int minuto;
  int segundo;
  int indice;
  char horario[20];
  int mensagem_gravada;
} data;

// STRUCT DE CADA SENSOR
typedef struct threads {
  pthread_t thread;
  int coordenada_x;
  int coordenada_y;
  char horario[20];
  data mensagens;
} no;

no vetor_thread[THREADS];

// DECLARANDO THREADS
pthread_t thread_central;
pthread_t thread_bombeiro;

// VARIÁVEL K É O NÚMERO DE THREADS
int k = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                                                  Thread fogo */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Colocando fogo em posições aleatorias */

void *fogo_no_parquinho(void *args) {

  // inicializa as variáveis
  int posicao_linha = linha + 1, posicao_coluna = coluna + 1;
  srand(time(NULL));

  // gera posição aleatória para o incendio
  while (posicao_linha > linha) {
    posicao_linha = rand() % 100;
  }

  while (posicao_coluna > coluna) {
    posicao_coluna = rand() % 100;
  }

  // coloca o incêndio no mapa
  mapa[posicao_linha][posicao_coluna] = '@';

  // Imprimindo o mapa
  for (int i = 0; i < linha; i++) {
    printf("\n");
    for (int j = 0; j < coluna; j++) {
      printf(" %c ", mapa[i][j]);
    }
    printf("\n");
  }

  sleep(5);
  printf("\n");

  pthread_exit(NULL);

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                                                  Thread Central e Bombeiro */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Verifica a coordenada recebida e apaga o fogo */
void *bombeiro(void *args) {

  int *tab = (int *)args;

  int c_x = vetor_thread[*tab].mensagens.coordenada_incendio_x;
  int c_y = vetor_thread[*tab].mensagens.coordenada_incendio_y;

  // Apos apagar o fogo ele coloca o valor '-1' que funciona como um aviso que o
  // fogo foi apagado

  vetor_thread[*tab].mensagens.coordenada_incendio_x = -1;
  vetor_thread[*tab].mensagens.coordenada_incendio_y = -1;
  mapa[c_x][c_y] = '-';
  return 0;
};

/* Coloca as informações em incendio.log e chama o bombeiro caso necessário */

void *central(void *args) {

  // Roda todas as threads centrais procurando uma informação de incendio
  for (int m = 1; m < linha; m += 3) {
    for (int n = 1; n < coluna; n += 3) {
      if (m == linha - 2 || m == 1 || n == 1 || n == coluna - 2) {
        for (int contador = 0; contador < THREADS; contador++) {
          if (m == vetor_thread[contador].coordenada_x &&
              n == vetor_thread[contador].coordenada_y) {
            if (vetor_thread[contador].mensagens.coordenada_incendio_x != -1) {

              arq = fopen("incendio.log", "a+");

              if (arq == NULL) {
                printf("\nERRO ao abrir arquivo");
                exit(1);
              }
              if (vetor_thread[contador].mensagens.coordenada_incendio_x !=
                      -1 &&
                  vetor_thread[contador].mensagens.coordenada_incendio_y !=
                      -1 &&
                  mapa[vetor_thread[contador].mensagens.coordenada_incendio_x]
                      [vetor_thread[contador]
                           .mensagens.coordenada_incendio_y] != '-') {
                fprintf(
                    arq,
                    "Incendio: [%d] [%d]\nID: %lu\nHorario do incendio: %d: "
                    "%d: %d\n\n",
                    vetor_thread[contador].mensagens.coordenada_incendio_x,
                    vetor_thread[contador].mensagens.coordenada_incendio_y,
                    vetor_thread[contador].mensagens.id,
                    vetor_thread[contador].mensagens.hora - 3,
                    vetor_thread[contador].mensagens.minuto,
                    vetor_thread[contador].mensagens.segundo);

                fclose(arq);

                pthread_create(&thread_bombeiro, NULL, bombeiro, &contador);
                pthread_join(thread_bombeiro, NULL);

                printf(" ");
              }
            }
          }
        }
      }
    }
  }

  // pthread_exit(NULL);
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                                                  Função Comunicação */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Faz a comunicação entre as threads */

void comunication(int s, int t, int thread_atual) {

  // Cria variaveis temporarias para nao alterar 'thread_atual'
  int temps = s;
  int tempt = t;
  int tempnt = n_t;
  int tempthread = thread_atual;
  if (s == linha - 2 || s == 1 || t == 1 || t == coluna - 2) {
    return;
  } else {

    // Antes de iniciar cada if, o valor de 's' e 't' é resetado

    // sensor acima
    s = temps;
    t = tempt;
    tempthread = thread_atual;
    if (mapa[s - 3][t] == 'T') {

      // Enquando não chegar nas bordas
      do {
        // Cima

        vetor_thread[tempthread - n_t].mensagens.coordenada_incendio_x =
            vetor_thread[tempthread].mensagens.coordenada_incendio_x;
        vetor_thread[tempthread - n_t].mensagens.coordenada_incendio_y =
            vetor_thread[tempthread].mensagens.coordenada_incendio_y;
        vetor_thread[tempthread - n_t].mensagens.id =
            vetor_thread[tempthread].mensagens.id;
        vetor_thread[tempthread - n_t].mensagens.hora =
            vetor_thread[tempthread].mensagens.hora;
        vetor_thread[tempthread - n_t].mensagens.minuto =
            vetor_thread[tempthread].mensagens.minuto;
        vetor_thread[tempthread - n_t].mensagens.segundo =
            vetor_thread[tempthread].mensagens.segundo;
        vetor_thread[tempthread - n_t].mensagens.mensagem_gravada = 1;

        // Direita

        vetor_thread[tempthread + 1].mensagens.coordenada_incendio_x =
            vetor_thread[tempthread].mensagens.coordenada_incendio_x;
        vetor_thread[tempthread + 1].mensagens.coordenada_incendio_y =
            vetor_thread[tempthread].mensagens.coordenada_incendio_y;
        vetor_thread[tempthread + 1].mensagens.id =
            vetor_thread[tempthread].mensagens.id;

        vetor_thread[tempthread + 1].mensagens.hora =
            vetor_thread[tempthread].mensagens.hora;
        vetor_thread[tempthread + 1].mensagens.minuto =
            vetor_thread[tempthread].mensagens.minuto;
        vetor_thread[tempthread + 1].mensagens.segundo =
            vetor_thread[tempthread].mensagens.segundo;

        vetor_thread[tempthread + 1].mensagens.mensagem_gravada = 1;

        // Esquerda

        vetor_thread[tempthread - 1].mensagens.coordenada_incendio_x =
            vetor_thread[tempthread].mensagens.coordenada_incendio_x;
        vetor_thread[tempthread - 1].mensagens.coordenada_incendio_y =
            vetor_thread[tempthread].mensagens.coordenada_incendio_y;
        vetor_thread[tempthread - 1].mensagens.id =
            vetor_thread[tempthread].mensagens.id;

        vetor_thread[tempthread - 1].mensagens.hora =
            vetor_thread[tempthread].mensagens.hora;
        vetor_thread[tempthread - 1].mensagens.minuto =
            vetor_thread[tempthread].mensagens.minuto;
        vetor_thread[tempthread - 1].mensagens.segundo =
            vetor_thread[tempthread].mensagens.segundo;

        vetor_thread[tempthread - 1].mensagens.mensagem_gravada = 1;

        s -= 3;
        tempthread -= n_t;
      } while (s != linha - 2 && s != 1);
    }
    // sensor abaixo
    s = temps;
    t = tempt;
    tempthread = thread_atual;
    if (mapa[s + 3][t] == 'T') {
      do {

        // Baixo

        vetor_thread[tempthread + n_t].mensagens.coordenada_incendio_x =
            vetor_thread[tempthread].mensagens.coordenada_incendio_x;
        vetor_thread[tempthread + n_t].mensagens.coordenada_incendio_y =
            vetor_thread[tempthread].mensagens.coordenada_incendio_y;
        vetor_thread[tempthread + n_t].mensagens.id =
            vetor_thread[tempthread].mensagens.id;

        vetor_thread[tempthread + n_t].mensagens.hora =
            vetor_thread[tempthread].mensagens.hora;
        vetor_thread[tempthread + n_t].mensagens.minuto =
            vetor_thread[tempthread].mensagens.minuto;
        vetor_thread[tempthread + n_t].mensagens.segundo =
            vetor_thread[tempthread].mensagens.segundo;

        vetor_thread[tempthread + n_t].mensagens.mensagem_gravada = 1;

        // Direita
        vetor_thread[tempthread + 1].mensagens.coordenada_incendio_x =
            vetor_thread[tempthread].mensagens.coordenada_incendio_x;
        vetor_thread[tempthread + 1].mensagens.coordenada_incendio_y =
            vetor_thread[tempthread].mensagens.coordenada_incendio_y;
        vetor_thread[tempthread + 1].mensagens.id =
            vetor_thread[tempthread].mensagens.id;

        vetor_thread[tempthread + 1].mensagens.hora =
            vetor_thread[tempthread].mensagens.hora;
        vetor_thread[tempthread + 1].mensagens.minuto =
            vetor_thread[tempthread].mensagens.minuto;
        vetor_thread[tempthread + 1].mensagens.segundo =
            vetor_thread[tempthread].mensagens.segundo;

        vetor_thread[tempthread + 1].mensagens.mensagem_gravada = 1;

        // Esquerda

        vetor_thread[tempthread - 1].mensagens.coordenada_incendio_x =
            vetor_thread[tempthread].mensagens.coordenada_incendio_x;
        vetor_thread[tempthread - 1].mensagens.coordenada_incendio_y =
            vetor_thread[tempthread].mensagens.coordenada_incendio_y;
        vetor_thread[tempthread - 1].mensagens.id =
            vetor_thread[tempthread].mensagens.id;
        vetor_thread[tempthread - 1].mensagens.hora =
            vetor_thread[tempthread].mensagens.hora;
        vetor_thread[tempthread - 1].mensagens.minuto =
            vetor_thread[tempthread].mensagens.minuto;
        vetor_thread[tempthread - 1].mensagens.segundo =
            vetor_thread[tempthread].mensagens.segundo;
        vetor_thread[tempthread - 1].mensagens.mensagem_gravada = 1;

        tempthread += n_t;
        s += 3;
      } while (s != linha - 2 && s != 1);
    }
    // sensor da direita
    s = temps;
    t = tempt;
    tempthread = thread_atual;
    if (mapa[s][t + 3] == 'T') {
      do {

        // Direita

        vetor_thread[tempthread + 1].mensagens.coordenada_incendio_x =
            vetor_thread[tempthread].mensagens.coordenada_incendio_x;
        vetor_thread[tempthread + 1].mensagens.coordenada_incendio_y =
            vetor_thread[tempthread].mensagens.coordenada_incendio_y;
        vetor_thread[tempthread + 1].mensagens.id =
            vetor_thread[tempthread].mensagens.id;

        vetor_thread[tempthread + 1].mensagens.hora =
            vetor_thread[tempthread].mensagens.hora;
        vetor_thread[tempthread + 1].mensagens.minuto =
            vetor_thread[tempthread].mensagens.minuto;
        vetor_thread[tempthread + 1].mensagens.segundo =
            vetor_thread[tempthread].mensagens.segundo;

        vetor_thread[tempthread + 1].mensagens.mensagem_gravada = 1;

        // Cima

        vetor_thread[tempthread - n_t].mensagens.coordenada_incendio_x =
            vetor_thread[tempthread].mensagens.coordenada_incendio_x;
        vetor_thread[tempthread - n_t].mensagens.coordenada_incendio_y =
            vetor_thread[tempthread].mensagens.coordenada_incendio_y;
        vetor_thread[tempthread - n_t].mensagens.id =
            vetor_thread[tempthread].mensagens.id;

        vetor_thread[tempthread - n_t].mensagens.hora =
            vetor_thread[tempthread].mensagens.hora;
        vetor_thread[tempthread - n_t].mensagens.minuto =
            vetor_thread[tempthread].mensagens.minuto;
        vetor_thread[tempthread - n_t].mensagens.segundo =
            vetor_thread[tempthread].mensagens.segundo;

        vetor_thread[tempthread - n_t].mensagens.mensagem_gravada = 1;

        // Baixo

        vetor_thread[tempthread + n_t].mensagens.coordenada_incendio_x =
            vetor_thread[tempthread].mensagens.coordenada_incendio_x;
        vetor_thread[tempthread + n_t].mensagens.coordenada_incendio_y =
            vetor_thread[tempthread].mensagens.coordenada_incendio_y;
        vetor_thread[tempthread + n_t].mensagens.id =
            vetor_thread[tempthread].mensagens.id;

        vetor_thread[tempthread + n_t].mensagens.hora =
            vetor_thread[tempthread].mensagens.hora;
        vetor_thread[tempthread + n_t].mensagens.minuto =
            vetor_thread[tempthread].mensagens.minuto;
        vetor_thread[tempthread + n_t].mensagens.segundo =
            vetor_thread[tempthread].mensagens.segundo;

        vetor_thread[tempthread + n_t].mensagens.mensagem_gravada = 1;

        tempthread += 1;
        t += 3;
      } while (t != 1 && t != coluna - 2);
    }
    // sensor da esquerda
    s = temps;
    t = tempt;
    tempthread = thread_atual;
    if (mapa[s][t - 3] == 'T') {
      do {

        vetor_thread[tempthread - 1].mensagens.coordenada_incendio_x =
            vetor_thread[tempthread].mensagens.coordenada_incendio_x;
        vetor_thread[tempthread - 1].mensagens.coordenada_incendio_y =
            vetor_thread[tempthread].mensagens.coordenada_incendio_y;
        vetor_thread[tempthread - 1].mensagens.id =
            vetor_thread[tempthread].mensagens.id;

        vetor_thread[tempthread - 1].mensagens.hora =
            vetor_thread[tempthread].mensagens.hora;
        vetor_thread[tempthread - 1].mensagens.minuto =
            vetor_thread[tempthread].mensagens.minuto;
        vetor_thread[tempthread - 1].mensagens.segundo =
            vetor_thread[tempthread].mensagens.segundo;

        vetor_thread[tempthread - 1].mensagens.mensagem_gravada = 1;

        // Cima

        vetor_thread[tempthread - n_t].mensagens.coordenada_incendio_x =
            vetor_thread[tempthread].mensagens.coordenada_incendio_x;
        vetor_thread[tempthread - n_t].mensagens.coordenada_incendio_y =
            vetor_thread[tempthread].mensagens.coordenada_incendio_y;
        vetor_thread[tempthread - n_t].mensagens.id =
            vetor_thread[tempthread].mensagens.id;

        vetor_thread[tempthread - n_t].mensagens.hora =
            vetor_thread[tempthread].mensagens.hora;
        vetor_thread[tempthread - n_t].mensagens.minuto =
            vetor_thread[tempthread].mensagens.minuto;
        vetor_thread[tempthread - n_t].mensagens.segundo =
            vetor_thread[tempthread].mensagens.segundo;

        vetor_thread[tempthread - n_t].mensagens.mensagem_gravada = 1;

        // Baixo

        vetor_thread[tempthread + n_t].mensagens.coordenada_incendio_x =
            vetor_thread[tempthread].mensagens.coordenada_incendio_x;
        vetor_thread[tempthread + n_t].mensagens.coordenada_incendio_y =
            vetor_thread[tempthread].mensagens.coordenada_incendio_y;
        vetor_thread[tempthread + n_t].mensagens.id =
            vetor_thread[tempthread].mensagens.id;
        vetor_thread[tempthread + n_t].mensagens.hora =
            vetor_thread[tempthread].mensagens.hora;
        vetor_thread[tempthread + n_t].mensagens.minuto =
            vetor_thread[tempthread].mensagens.minuto;
        vetor_thread[tempthread + n_t].mensagens.segundo =
            vetor_thread[tempthread].mensagens.segundo;
        vetor_thread[tempthread + n_t].mensagens.mensagem_gravada = 1;

        tempthread -= 1;
        t -= 3;
      } while (t != 1 && t != coluna - 2);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                                                  Thread sensor */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *sensor(void *args) {

  // posicao do incendio
  int x = -1, y = -1;

  // Thread atual
  int *th = (int *)args;

  // procura os vizinhos de cada thread sensor
  for (int s = (vetor_thread[*th].coordenada_x) - 1;
       s <= (vetor_thread[*th].coordenada_x) + 1; s++) {
    for (int t = (vetor_thread[*th].coordenada_y) - 1;
         t <= (vetor_thread[*th].coordenada_y) + 1; t++) {
      if (mapa[s][t] == 'T') {

        // quando acha a thread verifica se há incendio ao redor
        if (mapa[s - 1][t - 1] == '@') {

          // Posição do incêndio
          x = s - 1;
          y = t - 1;

          // Recebendo informaçõess do incêndio
          vetor_thread[*th].mensagens.coordenada_incendio_x = x;
          vetor_thread[*th].mensagens.coordenada_incendio_y = y;
          vetor_thread[*th].mensagens.id = pthread_self();
          vetor_thread[*th].mensagens.mensagem_gravada = 0;

          // Informações o horário do incêndioo
          struct tm *data_hora_atual;
          time_t segundos;
          time(&segundos);
          data_hora_atual = localtime(&segundos);
          vetor_thread[*th].mensagens.hora = data_hora_atual->tm_hour;
          vetor_thread[*th].mensagens.minuto = data_hora_atual->tm_min;
          vetor_thread[*th].mensagens.segundo = data_hora_atual->tm_sec;

          // Chamando função comunication
          comunication(s, t, *th);
        }
        if (mapa[s - 1][t] == '@') {
          x = s - 1;
          y = t;
          vetor_thread[*th].mensagens.coordenada_incendio_x = x;
          vetor_thread[*th].mensagens.coordenada_incendio_y = y;
          vetor_thread[*th].mensagens.id = pthread_self();
          vetor_thread[*th].mensagens.mensagem_gravada = 0;

          struct tm *data_hora_atual;
          time_t segundos;
          time(&segundos);
          data_hora_atual = localtime(&segundos);
          vetor_thread[*th].mensagens.hora = data_hora_atual->tm_hour;
          vetor_thread[*th].mensagens.minuto = data_hora_atual->tm_min;
          vetor_thread[*th].mensagens.segundo = data_hora_atual->tm_sec;

          comunication(s, t, *th);
        }
        if (mapa[s - 1][t + 1] == '@') {
          x = s - 1;
          y = t + 1;
          vetor_thread[*th].mensagens.coordenada_incendio_x = x;
          vetor_thread[*th].mensagens.coordenada_incendio_y = y;
          vetor_thread[*th].mensagens.id = pthread_self();
          vetor_thread[*th].mensagens.mensagem_gravada = 0;

          struct tm *data_hora_atual;
          time_t segundos;
          time(&segundos);
          data_hora_atual = localtime(&segundos);
          vetor_thread[*th].mensagens.hora = data_hora_atual->tm_hour;
          vetor_thread[*th].mensagens.minuto = data_hora_atual->tm_min;
          vetor_thread[*th].mensagens.segundo = data_hora_atual->tm_sec;

          comunication(s, t, *th);
        }
        if (mapa[s][t - 1] == '@') {
          x = s;
          y = t - 1;
          vetor_thread[*th].mensagens.coordenada_incendio_x = x;
          vetor_thread[*th].mensagens.coordenada_incendio_y = y;
          vetor_thread[*th].mensagens.id = pthread_self();
          vetor_thread[*th].mensagens.mensagem_gravada = 0;

          struct tm *data_hora_atual;
          time_t segundos;
          time(&segundos);
          data_hora_atual = localtime(&segundos);
          vetor_thread[*th].mensagens.hora = data_hora_atual->tm_hour;
          vetor_thread[*th].mensagens.minuto = data_hora_atual->tm_min;
          vetor_thread[*th].mensagens.segundo = data_hora_atual->tm_sec;

          comunication(s, t, *th);
        }
        if (mapa[s][t + 1] == '@') {
          x = s;
          y = t + 1;
          vetor_thread[*th].mensagens.coordenada_incendio_x = x;
          vetor_thread[*th].mensagens.coordenada_incendio_y = y;
          vetor_thread[*th].mensagens.id = pthread_self();
          vetor_thread[*th].mensagens.mensagem_gravada = 0;

          struct tm *data_hora_atual;
          time_t segundos;
          time(&segundos);
          data_hora_atual = localtime(&segundos);
          vetor_thread[*th].mensagens.hora = data_hora_atual->tm_hour;
          vetor_thread[*th].mensagens.minuto = data_hora_atual->tm_min;
          vetor_thread[*th].mensagens.segundo = data_hora_atual->tm_sec;

          comunication(s, t, *th);
        }
        if (mapa[s + 1][t - 1] == '@') {
          x = s + 1;
          y = t - 1;
          vetor_thread[*th].mensagens.coordenada_incendio_x = x;
          vetor_thread[*th].mensagens.coordenada_incendio_y = y;
          vetor_thread[*th].mensagens.id = pthread_self();
          vetor_thread[*th].mensagens.mensagem_gravada = 0;

          struct tm *data_hora_atual;
          time_t segundos;
          time(&segundos);
          data_hora_atual = localtime(&segundos);
          vetor_thread[*th].mensagens.hora = data_hora_atual->tm_hour;
          vetor_thread[*th].mensagens.minuto = data_hora_atual->tm_min;
          vetor_thread[*th].mensagens.segundo = data_hora_atual->tm_sec;

          comunication(s, t, *th);
        }
        if (mapa[s + 1][t] == '@') {
          x = s + 1;
          y = t;
          vetor_thread[*th].mensagens.coordenada_incendio_x = x;
          vetor_thread[*th].mensagens.coordenada_incendio_y = y;
          vetor_thread[*th].mensagens.id = pthread_self();
          vetor_thread[*th].mensagens.mensagem_gravada = 0;

          struct tm *data_hora_atual;
          time_t segundos;
          time(&segundos);
          data_hora_atual = localtime(&segundos);
          vetor_thread[*th].mensagens.hora = data_hora_atual->tm_hour;
          vetor_thread[*th].mensagens.minuto = data_hora_atual->tm_min;
          vetor_thread[*th].mensagens.segundo = data_hora_atual->tm_sec;

          comunication(s, t, *th);
        }
        if (mapa[s + 1][t + 1] == '@') {
          x = s + 1;
          y = t + 1;
          vetor_thread[*th].mensagens.coordenada_incendio_x = x;
          vetor_thread[*th].mensagens.coordenada_incendio_y = y;
          vetor_thread[*th].mensagens.id = pthread_self();
          vetor_thread[*th].mensagens.mensagem_gravada = 0;

          struct tm *data_hora_atual;
          time_t segundos;
          time(&segundos);
          data_hora_atual = localtime(&segundos);
          vetor_thread[*th].mensagens.hora = data_hora_atual->tm_hour;
          vetor_thread[*th].mensagens.minuto = data_hora_atual->tm_min;
          vetor_thread[*th].mensagens.segundo = data_hora_atual->tm_sec;

          comunication(s, t, *th);
        }
      }
    }
  }

  // Saindo da Thread sensor
  pthread_exit(NULL);
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                                                  Main */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {

  // CRIAÇÃO DO MAPA
  for (int i = 0; i < linha; i++) {
    for (int j = 0; j < coluna; j++) {
      mapa[i][j] = '-';
    }
  }

  // INICIANDO LAÇO INFINITO

  while (1) {
    k = 0;

    // COLOCANDO OS SENSORES NO MAPA "T" E CRIANDO AS RESPECTIVAS THREADS
    int v[THREADS];
    for (int i = 0; i < THREADS; i++) {
      v[i] = i;
    }
    for (int i = 1; i < linha; i += 3) {
      for (int j = 1; j < coluna; j += 3) {

        if (mapa[i][j] == '@') {
          // Destruindo Thread caso incêndio no sensor
          int pthread_kill(pthread_t thread, int sig);
        } else {
          mapa[i][j] = 'T';
        }

        // Mensagens do incêncdio
        vetor_thread[k].coordenada_x = i;
        vetor_thread[k].coordenada_y = j;
        vetor_thread[k].mensagens.indice = k;
        vetor_thread[k].mensagens.coordenada_incendio_x = -1;
        vetor_thread[k].mensagens.coordenada_incendio_x = -1;
        vetor_thread[k].mensagens.mensagem_gravada = 0;

        // Thread sensor
        pthread_create(&(vetor_thread[k].thread), NULL, sensor, &v[k]);

        k++;
      }
    }
    k = 0;

    for (int j = 0; j < THREADS; j++) {
      pthread_join(vetor_thread[j].thread, NULL);
    }

    // Thread central
    pthread_create(&thread_central, NULL, central, NULL);
    pthread_join(thread_central, NULL);

    // Thread de incêndio
    pthread_t threads_fogo;
    pthread_create(&threads_fogo, NULL, fogo_no_parquinho, NULL);
    pthread_join(threads_fogo, NULL);
    sleep(5);
    system("clear");
  }
  return 0;
}