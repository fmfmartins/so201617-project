/*
// Operações sobre contas, versao 1
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
*/

#ifndef CONTAS_H
#define CONTAS_H

#include <stdio.h>
#include <stdlib.h>

#define NUM_CONTAS 10
#define TAXAJURO 0.1
#define CUSTOMANUTENCAO 1
#define MAX_NUM_FILHOS 20
#define ATRASO 1

extern int sigusr1flag;
extern FILE* logfile;
int contasSaldos[NUM_CONTAS];
pthread_mutex_t accountsMutexes[NUM_CONTAS];
pthread_mutex_t logMutex;


void inicializarContas();
int contaExiste(int idConta);
int debitar(int idConta, int valor);
int creditar(int idConta, int valor);
int lerSaldo(int idConta);
int transferir(int idContaOrigem,int idContaDestino,int valor);
void simular(int numAnos);

#endif
