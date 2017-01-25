#include "contas.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h>

#define atrasar() sleep(ATRASO)

int forks = 0;

int contaExiste(int idConta) {
  return (idConta > 0 && idConta <= NUM_CONTAS);
}

void inicializarContas() {
  int i;
  for (i=0; i<NUM_CONTAS; i++)
    contasSaldos[i] = 0;
}

int debitar(int idConta, int valor) {
  atrasar();
  if (!contaExiste(idConta))
    return -1;
  pthread_mutex_lock(&accountsMutexes[idConta - 1]);
  if (contasSaldos[idConta - 1] < valor){
    pthread_mutex_unlock(&accountsMutexes[idConta - 1]);
    return -1;
  }
  contasSaldos[idConta - 1] -= valor;

  pthread_mutex_unlock(&accountsMutexes[idConta - 1]);
  pthread_mutex_lock(&logMutex);
  fprintf(logfile, "%lu: debitar %d %d\n", pthread_self(), idConta, valor);
  pthread_mutex_unlock(&logMutex);
  atrasar();
  return 0;
}

int creditar(int idConta, int valor) {
  atrasar();
  if (!contaExiste(idConta))
    return -1;
  pthread_mutex_lock(&accountsMutexes[idConta - 1]);
  contasSaldos[idConta - 1] += valor;
  pthread_mutex_unlock(&accountsMutexes[idConta - 1]);
  pthread_mutex_lock(&logMutex);
  fprintf(logfile, "%lu: creditar %d %d\n", pthread_self(), idConta, valor);
  pthread_mutex_unlock(&logMutex);
  atrasar();
  return 0;
}

int lerSaldo(int idConta) {
  int saldo;
  atrasar();
  if (!contaExiste(idConta))
    return -1;
  pthread_mutex_lock(&accountsMutexes[idConta - 1]);
  saldo = contasSaldos[idConta - 1];
  pthread_mutex_unlock(&accountsMutexes[idConta - 1]);
  pthread_mutex_lock(&logMutex);
  fprintf(logfile, "%lu: lerSaldo %d\n", pthread_self(), idConta);
  pthread_mutex_unlock(&logMutex);
  return saldo;
}

int transferir(int idContaOrigem,int idContaDestino,int valor){
  int a,b;
  atrasar();
  if (!contaExiste(idContaOrigem) || !contaExiste(idContaDestino))
    return -1;
  if(idContaOrigem == idContaDestino){
    return -1;
  }
  else if (idContaOrigem < idContaDestino){
    a = idContaOrigem;
    b = idContaDestino;
  }
  else{
    a = idContaDestino;
    b = idContaOrigem;
  }
  pthread_mutex_lock(&accountsMutexes[a-1]);
  pthread_mutex_lock(&accountsMutexes[b-1]);
  if(contasSaldos[idContaOrigem - 1] < valor){
    pthread_mutex_unlock(&accountsMutexes[a-1]);
    pthread_mutex_unlock(&accountsMutexes[b-1]);
    return -1;
  }
  else
    contasSaldos[idContaOrigem - 1] = contasSaldos[idContaOrigem - 1] - valor;
    contasSaldos[idContaDestino - 1] = contasSaldos[idContaDestino - 1] + valor;

    pthread_mutex_unlock(&accountsMutexes[a-1]);
    pthread_mutex_unlock(&accountsMutexes[b-1]);
    pthread_mutex_lock(&logMutex);
    fprintf(logfile, "%lu: transferir %d %d %d\n", pthread_self(),
            idContaOrigem, idContaDestino, valor);
    pthread_mutex_unlock(&logMutex);
    return 0;
}

void simular(int numAnos) {
  int id, ano, saldonovo, saldoatual, diff;
  pid_t pid;
  fflush(logfile);
  pid = fork();
  /* printf("PID = %d\n", (int)pid); DEBUG PRINT */
	atrasar();
  if (pid == -1 || (forks == MAX_NUM_FILHOS)){
    perror("simular failed. new process not created.");
    exit(EXIT_FAILURE);
  }
  else if (pid == 0){ /* FILHO */
    char simfilename[50];
    int sim_fd;
    sprintf(simfilename, "i-banco-sim-%d.txt", getpid());
    sim_fd = open(simfilename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    dup2(sim_fd,1);
    fclose(logfile);
    for(ano=0; ano <= numAnos; ano++){
      if(!sigusr1flag){
        printf("SIMULACAO: ANO %d\n==================\n", ano);
    		for(id=0; id<NUM_CONTAS; id++){
          saldoatual = lerSaldo(id+1);
    			printf("Conta %d, Saldo %d\n",id+1,saldoatual);
          saldonovo = (saldoatual*(1+TAXAJURO)-CUSTOMANUTENCAO);
          diff = saldonovo - saldoatual;
          if (diff > 0) {
            creditar(id+1, diff);
          }
          else{
            debitar(id+1, -diff);
          }
    		}
        printf("\n"); /* aesthetic newline */
      }
      else {
        printf("Simulacao terminada por signal.\n");
        exit(EXIT_FAILURE);
      }
  	}
    exit(EXIT_SUCCESS);
  }
  else{ /* PAI */
    forks++;
    return;
  }
}
