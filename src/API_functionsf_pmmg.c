/**
 * \file API_functionsf_pmmg.c
 * \brief Fortran API functions for PARMMG library.
 *
 * Define the Fortran API functions for PARMMG library: adds function
 * definitions with upcase, underscore and double underscore to match
 * any fortran compiler.
 *
 */
#include "parmmg.h"

/**
 * See \ref PMMG_Init_parmesh function in \ref libparmmg.h file.
 */
FORTRAN_VARIADIC ( PMMG_INIT_PARMESH, pmmg_init_parmesh,
                   (const int starter, ... ),
                   va_list argptr;
                   int     ier;

                   va_start(argptr, starter);

                   ier = PMMG_Init_parMesh_var_internal(argptr,0);

                   va_end(argptr);

                   if ( !ier ) exit(EXIT_FAILURE);

                   return;
  )

/**
 * See \ref PMMG_Set_inputMeshName function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_INPUTMESHNAME, pmmg_set_inputmeshname,
             (PMMG_pParMesh *parmesh, char* meshin, int *strlen, int* retval),
             (parmesh,meshin,strlen,retval)) {
  char *tmp = NULL;

  _MMG5_SAFE_MALLOC(tmp,(*strlen+1),char,);
  strncpy(tmp,meshin,*strlen);
  tmp[*strlen] = '\0';
  *retval = PMMG_Set_inputMeshName(*parmesh,tmp);
  _MMG5_SAFE_FREE(tmp);

  return;
}

/**
 * See \ref PMMG_Set_inputSolsName function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_INPUTSOLSNAME, pmmg_set_inputsolsname,
             (PMMG_pParMesh *parmesh,char* solin, int* strlen, int* retval),
             (parmesh,solin,strlen,retval)) {

  char *tmp = NULL;

  _MMG5_SAFE_MALLOC(tmp,(*strlen+1),char,);
  strncpy(tmp,solin,*strlen);
  tmp[*strlen] = '\0';
  *retval = PMMG_Set_inputSolsName(*parmesh,tmp);
  _MMG5_SAFE_FREE(tmp);

  return;
}

/**
 * See \ref PMMG_Set_inputMetName function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_INPUTMETNAME, pmmg_set_inputmetname,
             (PMMG_pParMesh *parmesh,char* metin, int* strlen, int* retval),
             (parmesh,metin,strlen,retval)) {

  char *tmp = NULL;

  _MMG5_SAFE_MALLOC(tmp,(*strlen+1),char,);
  strncpy(tmp,metin,*strlen);
  tmp[*strlen] = '\0';
  *retval = PMMG_Set_inputMetName(*parmesh,tmp);
  _MMG5_SAFE_FREE(tmp);

  return;
}

/**
 * See \ref PMMG_Set_outputMeshName function in libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_OUTPUTMESHNAME,pmmg_set_outputmeshname,
             (PMMG_pParMesh *parmesh, char* meshout, int* strlen,int* retval),
             (parmesh,meshout,strlen,retval)){
  char *tmp = NULL;

  _MMG5_SAFE_MALLOC(tmp,(*strlen+1),char,);
  strncpy(tmp,meshout,*strlen);
  tmp[*strlen] = '\0';
  *retval = PMMG_Set_outputMeshName(*parmesh, tmp);
  _MMG5_SAFE_FREE(tmp);

  return;
}

/**
 * See \ref PMMG_Set_outputSolsName function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_OUTPUTSOLSNAME,pmmg_set_outputsolsname,
             (PMMG_pParMesh *parmesh, char* solout,int* strlen, int* retval),
             (parmesh,solout,strlen,retval)){
  char *tmp = NULL;

  _MMG5_SAFE_MALLOC(tmp,(*strlen+1),char,);
  strncpy(tmp,solout,*strlen);
  tmp[*strlen] = '\0';
  *retval = PMMG_Set_outputSolsName(*parmesh,tmp);
  _MMG5_SAFE_FREE(tmp);

  return;
}

/**
 * See \ref PMMG_Set_outputMetName function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_OUTPUTMETNAME,pmmg_set_outputmetname,
             (PMMG_pParMesh *parmesh, char* metout,int* strlen, int* retval),
             (parmesh,metout,strlen,retval)){
  char *tmp = NULL;

  _MMG5_SAFE_MALLOC(tmp,(*strlen+1),char,);
  strncpy(tmp,metout,*strlen);
  tmp[*strlen] = '\0';
  *retval = PMMG_Set_outputMetName(*parmesh,tmp);
  _MMG5_SAFE_FREE(tmp);

  return;
}

/**
 * See \ref PMMG_Init_parameters function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_INIT_PARAMETERS,pmmg_init_parameters,
             (PMMG_pParMesh *parmesh,MPI_Comm *comm),
             (parmesh,comm)) {

  PMMG_Init_parameters(*parmesh,*comm);

  return;
}

/**
 * See \ref PMMG_Set_meshSize function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_MESHSIZE, pmmg_set_meshsize,
    (PMMG_pParMesh *parmesh, int* np, int* ne, int* nprism, int* nt,
     int* nquad, int* na, int* retval),
    (parmesh, np, ne, nprism, nt, nquad, na, retval)) {

  *retval = PMMG_Set_meshSize(*parmesh, *np, *ne, *nprism, *nt, *nquad,*na);

  return;
}

/**
 * See \ref PMMG_Set_solsAtVerticesSize function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_SOLSATVERTICESSIZE, pmmg_set_solsatverticessize,
    (PMMG_pParMesh *parmesh,int *nsols,int *nentities,int* typSol,
     int* retval),
    (parmesh, nsols, nentities, typSol, retval)) {
  *retval = PMMG_Set_solsAtVerticesSize(*parmesh,*nsols,*nentities,typSol);
  return;
}

/**
 * See \ref PMMG_Set_metSize function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_METSIZE, pmmg_set_metsize,
    (PMMG_pParMesh *parmesh,int* typEntity,int *np,int* typMet,
     int* retval),
    (parmesh, typEntity, np, typMet, retval)) {
  *retval = PMMG_Set_metSize(*parmesh,*typEntity,*np,*typMet);
  return;
}

/**
 * See \ref PMMG_Set_vertex function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_VERTEX,pmmg_set_vertex,
             (PMMG_pParMesh *parmesh, double* c0, double* c1, double* c2, int* ref,
              int* pos, int* retval),
             (parmesh,c0,c1,c2,ref,pos,retval)) {

  *retval = PMMG_Set_vertex(*parmesh,*c0,*c1,*c2,*ref,*pos);
  return;
}

/**
 * See \ref PMMG_Set_vertices function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_VERTICES,pmmg_set_vertices,
             (PMMG_pParMesh *parmesh, double* vertices, int* refs, int* retval),
             (parmesh,vertices,refs,retval)) {

  *retval = PMMG_Set_vertices(*parmesh,vertices,refs);
  return;
}

/**
 * See \ref PMMG_Set_tetrahedron function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_TETRAHEDRON,pmmg_set_tetrahedron,
             (PMMG_pParMesh *parmesh, int *v0, int *v1, int *v2, int *v3, int *ref,
              int *pos, int* retval),
             (parmesh,v0,v1,v2,v3,ref,pos,retval)){
  *retval = PMMG_Set_tetrahedron(*parmesh,*v0,*v1,*v2,*v3,*ref,*pos);
  return;
}

/**
 * See \ref PMMG_Set_tetrahedra function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_TETRAHEDRA,pmmg_set_tetrahedra,
             (PMMG_pParMesh *parmesh, int *tetra, int *refs, int* retval),
             (parmesh,tetra,refs,retval)){
  *retval = PMMG_Set_tetrahedra(*parmesh,tetra,refs);
  return;
}

/**
 * See \ref PMMG_Set_prism function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_PRISM,pmmg_set_prism,
             (PMMG_pParMesh *parmesh, int *v0, int *v1, int *v2, int *v3,
              int *v4, int *v5,int *ref,int *pos, int* retval),
             (parmesh,v0,v1,v2,v3,v4,v5,ref,pos,retval)){
  *retval = PMMG_Set_prism(*parmesh,*v0,*v1,*v2,*v3,*v4,*v5,*ref,*pos);
  return;
}

/**
 * See \ref PMMG_Set_prisms function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_PRISMS,pmmg_set_prisms,
             (PMMG_pParMesh *parmesh, int *prisms, int *refs, int* retval),
             (parmesh,prisms,refs,retval)){
  *retval = PMMG_Set_prisms(*parmesh,prisms,refs);
  return;
}

/**
 * See \ref PMMG_Set_triangle function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_TRIANGLE,pmmg_set_triangle,
             (PMMG_pParMesh *parmesh, int* v0, int* v1, int* v2, int* ref,int* pos,
              int* retval),
             (parmesh,v0,v1,v2,ref,pos,retval)) {
  *retval = PMMG_Set_triangle(*parmesh, *v0, *v1, *v2, *ref, *pos);
  return;
}

/**
 * See \ref PMMG_Set_triangles function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_TRIANGLES,pmmg_set_triangles,
             (PMMG_pParMesh *parmesh, int* tria, int* refs,
              int* retval),
             (parmesh,tria,refs,retval)) {
  *retval = PMMG_Set_triangles(*parmesh, tria, refs);
  return;
}

/**
 * See \ref PMMG_Set_quadrilateral function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_QUADRILATERAL,pmmg_set_quadrilateral,
             (PMMG_pParMesh *parmesh, int* v0, int* v1, int* v2,int *v3,
              int* ref,int* pos,int* retval),
             (parmesh,v0,v1,v2,v3,ref,pos,retval)) {
  *retval = PMMG_Set_quadrilateral(*parmesh, *v0, *v1, *v2, *v3,*ref, *pos);
  return;
}

/**
 * See \ref PMMG_Set_quadrilaterals function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_QUADRILATERALS,pmmg_set_quadrilaterals,
             (PMMG_pParMesh *parmesh, int* quads, int* refs,
              int* retval),
             (parmesh,quads,refs,retval)) {
  *retval = PMMG_Set_quadrilaterals(*parmesh, quads, refs);
  return;
}
/**
 * See \ref PMMG_Set_edge function in \ref parmmg/libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_EDGE,pmmg_set_edge,
             (PMMG_pParMesh *parmesh, int *v0, int *v1, int *ref, int *pos, int* retval),
             (parmesh,v0,v1,ref,pos,retval)){
  *retval = PMMG_Set_edge(*parmesh,*v0,*v1,*ref,*pos);
  return;
}
/**
 * See \ref PMMG_Set_edges function in \ref parmmg/libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_EDGES,pmmg_set_edges,
             (PMMG_pParMesh *parmesh, int *edges, int *refs,int* retval),
             (parmesh,edges,refs,retval)){
  *retval = PMMG_Set_edges(*parmesh,edges,refs);
  return;
}

/**
 * See \ref PMMG_Set_corner function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_CORNER,pmmg_set_corner,(PMMG_pParMesh *parmesh, int *k, int* retval),
             (parmesh,k,retval)) {
  *retval =  PMMG_Set_corner(*parmesh,*k);
  return;
}

/**
 * See \ref PMMG_Set_requiredVertex function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_REQUIREDVERTEX,pmmg_set_requiredvertex,
             (PMMG_pParMesh *parmesh, int *k, int* retval),
             (parmesh,k,retval)) {
  *retval =  PMMG_Set_requiredVertex(*parmesh,*k);
  return;
}

/**
 * See \ref PMMG_Set_requiredTetrahedron function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_REQUIREDTETRAHEDRON,pmmg_set_requiredtetrahedron,
             (PMMG_pParMesh *parmesh, int *k, int* retval),
             (parmesh,k,retval)) {
  *retval = PMMG_Set_requiredTetrahedron(*parmesh,*k);
  return;
}

/**
 * See \ref PMMG_Set_requiredTetrahedra function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_REQUIREDTETRAHEDRA,pmmg_set_requiredtetrahedra,
             (PMMG_pParMesh *parmesh, int *reqIdx, int *nreq, int* retval),
             (parmesh,reqIdx,nreq,retval)) {
  *retval = PMMG_Set_requiredTetrahedra(*parmesh,reqIdx, *nreq);
  return;
}

/**
 * See \ref PMMG_Set_requiredTriangle function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_REQUIREDTRIANGLE,pmmg_set_requiredtriangle,
             (PMMG_pParMesh *parmesh, int *k, int* retval),
             (parmesh,k,retval)) {
  *retval = PMMG_Set_requiredTriangle(*parmesh, *k);
  return;
}

/**
 * See \ref PMMG_Set_requiredTriangles function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_REQUIREDTRIANGLES,pmmg_set_requiredtriangles,
             (PMMG_pParMesh *parmesh, int *reqIdx, int *nreq, int* retval),
             (parmesh,reqIdx,nreq,retval)) {
  *retval = PMMG_Set_requiredTriangles(*parmesh, reqIdx, *nreq);
  return;
}

/**
 * See \ref PMMG_Set_ridge function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_RIDGE,pmmg_set_ridge,
             (PMMG_pParMesh *parmesh, int *k, int* retval),
             (parmesh,k,retval)) {
  *retval = PMMG_Set_ridge(*parmesh,*k);
  return;
}

/**
 * See \ref PMMG_Set_requiredEdge function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_REQUIREDEDGE,pmmg_set_requirededge,
             (PMMG_pParMesh *parmesh, int *k, int* retval),
             (parmesh,k,retval)) {
  *retval = PMMG_Set_requiredEdge(*parmesh,*k);
  return;
}

/**
 * See \ref PMMG_Set_normalAtVertex function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_NORMALATVERTEX,pmmg_set_normalatvertex,
             (PMMG_pParMesh *parmesh, int *k, double* n0, double* n1, double* n2,int* retval),
             (parmesh,k,n0,n1,n2,retval)) {
  *retval = PMMG_Set_normalAtVertex(*parmesh,*k, *n0, *n1, *n2);
  return;
}
/**
 * See \ref PMMG_Set_ithSol_inSolsAtVertices function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_ITHSOL_INSOLSATVERTICES,pmmg_set_ithsol_insolsatvertices,
             (PMMG_pParMesh *parmesh, int *i,double *s,int *pos,int* retval),
             (parmesh,i,s,pos,retval)) {
  int idx = *i-1;
  *retval = PMMG_Set_ithSol_inSolsAtVertices(*parmesh,idx,s,*pos);
  return;
}
/**
 * See \ref PMMG_Set_ithSols_inSolsAtVertices function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_ITHSOLS_INSOLSATVERTICES,pmmg_set_ithsols_insolsatvertices,
             (PMMG_pParMesh *parmesh, int *i,double *s, int* retval),
             (parmesh,i,s,retval)) {
  int idx = *i-1;
  *retval = PMMG_Set_ithSols_inSolsAtVertices(*parmesh,idx,s);
  return;
}

/**
 * See \ref PMMG_Set_scalarMet function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_SCALARMET,pmmg_set_scalarmet,
             (PMMG_pParMesh *parmesh, double *m, int *pos, int* retval),
             (parmesh,m,pos,retval)) {
  *retval = PMMG_Set_scalarMet(*parmesh,*m,*pos);
  return;
}

/**
 * See \ref PMMG_Set_scalaMets function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_SCALARMETS,pmmg_set_scalarmets,
             (PMMG_pParMesh *parmesh, double *m, int* retval),
             (parmesh,m,retval)) {
  *retval = PMMG_Set_scalarMets(*parmesh,m);
  return;
}

/**
 * See \ref PMMG_Set_vectorMet function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_VECTORMET,pmmg_set_vectormet,
             (PMMG_pParMesh *parmesh, double *vx, double *vy, double *vz,
              int *pos, int* retval),
             (parmesh,vx,vy,vz,pos,retval)) {
  *retval = PMMG_Set_vectorMet(*parmesh,*vx,*vy,*vz,*pos);
  return;
}

/**
 * See \ref PMMG_Set_vectorMets function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_VECTORMETS,pmmg_set_vectormets,
             (PMMG_pParMesh *parmesh, double *mets, int* retval),
             (parmesh,mets,retval)) {
  *retval = PMMG_Set_vectorMets(*parmesh,mets);
  return;
}

/**
 * See \ref PMMG_Set_tensorMet function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_TENSORMET,pmmg_set_tensormet,
             (PMMG_pParMesh *parmesh, double* m11,double *m12, double *m13,
              double* m22,double *m23, double *m33, int *pos, int* retval),
             (parmesh,m11,m12,m13,m22,m23,m33,pos,retval)) {
  *retval = PMMG_Set_tensorMet(*parmesh,*m11,*m12,*m13,*m22,*m23,*m33,*pos);
  return;
}

/**
 * See \ref PMMG_Set_tensorMets function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SET_TENSORMETS,pmmg_set_tensormets,
             (PMMG_pParMesh *parmesh, double* mets,int* retval),
             (parmesh,mets,retval)) {
  *retval = PMMG_Set_tensorMets(*parmesh,mets);
  return;
}

/**
 * See \ref PMMG_Get_meshSize function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_MESHSIZE, pmmg_get_meshsize,
    (PMMG_pParMesh *parmesh, int* np, int* ne, int* nprism, int* nt,
     int* nquad, int* na, int* retval),
    (parmesh, np, ne, nprism, nt, nquad, na, retval)) {

  *retval = PMMG_Get_meshSize(*parmesh, np, ne, nprism, nt, nquad, na);

  return;
}

/**
 * See \ref PMMG_Get_solsAtVerticesSize function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_SOLSATVERTICESSIZE, pmmg_get_solsatverticessize,
    (PMMG_pParMesh *parmesh,int *nsols,int *nentities,int* typSol,
     int* retval),
    (parmesh, nsols, nentities, typSol, retval)) {
  *retval = PMMG_Get_solsAtVerticesSize(*parmesh,nsols,nentities,typSol);
  return;
}

/**
 * See \ref PMMG_Get_metSize function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_METSIZE, pmmg_get_metsize,
    (PMMG_pParMesh *parmesh,int* typEntity,int *np,int* typMet,
     int* retval),
    (parmesh, typEntity, np, typMet, retval)) {
  *retval = PMMG_Get_metSize(*parmesh,typEntity,np,typMet);
  return;
}

/**
 * See \ref PMMG_Get_vertex function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_VERTEX,pmmg_get_vertex,
             (PMMG_pParMesh *parmesh, double* c0, double* c1, double* c2, int* ref,
              int* isCorner, int* isRequired, int* retval),
             (parmesh,c0,c1,c2,ref,isCorner,isRequired, retval)) {
  *retval = PMMG_Get_vertex(*parmesh,c0,c1,c2,ref,isCorner,isRequired);
  return;
}

/**
 * See \ref PMMG_Get_vertices function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_VERTICES,pmmg_get_vertices,
             (PMMG_pParMesh *parmesh, double* vertices, int* refs,
              int* areCorners, int* areRequired, int* retval),
             (parmesh,vertices,refs,areCorners,areRequired, retval)) {
  *retval = PMMG_Get_vertices(*parmesh,vertices,refs,areCorners,areRequired);
  return;
}

/**
 * See \ref PMMG_Get_tetrahedron function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_TETRAHEDRON,pmmg_get_tetrahedron,
             (PMMG_pParMesh *parmesh, int* v0, int* v1, int* v2, int *v3,
              int* ref, int* isRequired, int* retval),
             (parmesh,v0,v1,v2,v3,ref,isRequired,retval)) {
  *retval = PMMG_Get_tetrahedron(*parmesh,v0,v1,v2,v3,ref,isRequired);
  return;
}

/**
 * See \ref PMMG_Get_tetrahedra function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_TETRAHEDRA,pmmg_get_tetrahedra,
             (PMMG_pParMesh *parmesh, int* tetra, int* refs, int* areRequired,
              int* retval),
             (parmesh,tetra,refs,areRequired,retval)) {
  *retval = PMMG_Get_tetrahedra(*parmesh,tetra,refs,areRequired);
  return;
}

/**
 * See \ref PMMG_Get_prism function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_PRISM,pmmg_get_prism,
             (PMMG_pParMesh *parmesh, int* v0, int* v1, int* v2, int *v3,
              int *v4, int* v5,int* ref, int* isRequired, int* retval),
             (parmesh,v0,v1,v2,v3,v4,v5,ref,isRequired,retval)) {
  *retval = PMMG_Get_prism(*parmesh,v0,v1,v2,v3,v4,v5,ref,isRequired);
  return;
}

/**
 * See \ref PMMG_Get_prisms function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_PRISMS,pmmg_get_prisms,
             (PMMG_pParMesh *parmesh, int* prisms, int* refs, int* areRequired,
              int* retval),
             (parmesh,prisms,refs,areRequired,retval)) {
  *retval = PMMG_Get_prisms(*parmesh,prisms,refs,areRequired);
  return;
}

/**
 * See \ref PMMG_Get_triangle function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_TRIANGLE,pmmg_get_triangle,
             (PMMG_pParMesh *parmesh, int* v0, int* v1, int* v2, int* ref
              ,int* isRequired, int* retval),
             (parmesh,v0,v1,v2,ref,isRequired,retval)) {
  *retval = PMMG_Get_triangle(*parmesh,v0,v1,v2,ref,isRequired);
  return;
}

/**
 * See \ref PMMG_Get_triangles function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_TRIANGLES,pmmg_get_triangles,
             (PMMG_pParMesh *parmesh, int* tria, int* refs,int* areRequired,
              int* retval),
             (parmesh,tria,refs,areRequired,retval)) {
  *retval = PMMG_Get_triangles(*parmesh,tria,refs,areRequired);
  return;
}

/**
 * See \ref PMMG_Get_quadrilateral function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_QUADRILATERAL,pmmg_get_quadrilateral,
             (PMMG_pParMesh *parmesh, int* v0, int* v1, int* v2,int *v3,
               int* ref,int* isRequired, int* retval),
             (parmesh,v0,v1,v2,v3,ref,isRequired,retval)) {
  *retval = PMMG_Get_quadrilateral(*parmesh,v0,v1,v2,v3,ref,isRequired);
  return;
}

/**
 * See \ref PMMG_Get_quadrilaterals function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_QUADRILATERALS,pmmg_get_quadrilaterals,
             (PMMG_pParMesh *parmesh, int* quads, int* refs,int* areRequired,
              int* retval),
             (parmesh,quads,refs,areRequired,retval)) {
  *retval = PMMG_Get_quadrilaterals(*parmesh,quads,refs,areRequired);
  return;
}

/**
 * See \ref PMMG_Get_edge function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_EDGE,pmmg_get_edge,(PMMG_pParMesh *parmesh, int* e0, int* e1, int* ref
                                          ,int* isRidge, int* isRequired, int* retval),
             (parmesh,e0,e1,ref,isRidge,isRequired,retval)) {
  *retval = PMMG_Get_edge(*parmesh,e0,e1,ref,isRidge,isRequired);
  return;
}
/**
 * See \ref PMMG_Get_edges function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_EDGES,pmmg_get_edges,(PMMG_pParMesh *parmesh, int* edges, int* refs
                                            ,int* areRidges, int* areRequired, int* retval),
             (parmesh,edges,refs,areRidges,areRequired,retval)) {
  *retval = PMMG_Get_edges(*parmesh,edges,refs,areRidges,areRequired);
  return;
}

/**
 * See \ref PMMG_Get_normalAtVertex function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_NORMALATVERTEX,pmmg_get_normalatvertex,
             (PMMG_pParMesh *parmesh, int *k, double* n0, double* n1, double* n2,int* retval),
             (parmesh,k,n0,n1,n2,retval)) {
  *retval = PMMG_Get_normalAtVertex(*parmesh,*k, n0, n1, n2);
  return;
}
/**
 * See \ref PMMG_Get_ithSols_inSolsAtVertices function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_ITHSOL_INSOLSATVERTICES,pmmg_get_ithsol_insolsatvertices,
             (PMMG_pParMesh *parmesh, int *i,double *s,int *pos, int* retval),
             (parmesh,i,s,pos,retval)) {
  int idx = *i-1;
  *retval = PMMG_Get_ithSol_inSolsAtVertices(*parmesh,idx,s,*pos);
  return;
}

/**
 * See \ref PMMG_Get_ithSols_inSolsAtVertices function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_ITHSOLS_INSOLSATVERTICES,pmmg_get_ithsols_insolsatvertices,
             (PMMG_pParMesh *parmesh, int *i,double *s, int* retval),
             (parmesh,i,s,retval)) {
  int idx = *i-1;
  *retval = PMMG_Get_ithSols_inSolsAtVertices(*parmesh,idx,s);
  return;
}

/**
 * See \ref PMMG_Get_scalarMet function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_SCALARMET,pmmg_get_scalarmet,
             (PMMG_pParMesh *parmesh, double* m, int* retval),
             (parmesh,m,retval)) {
  *retval = PMMG_Get_scalarMet(*parmesh,m);
  return;
}

/**
 * See \ref PMMG_Get_scalarMets function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_SCALARMETS,pmmg_get_scalarmets,
             (PMMG_pParMesh *parmesh, double* m, int* retval),
             (parmesh,m,retval)) {
  *retval = PMMG_Get_scalarMets(*parmesh,m);
  return;
}

/**
 * See \ref PMMG_Get_vectorMet function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_VECTORMET,pmmg_get_vectormet,
             (PMMG_pParMesh *parmesh, double* vx,double *vy, double *vz, int* retval),
             (parmesh,vx,vy,vz,retval)) {
  *retval = PMMG_Get_vectorMet(*parmesh,vx,vy,vz);
  return;
}

/**
 * See \ref PMMG_Get_vectorMets function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_VECTORMETS,pmmg_get_vectormets,
             (PMMG_pParMesh *parmesh, double* mets, int* retval),
             (parmesh,mets,retval)) {
  *retval = PMMG_Get_vectorMets(*parmesh,mets);
  return;
}

/**
 * See \ref PMMG_Get_tensorMet function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_TENSORMET,pmmg_get_tensormet,
             (PMMG_pParMesh *parmesh, double* m11,double *m12, double *m13,
              double* m22,double *m23, double *m33, int* retval),
             (parmesh,m11,m12,m13,m22,m23,m33,retval)) {
  *retval = PMMG_Get_tensorMet(*parmesh,m11,m12,m13,m22,m23,m33);
  return;
}

/**
 * See \ref PMMG_Get_tensorMets function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_GET_TENSORMETS,pmmg_get_tensormets,
             (PMMG_pParMesh *parmesh, double* mets, int* retval),
             (parmesh,mets,retval)) {
  *retval = PMMG_Get_tensorMets(*parmesh,mets);
  return;
}

/**
 * See \ref PMMG_Free_all function in \ref mmg3d/libmmg3d.h file.
 */
