/***************************************************************************
 *            agocore.c
 *
 *  Thu Apr 19 16:34:30 2006
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
 Last Change -  Thu Jul 27 16:56:30 2006
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"
#include "agolog.h"
#include "agomatrix.h"
#include "ago.h"

static void makeNewTypeDataPackage();

// Informacoes do Inicio do MPI
int agoInit(int argc, char **argv, int *numprocs)
{
  int i_myid = -1;

  if (MPI_Init(&argc, &argv) == MPI_SUCCESS) {
    (void) MPI_Comm_size(MPI_COMM_WORLD, numprocs);
    (void) MPI_Comm_rank(MPI_COMM_WORLD, &i_myid);

    // Cria Tipo MPI agoDataPackage
    makeNewTypeDataPackage();

  }

  return (i_myid);
}

void agoFinalize()
{
  (void) MPI_Type_free(&agoDataType);

  if (MPI_Finalize() != MPI_SUCCESS) {
    (void) fprintf(stderr, "\nErro ao Finalizar o Ambiente MPI");
  }
}

// Create MPI Data Package for the agoDataPackageStr
void makeNewTypeDataPackage()
{
  int i = 0, i_base = 0, blocklen[6] = {1, 1, 1, 1, 1, 1};
  agoDataPackageStr *agoDataPck;
  MPI_Datatype type[6] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_DOUBLE};
  MPI_Aint disp[6] = {0, 0, 0, 0, 0, 0};

  agoDataPck = (agoDataPackageStr *) malloc (sizeof(agoDataPackageStr));

#ifdef VERBOSE
  if (agoDataPck == NULL) {
    (void) fprintf(stderr, "\n(makeNewTypeDataPackage) - Erro Alocando Mem. para agoDataPck - VERBOSE (STDERR)");
  }
#endif

  (void) MPI_Address(agoDataPck, disp);
  (void) MPI_Address(&agoDataPck->offSetA, disp + 1);
  (void) MPI_Address(&agoDataPck->offSetB, disp + 2);
  (void) MPI_Address(&agoDataPck->chunkToCalc, disp + 3);
  (void) MPI_Address(&agoDataPck->status, disp + 4);
  (void) MPI_Address(&agoDataPck->workerTime, disp + 5);

  i_base = disp[0];

  for (i = 0 ; i < 6 ; i++) {
    disp[i] -= i_base;
  }

  (void) MPI_Type_struct(6, blocklen, disp, type, &agoDataType);

  if (MPI_Type_commit(&agoDataType) != MPI_SUCCESS) {
    (void) fprintf(stderr, "\nErro ao Criar Tipo MPI (MPI_Type_Commit)");
  }

  free(agoDataPck);
}

void agoInitDataPackage(agoDataPackageStr *agoDataPackage, int numBlocks, int i_myid, agoLogStr *agoLog)
{

  int i_packageId, i_aux1, i_offSetA, i_offSetB;

  i_aux1 = i_offSetA = i_offSetB = 0;

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoInitDataPackage) - Iniciando a Criacao de %d Pacotes a serem enviados - (VERBOSE)",
                  numBlocks);
  registerLog(agoLog,  i_myid);
#endif

  i_packageId = 1;
  while (i_packageId < numBlocks) {
    if (i_offSetA > (MAX_MATRIX_SIZE - CHUNKSIZE)) {
      i_offSetA = 0;
      i_offSetB = (i_offSetB + CHUNKSIZE);
    }

    agoDataPackage[i_packageId].id = i_packageId;
    agoDataPackage[i_packageId].offSetA = i_offSetA;
    agoDataPackage[i_packageId].offSetB = i_offSetB;
    agoDataPackage[i_packageId].chunkToCalc = CHUNKSIZE;
    agoDataPackage[i_packageId].status = FLAG_FAIL;
    agoDataPackage[i_packageId].workerTime = 0.0;

    i_offSetA += CHUNKSIZE;
    i_packageId++;
  }

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "ultimo pacote id: %d de %d pacotes (blocos) prontos para serem enviados",
                  i_packageId, numBlocks);
  registerLog(agoLog,  i_myid);
}

void agoSendDataToWorker(agoDataPackageStr *agoDataPackage, int numBlocks, Matrix *matrixA, Matrix *matrixB,
                         int *i_offSetidPackage, unsigned int *workerList, int numWorkers, int firstWorker, int idFail, int id,
                         int *workerStatus, agoLogStr *agoLog)
{

  int i_workerId = firstWorker;
  Matrix *d_matrixChunkA, *d_matrixChunkB;

  d_matrixChunkA = (Matrix *) calloc ((CHUNKSIZE * MAX_MATRIX_SIZE), sizeof(double));
  d_matrixChunkB = (Matrix *) calloc ((CHUNKSIZE * MAX_MATRIX_SIZE), sizeof(double));

#ifdef VERBOSE
  if ((d_matrixChunkA == NULL) || (d_matrixChunkB == NULL)) {
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoSendDataToWorker) - Erro Alocando Mem. para Matrizes (A, B) - VERBOSE");
    registerLog(agoLog, id);
  }
#endif

  // To Put fault in a Worker
  if (workerList[i_workerId] == (unsigned) idFail) {
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "Informado ao worker id %d que ele deve falhar", idFail);
    registerLog(agoLog, id);
  }

  while (i_workerId < numWorkers) {
    if (workerStatus[i_workerId] == 3) {
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Falha no Worker: %d", workerList[i_workerId]);
      registerLog(agoLog, id);
    } else if (workerStatus[i_workerId] == 10) {
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(GC) - worker id %d eh um Gestor de Comunicacao", i_workerId);
      registerLog(agoLog, id);
    } else {
      memcpy(d_matrixChunkA, matrixA, (CHUNKSIZE * MAX_MATRIX_SIZE) * sizeof(double));
      memcpy(d_matrixChunkB, matrixB, (CHUNKSIZE * MAX_MATRIX_SIZE) * sizeof(double));

#ifdef VERBOSE
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoSendDataToWorker) - chunk da Matriz A,B copiados - (VERBOSE)");
      registerLog(agoLog, id);
#endif

      // Envia Instrucao para cada Worker informando Onde deve Comecar e Terminar Calculo
      (void) MPI_Send(agoDataPackage, numBlocks, agoDataType, workerList[i_workerId], TAG_DATA, MPI_COMM_WORLD);

#ifdef VERBOSE
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoSendDataToWorker) - Estrutura agoDataPackage Enviada (%d) - (VERBOSE)", numBlocks);
      registerLog(agoLog, id);
#endif

      (void) MPI_Send(i_offSetidPackage, 1, MPI_INT, workerList[i_workerId], TAG_DATA, MPI_COMM_WORLD);

#ifdef VERBOSE
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoSendDataToWorker) - i_offSetidPackage enviado %d (%d) - (VERBOSE)",
                      *i_offSetidPackage, agoDataPackage[*i_offSetidPackage].id);
      registerLog(agoLog, id);
#endif

      (void) MPI_Send(&idFail, 1, MPI_INT, workerList[i_workerId], TAG_DATA, MPI_COMM_WORLD);

#ifdef VERBOSE
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoSendDataToWorker) - Enviado idFail %d - (VERBOSE)", idFail);
      registerLog(agoLog, id);
#endif

      (void) MPI_Send(d_matrixChunkA, (CHUNKSIZE * MAX_MATRIX_SIZE), MPI_DOUBLE, workerList[i_workerId], TAG_DATA, MPI_COMM_WORLD);

#ifdef VERBOSE
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoSendDataToWorker) - d_matrixChunkA enviado (%d) - (VERBOSE)",
                      (CHUNKSIZE * MAX_MATRIX_SIZE));
      registerLog(agoLog, id);
#endif

      (void) MPI_Send(d_matrixChunkB, (CHUNKSIZE * MAX_MATRIX_SIZE), MPI_DOUBLE, workerList[i_workerId], TAG_DATA, MPI_COMM_WORLD);

#ifdef VERBOSE
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoSendDataToWorker) - d_matrixChunkB enviado (%d) - (VERBOSE)",
                      (CHUNKSIZE * MAX_MATRIX_SIZE));
      registerLog(agoLog, id);
#endif

      (void) snprintf(agoLog->logMsg, LOG_SIZE, "Enviado Pacote id %d (%d r/c) p/ Worker ID: %d - i_offSetA %d i_offSetB %d (%d)",
                      agoDataPackage[*i_offSetidPackage].id, agoDataPackage[*i_offSetidPackage].chunkToCalc, workerList[i_workerId],
                      agoDataPackage[*i_offSetidPackage].offSetA, agoDataPackage[*i_offSetidPackage].offSetB, *i_offSetidPackage);
      registerLog(agoLog, id);

      *i_offSetidPackage = *i_offSetidPackage + 1;
    }
    i_workerId++;
  }

  free(d_matrixChunkA);
  free(d_matrixChunkB);
}

// Recebe o Trabalho do Mestre
int agoRecvDataFromMaster(agoDataPackageStr *agoDataPackage, int numBlocks, Matrix *d_matrixChunkA, Matrix *d_matrixChunkB,
                          int masterId, int i_myid, agoLogStr *agoLog)
{
  int idPackage = 0, idFail = 0;

  MPI_Status statusWorker;

  (void) MPI_Recv(agoDataPackage, numBlocks, agoDataType, masterId, TAG_DATA, MPI_COMM_WORLD, &statusWorker);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoRecvDataFromMaster) - Recebido Estrutura agoDataPackage (numBlocks: %d) - (VERBOSE)",
                  numBlocks);
  registerLog(agoLog, i_myid);
#endif

  (void) MPI_Recv(&idPackage, 1, MPI_INT, masterId, TAG_DATA, MPI_COMM_WORLD, &statusWorker);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE,
                  "(agoRecvDataFromMaster) - Recebido idPackage: %d (agoDataPackage[idPackage].id: %d) - (VERBOSE)",
                  idPackage, agoDataPackage[idPackage].id);
  registerLog(agoLog, i_myid);
#endif

  (void) MPI_Recv(&idFail, 1, MPI_INT, masterId, TAG_DATA, MPI_COMM_WORLD, &statusWorker);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoRecvDataFromMaster) - Recebido idFail: %d - (VERBOSE)", idFail);
  registerLog(agoLog,  i_myid);
#endif

  (void) MPI_Recv(d_matrixChunkA, (CHUNKSIZE * MAX_MATRIX_SIZE), MPI_DOUBLE, masterId, TAG_DATA, MPI_COMM_WORLD, &statusWorker);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoRecvDataFromMaster) - Recebido d_matrixChunkA (Tam: %d) - (VERBOSE)",
                  (CHUNKSIZE * MAX_MATRIX_SIZE));
  registerLog(agoLog,  i_myid);
#endif

  (void) MPI_Recv(d_matrixChunkB, (CHUNKSIZE * MAX_MATRIX_SIZE), MPI_DOUBLE, masterId, TAG_DATA, MPI_COMM_WORLD, &statusWorker);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoRecvDataFromMaster) - Recebido d_matrixChunkB (Tam: %d) - (VERBOSE)",
                  (CHUNKSIZE * MAX_MATRIX_SIZE));
  registerLog(agoLog,  i_myid);
#endif

  if (idFail == i_myid) {
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) Injetando Falha em mim Worker id: %d", idFail);
    registerLog(agoLog,  i_myid);

    //
    // Ver Isto, Ver Isto - Injecao de Falha
    //
    //    #if TURN_ON_OFF_FAIL == 1
    //      (void) system(CMD_FAULT_WORKER_AGO_FT);
    //    #endif

  } else {
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "Recebido Pacote id %d do Mestre %d - i_offSetA %d i_offSetB %d",
                    agoDataPackage[idPackage].id, masterId, agoDataPackage[idPackage].offSetA, agoDataPackage[idPackage].offSetB);
    registerLog(agoLog,  i_myid);
  }

  return(idPackage);
}

// Envia Resultado e Tempo do Computo ao Mestre
void agoSendDataToMaster(agoDataPackageStr * agoDataPackage, Matrix *d_matrixChunkC, int masterIdSend, int idSend, agoLogStr *agoLog)
{
  int id = 0, masterId = 0;

  MPI_Request requestWorker;

  id = idSend;
  masterId = masterIdSend;

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoSendDataToMaster) - Envio Nao Blocante de Dados ((void) MPI_Isend) - (VERBOSE)");
  registerLog(agoLog,  id);
#endif

  (void) MPI_Isend(&id, 1, MPI_INT, masterId, TAG_INFO, MPI_COMM_WORLD, &requestWorker);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoSendDataToMaster) - enviado id %d para o mestre %d - (VERBOSE)", id, masterId);
  registerLog(agoLog,  id);
#endif

  (void) MPI_Isend(&agoDataPackage->id, 1, MPI_INT, masterId, id, MPI_COMM_WORLD, &requestWorker);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoSendDataToMaster) - enviado agoDataPacakge->id %d para o mestre %d - (VERBOSE)",
                  agoDataPackage->id, masterId);
  registerLog(agoLog,  id);
#endif

  (void) MPI_Isend(d_matrixChunkC, (CHUNKSIZE * MAX_MATRIX_SIZE), MPI_DOUBLE, masterId, id, MPI_COMM_WORLD, &requestWorker);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoSendDataToMaster) - enviado d_matrixChunkC (Tam: %d) para o mestre %d - (VERBOSE)",
                  (CHUNKSIZE * MAX_MATRIX_SIZE), masterId);
  registerLog(agoLog,  id);
#endif

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "Enviado resposta pacote id %d - i_offSetA %d i_offSetB %d - para o Mestre %d",
                  agoDataPackage->id, agoDataPackage->offSetA, agoDataPackage->offSetB, masterId);
  registerLog(agoLog,  id);

  if (MPI_Request_free(&requestWorker) != MPI_SUCCESS) {
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "ERRO Liberando requestWorker (MPI_Request)");
    registerLog(agoLog,  id);
  }
}

void agoRecvDataFromWorker(agoDataPackageStr *agoDataPackage, Matrix *matrixC, unsigned int *workerList, int i_myid,
                           agoLogStr *agoLog)
{

  int id = 0, i_packageId = 0, count = 0;
  MPI_Status statusMaster;

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoRecvDataFromWorker) - Iniciando Recebimento de Dados do Worker - (VERBOSE)");
  registerLog(agoLog,  i_myid);
#endif

  (void) MPI_Recv(&id, 1, MPI_INT, MPI_ANY_SOURCE, TAG_INFO, MPI_COMM_WORLD, &statusMaster);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoRecvDataFromWorker) - recebido id (%d) do worker (%d) - (VERBOSE)", id, workerList[id]);
  registerLog(agoLog,  i_myid);
#endif

  (void) MPI_Recv(&i_packageId, 1, MPI_INT, workerList[id], id, MPI_COMM_WORLD, &statusMaster);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoRecvDataFromWorker) - recebido i_packageId (%d) do worker (%d) - (VERBOSE)", i_packageId, id);
  registerLog(agoLog,  i_myid);
#endif

  (void) MPI_Recv(&matrixC[agoDataPackage[i_packageId].offSetA][0], (CHUNKSIZE * MAX_MATRIX_SIZE), MPI_DOUBLE, workerList[id], id,
                  MPI_COMM_WORLD, &statusMaster);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoRecvDataFromWorker) - recebido matrixC (%d) do worker (%d) - (VERBOSE)",
                  (CHUNKSIZE * MAX_MATRIX_SIZE), id);
  registerLog(agoLog,  i_myid);
#endif

  agoDataPackage[i_packageId].status = FLAG_DONE;

  MPI_Get_count(&statusMaster, MPI_DOUBLE, &count);

  snprintf(agoLog->logMsg, LOG_SIZE, "Received %d double(s) from task %d with tag %d \n", count, statusMaster.MPI_SOURCE, statusMaster.MPI_TAG);
  registerLog(agoLog,  i_myid);

  (void) snprintf(agoLog->logMsg, LOG_SIZE,
                  "Recebido Pacote id %d (%d r/c) do Worker id: %d - i_offSetA %d i_offSetB %d - (i_packageId: %d)",
                  agoDataPackage[i_packageId].id, agoDataPackage[i_packageId].chunkToCalc, workerList[id],
                  agoDataPackage[i_packageId].offSetA, agoDataPackage[i_packageId].offSetB, i_packageId);
  registerLog(agoLog,  i_myid);
}

void agoSignalWorkers(unsigned int *workerList, int numWorkers, int signalFlag, int firstWorker, int id, int *workerStatus,
                      agoLogStr *agoLog)
{
  int i_workerId = firstWorker;

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "Enviando Sinalizacao (%d) para %d Worker(s)", signalFlag, (numWorkers - 1));
  registerLog(agoLog,  id);

  // Sinaliza aos Trabalhadores
  while (i_workerId < numWorkers) {
    if (workerStatus[i_workerId] == 3) {
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Falha no Worker: %d", workerList[i_workerId]);
      registerLog(agoLog,  id);
    } else if (workerStatus[i_workerId] == 10) {
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(GC) - worker id %d eh um Gestor de Comunicacao", i_workerId);
      registerLog(agoLog,  id);
    } else {
      (void) MPI_Send(&signalFlag, 1, MPI_INT, workerList[i_workerId], TAG_INFO, MPI_COMM_WORLD);

      (void) snprintf(agoLog->logMsg, LOG_SIZE, "Enviado Sinalizacao (%d) ao Worker ID: %d", signalFlag, workerList[i_workerId]);
      registerLog(agoLog,  id);
    }
    i_workerId++;
  }
}


void agoGetCount(unsigned int *timeSignal, int numBlocksOrig, double countBlock, double timeFault100, double *timeFault,
                 int id, agoLogStr *agoLog)
{

  double d_t95 = 0.0, d_t75 = 0.0, d_t50 = 0.0, d_t25 = 0.0;

  d_t95 = (0.95 * numBlocksOrig);
  d_t75 = (0.75 * numBlocksOrig);
  d_t50 = (0.50 * numBlocksOrig);
  d_t25 = (0.25 * numBlocksOrig);

  (void) snprintf(agoLog->logMsg, LOG_SIZE, " ");

  if ((countBlock < (d_t25 + 1.0)) && (countBlock > (d_t25 - 1.0))) {
    if (timeFault[PERCENT_25] == 0.0) {
      timeFault[PERCENT_25] = (MPI_Wtime() - timeFault100);
      *timeSignal = 25;
      if (id == 0) {
        (void) snprintf(agoLog->logMsg, LOG_SIZE, "===> 25%% dos Pronto RECEBIDOS (PROCESSADOS) <=== Tempo %f", timeFault[PERCENT_25]);
      } else if (id == 1) {
        (void) snprintf(agoLog->logMsg, LOG_SIZE,
                        "===> 25%% dos Pacotes RECEBIDOS (PROCESSADOS) DESDE QUE ASSUMIU A FALHA (Mestre-Worker)<=== Tempo %f",
                        timeFault[PERCENT_25]);
      }
      registerLog(agoLog,  id);
    }
  } else if ((countBlock < (d_t50 + 1.0)) && (countBlock > (d_t50 - 1.0))) {
    if (timeFault[PERCENT_50] == 0.0) {
      timeFault[PERCENT_50] = (MPI_Wtime() - timeFault100);
      *timeSignal = 50;

      if (id == 0) {
        (void) snprintf(agoLog->logMsg, LOG_SIZE, "===> 50%% dos Pronto RECEBIDOS (PROCESSADOS) <=== Tempo %f", timeFault[PERCENT_50]);
      } else if (id == 1) {
        (void) snprintf(agoLog->logMsg, LOG_SIZE,
                        "===> 50%% dos Pacotes RECEBIDOS (PROCESSADOS) DESDE QUE ASSUMIU A FALHA (Mestre-Worker)<=== Tempo %f",
                        timeFault[PERCENT_50]);
      }
      registerLog(agoLog,  id);
    }
  } else if ((countBlock < (d_t75 + 1.0)) && (countBlock > (d_t75 - 1.0))) {
    if (timeFault[PERCENT_75] == 0.0) {
      timeFault[PERCENT_75] = (MPI_Wtime() - timeFault100);
      *timeSignal = 75;

      if (id == 0) {
        (void) snprintf(agoLog->logMsg, LOG_SIZE, "===> 75%% dos Pronto RECEBIDOS (PROCESSADOS) <=== Tempo %f", timeFault[PERCENT_75]);
      } else if (id == 1) {
        (void) snprintf(agoLog->logMsg, LOG_SIZE,
                        "===> 75%% dos Pacotes RECEBIDOS (PROCESSADOS) DESDE QUE ASSUMIU A FALHA (Mestre-Worker)<=== Tempo %f",
                        timeFault[PERCENT_75]);
      }
      registerLog(agoLog,  id);
    }
  } else if ((countBlock < (d_t95 + 2.0)) && (countBlock > (d_t95 - 2.0))) {
    if (timeFault[PERCENT_95] == 0.0) {
      timeFault[PERCENT_95] = (MPI_Wtime() - timeFault100);
      *timeSignal = 95;

      if (id == 0) {
        (void) snprintf(agoLog->logMsg, LOG_SIZE, "===> 95%% dos Pronto RECEBIDOS (PROCESSADOS) <=== Tempo %f", timeFault[PERCENT_95]);
      } else if (id == 1) {
        (void) snprintf(agoLog->logMsg, LOG_SIZE,
                        "===> 95%% dos Pacotes RECEBIDOS (PROCESSADOS) DESDE QUE ASSUMIU A FALHA (Mestre-Worker)<=== Tempo %f",
                        timeFault[PERCENT_95]);
      }
      registerLog(agoLog,  id);
    }
  }
}

/* agocore.c */
/*  Under the Terms of the GPL - 2005,06 - Marcelo Soares Souza - CEBACAD (http://www.cebacad.net) */
