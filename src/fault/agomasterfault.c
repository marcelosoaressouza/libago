/***************************************************************************
 *            agomasterfault.c
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

#include <pthread.h>
#include <unistd.h>
#include "agofault.h"
#include "agoft.h"

// Variaveis Globais
static int si_myidMaster, si_numprocsMaster;

void agoReconfigCluster(agoDataPackageStr *agoDataPackageMaster, Matrix *d_matrixA, Matrix *d_matrixB, Matrix *d_matrixC,
                        unsigned int *ui_workerList, int i_numBlocks, agoLogStr *agoLog)
{

  int i_workerId, i_loopFail, i_aux1, i_wFail, i_wOk, i_pckFail;
  int i_workerFails[si_numprocsMaster], i_packageIdList[i_numBlocks / 2];
  unsigned int ui_workerOk[si_numprocsMaster];

  i_workerId = i_loopFail = i_aux1 = i_wFail = i_wOk = i_pckFail = 1;

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Procedimento de Tolerancia a Falha Iniciado\n");
  registerLog(agoLog,  si_myidMaster);

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Quantidade de Pacotes %d", i_numBlocks);
  registerLog(agoLog,  si_myidMaster);

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoReconfigCluster) - Lista de Pacotes nao Processados - (VERBOSE)");
  registerLog(agoLog,  si_myidMaster);
#endif

  for (i_aux1 = 1 ; i_aux1 <= i_numBlocks ; i_aux1++) {
    if (agoDataPackageMaster[i_aux1].status == FLAG_FAIL) {
      (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Pacote id %d nao foi processado", agoDataPackageMaster[i_aux1].id);
      registerLog(agoLog,  si_myidMaster);

      i_packageIdList[i_pckFail] = agoDataPackageMaster[i_aux1].id;
      i_pckFail++;
    }
  }

#ifdef VERBOSE
  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(agoReconfigCluster) - Verificando Workers que Falharam - Refazendo Lista - (VERBOSE)");
  registerLog(agoLog,  si_myidMaster);
#endif

  ui_workerOk[0] = 0;
  i_packageIdList[0] = 0;

  i_aux1 = 1;
  while (i_aux1 < NUM_WORKERS) {
    if (workerStatus_ago_ft[i_aux1] == 3) {
      i_workerFails[i_wFail] = ui_workerList[i_aux1];
      i_wFail++;
    } else {
      ui_workerOk[i_wOk] = ui_workerList[i_aux1];
      workerStatus_ago_ft[i_wOk] = FLAG_START;
      i_wOk++;
    }

    i_aux1++;
  }

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - %d Workers Falharam e %d Completaram o Trabalho", (i_wFail - 1), (i_wOk - 1));
  registerLog(agoLog,  si_myidMaster);

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Nova Lista de Workers (Re-Config)");
  registerLog(agoLog,  si_myidMaster);

  i_aux1 = 1;
  while (i_aux1 < i_wOk) {
    (void) snprintf(agoLog->logMsg, LOG_SIZE, "Worker id %d (%d)", ui_workerOk[i_aux1], i_aux1);
    registerLog(agoLog,  si_myidMaster);

    i_aux1++;
  }

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Quantidade de Pacotes a serem Re-Enviados: %d", (i_pckFail - 1));
  registerLog(agoLog,  si_myidMaster);

  agoSignalWorkers(ui_workerOk, i_pckFail, FLAG_START_FAIL, 1, si_myidMaster, workerStatus_ago_ft, agoLog);

  while (i_workerId < i_pckFail) {
    agoSendDataFT(agoDataPackageMaster, i_numBlocks, d_matrixA, d_matrixB, i_packageIdList[i_workerId], i_workerId,
                  ui_workerOk, 0, si_myidMaster, agoLog);
    i_workerId++;
  }

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Pacotes distribuidos - Esperando Resposta do Worker(s)\n");
  registerLog(agoLog,  si_myidMaster);

  i_workerId = 1;
  while (i_workerId < i_pckFail) {
    agoRecvDataFromWorkerFT(agoDataPackageMaster, d_matrixC, ui_workerOk, i_workerId, si_myidMaster, agoLog);
    i_workerId++;
  }

  (void) snprintf(agoLog->logMsg, LOG_SIZE, "(FT) - Procedimento de Tolerancia a Falha Finalizado");
  registerLog(agoLog,  si_myidMaster);
}

/* agomasterfault.c */
/*  Under the Terms of the GPL - 2005,06 - Marcelo Soares Souza - (http://marcelo.cebacad.net) */
