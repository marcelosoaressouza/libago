/***************************************************************************
 *            agocorefault.c
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
#include "agofault.h"
#include "agoft.h"

void agoSendDataFT(agoDataPackageStr *agoDataPackage, int numBlocks, Matrix *matrixA, Matrix *matrixB,
                   int idPackage, int i_workerId, unsigned int *workerList, int idFail, int id, agoLogStr *agoLog)
{

  Matrix *d_matrixChunkA, *d_matrixChunkB;

  d_matrixChunkA = (Matrix *) calloc ((CHUNKSIZE * MAX_MATRIX_SIZE), sizeof(double));
  d_matrixChunkB = (Matrix *) calloc ((CHUNKSIZE * MAX_MATRIX_SIZE), sizeof(double));

#ifdef VERBOSE
  if ((d_matrixChunkA == NULL) || (d_matrixChunkB == NULL)) {
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoSendDataFT) - Erro Alocando Mem. para Matrizes (A, B) - VERBOSE");
    registerLog(agoLog, id);
  }
#endif

  memcpy(d_matrixChunkA, matrixA, (CHUNKSIZE * MAX_MATRIX_SIZE) * sizeof(double));
  memcpy(d_matrixChunkB, matrixB, (CHUNKSIZE * MAX_MATRIX_SIZE) * sizeof(double));

  // Envia Instrucao para cada Worker informando Onde deve Comecar e Terminar Calculo
  (void) MPI_Send(agoDataPackage, numBlocks, agoDataType, workerList[i_workerId], TAG_DATA, MPI_COMM_WORLD);
  (void) MPI_Send(&idPackage, 1, MPI_INT, workerList[i_workerId], TAG_DATA, MPI_COMM_WORLD);

  (void) MPI_Send(&idFail, 1, MPI_INT, workerList[i_workerId], TAG_DATA, MPI_COMM_WORLD);

  (void) MPI_Send(d_matrixChunkA, (CHUNKSIZE * MAX_MATRIX_SIZE), MPI_DOUBLE, workerList[i_workerId], TAG_DATA, MPI_COMM_WORLD);
  (void) MPI_Send(d_matrixChunkB, (CHUNKSIZE * MAX_MATRIX_SIZE), MPI_DOUBLE, workerList[i_workerId], TAG_DATA, MPI_COMM_WORLD);

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "Enviado Pacote id %d (%d r/c) p/ Worker ID: %d - i_offSetA %d i_offSetB %d (%d)",
                  agoDataPackage[idPackage].id, agoDataPackage[idPackage].chunkToCalc, workerList[i_workerId],
                  agoDataPackage[idPackage].offSetA, agoDataPackage[idPackage].offSetB, idPackage);
  registerLog(agoLog,  id);

  free(d_matrixChunkA);
  free(d_matrixChunkB);
}

// Envia Resultado e Tempo do Computo ao Mestre (FT)
void agoSendDataToMasterFT(agoDataPackageStr * agoDataPackage, Matrix *d_matrixChunkC, int masterId, int id, agoLogStr *agoLog)
{
  (void) MPI_Send(&agoDataPackage->id, 1, MPI_INT, masterId, id, MPI_COMM_WORLD);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoSendDataToMasterFT) - enviado package id: %d do worker %d - (VERBOSE)",
                  agoDataPackage->id, id);
  registerLog(agoLog,  id);
#endif

  (void) MPI_Send(d_matrixChunkC, (CHUNKSIZE * MAX_MATRIX_SIZE), MPI_DOUBLE, masterId, agoDataPackage->id, MPI_COMM_WORLD);

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Enviado Pacote id %d - i_offSetA %d i_offSetB %d - p/ o Mestre (%d)",
                  agoDataPackage->id, agoDataPackage->offSetA, agoDataPackage->offSetB, masterId);
  registerLog(agoLog,  id);
}

void agoRecvDataFromWorkerFT(agoDataPackageStr *agoDataPackage, Matrix *matrixC, unsigned int *workerList, int i_workerId,
                             int id, agoLogStr *agoLog)
{

  int i_packageId = 0;
  MPI_Status statusMaster;

  (void) MPI_Recv(&i_packageId, 1, MPI_INT, workerList[i_workerId], workerList[i_workerId], MPI_COMM_WORLD, &statusMaster);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoRecvDataFromWorkerFT) - recebido package id: %d do worker %d (%d) - (VERBOSE)",
                  i_packageId, workerList[i_workerId], i_workerId);
  registerLog(agoLog,  id);
#endif

  (void) MPI_Recv(&matrixC[agoDataPackage[i_packageId].offSetA][0], (CHUNKSIZE * MAX_MATRIX_SIZE), MPI_DOUBLE, workerList[i_workerId],
                  i_packageId, MPI_COMM_WORLD, &statusMaster);

  agoDataPackage[i_packageId].status = FLAG_DONE;

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "Recebido Pacote id %d (%d r/c) do Worker ID: %d - i_offSetA %d i_offSetB %d - (%d)",
                  agoDataPackage[i_packageId].id, agoDataPackage[i_packageId].chunkToCalc, workerList[i_workerId],
                  agoDataPackage[i_packageId].offSetA, agoDataPackage[i_packageId].offSetB, i_packageId);
  registerLog(agoLog, id);
}

void agoSendInfoToMasterFT(int packageDone, int id, int masterId, agoLogStr *agoLog)
{
  (void) MPI_Send(&id, 1, MPI_INT, masterId, TAG_INFO, MPI_COMM_WORLD);
  (void) MPI_Send(&packageDone, 1, MPI_INT, masterId, TAG_DATA, MPI_COMM_WORLD);

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "Enviado Info para Mestre (%d): %d Pacotes Processados", masterId, packageDone);
}

int agoRecvInfoFromWorker(int i_workerId, agoLogStr *agoLog)
{
  int i_numPackages = 0;
  MPI_Status statusMaster;

  (void) MPI_Recv(&i_numPackages, 1, MPI_INT, i_workerId, TAG_DATA, MPI_COMM_WORLD, &statusMaster);

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "Worker id %d processou %d pacotes ", i_workerId, i_numPackages);

  return(i_numPackages);
}

/* agocorefault.c */
/*  Under the Terms of the GPL - 2005,06 - Marcelo Soares Souza - CEBACAD (http://www.cebacad.net) */
