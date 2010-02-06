/***************************************************************************
 *            agomaster.c
 *
 *  Thu Dec 19 18:34:30 2006
 *  Copyright  2005,06  Marcelo Soares Souza - CEBACAD (http://www.cebacad.net)
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
 Last Change -  Thu Jul 27 20:56:30 2006
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
#endif
#include "agoft.h"

// Variaveis Globais
static int si_myidMaster, si_numprocsMaster;

// Inicia o Mestre (Threads)
void agoMasterInit(int myid, int numprocs)
{
  si_myidMaster = myid;
  si_numprocsMaster = numprocs;

#if TURN_ON_OFF_FAIL == 1
  reconfigWorker_ago_ft = FLAG_STOP;
  masterSocketLoop_ago_ft = waitWorker_ago_ft = FLAG_START;
#endif

  // Inicia Tres Threads - Mestre, Servidor de Sinal e CheckWorker
#if TURN_ON_OFF_FAIL == 1
  int  i_mPtrThr1, i_mPtrThr2, i_mPtrThr3;
  pthread_t masterThread1, masterThread2, masterThread3;

  i_mPtrThr1 = pthread_create(&masterThread1, NULL, agoSendSignalMaster, (void *) si_numprocsMaster);
  i_mPtrThr2 = pthread_create(&masterThread2, NULL, agoCheckStatusWorker, (void *) si_numprocsMaster);
  i_mPtrThr3 = pthread_create(&masterThread3, NULL, agoMasterProcess, NULL);

  pthread_join(masterThread1, NULL);
  pthread_join(masterThread2, NULL);
  pthread_join(masterThread3, NULL);
#else
  (void) agoMasterProcess(NULL);
#endif

  agoFinalize();
}

void *agoMasterProcess()
{

  int i_workerId = 1, i_numBlocks = (NUM_BLOCKS - 1), i_offSetidPackage = 1, i_numBlocksOrig = NUM_BLOCKS,
                                    i_idFail = 0;

  unsigned int ui_sync1 = 0, ui_sync2 = 0, ui_loopMaster = FLAG_START, ui_timeSignal = 0, ui_workerList[NUM_WORKERS];

  double d_masterTime100 = 0.0, da_masterTime[4] = {0, 0, 0, 0}, d_countBlock = 0.0;

#if TURN_ON_OFF_FAIL == 1
  int    i_idFailTmp = 0, i_falhaPerc = 0;
  double d_timeFail = 0.0;
#endif

  Matrix *d_matrixA, *d_matrixB, *d_matrixC;

  agoDataPackageStr *agoDataPackageMaster;
  agoLogStr *agoLog;

  agoLog = (agoLogStr *) malloc (sizeof(agoLogStr));
  agoDataPackageMaster = (agoDataPackageStr *) calloc (sizeof(agoDataPackageStr), NUM_BLOCKS);

#ifdef VERBOSE
  if ((agoDataPackageMaster == NULL) || (agoLog == NULL)) {
    snprintf(agoLog->logMsg, LOG_SIZE, "(agoMasterProcess) - Erro Alocando Mem. agoDataPackageMaster ou agoLog - VERBOSE");
    registerLog(agoLog,  si_myidMaster);
  }
#endif

  startLog(agoLog, si_myidMaster);

  // Inicia o Pacote de Dados a serem processados
  // Informacoes Gerais do Processamento / Inicializando o Mestre
  (void) snprintf(agoLog->logMsg, LOG_SIZE,
                  "Tamanho Matriz: %d - Tamanho Pacote (Chunk): %d - Num de Pacotes: %d - Quantidade Workers: %d",
                  MAX_MATRIX_SIZE, CHUNKSIZE, i_numBlocks, (si_numprocsMaster - 1));
  registerLog(agoLog, si_myidMaster);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE,
                  "Tamanho Matriz: %d - %d bytes", sizeof(*d_matrixA), sizeof(*d_matrixB));
  registerLog(agoLog, si_myidMaster);
#endif

  // Aloca Memoria para as Matrizes
  d_matrixA = (Matrix *) malloc ((MAX_MATRIX_SIZE * MAX_MATRIX_SIZE) * sizeof(double));
  d_matrixB = (Matrix *) malloc ((MAX_MATRIX_SIZE * MAX_MATRIX_SIZE) * sizeof(double));
  d_matrixC = (Matrix *) malloc ((MAX_MATRIX_SIZE * MAX_MATRIX_SIZE) * sizeof(double));

  snprintf(agoLog->logMsg, LOG_SIZE, "Alocado Memoria para as Matrizes");
  registerLog(agoLog,  si_myidMaster);

#ifdef VERBOSE
  if ((d_matrixA == NULL) || (d_matrixB == NULL) || (d_matrixC == NULL)) {
    snprintf(agoLog->logMsg, LOG_SIZE, "(agoMasterProcess) - Erro Alocando Mem. Matrizes (A, B, C) - VERBOSE");
    registerLog(agoLog,  si_myidMaster);
  }
#endif

  matrixCreate(d_matrixA, d_matrixB, (MAX_MATRIX_SIZE * MAX_MATRIX_SIZE), si_myidMaster, agoLog);

#if TURN_ON_OFF_FAIL == 1
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Tolerancia a Falhas Ativo - Mandando Sinal para %s\n", MASTER_ADDR_AGO_FT);
  registerLog(agoLog, si_myidMaster);
#else
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Tolerancia a Falhas Nao Ativo\n");
  registerLog(agoLog, si_myidMaster);
#endif

  // Informacoes Gerais do Processamento / Inicializando o Mestre
  agoInitDataPackage(agoDataPackageMaster, NUM_BLOCKS, si_myidMaster, agoLog);

  i_workerId = 1;
  while (i_workerId < i_offSetidPackage) {
    agoDataPackageMaster[i_workerId].status = FLAG_DONE;
    i_workerId++;
  }

  ui_workerList[0] = 0;

  // Lista de Trabalhadores
  while (i_workerId < si_numprocsMaster) {
    ui_workerList[i_workerId] = i_workerId;
    i_workerId++;
  }

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoWorkerProcess) - Lista de Workers Concluida %d (%d) - (VERBOSE)",
                  i_workerId, si_numprocsMaster);
  registerLog(agoLog, si_myidMaster);

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoMasterProcess) - Numero de Blocos %d - (VERBOSE)", i_numBlocks);
  registerLog(agoLog, si_myidMaster);
#endif

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "Iniciando Contagem do Tempo do Mestre Local");
  registerLog(agoLog, si_myidMaster);

  // Inicio Contagem de Tempo do Processo Mestre
  d_masterTime100 = MPI_Wtime();

  // Inicio Tarefa Mestre
  do {
    while (i_numBlocks) {
      // Sinaliza Inicio de Trabalho aos Workers
      agoSignalWorkers(ui_workerList, NUM_WORKERS, FLAG_START, 1, si_myidMaster, workerStatus_ago_ft, agoLog);

      (void) snprintf(agoLog->logMsg, LOG_SIZE, "Iniciando Envio de Pacotes para Workers (%d)\n",  (si_numprocsMaster - 1));
      registerLog(agoLog, si_myidMaster);

      // Envio de Dados para Trabalhadores
      agoSendDataToWorker(agoDataPackageMaster, i_numBlocksOrig, d_matrixA, d_matrixB, &i_offSetidPackage,
                          ui_workerList, NUM_WORKERS, 1, i_idFail, si_myidMaster, workerStatus_ago_ft, agoLog);


      (void) snprintf(agoLog->logMsg, LOG_SIZE, "Bloco(s) Distribuido(s) para os Workers (%d)\n",  (si_numprocsMaster - 1));
      registerLog(agoLog, si_myidMaster);

#if TURN_ON_OFF_FAIL == 1
      if (i_idFail > 0) {
        i_falhaPerc = ui_timeSignal;
        d_timeFail = (MPI_Wtime() - d_masterTime100);

        (void) snprintf(agoLog->logMsg, LOG_SIZE, "Introducao da Falha Completa - worker id %d no tempo %f",  i_idFail, d_timeFail);
        registerLog(agoLog, si_myidMaster);

        sleep(WAIT_TIME_FAIL_AGO_FT);

        d_masterTime100 = (d_masterTime100 - WAIT_TIME_FAIL_AGO_FT);
        i_idFailTmp = i_idFail;
        i_idFail = 0;
      }
#endif

      i_workerId = 1;
      while (i_workerId < si_numprocsMaster) {
#ifdef VERBOSE
        (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoMasterProcess) - worker id %d (%d) - (VERBOSE)",
                        i_workerId, ui_workerList[i_workerId]);
        registerLog(agoLog, si_myidMaster);
#endif

        agoRecvDataFromWorker(agoDataPackageMaster, d_matrixC, ui_workerList, si_myidMaster, agoLog);

        d_countBlock++;
        i_workerId++;

        agoGetCount(&ui_timeSignal, i_numBlocksOrig, d_countBlock, d_masterTime100, da_masterTime, si_myidMaster, agoLog);

#ifdef VERBOSE
        (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoMasterProcess) - Blocos Recebidos %f - worker id %d - (VERBOSE)",
                        d_countBlock, i_workerId);
        registerLog(agoLog, si_myidMaster);
#endif
      }

      i_numBlocks = i_numBlocks - (si_numprocsMaster - 1);

#if TURN_ON_OFF_FAIL == 1
      if (reconfigWorker_ago_ft == 1) {
        (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Ocorreu uma Falha: %d Ativos", (si_numprocsMaster - 1));
        registerLog(agoLog, si_myidMaster);
      }
#endif

      (void) snprintf(agoLog->logMsg, LOG_SIZE, "Faltam %d Bloco(s) a serem Distribuido(s) para os Workers (%d)\n",
                      i_numBlocks, (si_numprocsMaster - 1));
      registerLog(agoLog, si_myidMaster);

      if ((i_numBlocks < (si_numprocsMaster - 1)) && (i_numBlocks > 0)) {
        if ((ui_sync1 == 0)
#if TURN_ON_OFF_FAIL == 1
            && (reconfigWorker_ago_ft == 0)
#endif
           ) {
          ui_sync1 = ui_sync2 = (i_numBlocks + 1);

          agoSignalWorkers(ui_workerList, ui_sync2, FLAG_START, 1, si_myidMaster, workerStatus_ago_ft, agoLog);

          (void) snprintf(agoLog->logMsg, LOG_SIZE, "Iniciando Envio de Pacotes para Workers (%d)\n",  (si_numprocsMaster - 1));
          registerLog(agoLog, si_myidMaster);

          i_idFail = 0;
          agoSendDataToWorker(agoDataPackageMaster, i_numBlocksOrig, d_matrixA, d_matrixB, &i_offSetidPackage,
                              ui_workerList, ui_sync2, 1, i_idFail, si_myidMaster, workerStatus_ago_ft, agoLog);


          (void) snprintf(agoLog->logMsg, LOG_SIZE, "Bloco(s) Distribuido(s) para os Workers (%d)\n",  (si_numprocsMaster - 1));
          registerLog(agoLog, si_myidMaster);

          i_workerId = 1;

          while (i_workerId < (int) ui_sync2) {
            agoRecvDataFromWorker(agoDataPackageMaster, d_matrixC, ui_workerList, si_myidMaster, agoLog);
            d_countBlock++;
            i_workerId++;

            agoGetCount(&ui_timeSignal, i_numBlocksOrig, d_countBlock, d_masterTime100, da_masterTime, si_myidMaster, agoLog);
          }
          i_numBlocks = 0;
        }
#if TURN_ON_OFF_FAIL == 1
        if (reconfigWorker_ago_ft == 1) {
          i_numBlocks = 0;
        }
#endif
      }

#if TURN_ON_OFF_FAIL == 1
      i_idFail = agoPutFaultOnWorker(ui_timeSignal, agoLog);
#endif
    }
    // Termina o Thread para Verificar Falhas atraves de Socket
#if TURN_ON_OFF_FAIL == 1
    syncWorker_ago_ft = waitWorker_ago_ft = FLAG_STOP;
#endif

    // Se foi detectado erro (reconfigWorker_ago_ft = 1) reconfigure o Cluster
#if TURN_ON_OFF_FAIL == 1
    if (reconfigWorker_ago_ft == 1) {
      agoReconfigCluster(agoDataPackageMaster, d_matrixA, d_matrixB, d_matrixC, ui_workerList, i_numBlocksOrig, agoLog);
    }
#endif

    ui_loopMaster = FLAG_STOP;

  } while (ui_loopMaster);

  agoSignalWorkers(ui_workerList, NUM_WORKERS, FLAG_STOP, 1, si_myidMaster, workerStatus_ago_ft, agoLog);

  // Finaliza Contagem de Tempo do Processo Mestre
  d_masterTime100 = (MPI_Wtime() - d_masterTime100);

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "Finalizado a Contagem do Tempo (%f) do Mestre Local\n", d_masterTime100);
  registerLog(agoLog, si_myidMaster);

#if TURN_ON_OFF_FAIL == 1
  // Termina o Thread o i_syncMaster (Servidor Sinalizacao)
  masterSocketLoop_ago_ft = FLAG_STOP;
#endif

  free(d_matrixA);
  free(d_matrixB);
  free(d_matrixC);
  free(agoDataPackageMaster);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoMasterProcess) - Liberado (free) memoria alocada (A, B, C) - (VERBOSE)");
  registerLog(agoLog, si_myidMaster);
#endif

  (void) snprintf(agoLog->logMsg, LOG_SIZE,
                  "Tempo de Processamento\n - Tempo 25%%: %f\n - Tempo 50%%: %f\n - Tempo 75%%: %f\n - Tempo 95%%: %f",
                  da_masterTime[PERCENT_25], da_masterTime[PERCENT_50], da_masterTime[PERCENT_75], da_masterTime[PERCENT_95]);
  registerLog(agoLog, si_myidMaster);

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "Finalizado - Tempo Total Local (100%%): %f", d_masterTime100);
  registerLog(agoLog, si_myidMaster);

#if TURN_ON_OFF_FAIL == 1
  if (reconfigWorker_ago_ft == 1) {
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "Falha no id %d introduzida com (%d%%) do trabalho no tempo: %f",
                    i_idFailTmp, i_falhaPerc, d_timeFail);
    registerLog(agoLog, si_myidMaster);

    (void) snprintf(agoLog->logMsg, LOG_SIZE, "Finalizado - Tempo Total (100%%) Com Falha: %f", d_masterTime100);
    registerLog(agoLog, si_myidMaster);
  }
#endif

#ifdef VERBOSE
#if TURN_ON_OFF_FAIL == 1
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoMasterProcess) - Tolerancia a Falha Ativo - (VERBOSE)");
  registerLog(agoLog, si_myidMaster);
#else
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoMasterProcess) - Tolerancia a Falha Nao Ativo - (VERBOSE)");
  registerLog(agoLog, si_myidMaster);
#endif

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoMasterProcess) - Chamando MPI_Finalize - (VERBOSE)");
  registerLog(agoLog, si_myidMaster);
#endif

  closeLog(agoLog);
  free(agoLog);

  return(NULL);
}

/* agomaster.c */
/*  Under the Terms of the GPL - 2005,06 - Marcelo Soares Souza - (http://marcelo.cebacad.net) */