FORTRAN_VARIADIC(PMMG_FREE_ALL,pmmg_free_all,
                 (const int starter,...),
                 va_list argptr;
                 int     ier;

                 va_start(argptr, starter);

                 ier = PMMG_Free_all_var(argptr);

                 va_end(argptr);

                 if ( !ier ) exit(EXIT_FAILURE);
                 return;
  )


/**
 * See \ref PMMG_parmmglib_distributed function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_PARMMGLIB_DISTRIBUTED,pmmg_parmmglib_distributed,
             (PMMG_pParMesh *parmesh,int* retval),
             (parmesh,retval)) {
  *retval = PMMG_parmmglib_distributed(*parmesh);
  return;
}

/**
 * See \ref PMMG_parmmglib_centralized function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_PARMMGLIB_CENTRALIZED,pmmg_parmmglib_centralized,
             (PMMG_pParMesh *parmesh,int* retval),
             (parmesh,retval)) {
  *retval = PMMG_parmmglib_centralized(*parmesh);
  return;
}

/**
 * See \ref PMMG_loadMesh function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_LOADMESH_CENTRALIZED,pmmg_loadmesh_centralized,
             (PMMG_pParMesh *parmesh,char* filename, int *strlen,int* retval),
             (parmesh,filename,strlen, retval)){
  char *tmp = NULL;

  _MMG5_SAFE_MALLOC(tmp,(*strlen+1),char,);
  strncpy(tmp,filename,*strlen);
  tmp[*strlen] = '\0';

  *retval = PMMG_loadMesh_centralized(*parmesh,tmp);

  _MMG5_SAFE_FREE(tmp);

  return;
}

/**
 * See \ref PMMG_loadMet_centralized function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_LOADMET_CENTRALIZED,pmmg_loadmet_centralized,
             (PMMG_pParMesh *parmesh,char* filename, int *strlen,int* retval),
             (parmesh,filename,strlen,retval)){
  char *tmp = NULL;

  _MMG5_SAFE_MALLOC(tmp,(*strlen+1),char,);
  strncpy(tmp,filename,*strlen);
  tmp[*strlen] = '\0';

  *retval = PMMG_loadMet_centralized(*parmesh,tmp);

  _MMG5_SAFE_FREE(tmp);

  return;
}

/**
 * See \ref PMMG_loadAllSols_centralized function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_LOADALLSOLS_CENTRALIZED,pmmg_loadallsols_centralized,
             (PMMG_pParMesh *parmesh,char* filename, int *strlen,int* retval),
             (parmesh,filename,strlen,retval)){
  char *tmp = NULL;

  _MMG5_SAFE_MALLOC(tmp,(*strlen+1),char,);
  strncpy(tmp,filename,*strlen);
  tmp[*strlen] = '\0';

  *retval = PMMG_loadAllSols_centralized(*parmesh,tmp);

  _MMG5_SAFE_FREE(tmp);

  return;
}

/**
 * See \ref PMMG_saveMesh_centralized function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SAVEMESH_CENTRALIZED,pmmg_savemesh_centralized,
             (PMMG_pParMesh *parmesh,char* filename, int *strlen,int* retval),
             (parmesh,filename,strlen, retval)){
  char *tmp = NULL;

  _MMG5_SAFE_MALLOC(tmp,(*strlen+1),char,);
  strncpy(tmp,filename,*strlen);
  tmp[*strlen] = '\0';

  *retval = PMMG_saveMesh_centralized(*parmesh,tmp);

  _MMG5_SAFE_FREE(tmp);

  return;
}

/**
 * See \ref PMMG_saveMet_centralized function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SAVEMET_CENTRALIZED,pmmg_savemet_centralized,
             (PMMG_pParMesh *parmesh,char* filename, int *strlen,int* retval),
             (parmesh,filename,strlen,retval)){
  char *tmp = NULL;

  _MMG5_SAFE_MALLOC(tmp,(*strlen+1),char,);
  strncpy(tmp,filename,*strlen);
  tmp[*strlen] = '\0';

  *retval = PMMG_saveMet_centralized(*parmesh,tmp);

  _MMG5_SAFE_FREE(tmp);

  return;
}

/**
 * See \ref PMMG_saveAllSols_centralized function in \ref libparmmg.h file.
 */
FORTRAN_NAME(PMMG_SAVEALLSOLS_CENTRALIZED,pmmg_saveallsols_centralized,
             (PMMG_pParMesh *parmesh,char* filename, int *strlen,int* retval),
             (parmesh,filename,strlen,retval)){
  char *tmp = NULL;

  _MMG5_SAFE_MALLOC(tmp,(*strlen+1),char,);
  strncpy(tmp,filename,*strlen);
  tmp[*strlen] = '\0';

  *retval = PMMG_saveAllSols_centralized(*parmesh,tmp);

  _MMG5_SAFE_FREE(tmp);

  return;
}
