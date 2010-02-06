/***************************************************************************
 *            agoft.h
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

#define NUM_MAX_WORKER 32

#define STOP_AGO_FT        0
#define START_AGO_FT       1

#define WORKER_TEST_CHECK_MASTER_AGO_FT 6  // Quanto Tempo (segundos) o Worker deve enviar sinal ao mestre
#define MASTER_TEST_TIME_AGO_FT 3 // Quanto Tempo (segundos) o Master deve verificar sinal Worker

#define MASTER_ADDR_AGO_FT "sauron" // Endereco do Master que o Worker deve sinalizar
#define MASTER_PORT_AGO_FT 1999 // Porta de Comunicacao Socket

// Injecao de Falha (agoPutFaultOnWorker)
#define FAULT_TYPE_AGO_FT 0 // 0 -> Injeta falha no Worker / 1 -> Injeta Falha no Mestre
#define WORKER_ID_TO_FAIL_AGO_FT 2 // Worker id que deve falhar
#define PUT_FAULT_AGO_FT 0 // Falha depois de Quant. de Trabalho feito - Valores aceitos 0, 25, 50, 75, 95 (%)
#define WAIT_TIME_FAIL_AGO_FT 5
#define CMD_FAULT_WORKER_AGO_FT "/usr/bin/sudo /sbin/ifdown eth0"
#define CMD_FAULT_MASTER_AGO_FT "/usr/bin/sudo /sbin/ifdown eth0"
#define WORKER_TEST_TIME_AGO_FT 2  // Quanto Tempo (segundos) o Worker deve enviar sinal ao mestre

int syncWorker_ago_ft, workerStatus_ago_ft[NUM_MAX_WORKER], workerSocketLoop_ago_ft, masterFail_ago_ft, waitMaster_ago_ft,
syncMaster_ago_ft, masterStatus_ago_ft, reconfigWorker_ago_ft, masterSocketLoop_ago_ft, waitWorker_ago_ft;

void * agoCheckStatusWorker(void *numprocs);
void * agoCheckStatusMaster();

void * agoSendSignalMaster(void *numprocs);
void * agoSendSignalWorker(void *myMPIid);
void sigchld_handler(int s);

int agoPutFaultOnWorker(int signalTime, agoLogStr *agoLog);

/* agoft.h */
/*  Under the Terms of the GPL - 2005,06 - Marcelo Soares Souza - CEBACAD (http://www.cebacad.net) */
