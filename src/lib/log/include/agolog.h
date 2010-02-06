/***************************************************************************
 *            agolog.h
 *
 *  Thu Apr 19 16:34:30 2006
 *  Copyright  2005-2006  Marcelo Soares Souza - CEBACAD (http://www.cebacad.net)
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

#define LOG_SIZE 128

typedef struct agoLogStruct {
  char logMsg[LOG_SIZE];
  FILE *fdLog;
}
agoLogStr;

// Funcoes
void startLog(agoLogStr *agoLog, int id);
void registerLog(agoLogStr *agoLog, int id);
void closeLog(agoLogStr *agoLog);
/* agolog.h */
/*  Under the Terms of the GPL - 2005,06 - Marcelo Soares Souza - CEBACAD (http://www.cebacad.net) */
