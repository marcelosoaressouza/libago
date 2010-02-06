/***************************************************************************
 *            agolog.c
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
 Last Change -  Thu Apr 19 16:56:30 2006
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "agolog.h"

void startLog(agoLogStr *agoLog, int id)
{
  char c_fileName[16];

  if (id == 0) {
    snprintf(c_fileName, sizeof(c_fileName), "logMaster");
  } else if (id == 1024) {
    snprintf(c_fileName, sizeof(c_fileName), "logSocket");
  } else if (id == 2048) {
    snprintf(c_fileName, sizeof(c_fileName), "logCheck");
  } else {
    snprintf(c_fileName, sizeof(c_fileName), "logWorker%d", id);
  }

  agoLog->fdLog = fopen(c_fileName, "w");

  if (agoLog->fdLog == NULL) {
    perror("Erro Arquivo de Log");
  } else {
    fprintf(agoLog->fdLog, "\n(agolog) - Iniciando Arquivo de Log\n");
  }
}

void registerLog(agoLogStr *agoLog, int id)
{
  time_t timer;
  timer = time(NULL);

  if (id == 0) {
    fprintf(stdout, "Master - %s\n", agoLog->logMsg);
    fprintf(agoLog->fdLog, "Master - %s\n", agoLog->logMsg);
  } else if (id == 1024) {
    fprintf(agoLog->fdLog, "Socket (%d) - %s - %s", id, agoLog->logMsg, asctime(localtime(&timer)));
  } else if (id == 2048) {
    fprintf(agoLog->fdLog, "CheckList (%d) - %s - %s", id, agoLog->logMsg, asctime(localtime(&timer)));
  } else {
    fprintf(agoLog->fdLog, "Worker (%d) - %s - %s", id, agoLog->logMsg, asctime(localtime(&timer)));
  }

  fflush(agoLog->fdLog);
}

void closeLog(agoLogStr *agoLog)
{
  fprintf(agoLog->fdLog, "\n(agolog) - Fechando Log\n");
  fclose(agoLog->fdLog);
}
/* agolog.c */
/*  Under the Terms of the GPL - 2005,06 - Marcelo Soares Souza - CEBACAD (http://www.cebacad.net) */
