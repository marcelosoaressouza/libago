/***************************************************************************
 *            agoworker.c
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

#include "mpi.h"
#include "agolog.h"
#include "agomatrix.h"
#include "ago.h"

#if TURN_ON_OFF_FAIL == 1
#include <pthread.h>
#include <unistd.h>
#include "agofault.h"
#include "agoft.h"
#endif


// Variaveis Globais
static int i_myidWorker, i_numprocsWorker;

// Inicia dois Threads - Worker e Sinalizacao ao Mestre
void agoWorkerInit(int myid, int numprocs)
{
  i_myidWorker = myid;
  i_numprocsWorker = numprocs;

#if TURN_ON_OFF_FAIL == 1
  masterFail_ago_ft = FLAG_STOP;
  workerSocketLoop_ago_ft = waitMaster_ago_ft = FLAG_START;
  syncMaster_ago_ft = masterStatus_ago_ft = 0;
#endif

#if TURN_ON_OFF_FAIL == 1
  int  i_wPtrThr1, i_wPtrThr2, i_wPtrThr3;
  pthread_t workerThread1, workerThread2, workerThread3;

  i_wPtrThr1 = pthread_create(&workerThread1, NULL, agoWorkerProcess, NULL);
  i_wPtrThr2 = pthread_create(&workerThread2, NULL, agoSendSignalWorker, (void*) i_myidWorker);
  i_wPtrThr3 = pthread_create(&workerThread3, NULL, agoCheckStatusMaster, NULL);

  pthread_join(workerThread1, NULL);
  pthread_join(workerThread2, NULL);
  pthread_join(workerThread3, NULL);
#else
  (void) agoWorkerProcess(NULL);
#endif

  agoFinalize();
}

// Funcao Worker Principal
void * agoWorkerProcess()
{
  char c_fileName[32];

  int i_aux1 = 1, i_signalFlag = FLAG_STOP, i_workerId = 0, i_masterId = 0, i_packageId = 0,
                                 i_numBlocksOrig = NUM_BLOCKS, i_listPckId[NUM_BLOCKS];
  unsigned int ui_workerList[NUM_WORKERS];

  double d_workerTime100 = 0.0;

  Matrix *d_matrixChunkA, *d_matrixChunkB, *d_matrixChunkC;
  agoDataPackageStr *agoDataPackageWorker;
  agoLogStr *agoLog;
  MPI_Status statusWorker;

  agoLog = (agoLogStr *) malloc (sizeof(agoLogStr));
  agoDataPackageWorker = (agoDataPackageStr *) calloc (sizeof(agoDataPackageStr), NUM_BLOCKS);

#ifdef VERBOSE
  if ((agoDataPackageWorker == NULL) || (agoLog == NULL)) {
    snprintf(agoLog->logMsg, LOG_SIZE, "(agoWorkerProcess) - Erro Alocando Mem. agoDataPackageWorker ou agoLog - VERBOSE");
    registerLog(agoLog,  i_myidWorker);
  }
#endif

  startLog(agoLog, i_myidWorker);

  snprintf(agoLog->logMsg, LOG_SIZE, "Iniciando Worker");
  registerLog(agoLog, i_myidWorker);

  d_matrixChunkA = (Matrix *) calloc ((CHUNKSIZE * MAX_MATRIX_SIZE), sizeof(double));
  d_matrixChunkB = (Matrix *) calloc ((CHUNKSIZE * MAX_MATRIX_SIZE), sizeof(double));
  d_matrixChunkC = (Matrix *) calloc ((CHUNKSIZE * MAX_MATRIX_SIZE), sizeof(double));

  snprintf(agoLog->logMsg, LOG_SIZE, "Alocando Memorias para Matrizes");
  registerLog(agoLog, i_myidWorker);

#ifdef VERBOSE
  if ((d_matrixChunkA == NULL) || (d_matrixChunkB == NULL) || (d_matrixChunkC == NULL)) {
    snprintf(agoLog->logMsg, LOG_SIZE, "(agoWorkerProcess) - Erro Alocando Mem. Matrizes (A, B, C) - VERBOSE");
    registerLog(agoLog,  i_myidWorker);
  }
#endif

  i_workerId = 1;
  while (i_workerId < NUM_WORKERS) {
    ui_workerList[i_workerId] = i_workerId;
    i_workerId++;
  }

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoWorkerProcess) - Lista de Workers Concludida %d (%d) - (VERBOSE)",
                  i_workerId, i_numprocsWorker);
  registerLog(agoLog, i_myidWorker);
#endif

  i_packageId = 1;
  while (i_packageId < i_numBlocksOrig) {
    i_listPckId[i_packageId] = 0;
    i_packageId++;
  }

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoWorkerProcess) - Estrutura i_listPckId Iniciada %d (%d) - (VERBOSE)",
                  i_packageId, i_numBlocksOrig);
  registerLog(agoLog,  i_myidWorker);
#endif

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg,
                  LOG_SIZE, "(agoWorkerProcess) - Aguardando Sinalizacao do Mestre para Iniciar Processo (%d) - (VERBOSE)",
                  i_signalFlag);
  registerLog(agoLog,  i_myidWorker);
#endif

  // Recebe Flag sinalizando o Inicio do Processo - i_signalFlag = FLAG_START
  MPI_Recv(&i_signalFlag, 1, MPI_INT, MPI_ANY_SOURCE, TAG_INFO, MPI_COMM_WORLD, &statusWorker);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoWorkerProcess) - worker id %d recebi flag %d - (VERBOSE)", i_myidWorker, i_signalFlag);
  registerLog(agoLog,  i_myidWorker);
#endif

  // Inicia Contagem de Tempo do Worker
  snprintf(agoLog->logMsg, LOG_SIZE, "Iniciei a Contagem do Tempo Total do Worker Local");
  registerLog(agoLog, i_myidWorker);

  d_workerTime100 = MPI_Wtime();

  // Enquanto o i_signalFlag for 1 receba blocos e processe
  while (i_signalFlag) {
    if ((i_signalFlag == FLAG_START) || (i_signalFlag == FLAG_START_FAIL)) {

      snprintf(agoLog->logMsg, LOG_SIZE, "Aguardando Pacotes do Mestre");
      registerLog(agoLog, i_myidWorker);

      i_packageId = agoRecvDataFromMaster(agoDataPackageWorker, i_numBlocksOrig, d_matrixChunkA, d_matrixChunkB,
                                          i_masterId, i_myidWorker, agoLog);

      // Multiplicao de Matrizes (Calcular Tempo)
      agoDataPackageWorker[i_packageId].workerTime = matrixMultiplyParallel(d_matrixChunkA, d_matrixChunkB, d_matrixChunkC,
          agoDataPackageWorker[i_packageId].id,
          agoDataPackageWorker[i_packageId].offSetA,
          agoDataPackageWorker[i_packageId].offSetB,
          agoDataPackageWorker[i_packageId].chunkToCalc,
          i_myidWorker, agoLog);

      if (i_signalFlag == FLAG_START_FAIL) {
#if TURN_ON_OFF_FAIL == 1
        agoSendDataToMasterFT(&agoDataPackageWorker[i_packageId], d_matrixChunkC, i_masterId, i_myidWorker, agoLog);
#endif
      } else {
        agoSendDataToMaster(&agoDataPackageWorker[i_packageId], d_matrixChunkC, i_masterId, i_myidWorker, agoLog);
      }

      (void) snprintf(c_fileName, LOG_SIZE, "d_matrixC%d", agoDataPackageWorker[i_packageId].id);
      matrixSaveToDisk(c_fileName, d_matrixChunkC, (CHUNKSIZE * MAX_MATRIX_SIZE), agoLog);
      registerLog(agoLog, i_myidWorker);

      i_listPckId[i_aux1] = agoDataPackageWorker[i_packageId].id;

#ifdef VERBOSE
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoWorkerProcess) - Pacote id %d (%d) %d - Processado- (VERBOSE)",
                      i_listPckId[i_aux1], agoDataPackageWorker[i_packageId].id, i_aux1);
      registerLog(agoLog,  i_myidWorker);
#endif

      i_aux1++;
    }

    if (i_signalFlag == FLAG_FAIL) {
      free(d_matrixChunkA);
      free(d_matrixChunkB);
      free(d_matrixChunkC);

#if TURN_ON_OFF_FAIL == 1
      workerSocketLoop_ago_ft = FLAG_STOP;
#endif

#if TURN_ON_OFF_FAIL == 1
      i_signalFlag = agoFaultToleranceWorker(agoDataPackageWorker, ui_workerList, i_listPckId, i_masterId,
                                             i_numBlocksOrig, &d_workerTime100, agoLog);
#endif
    }

    if ((i_signalFlag == FLAG_STOP) && (i_myidWorker == 1)) {
      snprintf(agoLog->logMsg, LOG_SIZE, "Finalizado Worker-Mestre");
      registerLog(agoLog,  i_myidWorker);
    } else {
      snprintf(agoLog->logMsg, LOG_SIZE, "Aguardando Novas Instrucoes");
      registerLog(agoLog,  i_myidWorker);

      i_signalFlag = agoWaitMasterEnd(i_masterId, agoLog);
    }
  }

  free(agoDataPackageWorker);

  // Sai do Sinalizador para o Mestre
#if TURN_ON_OFF_FAIL == 1
  workerSocketLoop_ago_ft = FLAG_STOP;
#endif

  // Termina Contagem de Tempo do Worker
  d_workerTime100 = (MPI_Wtime() - d_workerTime100);
  snprintf(agoLog->logMsg, LOG_SIZE, "Finalizada a Contagem do Tempo Total do Worker Local");
  registerLog(agoLog,  i_myidWorker);

#if TURN_ON_OFF_FAIL == 1
  if (masterFail_ago_ft == FLAG_START) {
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Tempo Total Worker com Falha no Mestre: %f", d_workerTime100);
    registerLog(agoLog,  i_myidWorker);
  } else {
#endif
    free(d_matrixChunkA);
    free(d_matrixChunkB);
    free(d_matrixChunkC);

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "Tempo Total Worker Local:  %f", d_workerTime100);
    registerLog(agoLog,  i_myidWorker);
#if TURN_ON_OFF_FAIL == 1
  }
#endif

  snprintf(agoLog->logMsg, LOG_SIZE, "Finalizado");
  registerLog(agoLog,  i_myidWorker);

  closeLog(agoLog);
  free(agoLog);

  return(NULL);
}

int agoWaitMasterEnd(int i_masterId, agoLogStr *agoLog)
{

  int i_aux1 = 0, i_signalFlag = FLAG_START, ok = 0;
  MPI_Status statusWorker;

  while (!i_aux1) {
    MPI_Iprobe(MPI_ANY_SOURCE, TAG_INFO, MPI_COMM_WORLD, &i_aux1, &statusWorker);
#if TURN_ON_OFF_FAIL == 1
    if ((i_myidWorker == 1) && (masterFail_ago_ft == 1)) {
      i_signalFlag = FLAG_FAIL;
      ok = i_aux1 = 1;
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "Falha (%d) - Worker id 1 assumindo", i_signalFlag);
      registerLog(agoLog,  i_myidWorker);
    }
#endif
  }

  if (ok == 0) {
#ifdef VERBOSE
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoWaitMasterEnd) - Aguardando Flag - (VERBOSE)");
    registerLog(agoLog,  i_myidWorker);
#endif

    MPI_Recv(&i_signalFlag, 1, MPI_INT, MPI_ANY_SOURCE, TAG_INFO, MPI_COMM_WORLD, &statusWorker);

    if (i_signalFlag == FLAG_START) {
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "FLAG (%d) do Mestre %d - Iniciar Worker", i_signalFlag, i_masterId);
    } else if (i_signalFlag == FLAG_STOP) {
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "FLAG (%d) do Mestre (%d) - Terminar Worker", i_signalFlag, i_masterId);
    } else if (i_signalFlag == FLAG_FAIL) {
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "FLAG (%d) do Mestre (%d) - Houve Falha do Mestre - Re-Enviar Trabalho",
                      i_signalFlag, i_masterId);
    } else if (i_signalFlag == FLAG_START_FAIL) {
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "FLAG (%d) do Mestre (%d) - Houve Falha em um Worker", i_signalFlag, i_masterId);
    } else {
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "Flag Inesperada - ERROR");
    }
    registerLog(agoLog,  i_myidWorker);
  }

  return(i_signalFlag);
}

/* agoworker.c */
/*  Under the Terms of the GPL - 2005 - Marcelo Soares Souza - (http://marcelo.cebacad.net) */
