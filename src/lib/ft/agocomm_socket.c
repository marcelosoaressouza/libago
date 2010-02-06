/***************************************************************************
 *            agocomm_socket.c
 *
 *  Thu Apr 19 18:34:30 2006
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
 Last Change -  Thu Apr 19 12:56:30 2006
*/

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netdb.h>
#include <signal.h>

#include "agolog.h"
#include "agoft.h"

void * agoSendSignalMaster(void *numprocs)
{
  int i_workerId = 0, i_sockfd = 0, i_sockFdMessage = 0, i_yes = 1, i_numbytes = 0;
  char c_id[4];
  struct sockaddr_in my_addr;
  struct sockaddr_in their_addr;
  struct sigaction sa;
  socklen_t sin_size;

#ifdef VERBOSE
  agoLogStr *agoLog;
  agoLog = (agoLogStr *) malloc (sizeof(agoLogStr));

  startLog(agoLog, 1024);

  snprintf(agoLog->logMsg, LOG_SIZE, "(signalMaster) - Logado Atividades do Socket - (VERBOSE_FT)");
  registerLog(agoLog, 1024);
#endif

  for (i_workerId = 1 ; i_workerId < (int) numprocs ; i_workerId++) {
    workerStatus_ago_ft[i_workerId] = 0;
  }

  if ((i_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
#ifdef VERBOSE
    snprintf(agoLog->logMsg, LOG_SIZE,  "Houve um Problema de Socket - (SOCKET)");
    registerLog(agoLog, 1024);
#endif
  }

  if (setsockopt(i_sockfd, SOL_SOCKET, SO_REUSEADDR, &i_yes, sizeof(int)) == -1) {
#ifdef VERBOSE
    snprintf(agoLog->logMsg, LOG_SIZE,  "Houve um Problema de Socket - (SETSOCKOPT)");
    registerLog(agoLog, 1024);
#endif
  }

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(MASTER_PORT_AGO_FT);

  my_addr.sin_addr.s_addr = INADDR_ANY;

  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  memset(&(my_addr.sin_zero), 0, 8);

  if (bind(i_sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
#ifdef VERBOSE
    snprintf(agoLog->logMsg, LOG_SIZE,  "Houve um Problema de Socket - (BIND)");
    registerLog(agoLog, 1024);
#endif
  }

  if (listen(i_sockfd, 10) == -1) {
#ifdef VERBOSE
    snprintf(agoLog->logMsg, LOG_SIZE,  "Houve um Problema de Socket - (LISTEN)");
    registerLog(agoLog, 1024);
#endif
  }

  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
#ifdef VERBOSE
    snprintf(agoLog->logMsg, LOG_SIZE,  "Houve um Problema de Socket - (SIGACTION)");
    registerLog(agoLog, 1024);
#endif
  }

#ifdef VERBOSE
  snprintf(agoLog->logMsg, LOG_SIZE, "Iniciando masterSocketLoop");
  registerLog(agoLog, 1024);
#endif

  while (masterSocketLoop_ago_ft) {
    sin_size = sizeof(struct sockaddr_in);

    if (((i_sockFdMessage = accept(i_sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1)) {
#ifdef VERBOSE
      snprintf(agoLog->logMsg, LOG_SIZE,  "Houve um Problema de Socket - (ACCEPT)");
      registerLog(agoLog, 1024);
#endif
    }

    // Recebe Confirmacao dos Workers e da Ok na Lista
    if ((i_numbytes = recv(i_sockFdMessage, c_id, (4 - 1), 0)) == -1) {
#ifdef VERBOSE
      snprintf(agoLog->logMsg, LOG_SIZE,  "Houve um Problema de Socket - (RECV)");
      registerLog(agoLog, 1024);
#endif
    } else {
      // pthread_mutex_lock(&mutex);

      if (workerStatus_ago_ft[atoi(c_id)] != 10) {
        workerStatus_ago_ft[atoi(c_id)] = 1;
      }

      syncWorker_ago_ft = 1;

      // pthread_mutex_unlock(&mutex);

#ifdef VERBOSE
      snprintf(agoLog->logMsg, LOG_SIZE, "worker c_id: %d sinalizou (%d)", atoi(c_id), workerStatus_ago_ft[atoi(c_id)]);
      registerLog(agoLog, 1024);
#endif
    }

    if ((close(i_sockFdMessage)) == -1) {
#ifdef VERBOSE
      snprintf(agoLog->logMsg, LOG_SIZE, "Houve um Problema de Socket - (CLOSE)");
      registerLog(agoLog, 1024);
#endif
    }
  }

  if ((close(i_sockfd)) == -1) {
#ifdef VERBOSE
    snprintf(agoLog->logMsg, LOG_SIZE, "Houve um Problema de Socket - (CLOSE)");
    registerLog(agoLog, 1024);
#endif
  }

#ifdef VERBOSE
  snprintf(agoLog->logMsg, LOG_SIZE, "Socket Terminado");
  registerLog(agoLog, 1024);

  closeLog(agoLog);
#endif
  return(NULL);
}

void sigchld_handler(int s)
{
  fprintf(stdout, "%d", s);
  while (wait(NULL) > 0);
}

void * agoSendSignalWorker(void *myMPIid)
{
  int i_sockfd, i_failSocket = 0;
  char c_bufSignal[4];
  struct hostent *he;
  struct sockaddr_in their_addr;

#ifdef VERBOSE
  agoLogStr *agoLog;
  agoLog = (agoLogStr *) malloc (sizeof(agoLogStr));

  startLog(agoLog, 1024);

  snprintf(agoLog->logMsg, LOG_SIZE, "(signalWorker) - Logado Atividades do Socket - (VERBOSE_FT)");
  registerLog(agoLog, 1024);
#endif

  he = gethostbyname(MASTER_ADDR_AGO_FT);
  their_addr.sin_port = htons(MASTER_PORT_AGO_FT);

  their_addr.sin_family = AF_INET;
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);

  memset(&(their_addr.sin_zero), 0, 8);

  snprintf(c_bufSignal, sizeof(c_bufSignal), "%d", (int) myMPIid);

  while (workerSocketLoop_ago_ft) {
    sleep(WORKER_TEST_TIME_AGO_FT);

    if ((i_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
#ifdef VERBOSE
      snprintf(agoLog->logMsg, LOG_SIZE,  "Houve um Problema de Socket - (SOCKET)");
      registerLog(agoLog, 1024);
#endif
      masterFail_ago_ft = i_failSocket = 1;
    }

    if (connect(i_sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
      if (workerSocketLoop_ago_ft == START_AGO_FT || waitMaster_ago_ft == START_AGO_FT) {
#ifdef VERBOSE
        snprintf(agoLog->logMsg, LOG_SIZE,  "Houve um Problema de Socket - (CONNECT)");
        registerLog(agoLog, 1024);
#endif

        masterFail_ago_ft = i_failSocket = 1;
        workerSocketLoop_ago_ft = STOP_AGO_FT;
      } else {
#ifdef VERBOSE
        snprintf(agoLog->logMsg, LOG_SIZE,  "Saindo do signalWorkers - (CONNECT)");
        registerLog(agoLog, 1024);
#endif
      }

      // pthread_exit(NULL);
    }

    if (send(i_sockfd, c_bufSignal, 4, 0) == -1 ) {
      if (workerSocketLoop_ago_ft == START_AGO_FT || waitMaster_ago_ft == START_AGO_FT) {
#ifdef VERBOSE
        snprintf(agoLog->logMsg, LOG_SIZE,  "Houve um Problema de Socket - (SEND)");
        registerLog(agoLog, 1024);
#endif

        masterFail_ago_ft = i_failSocket = 1;
        workerSocketLoop_ago_ft = STOP_AGO_FT;
      } else {
#ifdef VERBOSE
        snprintf(agoLog->logMsg, LOG_SIZE,  "Saindo do signalWorkers - (SEND)");
        registerLog(agoLog, 1024);
#endif
      }

      // pthread_exit(NULL);
    } else {
#ifdef VERBOSE
      snprintf(agoLog->logMsg, LOG_SIZE,  "Send para o Master Ok - (CONNECT)");
      registerLog(agoLog, 1024);
#endif

      // pthread_mutex_lock(&mutex);
      syncMaster_ago_ft = masterStatus_ago_ft = 1;
      // pthread_mutex_unlock(&mutex);
    }

    if ((close(i_sockfd)) == -1 ) {
#ifdef VERBOSE
      snprintf(agoLog->logMsg, LOG_SIZE,  "Houve um Problema no Socket - (CLOSE)");
      registerLog(agoLog, 1024);
#endif
      masterFail_ago_ft = i_failSocket = 1;
    }
  }
#ifdef VERBOSE
  snprintf(agoLog->logMsg, LOG_SIZE,  "Socket Terminado");
  registerLog(agoLog, 1024);

  closeLog(agoLog);
#endif

  return(NULL);
}

/* agocomm_socket.c */
/*  Under the Terms of the GPL - 2005,06 - Marcelo Soares Souza - (http://marcelo.cebacad.net) */
