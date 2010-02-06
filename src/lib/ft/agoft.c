/***************************************************************************
 *            agoft.c
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
 Last Change -  Thu Jul 31 16:56:30 2006
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "agolog.h"
#include "agoft.h"

void * agoCheckStatusWorker(void *i_numprocs)
{
  int i_id = 0, i_nump = 0;

  while (waitWorker_ago_ft) {
    sleep(MASTER_TEST_TIME_AGO_FT);

    if ((syncWorker_ago_ft == 1) && (waitWorker_ago_ft == 1)) {
      // pthread_mutex_lock(&mutex);

      for (i_id = 1; i_id < (int) i_numprocs ; i_id++) {
        if (workerStatus_ago_ft[i_id] == 0) {
          workerStatus_ago_ft[i_id] = 2;
        } else if (workerStatus_ago_ft[i_id] == 1) {
          workerStatus_ago_ft[i_id] = 0;
        } else if (workerStatus_ago_ft[i_id] == 2) {
          // Houve falha/erro - retira esta(s) da lista de Worker(s)
          workerStatus_ago_ft[i_id] = 3;
          i_nump = (int) i_numprocs;
          i_nump--;

          i_numprocs = (void *) i_nump;
          reconfigWorker_ago_ft = START_AGO_FT;
        }
      }

      syncWorker_ago_ft = STOP_AGO_FT;
      // pthread_mutex_unlock(&mutex);
    }
  }
  return(NULL);
}

void * agoCheckStatusMaster()
{
  while (waitMaster_ago_ft) {
    sleep(WORKER_TEST_CHECK_MASTER_AGO_FT);

    if ((syncMaster_ago_ft == 1) && (waitMaster_ago_ft == 1)) {
      // pthread_mutex_lock(&mutex);
      if (masterStatus_ago_ft == 0) {
        masterFail_ago_ft = 1;
        waitMaster_ago_ft = 0;
      } else {
        masterStatus_ago_ft = 0;
      }

      // pthread_mutex_unlock(&mutex);
    }
  }
  return(NULL);

}

int agoPutFaultOnWorker(int signalTime, agoLogStr *agoLog)
{
  int idFail = 0;

  if (reconfigWorker_ago_ft == 0) {
    if (signalTime == 25) {
      if (PUT_FAULT_AGO_FT == 25) {
        if (FAULT_TYPE_AGO_FT == 0) {
          idFail = WORKER_ID_TO_FAIL_AGO_FT;
          snprintf(agoLog->logMsg, LOG_SIZE, " ===> Colocando Falha no Worker i_id %d em (25%%) do Processo <===", idFail);
          registerLog(agoLog, 4096);
        } else if (FAULT_TYPE_AGO_FT == 1) {
          snprintf(agoLog->logMsg, LOG_SIZE, " ===> Colocando Falha no Mestre (25%%) <===");
          registerLog(agoLog, 4096);
          system(CMD_FAULT_MASTER_AGO_FT);
        }
      }
    } else if (signalTime == 50) {
      if (PUT_FAULT_AGO_FT == 50) {
        if (FAULT_TYPE_AGO_FT == 0) {
          idFail = WORKER_ID_TO_FAIL_AGO_FT;
          snprintf(agoLog->logMsg, LOG_SIZE, " ===> Colocando Falha no Worker i_id %d em (50%%) do Processo <===", idFail);
          registerLog(agoLog, 4096);
        } else if (FAULT_TYPE_AGO_FT == 1) {
          snprintf(agoLog->logMsg, LOG_SIZE, " ===> Colocando Falha no Mestre (50%%) <===");
          registerLog(agoLog, 4096);
          system(CMD_FAULT_MASTER_AGO_FT);
        }
      }
    } else if (signalTime == 75) {
      if (PUT_FAULT_AGO_FT == 75) {
        if (FAULT_TYPE_AGO_FT == 0) {
          idFail = WORKER_ID_TO_FAIL_AGO_FT;
          snprintf(agoLog->logMsg, LOG_SIZE, " ===> Colocando Falha no Worker i_id %d em (75%%) do Processo <===", idFail);
          registerLog(agoLog, 4096);
        } else if (FAULT_TYPE_AGO_FT == 1) {
          snprintf(agoLog->logMsg, LOG_SIZE, " ===> Colocando Falha no Mestre (75%%) <===");
          registerLog(agoLog, 4096);
          system(CMD_FAULT_MASTER_AGO_FT);
        }
      }
    } else if (signalTime == 95) {
      if (PUT_FAULT_AGO_FT == 95) {
        if (FAULT_TYPE_AGO_FT == 0) {
          idFail = WORKER_ID_TO_FAIL_AGO_FT;
          snprintf(agoLog->logMsg, LOG_SIZE, " ===> Colocando Falha no Worker i_id %d em (95%%) do Processo <===", idFail);
          registerLog(agoLog, 4096);
        } else if (FAULT_TYPE_AGO_FT == 1) {
          snprintf(agoLog->logMsg, LOG_SIZE, " ===> Colocando Falha no Mestre (95%%) <===");
          registerLog(agoLog, 4096);
          system(CMD_FAULT_MASTER_AGO_FT);
        }
      }
    }
  }
  return(idFail);

}

/* agoft.c */
/*  Under the Terms of the GPL - 2005,06 - Marcelo Soares Souza - CEBACAD (http://www.cebacad.net) */

