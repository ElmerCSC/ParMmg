/**
 * \file parmmg.h
 * \brief internal functions headers for parmmg
 * \author
 * \version
 * \date 11 2016
 * \copyright
 */

#ifndef _PARMMG_H
#define _PARMMG_H

#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <mpi.h>

#include "libparmmg.h"

#ifdef __cplusplus
extern "C" {
#endif


#define ERROR_AT(msg1,msg2) do {                                     \
  fprintf( stderr, msg1 msg2 " function: %s, file: %s, line: %d \n", \
           __func__, __FILE__, __LINE__ );                           \
  }while(0)

#define MEM_CHK_AVAIL(mesh,bytes,msg) do {            \
  if (   ((mesh)->memCur + (bytes) > (mesh)->memMax ) \
      || ((mesh)->memCur + (bytes) < 0 ) ) {          \
    ERROR_AT(msg," Exceeded max memory allowed or tried to free more mem than allocated: " );\
    stat = PMMG_FAILURE;                              \
  }                                                   \
  stat = PMMG_SUCCESS;                                \
  } while(0)

#define PMMG_FREE(mesh,ptr,size,type,msg) do {  \
  int stat = PMMG_SUCCESS;                      \
  MEM_CHK_AVAIL(mesh,-(size)*sizeof(type),msg); \
  if ( stat == PMMG_SUCCESS )                   \
    (mesh)->memCur -= (size) * sizeof(type);    \
  free( ptr );                                  \
  ptr = NULL;                                   \
  } while(0)

#define PMMG_MALLOC(mesh,ptr,size,type,msg,on_failure) do { \
  int stat = PMMG_SUCCESS;                                  \
  MEM_CHK_AVAIL(mesh,(size)*sizeof(type),msg );             \
  if ( stat == PMMG_SUCCESS ) {                             \
    ptr = malloc( (size) * sizeof(type) );                  \
    if ( ptr == NULL ) {                                    \
      ERROR_AT( msg, " malloc failed: " );                  \
      on_failure;                                           \
    } else {                                                \
      (mesh)->memCur += (size) * sizeof(type);              \
      stat = PMMG_SUCCESS;                                  \
    }                                                       \
  } else {                                                  \
    on_failure;                                             \
  } } while(0)

#define PMMG_CALLOC(mesh,ptr,size,type,msg,on_failure) do { \
  int stat = PMMG_SUCCESS;                                  \
  MEM_CHK_AVAIL(mesh,(size)*sizeof(type),msg);              \
  if ( stat == PMMG_SUCCESS ) {                             \
    ptr = calloc( (size), sizeof(type) );                   \
    if ( ptr == NULL ) {                                    \
      ERROR_AT(msg," calloc failed: ");                     \
      on_failure;                                           \
    } else {                                                \
      (mesh)->memCur += (size) * sizeof(type);              \
    }                                                       \
  } else {                                                  \
    on_failure;                                             \
  } } while(0)

#define PMMG_REALLOC(mesh,ptr,newsize,oldsize,type,msg,on_failure) do {       \
  int stat = PMMG_SUCCESS;                                                    \
  if ( ptr == NULL ) {                                                        \
    assert(((oldsize)==0) && "NULL pointer pointing to non 0 sized memory?"); \
    PMMG_MALLOC(mesh,ptr,(newsize),type,msg,on_failure);                      \
  } else if ((newsize)==0) {                                                  \
      PMMG_FREE(mesh,ptr,(oldsize),type,msg);                                 \
  } else if ((newsize) < (oldsize)) {                                         \
    ptr = realloc( ptr, (newsize) * sizeof(type));                            \
    if ( ptr == NULL ) {                                                      \
      ERROR_AT(msg," Realloc failed: ");                                      \
      on_failure;                                                             \
    } else {                                                                  \
      (mesh)->memCur += ((newsize)-(oldsize)) * sizeof(type);                 \
      on_failure;                                                             \
    }                                                                         \
  } else {                                                                    \
    MEM_CHK_AVAIL(mesh,((newsize)-(oldsize))*sizeof(type),msg);               \
    if ( stat == PMMG_SUCCESS ) {                                             \
      ptr = realloc(ptr, (newsize) * sizeof(type));                           \
      if ( ptr == NULL ) {                                                    \
        ERROR_AT(msg, " Realloc failed: " );                                  \
        on_failure;                                                           \
      } else {                                                                \
        (mesh)->memCur += ( ((newsize)-(oldsize))*sizeof(type));              \
      }                                                                       \
    }                                                                         \
  } } while(0)

#define PMMG_RECALLOC(mesh,ptr,newsize,oldsize,type,msg,on_failure) do { \
  int my_stat = PMMG_SUCCESS;                                            \
  PMMG_REALLOC(mesh,ptr,newsize,oldsize,type,msg,my_stat=PMMG_FAILURE);  \
  if ( (my_stat == PMMG_SUCCESS ) && ((newsize) > (oldsize)) )           \
    memset( (ptr) + oldsize, 0, ((newsize)-(oldsize))*sizeof(type));     \
  } while(0)



void PMMG_PMesh_SetMaxMem( long long int *memmax, long long int memReq );

int PMMG_check_inputData ( PMMG_pParMesh parmesh );

int PMMG_parmmglib1 ( PMMG_pParMesh parmesh );

int  PMMG_mergeGrps ( PMMG_pParMesh parmesh );
int  PMMG_bcastMesh ( PMMG_pParMesh parmesh );
int  PMMG_bdryStore ( MMG5_pMesh mesh );
int  PMMG_bdryUpdate( MMG5_pMesh mesh );

/**
 * \param argc number of command line arguments.
 * \param argv command line arguments.
 * \param mesh pointer toward the mesh structure.
 * \param met pointer toward the sol structure.
 * \return 1.
 *
 * Store command line arguments.
 *
 * \remark no matching fortran function.
 *
 */
int PMMG_parsar( int argc, char *argv[], PMMG_pParMesh parmesh );

void PMMG_return_and_free( PMMG_pParMesh parmesh, const int val );
#ifdef __cplusplus
}
#endif

#endif
