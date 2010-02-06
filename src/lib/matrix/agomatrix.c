/***************************************************************************
 *            agomatrix.c
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
#include <errno.h>

#include "agolog.h"
#include "agomatrix.h"

void matrixCreate(Matrix *matrixA, Matrix *matrixB, int sizeMatrix, int id, agoLogStr *agoLog)
{
  int i = 0, j = 0, i_numItems[2] = {0, 0};
  FILE *fdA, *fdB;

  // Caso nao exista as Matrizes - Crie
  if (fopen(FILE_NAME_MATRIX_A, "r") == NULL) {
    snprintf(agoLog->logMsg, LOG_SIZE, "Arquivo de Matriz nao Encontrado - Criando");
    registerLog(agoLog, id);

    fdA = fopen(FILE_NAME_MATRIX_A, "w+");
    fdB = fopen(FILE_NAME_MATRIX_B, "w+");

#ifdef VERBOSE
    if ((fdA == NULL) || (fdB == NULL)) {
      snprintf(agoLog->logMsg, LOG_SIZE, "(matrixCreate) - Erro ao Carregar Arquivos de Matrizes (%s, %s) - (VERBOSE)"
               FILE_NAME_MATRIX_A, FILE_NAME_MATRIX_B, FILE_NAME_MATRIX_B);
      registerLog(agoLog, id);
    }
#endif

    for (i = 0; i < MAX_MATRIX_SIZE; i++) {
      for (j = 0; j < MAX_MATRIX_SIZE; j++) {
        matrixA[i][j] = 2.0 + (double) (100.0 * rand() / (RAND_MAX + 1.0));
        matrixB[i][j] = 2.0 + (double) (100.0 * rand() / (RAND_MAX + 1.0));
      }
    }

#ifdef VERBOSE
    snprintf(agoLog->logMsg, LOG_SIZE, "(matrixCreate) - Matrizes (A, B) Criadas em Memoria - (VERBOSE)");
    registerLog(agoLog, id);
#endif

    i_numItems[0] = fwrite(matrixA, sizeof(double), sizeMatrix, fdA);
    i_numItems[1] = fwrite(matrixB, sizeof(double), sizeMatrix, fdB);

#ifdef VERBOSE
    snprintf(agoLog->logMsg, LOG_SIZE, "(matrixCreate) - Arq. das Matrizes (A, B) Salvo em Disco (%s (%d), %s (%d)) - VERBOSE",
             FILE_NAME_MATRIX_A, i_numItems[0], FILE_NAME_MATRIX_B, i_numItems[1]);
    registerLog(agoLog, id);
#endif

    fclose(fdA);
    fclose(fdB);
  } else {
    snprintf(agoLog->logMsg, LOG_SIZE, "Arquivo de Matriz Encontrado - Carregando do Arquivo - (VERBOSE)");

    fdA = fopen(FILE_NAME_MATRIX_A, "r");
    fdB = fopen(FILE_NAME_MATRIX_B, "r");

#ifdef VERBOSE
    if ((fdA == NULL) || (fdB == NULL)) {
      snprintf(agoLog->logMsg, LOG_SIZE, "(matrixCreate) - Erro Carregando Arquivos (%s, %s)",
               FILE_NAME_MATRIX_A, FILE_NAME_MATRIX_B);
      registerLog(agoLog, id);
    }
#endif

    i_numItems[0] = fread(matrixA, sizeof(double), sizeMatrix, fdA);
    i_numItems[1] = fread(matrixB, sizeof(double), sizeMatrix, fdB);

#ifdef VERBOSE
    snprintf(agoLog->logMsg, LOG_SIZE, "(matrixCreate) - Arq. das Matrizes (A, B) Carregados (%s (%d), %s (%d)) - VERBOSE",
             FILE_NAME_MATRIX_A, i_numItems[0], FILE_NAME_MATRIX_B, i_numItems[1]);
    registerLog(agoLog, id);
#endif

    fclose(fdA);
    fclose(fdB);

#ifdef VERBOSE
    snprintf(agoLog->logMsg, LOG_SIZE, "(matrixCreate) - Matrizes Carregadas - (VERBOSE)");
    registerLog(agoLog, id);
#endif
  }
}

void matrixLoad(Matrix *matrixA, Matrix *matrixB, int sizeMatrix, agoLogStr *agoLog)
{
  FILE *fdA, *fdB;
  int i_numItems[2] = {0, 0};

  fdA = fopen(FILE_NAME_MATRIX_A, "r");
  fdB = fopen(FILE_NAME_MATRIX_B, "r");

#ifdef VERBOSE
  if ((fdA == NULL) || (fdB == NULL)) {
    snprintf(agoLog->logMsg, LOG_SIZE, "(matrixLoad) - Erro Carregando Arquivos (%s, %s)",
             FILE_NAME_MATRIX_A, FILE_NAME_MATRIX_B);
//      registerLog(agoLog, id);
  }
#endif

  i_numItems[0] = fread(matrixA, sizeof(double), sizeMatrix, fdA);
  i_numItems[1] = fread(matrixB, sizeof(double), sizeMatrix, fdB);

#ifdef VERBOSE
  snprintf(agoLog->logMsg, LOG_SIZE, "(matrixLoad) - Arq. das Matrizes (A, B) Carregados (%s (%d), %s (%d)) - VERBOSE",
           FILE_NAME_MATRIX_A, i_numItems[0], FILE_NAME_MATRIX_B, i_numItems[1]);
//    registerLog(agoLog, id);
#endif

  fclose(fdA);
  fclose(fdB);

  snprintf(agoLog->logMsg, LOG_SIZE, "Carregado Matrizes do Arquivo - Tamanho %d", sizeMatrix);
}

double matrixMultiplyParallel(Matrix *matrixChunkA, Matrix *matrixChunkB, Matrix *matrixC, int idPackage, int offSetA, int offSetB,
                              int chunkToCalc, int id, agoLogStr *agoLog)
{
  int i = 0, j = 0, k = 0;
  double d_multiplyTime = 0.0;

  snprintf(agoLog->logMsg, LOG_SIZE, "Multiplicando pacote id %d: OffSetA %d OffSetB %d", idPackage, offSetA, offSetB);
  registerLog(agoLog, id);

  for (k = 0; k < chunkToCalc; k++) {
    for (i = 0; i < chunkToCalc; i++) {
      for (j = 0; j < MAX_MATRIX_SIZE; j++) {
        matrixC[i][k] = matrixC[i][k] + (matrixChunkA[i][j] * matrixChunkB[i][j]);
      }
    }
  }

  snprintf(agoLog->logMsg, LOG_SIZE, "Multiplicao Concluida pacote id %d: OffSetA %d OffSetB %d", idPackage, offSetA, offSetB);
  registerLog(agoLog, id);

  return(d_multiplyTime);
}

void matrixSaveToDisk(char *fileName, Matrix *matrixToSave, int sizeMatrix, agoLogStr *agoLog)
{
  FILE *fdOut;

  fdOut = fopen(fileName, "w+");

  if (fdOut == NULL) {
    perror("Erro Carregando Matriz");
  }

  (void) fwrite(matrixToSave, sizeof(double), sizeMatrix, fdOut);

  snprintf(agoLog->logMsg, LOG_SIZE, "Salvando Matriz %s", fileName);

  fclose(fdOut);
}

void matrixReadFromFile(char *fileName, Matrix *matrixToLoad, int sizeMatrix, int myid, agoLogStr *agoLog)
{
  FILE *fdIn;

  if ((fdIn = fopen(fileName, "r")) == NULL) {
    snprintf(agoLog->logMsg, LOG_SIZE, "Erro ao tentar carregar Matriz do Arquivo %s", fileName);
    registerLog(agoLog, myid);
  } else {
    if ((fread(matrixToLoad, sizeof(double), sizeMatrix, fdIn)) == 0) {
      snprintf(agoLog->logMsg, LOG_SIZE, "Erro ao Carregar Matriz do Arquivo %s", fileName);
      registerLog(agoLog, myid);
    } else {
      snprintf(agoLog->logMsg, LOG_SIZE, "Carregado Matriz do Arquivo %s", fileName);
      registerLog(agoLog, myid);
    }
  }

  fclose(fdIn);
}

void matrixShow(Matrix *matrix, int maxI, int maxJ)
{
  int i = 0, j = 0;

  for (i = 0; i < maxI; i++) {
    for (j = 0; j < maxJ; j++) {
      fprintf(stdout, " %2.f ", matrix[i][j]);
    }
    fprintf(stdout, "\n");
  }

}
/* agomatrix.c */
/*  Under the Terms of the GPL - 2005,06 - Marcelo Soares Souza - CEBACAD (http://www.cebacad.net) */
