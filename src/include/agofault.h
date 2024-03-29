/***************************************************************************
 *            agofault.h
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

// AgoCore (agocorefault.c)
void agoSendInfoToMasterFT(int, int, int, agoLogStr *);
void agoSendDataToMasterFT(agoDataPackageStr *, Matrix *, int, int, agoLogStr *);
void agoSendDataFT(agoDataPackageStr *, int, Matrix *, Matrix *, int, int, unsigned int *, int, int, agoLogStr *);
void agoRecvDataFromWorkerFT(agoDataPackageStr *, Matrix *, unsigned int *, int, int, agoLogStr *);
int agoRecvInfoFromWorker(int, agoLogStr *);

// Master (agomasterfault.c)
void agoReconfigCluster(agoDataPackageStr *, Matrix *, Matrix *, Matrix *, unsigned int *, int, agoLogStr *);

// Worker (agoworkerfault.c)
int agoFaultToleranceWorker(agoDataPackageStr *, unsigned int *, int *, int, int, double *, agoLogStr *);
void agoSendRepeatToWorker(unsigned int *, int, agoLogStr *);

/* agofault.h */
/*  Under the Terms of the GPL - 2005,06 - Marcelo Soares Souza - CEBACAD (http://www.cebacad.net) */
