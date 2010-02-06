/***************************************************************************
 *            agoworkerfault.c
 *
 *  Thu Dec 06 18:34:30 2005
 *  Copyright  2005  Marcelo Soares Souza - CEBACAD (http://www.cebacad.net)
                     Josemar Rodrigues de Souza - CAOS (http://www.caos.uab.es/)
 *  E-Mail: marcelo at cebacad.net
            josemar at cebacad.net
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 Last Change -  Thu Jul 30 19:56:30 2006
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "mpi.h"
#include "agolog.h"
#include "agomatrix.h"
#include "ago.h"
#include "agofault.h"
#include "agoft.h"

static int i_myidWorker, i_numprocsWorker;

void agoSendRepeatToWorker(unsigned int *ui_workerList, int numWorkers, agoLogStr *agoLog)
{

  int i_signalFlag = FLAG_FAIL, i_workerId = 2;

  while (i_workerId < numWorkers) {
    MPI_Send(&i_signalFlag, 1, MPI_INT, ui_workerList[i_workerId], TAG_INFO, MPI_COMM_WORLD);

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Enviado Repeat para Worker ID: %d", ui_workerList[i_workerId]);
    registerLog(agoLog,  i_myidWorker);

    i_workerId++;
  }
}

// Funcao para Tolerar Falha do Mestre
int agoFaultToleranceWorker(agoDataPackageStr * agoDataPackageWorker, unsigned int *ui_workerList, int *listi_packageId,
                            int i_masterId, int numBlocks, double *d_workerTime, agoLogStr *agoLog)
{
  char c_fileName[32];
  int i_aux1 = 0, numPackageDone = 0, i_workerId = 1, i_signalFlag = 0, idFail = 0,
                                   i_packageId = 0, i_countPackageDone = 0, offSetidPackage = 0, i_numBlocksOrig = NUM_BLOCKS;
  unsigned int i_sync1 = 0, i_sync2 = 0;
  double faultTime = 0.0;

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Iniciando Procedimento de Tolerancia a Falhas");
  registerLog(agoLog,  i_myidWorker);

  i_masterId = masterFail_ago_ft = 1;

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoFaultToleranceWorker) - Lista de Pacotes Processados neste Worker (Total %d) - (VERBOSE)",
                  QUANT_BLOCKS);
  registerLog(agoLog,  i_myidWorker);
#endif

  i_aux1 = 1;
  while (i_aux1 <= NUM_BLOCKS) {
    if (listi_packageId[i_aux1] > 0) {
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "Pacote id %d (%d) foi processado neste Worker (%d)",
                      listi_packageId[i_aux1], i_aux1, i_myidWorker);
      registerLog(agoLog,  i_myidWorker);

      i_countPackageDone = i_countPackageDone + 1;
    }
    i_aux1++;
  }

  if (i_myidWorker == 1) { // Novo Mestre-Worker
    Matrix *d_matrixA, *d_matrixB, *d_matrixC;
    unsigned int timeSignal = 0;
    double d_countBlock = 0.0, da_workerTime[4] = {0, 0, 0, 0};

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Assumindo a Tarefa do Master no Tempo %f", (MPI_Wtime() - *d_workerTime));
    registerLog(agoLog,  i_myidWorker);

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Quantidade Workers: %d", (i_numprocsWorker - 2));
    registerLog(agoLog,  i_myidWorker);

    // Aloca Memoria para as Matrizes
    d_matrixA = (Matrix *) calloc ((MAX_MATRIX_SIZE * MAX_MATRIX_SIZE), sizeof(double));
    d_matrixB = (Matrix *) calloc ((MAX_MATRIX_SIZE * MAX_MATRIX_SIZE), sizeof(double));
    d_matrixC = (Matrix *) calloc ((MAX_MATRIX_SIZE * MAX_MATRIX_SIZE), sizeof(double));

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Alocado Memoria para as Matrizes");
    registerLog(agoLog,  i_myidWorker);

#ifdef VERBOSE
    if ((d_matrixA == NULL) || (d_matrixB == NULL) || (d_matrixC == NULL)) {
      snprintf(agoLog->logMsg, LOG_SIZE, "(agoWorkerProcess) - Erro Alocando Mem. Matrizes (A, B, C) - VERBOSE");
      registerLog(agoLog,  i_myidWorker);
    }
#endif

    // Cria as Matrizes a serem Calculadas pelos Workers
    matrixLoad(d_matrixA, d_matrixB, (MAX_MATRIX_SIZE * MAX_MATRIX_SIZE), agoLog);
    registerLog(agoLog,  i_myidWorker);

#ifdef VERBOSE
    (void) snprintf(agoLog->logMsg, LOG_SIZE,
                    "(agoFaultToleranceWorker) - Lista de Pacotes Processados neste Worker (Total %d) - (VERBOSE)",
                    QUANT_BLOCKS);
    registerLog(agoLog,  i_myidWorker);
#endif

    i_workerId = 1;
    while (i_workerId < NUM_WORKERS) {
      workerStatus_ago_ft[i_workerId] = 1;
      i_workerId++;
    }

    // Inicia Contagem de Tempo do Worker-Master
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Aqui comeca a Contar o Tempo do --> Novo Mestre (Worker) <--");
    registerLog(agoLog,  i_myidWorker);
    faultTime = MPI_Wtime();

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Requisitando Informacoes dos Workers");
    registerLog(agoLog,  i_myidWorker);

    agoSendRepeatToWorker(ui_workerList, NUM_WORKERS, agoLog);

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Aguardando Informacoes dos Workers");
    registerLog(agoLog,  i_myidWorker);

#ifdef VERBOSE
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoFaultToleranceWorker) - Recebendo Informacoes dos Workers - (VERBOSE)");
    registerLog(agoLog,  i_myidWorker);
#endif

    i_workerId = 2;
    while (i_workerId < i_numprocsWorker) {
      numPackageDone = (numPackageDone + agoRecvInfoFromWorker(i_workerId, agoLog));
      registerLog(agoLog,  i_myidWorker);
      i_workerId++;
    }

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Workers informaram que %d Pacotes foram completados", numPackageDone);
    registerLog(agoLog,  i_myidWorker);

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Aguardando Pacotes Processados nos Workers");
    registerLog(agoLog,  i_myidWorker);

    i_packageId = 1;
    while (i_packageId <= numPackageDone) {
      i_workerId = 2;
      while (i_workerId < i_numprocsWorker) {
        agoRecvDataFromWorkerFT(agoDataPackageWorker, d_matrixC, ui_workerList, i_workerId, i_myidWorker, agoLog);

        i_workerId++;
        i_packageId++;
        d_countBlock++;

        agoGetCount(&timeSignal, i_numBlocksOrig, d_countBlock, faultTime, da_workerTime, i_myidWorker, agoLog);
      }
    }

    offSetidPackage = ((numPackageDone + i_countPackageDone) + 1);
    numBlocks = ((numBlocks - numPackageDone) - i_countPackageDone);

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Faltam %d Blocos a serem distribuidos/processados", numBlocks);
    registerLog(agoLog,  i_myidWorker);

    while (numBlocks) {
      // Sinaliza Inicio de Trabalho aos Workers
      agoSignalWorkers(ui_workerList, i_numprocsWorker, FLAG_START, 2, i_myidWorker, workerStatus_ago_ft, agoLog);

      // Envio de Dados para Trabalhadores
      agoSendDataToWorker(agoDataPackageWorker, i_numBlocksOrig, d_matrixA, d_matrixB, &offSetidPackage, ui_workerList, i_numprocsWorker,
                          2, idFail, i_myidWorker, workerStatus_ago_ft, agoLog);

      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - %d Bloco(s) Distribuido(s) para os Workers (%d)",
                      (i_numprocsWorker - 2), (i_numprocsWorker - 2));
      registerLog(agoLog,  i_myidWorker);

      i_workerId = 2;
      while (i_workerId < i_numprocsWorker) {
        agoRecvDataFromWorkerFT(agoDataPackageWorker, d_matrixC, ui_workerList, i_workerId, i_myidWorker, agoLog);

        i_workerId++;
        d_countBlock++;

        agoGetCount(&timeSignal, i_numBlocksOrig, d_countBlock, faultTime, da_workerTime, i_myidWorker, agoLog);
      }

      numBlocks = numBlocks - (i_numprocsWorker - 2);

      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Faltam %d Bloco(s) a serem Distribuido(s) para os Workers (%d)",
                      numBlocks, (i_numprocsWorker - 2));
      registerLog(agoLog,  i_myidWorker);

      if ((numBlocks < (i_numprocsWorker - 2)) && (numBlocks > 0)) {
        if (i_sync1 == 0) {
          i_sync1 = i_sync2 = (numBlocks + 2);

          // Sinaliza Inicio de Trabalho aos Workers
          agoSignalWorkers(ui_workerList, i_sync2, FLAG_START, 2, i_myidWorker, workerStatus_ago_ft, agoLog);

          // Envio de Dados para Trabalhadores
          agoSendDataToWorker(agoDataPackageWorker, i_numBlocksOrig, d_matrixA, d_matrixB, &offSetidPackage, ui_workerList,
                              i_sync2, 2, idFail, i_myidWorker, workerStatus_ago_ft, agoLog);

          (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - %d Bloco(s) Distribuido(s) para os Workers (%d)",
                          (i_sync2 - 1), (i_sync2 - 1));
          registerLog(agoLog,  i_myidWorker);

          i_workerId = 2;
          while (i_workerId < (int) i_sync2) {
            agoRecvDataFromWorkerFT(agoDataPackageWorker, d_matrixC, ui_workerList, i_workerId, i_myidWorker, agoLog);
            i_workerId++;
          }
        }
        numBlocks = 0;
      }
    }

    // Termino Contagem de Tempo do Worker-Master
    faultTime = (MPI_Wtime() - faultTime);

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Aqui Termina de Contar o Tempo do --> Novo Mestre (WORKER) <--");
    registerLog(agoLog,  i_myidWorker);

    (void) snprintf(agoLog->logMsg, LOG_SIZE,
                    "(FT) - Tempo de Processamento\n - Tempo 25%%: %f\n - Tempo 50%%: %f\n - Tempo 75%%: %f\n - Tempo 95%%: %f",
                    da_workerTime[PERCENT_25], da_workerTime[PERCENT_50], da_workerTime[PERCENT_75], da_workerTime[PERCENT_95]);
    registerLog(agoLog,  i_myidWorker);

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Tempo da Tolerancia a Falha (Worker-Master): %f", faultTime);
    registerLog(agoLog,  i_myidWorker);

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Enviando Finalizacao aos Workers");
    registerLog(agoLog,  i_myidWorker);

    i_signalFlag = FLAG_STOP;
    agoSignalWorkers(ui_workerList, i_numprocsWorker, FLAG_STOP, 2, i_myidWorker, workerStatus_ago_ft, agoLog);

#ifdef VERBOSE
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoFaultToleranceWorker) - Liberando Memoria Alocada (A, B, C) - (VERBOSE)");
    registerLog(agoLog,  i_myidWorker);
#endif

    free(d_matrixA);
    free(d_matrixB);
    free(d_matrixC);
  } // Fim novo Mestre
  else { // Inicio Workers Assumindo Novo Mestre
    Matrix *d_d_matrixChunkA, *d_d_matrixChunkB, *d_d_matrixChunkC;

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Novo Master id %d - Enviando Informacoes", i_masterId);
    registerLog(agoLog, i_myidWorker);

#ifdef VERBOSE
    (void) snprintf(agoLog->logMsg, LOG_SIZE,
                    "(agoFaultToleranceWorker) - Enviando Informacoes (%d) ao Novo Master - (VERBOSE)", i_countPackageDone);
    registerLog(agoLog,  i_myidWorker);
#endif

    agoSendInfoToMasterFT(i_countPackageDone, i_myidWorker, i_masterId, agoLog);
    registerLog(agoLog,  i_myidWorker);

    d_d_matrixChunkC = (Matrix *) calloc ((CHUNKSIZE * MAX_MATRIX_SIZE), sizeof(double));
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Alocado Memoria para a Matriz C");
    registerLog(agoLog,  i_myidWorker);

#ifdef VERBOSE
    if (d_d_matrixChunkC == NULL) {
      snprintf(agoLog->logMsg, LOG_SIZE, "(agoFaultToleranceWorker) - Erro Alocando Mem. Matrizes (C) - VERBOSE");
      registerLog(agoLog,  i_myidWorker);
    }
#endif

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Enviando Pacotes armazenados (%d) para o Mestre-Trabalhador: %d",
                    i_countPackageDone, i_masterId);
    registerLog(agoLog,  i_myidWorker);

    i_packageId = 1;
    while (i_packageId <= i_countPackageDone) {
      (void) snprintf(c_fileName, LOG_SIZE, "d_matrixC%d", agoDataPackageWorker[listi_packageId[i_packageId]].id);
      matrixReadFromFile(c_fileName, d_d_matrixChunkC, (CHUNKSIZE * MAX_MATRIX_SIZE), i_myidWorker, agoLog);

      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Carregado do disco package id %d (%d - %d) - arquivo matriz c %s",
                      agoDataPackageWorker[listi_packageId[i_packageId]].id, listi_packageId[i_packageId], i_packageId, c_fileName);
      registerLog(agoLog,  i_myidWorker);

      agoSendDataToMasterFT(&agoDataPackageWorker[listi_packageId[i_packageId]], d_d_matrixChunkC, i_masterId, i_myidWorker, agoLog);
      i_packageId++;
    }

    free(d_d_matrixChunkC);
#ifdef VERBOSE
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoFaultToleranceWorker) - Liberada Memoria Matriz C - VERBOSE");
    registerLog(agoLog,  i_myidWorker);
#endif

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Fim Primeira Fase");
    registerLog(agoLog,  i_myidWorker);

    d_d_matrixChunkA = (Matrix *) calloc ((CHUNKSIZE * MAX_MATRIX_SIZE), sizeof(double));
    d_d_matrixChunkB = (Matrix *) calloc ((CHUNKSIZE * MAX_MATRIX_SIZE), sizeof(double));
    d_d_matrixChunkC = (Matrix *) calloc ((CHUNKSIZE * MAX_MATRIX_SIZE), sizeof(double));

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Alocado Memoria para as Matrizes (Chunk: A, B e C)");
    registerLog(agoLog,  i_myidWorker);

#ifdef VERBOSE
    if ((d_d_matrixChunkA == NULL) || (d_d_matrixChunkB == NULL) || (d_d_matrixChunkC == NULL)) {
      snprintf(agoLog->logMsg, LOG_SIZE, "(agoFaultToleranceWorker) - Erro Alocando Mem. Matrizes (A, B, C) - VERBOSE");
      registerLog(agoLog,  i_myidWorker);
    }
#endif

    while (i_signalFlag) {
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Aguardando Pacotes do Mestre");
      registerLog(agoLog,  i_myidWorker);

      i_packageId = agoRecvDataFromMaster(agoDataPackageWorker, i_numBlocksOrig, d_d_matrixChunkA, d_d_matrixChunkB, i_masterId, i_myidWorker, agoLog);
      registerLog(agoLog,  i_myidWorker);

      // Multiplicao de Matrizes (Calcular Tempo)
      (void) matrixMultiplyParallel(d_d_matrixChunkA, d_d_matrixChunkB, d_d_matrixChunkC,
                                    agoDataPackageWorker[i_packageId].id,
                                    agoDataPackageWorker[i_packageId].offSetA,
                                    agoDataPackageWorker[i_packageId].offSetB,
                                    agoDataPackageWorker[i_packageId].chunkToCalc,
                                    i_myidWorker, agoLog);

      agoSendDataToMasterFT(&agoDataPackageWorker[i_packageId], d_d_matrixChunkC, i_masterId, i_myidWorker, agoLog);

      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Aguardando Novas Instrucoes");
      registerLog(agoLog,  i_myidWorker);

      i_signalFlag = agoWaitMasterEnd(i_masterId, agoLog);
    } // Fim Workers Assumindo Novo Mestre

#ifdef VERBOSE
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoFaultToleranceWorker) - Liberando (free) Memoria Alocada (Chunk: A, B e C) - (VERBOSE)");
    registerLog(agoLog,  i_myidWorker);
#endif

    free(d_d_matrixChunkA);
    free(d_d_matrixChunkB);
    free(d_d_matrixChunkC);
  }

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Finalizando procedimentos de tolerancia a falhas");
  registerLog(agoLog,  i_myidWorker);

  return(i_signalFlag);
}

/* agoworkerfault.c */
/*  Under the Terms of the GPL - 2005 - Marcelo Soares Souza - (http://marcelo.cebacad.net) */
