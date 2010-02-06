/***************************************************************************
 *            ago.h
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

#define NUM_WORKERS 4

#define FLAG_STOP        0
#define FLAG_START       1
#define FLAG_FAIL        2
#define FLAG_DONE        3
#define FLAG_MASTER_FAIL 4
#define FLAG_START_FAIL  5

#define TAG_INFO 0
#define TAG_DATA 1

#define PERCENT_25 0
#define PERCENT_50 1
#define PERCENT_75 2
#define PERCENT_95 3

#define TURN_ON_OFF_FAIL 0

typedef struct agoDataPackageStruct {
  int id;
  int offSetA;
  int offSetB;
  int chunkToCalc;
  int status;
  double workerTime;
}
agoDataPackageStr;

MPI_Datatype agoDataType;

// Init/Finalize MPI Enviroment
int agoInit(int argc, char **argv, int *numprocs);
void agoFinalize();

//Init Package
void agoInitDataPackage(agoDataPackageStr *, int, int, agoLogStr *);

// Signaling to Workers
void agoSignalWorkers(unsigned int *, int, int, int, int, int *, agoLogStr *);

// Function to Send Data
void agoSendDataToMaster(agoDataPackageStr *, Matrix *, int, int, agoLogStr *);
void agoSendDataToWorker(agoDataPackageStr *, int, Matrix *, Matrix *, int *, unsigned int *, int, int, int, int, int *,
                         agoLogStr *);

// Function to Recv Data
void agoRecvDataFromWorker(agoDataPackageStr *, Matrix *, unsigned int *, int, agoLogStr *);

// Capture the Number of Package Worked
void agoGetCount(unsigned int *, int, double, double, double *, int, agoLogStr *);

void agoMasterInit(int, int);
void * agoMasterProcess();
void agoWorkerInit(int, int);

void * agoWorkerProcess();
int agoWaitMasterEnd(int masterId, agoLogStr *agoLog);

int agoRecvDataFromMaster(agoDataPackageStr *agoDataPackage, int numBlocks, Matrix *d_matrixChunkA, Matrix *d_matrixChunkB,
                          int masterId, int i_myid, agoLogStr *agoLog);


/* ago.h */
/*  Under the Terms of the GPL - 2005,06 - Marcelo Soares Souza - CEBACAD (http://www.cebacad.net) */
/* - Marcelo Soares Souza - CEBACAD (http://www.cebacad.net) */
