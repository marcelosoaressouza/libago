/***************************************************************************
 *            main.c
 *
 *  Thu Apr 19 18:34:30 2005
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
 Last Change -  Thu Apr 19 15:55:30 2006
*/

#include <stdio.h>
#include <stdlib.h>

#include "mpi.h"
#include "agolog.h"
#include "agomatrix.h"
#include "ago.h"

int main (int argc, char **argv)
{
  int i_processId = 0, i_numprocs = 0;

  i_processId = agoInit(argc, argv, &i_numprocs);

  if (i_processId == 0) {
    agoMasterInit(i_processId, i_numprocs);
  } else if (i_processId > 0) {
    agoWorkerInit(i_processId, i_numprocs);
  }

  exit(0);
}
/* main.c */
/*  Under the Terms of the GPL - 2005,06 - Marcelo Soares Souza - CEBACAD (http://www.cebacad.net) */
