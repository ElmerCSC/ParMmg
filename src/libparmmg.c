/* =============================================================================
**  This file is part of the parmmg software package for parallel tetrahedral
**  mesh modification.
**  Copyright (c) Bx INP/Inria/UBordeaux, 2017-
**
**  parmmg is free software: you can redistribute it and/or modify it
**  under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  parmmg is distributed in the hope that it will be useful, but WITHOUT
**  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
**  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
**  License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License and of the GNU General Public License along with parmmg (in
**  files COPYING.LESSER and COPYING). If not, see
**  <http://www.gnu.org/licenses/>. Please read their terms carefully and
**  use this copy of the parmmg distribution only if you accept them.
** =============================================================================
*/

/**
 * \file libparmmg.c
 * \brief Wrapper for the parallel remeshing library.
 * \author Cécile Dobrzynski (Bx INP/Inria/UBordeaux)
 * \author Algiane Froehly (InriaSoft)
 * \author Nikos Pattakos (Inria)
 * \version 1
 * \copyright GNU Lesser General Public License.
 *
 * Mmain library functions (parallel remeshing starting from centralized or
 * distributed data.
 *
 */

#include "parmmg.h"
#include "git_log_pmmg.h"

/* Declared in the header, but defined at compile time */
int (*PMMG_interp4bar)(MMG5_pMesh mesh,MMG5_pSol met,MMG5_pSol oldMet,MMG5_pTetra pt,int,PMMG_barycoord *barycoord);
int (*PMMG_interp3bar)(MMG5_pMesh mesh,MMG5_pSol met,MMG5_pSol oldMet,MMG5_pTria ptr,int,PMMG_barycoord *barycoord);
int (*PMMG_interp2bar)(MMG5_pMesh mesh,MMG5_pSol met,MMG5_pSol oldMet,MMG5_pTria ptr,int ip,int l,PMMG_barycoord *barycoord);

/**
 * \param parmesh pointer toward the parmesh structure.
 *
 * \return 0 if fail, 1 otherwise
 *
 * Check the validity of the input mesh data (tetra orientation, solution
 * compatibility with respect to the provided mesh, Mmg options).
 *
 */
int PMMG_check_inputData(PMMG_pParMesh parmesh)
{
  MMG5_pMesh mesh;
  MMG5_pSol  met;
  int        k;

  if ( parmesh->info.imprim > PMMG_VERB_VERSION )
    fprintf(stdout,"\n  -- PMMG: CHECK INPUT DATA\n");

  for ( k=0; k<parmesh->ngrp; ++k ) {
    mesh = parmesh->listgrp[k].mesh;
    met  = parmesh->listgrp[k].met;

    /* Check options */
    if ( mesh->info.lag > -1 ) {
      fprintf(stderr,
              "  ## Error: lagrangian mode unavailable (MMG3D_IPARAM_lag):\n");
      return 0;
    } else if ( mesh->info.iso ) {
      fprintf(stderr,"  ## Error: level-set discretisation unavailable"
              " (MMG3D_IPARAM_iso):\n");
      return 0;
    } else if ( mesh->info.optimLES && met->size==6 ) {
      fprintf(stdout,"  ## Error: strong mesh optimization for LES methods"
              " unavailable (MMG3D_IPARAM_optimLES) with an anisotropic metric.\n");
      return 0;
    }
    /* specific meshing */
    if ( met->np ) {
      if ( mesh->info.optim ) {
        printf("\n  ## ERROR: MISMATCH OPTIONS: OPTIM OPTION CAN NOT BE USED"
               " WITH AN INPUT METRIC.\n");
        return 0;
      }

      if ( mesh->info.hsiz>0. ) {
        printf("\n  ## ERROR: MISMATCH OPTIONS: HSIZ OPTION CAN NOT BE USED"
               " WITH AN INPUT METRIC.\n");
        return 0;
      }
    }

    if ( mesh->info.optim &&  mesh->info.hsiz>0. ) {
      printf("\n  ## ERROR: MISMATCH OPTIONS: HSIZ AND OPTIM OPTIONS CAN NOT BE USED"
             " TOGETHER.\n");
      return 0;
    }

    /* load data */
    MMG5_warnOrientation(mesh);

    if ( met->np && (met->np != mesh->np) ) {
      fprintf(stdout,"  ## WARNING: WRONG METRIC NUMBER. IGNORED\n");
      MMG5_DEL_MEM(mesh,met->m);
      met->np = 0;
    } else if ( met->size!=1 && met->size!=6 ) {
      fprintf(stderr,"  ## ERROR: WRONG DATA TYPE.\n");
      return 0;
    }
  }

  return 1;
}

/**
 * \param  parmesh pointer to parmesh structure
 *
 * \return PMMG_SUCCESS if success, PMMG_LOWFAILURE if fail and return an
 * unscaled mesh, PMMG_STRONGFAILURE if fail and return a scaled mesh.
 *
 * Mesh preprocessing: set function pointers, scale mesh, perform mesh
 * analysis and display length and quality histos.
 */
int PMMG_preprocessMesh( PMMG_pParMesh parmesh )
{
  MMG5_pMesh mesh;
  MMG5_pSol  met;

  mesh = parmesh->listgrp[0].mesh;
  met  = parmesh->listgrp[0].met;

  assert ( ( mesh != NULL ) && ( met != NULL ) && "Preprocessing empty args");

  /** Function setters (must be assigned before quality computation) */
  MMG3D_Set_commonFunc();

  /** Mesh scaling and quality histogram */
  if ( !MMG5_scaleMesh(mesh,met,NULL) ) {
    return PMMG_LOWFAILURE;
  }
  /* Don't reset the hmin value computed when unscaling the mesh */
  if ( !parmesh->info.sethmin ) {
    mesh->info.sethmin = 1;
  }
  /* Don't reset the hmax value computed when unscaling the mesh */
  if ( !parmesh->info.sethmax ) {
    mesh->info.sethmax = 1;
  }

  /** specific meshing */
  if ( mesh->info.optim && !met->np ) {
    if ( !MMG3D_doSol(mesh,met) ) {
      return PMMG_STRONGFAILURE;
    }
    MMG5_solTruncatureForOptim(mesh,met);
  }

  if ( mesh->info.hsiz > 0. ) {
    if ( !MMG3D_Set_constantSize(mesh,met) ) {
      return PMMG_STRONGFAILURE;
    }
  }

  MMG3D_setfunc(mesh,met);
  PMMG_setfunc(parmesh);

  if ( !MMG3D_tetraQual( mesh, met, 0 ) ) {
    return PMMG_STRONGFAILURE;
  }

  if ( !PMMG_qualhisto(parmesh,PMMG_INQUA,1) ) {
    return PMMG_STRONGFAILURE;
  }

  /** Mesh analysis */
  if ( !MMG3D_analys(mesh) ) {
    return PMMG_STRONGFAILURE;
  }

  if ( parmesh->info.imprim0 > PMMG_VERB_ITWAVES && (!mesh->info.iso) && met->m ) {
    PMMG_prilen(parmesh,0,1);
  }

  /** Mesh unscaling */
  if ( !MMG5_unscaleMesh(mesh,met,NULL) ) {
    return PMMG_STRONGFAILURE;
  }

  return PMMG_SUCCESS;
}

/**
 * \param  parmesh pointer to parmesh structure
 *
 * \return PMMG_SUCCESS if success, PMMG_LOWFAILURE if fail and return an
 * unscaled mesh, PMMG_STRONGFAILURE if fail and return a scaled mesh.
 *
 * Mesh preprocessing (distributed input mesh): set function pointers,
 * scale mesh, perform mesh analysis, display length and quality histos,
 * and build communicators.
 */
int PMMG_preprocessMesh_distributed( PMMG_pParMesh parmesh )
{
  MMG5_pMesh mesh;
  MMG5_pSol  met;

  mesh = parmesh->listgrp[0].mesh;
  met  = parmesh->listgrp[0].met;

  assert ( ( mesh != NULL ) && ( met != NULL ) && "Preprocessing empty args");

  /** Check distributed API mode. Interface faces OR nodes need to be set by the
   * user through the API interface at this point, meening that the
   * corresponding external comm is set to the correct size, and filled with
   * local entity indices (for node comms, also itosend and itorecv arrays are
   * filled with local/global node IDs).
  */
  if( parmesh->nprocs >1 ) {
    if( parmesh->info.API_mode == PMMG_APIDISTRIB_faces && !parmesh->next_face_comm ) {
      fprintf(stderr," ## Error: %s: parallel interface faces must be set through the API interface\n",__func__);
      return PMMG_STRONGFAILURE;
    } else if( parmesh->info.API_mode == PMMG_APIDISTRIB_nodes && !parmesh->next_node_comm ) {
      fprintf(stderr," ## Error: %s: parallel interface nodes must be set through the API interface\n",__func__);
      return PMMG_STRONGFAILURE;
    }
  }

  /** Function setters (must be assigned before quality computation) */
  MMG3D_Set_commonFunc();

  /** Mesh scaling and quality histogram */
  if ( !MMG5_scaleMesh(mesh,met,NULL) ) {
    return PMMG_LOWFAILURE;
  }
#warning hmin/hmax computed on each proc while we want a global value from the global bounding box and/or the global metric field...
  /* Don't reset the hmin value computed when unscaling the mesh */
  if ( !parmesh->info.sethmin ) {
    mesh->info.sethmin = 1;
  }
  /* Don't reset the hmax value computed when unscaling the mesh */
  if ( !parmesh->info.sethmax ) {
    mesh->info.sethmax = 1;
  }

  /** specific meshing */
  if ( mesh->info.optim && !met->np ) {
    if ( !MMG3D_doSol(mesh,met) ) {
      return PMMG_STRONGFAILURE;
    }
    MMG5_solTruncatureForOptim(mesh,met);
  }

  if ( mesh->info.hsiz > 0. ) {
    if ( !MMG3D_Set_constantSize(mesh,met) ) {
      return PMMG_STRONGFAILURE;
    }
  }

  MMG3D_setfunc(mesh,met);
  PMMG_setfunc(parmesh);

  if ( !MMG3D_tetraQual( mesh, met, 0 ) ) {
    return PMMG_STRONGFAILURE;
  }

  /* For both API modes, build communicators indices */
  switch( parmesh->info.API_mode ) {
    case PMMG_APIDISTRIB_faces :
      /* Set face communicators indexing */
      if( !PMMG_build_faceCommIndex( parmesh ) ) return 0;
      break;
    case PMMG_APIDISTRIB_nodes :
      /* Set node communicators indexing */
      if( !PMMG_build_nodeCommIndex( parmesh ) ) return 0;
      break;
  }

  /** Mesh analysis, face/node communicators indices construction (depending
   * from the API mode), build face comms from node ones */
  if ( !PMMG_analys_tria(parmesh,mesh) ) {
    return PMMG_STRONGFAILURE;
  }
  if( parmesh->info.API_mode == PMMG_APIDISTRIB_faces ) {
    /* Convert tria index into iel face index (it needs a valid cc field in
     * each tria), and tag xtetra face as PARBDY before the tag is transmitted
     * to edges and nodes */
    PMMG_tria2elmFace_coords( parmesh );
  }
  if ( !PMMG_analys(parmesh,mesh) ) {
    return PMMG_STRONGFAILURE;
  }

  if ( parmesh->info.imprim > PMMG_VERB_ITWAVES && (!mesh->info.iso) && met->m ) {
#warning Luca: check this function
    MMG3D_prilen(mesh,met,0);
  }

  /** Mesh unscaling */
  if ( !MMG5_unscaleMesh(mesh,met,NULL) ) {
    return PMMG_STRONGFAILURE;
  }

  /* For both API modes, build communicators indices and set xtetra as PARBDY */
  switch( parmesh->info.API_mode ) {
    case PMMG_APIDISTRIB_faces :
      /* Build node communicators from face ones (here because the (mesh needs
       * to be unscaled) */
      PMMG_parmesh_ext_comm_free( parmesh,parmesh->ext_node_comm,parmesh->next_node_comm);
      PMMG_DEL_MEM(parmesh, parmesh->ext_node_comm,PMMG_Ext_comm,"ext node comm");
      parmesh->next_node_comm = 0;
      PMMG_DEL_MEM(parmesh, parmesh->int_node_comm,PMMG_Int_comm,"int node comm");
      PMMG_CALLOC(parmesh,parmesh->int_node_comm,1,PMMG_Int_comm,"int node comm",return 0);
      if ( !PMMG_build_nodeCommFromFaces(parmesh) ) return PMMG_STRONGFAILURE;
      break;
    case PMMG_APIDISTRIB_nodes :
      /* Build face comms from node ones and set xtetra tags */
      PMMG_parmesh_ext_comm_free( parmesh,parmesh->ext_face_comm,parmesh->next_face_comm);
      PMMG_DEL_MEM(parmesh, parmesh->ext_face_comm,PMMG_Ext_comm,"ext face comm");
      parmesh->next_face_comm = 0;
      PMMG_DEL_MEM(parmesh, parmesh->int_face_comm,PMMG_Int_comm,"int face comm");
      if ( !PMMG_build_faceCommFromNodes(parmesh) ) return PMMG_STRONGFAILURE;
      break;
  }

  /* Tag parallel faces on material interfaces as boundary */
  if( !PMMG_parbdySet( parmesh ) ) {
    fprintf(stderr,"\n  ## Unable to recognize parallel faces on material interfaces. Exit program.\n");
    return 0;
  }


  if ( !PMMG_qualhisto(parmesh,PMMG_INQUA,0) ) {
    return PMMG_STRONGFAILURE;
  }

  MMG5_DEL_MEM(mesh,mesh->tria);
  mesh->nt = 0;

  assert ( PMMG_check_extFaceComm ( parmesh ) );
  assert ( PMMG_check_intFaceComm ( parmesh ) );
  assert ( PMMG_check_extNodeComm ( parmesh ) );
  assert ( PMMG_check_intNodeComm ( parmesh ) );

  return PMMG_SUCCESS;
}

int PMMG_distributeMesh_centralized_timers( PMMG_pParMesh parmesh,mytime *ctim ) {
  MMG5_pMesh    mesh;
  MMG5_pSol     met;
  int           ier,iresult;
  int8_t        tim;
  char          stim[32];

  /** Check input data */
  tim = 1;
  chrono(ON,&(ctim[tim]));

  ier = PMMG_check_inputData( parmesh );
  MPI_Allreduce( &ier, &iresult, 1, MPI_INT, MPI_MIN, parmesh->comm );
  if ( !iresult ) return PMMG_LOWFAILURE;

  chrono(OFF,&(ctim[tim]));
  printim(ctim[tim].gdif,stim);
  if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
    fprintf(stdout,"  -- CHECK INPUT DATA COMPLETED.     %s\n",stim);
  }

  chrono(ON,&(ctim[2]));
  if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
    fprintf(stdout,"\n  -- PHASE 1 : ANALYSIS AND MESH DISTRIBUTION\n");
  }

  /** Mesh preprocessing: set function pointers, scale mesh, perform mesh
   * analysis and display length and quality histos. */
  if( parmesh->myrank == parmesh->info.root ) {
    tim = 7;
    if ( parmesh->info.imprim >= PMMG_VERB_STEPS ) {
      chrono(ON,&(ctim[tim]));
      fprintf(stdout,"\n  -- ANALYSIS" );
    }
    ier = PMMG_preprocessMesh( parmesh );
    if ( parmesh->info.imprim >= PMMG_VERB_STEPS ) {
      chrono(OFF,&(ctim[tim]));
      printim(ctim[tim].gdif,stim);
      fprintf(stdout,"\n  -- ANALYSIS COMPLETED    %s\n",stim );
    }

    mesh = parmesh->listgrp[0].mesh;
    met  = parmesh->listgrp[0].met;
    if ( (ier==PMMG_STRONGFAILURE) && MMG5_unscaleMesh( mesh, met, NULL ) ) {
      ier = PMMG_LOWFAILURE;
    }

    /* Memory repartition */
    if ( !PMMG_parmesh_updateMemMax( parmesh,50,1 ) ) ier = 3;

  } else {
    ier = PMMG_SUCCESS;
  }

  MPI_Allreduce( &ier, &iresult, 1, MPI_INT, MPI_MAX, parmesh->comm );
  if ( iresult!=PMMG_SUCCESS ) {
    return iresult;
  }

  /** Send mesh partionning to other procs */
  tim = 8;
  if ( parmesh->info.imprim >= PMMG_VERB_STEPS ) {
    chrono(ON,&(ctim[tim]));
    fprintf(stdout,"\n  -- PARTITIONING" );
  }
  if ( !PMMG_distribute_mesh( parmesh ) ) {
    PMMG_CLEAN_AND_RETURN(parmesh,PMMG_LOWFAILURE);
  }
  if ( parmesh->info.imprim >= PMMG_VERB_STEPS ) {
    chrono(OFF,&(ctim[tim]));
    printim(ctim[tim].gdif,stim);
    fprintf(stdout,"\n  -- PARTITIONING COMPLETED    %s\n",stim );
  }

  /** Function setters (must be assigned before quality computation) */
  if( parmesh->myrank != parmesh->info.root ) {
    mesh = parmesh->listgrp[0].mesh;
    met  = parmesh->listgrp[0].met;
    MMG3D_Set_commonFunc();
    MMG3D_setfunc(mesh,met);
    PMMG_setfunc(parmesh);
  }

  chrono(OFF,&(ctim[2]));
  if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
    printim(ctim[2].gdif,stim);
    fprintf(stdout,"  -- PHASE 1 COMPLETED.     %s\n",stim);
  }

  iresult = PMMG_SUCCESS;
  return iresult;
}

/**
 * \param parmesh pointer toward the parmesh
 *
 * \return 1 if success, 0 otherwise
 *
 * Give memory to Mmg and build the boundary entities (triangles, edges...).
 *
 */
static inline
int PMMG_bdryBuild ( PMMG_pParMesh parmesh ) {
  MMG5_pMesh mesh;
  size_t     tmpmem;
  int        npmax,xpmax,nemax,xtmax;

//  tmpmem = parmesh->memMax - parmesh->memCur;
//  parmesh->memMax = parmesh->memCur;
//  parmesh->listgrp[0].mesh->memMax += tmpmem;

  mesh = parmesh->listgrp[0].mesh;
  mesh  = parmesh->listgrp[0].mesh;
  npmax = mesh->npmax;
  nemax = mesh->nemax;
  xpmax = mesh->xpmax;
  xtmax = mesh->xtmax;
  mesh->npmax = mesh->np;
  mesh->nemax = mesh->ne;
  mesh->xpmax = mesh->xp;
  mesh->xtmax = mesh->xt;

  if ( !PMMG_setMemMax_realloc( mesh, npmax, xpmax, nemax, xtmax ) ) {
    fprintf(stdout,"\n\n\n  -- LACK OF MEMORY\n\n\n");
    return 0;
  }

  if ( (!MMG3D_hashTetra( mesh, 0 )) || (-1 == MMG3D_bdryBuild( mesh )) ) {
    /** Impossible to rebuild the triangle */
    return 0;
  }
  return 1;
}

int PMMG_Compute_trianglesGloNum( PMMG_pParMesh parmesh ) {
  PMMG_pInt_comm int_face_comm;
  PMMG_pExt_comm ext_face_comm;
  PMMG_pGrp      grp;
  MMG5_pMesh     mesh;
  MMG5_pTetra    pt;
  MMG5_pxTetra   pxt;
  MMG5_pTria     ptr;
  int            *intvalues,*xtet2tria,*itosend,*itorecv;
  int            ie,ifac,k,i,idx,xt,nxt,pos;
  int            icomm,nitem,color;
  int            iglob,nglob,*nglobvec,offset;
  int            ier = 0;
  MPI_Status     status;

  assert( parmesh->ngrp == 1 );
  grp = &parmesh->listgrp[0];
  mesh = grp->mesh;
  assert( mesh->nt );


  /** Step 0: Count and compact xtetra numbering, and allocate xtetra->tria map
   *  to store local tria index, global tria index, and owner process.
   */

  /* Count xtetra and and store compact index in tetra flag */
  nxt = 0;
  for( ie = 1; ie <= mesh->ne; ie++ ) {
    pt = &mesh->tetra[ie];
    if( !MG_EOK(pt) ) continue;
    if( !pt->xt ) continue;
    pt->flag = ++nxt;
  }

  /* Allocate xtetra->tria map. Initialize local and global index to 0, and
   * initialize owner to myrank. */
  PMMG_MALLOC(parmesh,xtet2tria,12*nxt,int,"xtet2tria",ier = 1 );
  if( ier ) return 0;
  for( xt = 1; xt <= nxt; xt++ ) {
    for( ifac = 0; ifac < 4; ifac++ ) {
      pos = 12*(xt-1)+3*ifac;
      xtet2tria[pos+0] = 0;
      xtet2tria[pos+1] = 0;
      xtet2tria[pos+2] = parmesh->myrank;
    }
  }


  /** Step 1: Mark not-owned triangles.
   */

  /* Allocate internal communicator */
  int_face_comm = parmesh->int_face_comm;
  PMMG_MALLOC(parmesh,int_face_comm->intvalues,int_face_comm->nitem,int,"intvalues",ier = 1 );
  if( ier ) {
    PMMG_DEL_MEM(parmesh,xtet2tria,int,"xtet2tria");
    return 0;
  }
  intvalues = int_face_comm->intvalues;

  /** Store outer color in the internal communicator */
  for( k = 0; k < parmesh->next_face_comm; k++ ) {
    ext_face_comm = &parmesh->ext_face_comm[k];
    for( i = 0; i < ext_face_comm->nitem; i++ ) {
      idx = ext_face_comm->int_comm_index[i];
      intvalues[idx] = ext_face_comm->color_out;
    }
  }

  /** Retrieve outer color from the internal communicator and compare it with
   *  the current rank */
  for( i = 0; i < grp->nitem_int_face_comm; i++ ) {
    ie   = (grp->face2int_face_comm_index1[i] / 12);
    ifac = (grp->face2int_face_comm_index1[i] % 12) / 3;
    idx  =  grp->face2int_face_comm_index2[i];

    assert(ie);
    pt = &mesh->tetra[ie];

    assert(pt->xt);

    xt = pt->flag;
    pos = 12*(xt-1)+3*ifac;

    if( intvalues[idx] > parmesh->myrank ) {
      assert( xtet2tria[pos+2] == parmesh->myrank );
      xtet2tria[pos+2] = intvalues[idx];
    }
  }


  /** Step 2: Assign a global numbering, skip not-owned PARBDYBDY triangles and
   *  purely PARBDY triangles.
   */

  /* Count and enumerate owned BDY triangles */
  nglob = 0;
  for( k = 1; k <= mesh->nt; k++ ) {
    ptr  = &mesh->tria[k];
    ie   = ptr->cc / 4;
    ifac = ptr->cc % 4;

    assert(ie);
    pt = &mesh->tetra[ie];

    assert(pt->xt);
    pxt = &mesh->xtetra[pt->xt];
    assert( pxt->ftag[ifac] & MG_BDY );

    xt = pt->flag;
    pos = 12*(xt-1)+3*ifac;

    /* Store local triangle index for every triangle */
    assert( !xtet2tria[pos] );
    xtet2tria[pos] = k;

    /* Skip purely parallel faces */
    if(  (pxt->ftag[ifac] & MG_PARBDY) &&
        !(pxt->ftag[ifac] & MG_PARBDYBDY) ) continue;

    /* Skip not-owned */
    if( xtet2tria[pos+2] != parmesh->myrank ) continue;

    /* Global index (without processor offset): it should be still set to 0 */
    assert( !xtet2tria[pos+1] );
    xtet2tria[pos+1] = ++nglob;
  }


  /** Compute a first numbering offsets among procs and apply it */
  PMMG_CALLOC(parmesh,nglobvec,parmesh->nprocs+1,int,"nglobvec",ier = 1 );
  if( ier ) {
    PMMG_DEL_MEM(parmesh,int_face_comm->intvalues,int,"intvalues");
    PMMG_DEL_MEM(parmesh,xtet2tria,int,"xtet2tria");
    return 0;
  }

  MPI_CHECK(
      MPI_Allgather( &nglob,1,MPI_INT, &nglobvec[1],1,MPI_INT,parmesh->comm ),
      ier = 1 );
  if( ier ) {
    PMMG_DEL_MEM(parmesh,nglobvec,int,"nglobvec");
    PMMG_DEL_MEM(parmesh,int_face_comm->intvalues,int,"intvalues");
    PMMG_DEL_MEM(parmesh,xtet2tria,int,"xtet2tria");
    return 0;
  }

  offset = 0;
  for( k = 0; k < parmesh->nprocs; k++ )
    nglobvec[k+1] += nglobvec[k];
  offset = nglobvec[parmesh->myrank];

  /* Apply the offset to every triangle */
  for( xt = 1; xt <= nxt; xt++ ) {
    for( ifac = 0; ifac < 4; ifac++ ) {
      pos = 12*(xt-1)+3*ifac;
      xtet2tria[pos+1] += offset;
    }
  }


  /** Step 3: Assign a global numbering on simply PARBDY triangles.
   */

  /* Count and enumerate simply PARBDY triangles */
  nglob = 0;
  for( k = 1; k <= mesh->nt; k++ ) {
    ptr  = &mesh->tria[k];
    ie   = ptr->cc / 4;
    ifac = ptr->cc % 4;

    assert(ie);
    pt = &mesh->tetra[ie];

    assert(pt->xt);
    pxt = &mesh->xtetra[pt->xt];
    assert( pxt->ftag[ifac] & MG_BDY );

    xt = pt->flag;
    pos = 12*(xt-1)+3*ifac;
    /* Local index has been assigned in the previous pass */
    assert( xtet2tria[pos] == k );

    /* Skip non-parallel or PARBDYBDY faces */
    if( !(pxt->ftag[ifac] & MG_PARBDY) ||
         (pxt->ftag[ifac] & MG_PARBDYBDY) ) continue;

    /* Skip not-owned */
    if( xtet2tria[pos+2] != parmesh->myrank ) continue;

    /* Global index: it should still be set to the last process offset */
    assert( xtet2tria[pos+1] == offset );
    xtet2tria[pos+1] = ++nglob;
  }

  /** Compute a second numbering offsets among procs and apply it */
  nglobvec[0] = nglobvec[parmesh->nprocs];
  MPI_CHECK(
      MPI_Allgather( &nglob,1,MPI_INT, &nglobvec[1],1,MPI_INT,parmesh->comm ),
      ier = 1 );
  if( ier ) {
    PMMG_DEL_MEM(parmesh,nglobvec,int,"nglobvec");
    PMMG_DEL_MEM(parmesh,int_face_comm->intvalues,int,"intvalues");
    PMMG_DEL_MEM(parmesh,xtet2tria,int,"xtet2tria");
    return 0;
  }

  offset = 0;
  for( k = 0; k < parmesh->nprocs; k++ )
    nglobvec[k+1] += nglobvec[k];
  offset = nglobvec[parmesh->myrank];

  /* Apply this second offset only to simply parallel triangles */
  for( k = 1; k <= mesh->nt; k++ ) {
    ptr  = &mesh->tria[k];
    ie   = ptr->cc / 4;
    ifac = ptr->cc % 4;

    assert(ie);
    pt = &mesh->tetra[ie];

    assert(pt->xt);
    pxt = &mesh->xtetra[pt->xt];
    xt = pt->flag;

    pos = 12*(xt-1)+3*ifac;

    /* Skip non-parallel or PARBDYBDY faces */
    if( !(pxt->ftag[ifac] & MG_PARBDY) ||
         (pxt->ftag[ifac] & MG_PARBDYBDY) ) continue;

    /* Skip not-owned */
    if( xtet2tria[pos+2] != parmesh->myrank ) continue;

    /* Apply processor offset) */
    assert( xtet2tria[pos+1] );
    xtet2tria[pos+1] += offset;
  }

#ifndef NDEBUG
  for( k = 1; k <= mesh->nt; k++ ) {
    ptr  = &mesh->tria[k];
    ie   = ptr->cc / 4;
    ifac = ptr->cc % 4;

    assert(ie);
    pt = &mesh->tetra[ie];

    assert(pt->xt);
    pxt = &mesh->xtetra[pt->xt];
    xt = pt->flag;

    pos = 12*(xt-1)+3*ifac;
    assert( xtet2tria[pos] == k );
  }
#endif


  /** Step 4: Communicate global numbering and retrieve it on not-owned
   *  triangles.
   */

  /* Store numbering in the internal communicator */
  for( i = 0; i < grp->nitem_int_face_comm; i++ ) {
    ie   = (grp->face2int_face_comm_index1[i] / 12);
    ifac = (grp->face2int_face_comm_index1[i] % 12) / 3;
    idx  =  grp->face2int_face_comm_index2[i];

    assert(ie);
    pt = &mesh->tetra[ie];

    assert(pt->xt);
    xt = pt->flag;

    pos = 12*(xt-1)+3*ifac;
    intvalues[idx] = xtet2tria[pos+1];
  }

  /* Send and receive external communicators */
  for( icomm = 0; icomm < parmesh->next_face_comm; icomm++ ) {
    ext_face_comm = &parmesh->ext_face_comm[icomm];
    nitem         = ext_face_comm->nitem;
    color         = ext_face_comm->color_out;

    PMMG_CALLOC(parmesh,ext_face_comm->itosend,nitem,int,"itosend",ier = 1);
    if( ier ) {
      for( k = 0; k < icomm; k++ ) {
        PMMG_DEL_MEM(parmesh,parmesh->ext_face_comm[k].itosend,int,"itosend");
        PMMG_DEL_MEM(parmesh,parmesh->ext_face_comm[k].itorecv,int,"itorecv");
      }
      PMMG_DEL_MEM(parmesh,nglobvec,int,"nglobvec");
      PMMG_DEL_MEM(parmesh,int_face_comm->intvalues,int,"intvalues");
      PMMG_DEL_MEM(parmesh,xtet2tria,int,"xtet2tria");
      return 0;
    }
    itosend = ext_face_comm->itosend;

    PMMG_CALLOC(parmesh,ext_face_comm->itorecv,nitem,int,"itorecv",ier = 1);
    if( ier ) {
      for( k = 0; k < icomm; k++ ) {
        PMMG_DEL_MEM(parmesh,parmesh->ext_face_comm[k].itosend,int,"itosend");
        PMMG_DEL_MEM(parmesh,parmesh->ext_face_comm[k].itorecv,int,"itorecv");
      }
      PMMG_DEL_MEM(parmesh,parmesh->ext_face_comm[icomm].itosend,int,"itosend");
      PMMG_DEL_MEM(parmesh,nglobvec,int,"nglobvec");
      PMMG_DEL_MEM(parmesh,int_face_comm->intvalues,int,"intvalues");
      PMMG_DEL_MEM(parmesh,xtet2tria,int,"xtet2tria");
      return 0;
    }
    itorecv = ext_face_comm->itorecv;

    for( i = 0; i < nitem; i++ ) {
      idx        = ext_face_comm->int_comm_index[i];
      itosend[i] = intvalues[idx];
    }

    MPI_CHECK(
      MPI_Sendrecv(itosend,nitem,MPI_INT,color,MPI_COMMUNICATORS_REF_TAG,
                   itorecv,nitem,MPI_INT,color,MPI_COMMUNICATORS_REF_TAG,
                   parmesh->comm,&status),return 0 );

    /* Store the info in intvalues */
    for( i = 0; i < nitem; i++ ) {
      idx            = ext_face_comm->int_comm_index[i];
      intvalues[idx] = itorecv[i];
    }
  }

  /* Retrieve numbering from the internal communicator */
  for( i = 0; i < grp->nitem_int_face_comm; i++ ) {
    ie   = (grp->face2int_face_comm_index1[i] / 12);
    ifac = (grp->face2int_face_comm_index1[i] % 12) / 3;
    idx  =  grp->face2int_face_comm_index2[i];

    assert(ie);
    pt = &mesh->tetra[ie];

    assert(pt->xt);
    xt = pt->flag;

    pos = 12*(xt-1)+3*ifac;

    /* Retrieve global index only if the triangle is owned by another process */
    if( xtet2tria[pos+2] != parmesh->myrank )
      xtet2tria[pos+1] = intvalues[idx];
  }

#ifndef NDEBUG
  /* Store numbering in the internal communicator */
  for( i = 0; i < grp->nitem_int_face_comm; i++ ) {
    ie   = (grp->face2int_face_comm_index1[i] / 12);
    ifac = (grp->face2int_face_comm_index1[i] % 12) / 3;
    idx  =  grp->face2int_face_comm_index2[i];

    assert(ie);
    pt = &mesh->tetra[ie];

    assert(pt->xt);
    xt = pt->flag;

    pos = 12*(xt-1)+3*ifac;
    intvalues[idx] = xtet2tria[pos+1];
  }

  /* Send and receive external communicators */
  for( icomm = 0; icomm < parmesh->next_face_comm; icomm++ ) {
    ext_face_comm = &parmesh->ext_face_comm[icomm];
    nitem         = ext_face_comm->nitem;
    color         = ext_face_comm->color_out;

    PMMG_CALLOC(parmesh,ext_face_comm->itosend,nitem,int,"itosend",ier = 1);
    if( ier ) {
      for( k = 0; k < icomm; k++ ) {
        PMMG_DEL_MEM(parmesh,parmesh->ext_face_comm[k].itosend,int,"itosend");
        PMMG_DEL_MEM(parmesh,parmesh->ext_face_comm[k].itorecv,int,"itorecv");
      }
      PMMG_DEL_MEM(parmesh,nglobvec,int,"nglobvec");
      PMMG_DEL_MEM(parmesh,int_face_comm->intvalues,int,"intvalues");
      PMMG_DEL_MEM(parmesh,xtet2tria,int,"xtet2tria");
      return 0;
    }
    itosend = ext_face_comm->itosend;

    PMMG_CALLOC(parmesh,ext_face_comm->itorecv,nitem,int,"itorecv",ier = 1);
    if( ier ) {
      for( k = 0; k < icomm; k++ ) {
        PMMG_DEL_MEM(parmesh,parmesh->ext_face_comm[k].itosend,int,"itosend");
        PMMG_DEL_MEM(parmesh,parmesh->ext_face_comm[k].itorecv,int,"itorecv");
      }
      PMMG_DEL_MEM(parmesh,parmesh->ext_face_comm[icomm].itosend,int,"itosend");
      PMMG_DEL_MEM(parmesh,nglobvec,int,"nglobvec");
      PMMG_DEL_MEM(parmesh,int_face_comm->intvalues,int,"intvalues");
      PMMG_DEL_MEM(parmesh,xtet2tria,int,"xtet2tria");
      return 0;
    }
    itorecv = ext_face_comm->itorecv;

    for( i = 0; i < nitem; i++ ) {
      idx        = ext_face_comm->int_comm_index[i];
      itosend[i] = intvalues[idx];
    }

    MPI_CHECK(
      MPI_Sendrecv(itosend,nitem,MPI_INT,color,MPI_COMMUNICATORS_REF_TAG,
                   itorecv,nitem,MPI_INT,color,MPI_COMMUNICATORS_REF_TAG,
                   parmesh->comm,&status),return 0 );

    /* Assert that the sent/received values are the same*/
    for( i = 0; i < nitem; i++ ) {
      assert( itorecv[i] == itosend[i] );
    }
  }
#endif


  /** Step 5: Store the numbering and the owners in the tria structure.
   */
  for( k = 1; k <= mesh->nt; k++ ) {
    ptr  = &mesh->tria[k];
    ie   = ptr->cc / 4;
    ifac = ptr->cc % 4;

    assert(ie);
    pt = &mesh->tetra[ie];

    assert(pt->xt);
    pxt = &mesh->xtetra[pt->xt];
    xt = pt->flag;

    pos   = 12*(xt-1)+3*ifac;
    k     = xtet2tria[pos];

    assert(k);
    ptr = &mesh->tria[k];

    /* Store global index */
    ptr->flag = xtet2tria[pos+1];

    /* Store owner if boundary, unset otherwise */
    if( !(pxt->ftag[ifac] & MG_PARBDY) ||
         (pxt->ftag[ifac] & MG_PARBDYBDY) )
      ptr->base = xtet2tria[pos+2];
    else
      ptr->base = PMMG_UNSET;
  }


  /** Free memory */
  for( k = 0; k < parmesh->next_face_comm; k++ ) {
    PMMG_DEL_MEM(parmesh,parmesh->ext_face_comm[k].itosend,int,"itosend");
    PMMG_DEL_MEM(parmesh,parmesh->ext_face_comm[k].itorecv,int,"itorecv");
  }
  PMMG_DEL_MEM(parmesh,nglobvec,int,"nglobvec");
  PMMG_DEL_MEM(parmesh,int_face_comm->intvalues,int,"intvalues");
  PMMG_DEL_MEM(parmesh,xtet2tria,int,"xtet2tria");

  return 1;
}

int PMMG_Compute_verticesGloNum( PMMG_pParMesh parmesh ){
  PMMG_pGrp      grp;
  MMG5_pMesh     mesh;
  MMG5_pPoint    ppt;
  PMMG_pInt_comm int_node_comm;
  PMMG_pExt_comm ext_node_comm;
  MPI_Request    request;
  MPI_Status     status;
  int            *intvalues,*iproc2comm;
  int            *itosend,*itorecv,src,dst,tag;
  int            nitem,color;
  int            nowned,*offsets;
  int            iproc,icomm,i,idx,ip,counter;

  /* Groups should be merged */
  assert( parmesh->ngrp == 1 );
  grp = &parmesh->listgrp[0];
  mesh = parmesh->listgrp[0].mesh;

  /* Allocate internal communicator */
  int_node_comm = parmesh->int_node_comm;
  PMMG_MALLOC(parmesh,int_node_comm->intvalues,int_node_comm->nitem,int,"intvalues",return 0);
  intvalues = int_node_comm->intvalues;


  /** Step 0: Count nowned nodes */

  /* Array to reorder communicators */
  PMMG_MALLOC(parmesh,iproc2comm,parmesh->nprocs,int,"iproc2comm",return 0);

  for( iproc = 0; iproc < parmesh->nprocs; iproc++ )
    iproc2comm[iproc] = PMMG_UNSET;

  for( icomm = 0; icomm < parmesh->next_node_comm; icomm++ ) {
    ext_node_comm = &parmesh->ext_node_comm[icomm];
    iproc = ext_node_comm->color_out;
    iproc2comm[iproc] = icomm;
  }

  /* Mark nodes with the owner color (overwritten by higher-rank procs) */
  for( iproc = 0; iproc < parmesh->nprocs; iproc++ ) {
    icomm = iproc2comm[iproc];
    if( icomm == PMMG_UNSET ) continue;
    ext_node_comm = &parmesh->ext_node_comm[icomm];
    color = MG_MAX(parmesh->myrank,ext_node_comm->color_out);
    /* Mark nodes */
    for( i = 0; i < ext_node_comm->nitem; i++ ) {
      idx = ext_node_comm->int_comm_index[i];
      intvalues[idx] = color;
    }
  }

  /* Store owner in the point flag */
  for( ip = 1; ip <= mesh->np; ip++ ) {
    ppt = &mesh->point[ip];
    ppt->flag = parmesh->myrank;
  }

  /* Overwrite flag for communicator nodes */
  for( i = 0; i < grp->nitem_int_node_comm; i++ ){
    ip   = grp->node2int_node_comm_index1[i];
    idx  = grp->node2int_node_comm_index2[i];
    mesh->point[ip].flag = intvalues[idx];
  }

  /* Count owned nodes */
  nowned = mesh->np;
  for( idx = 0; idx < int_node_comm->nitem; idx++ ) {
    if( intvalues[idx] != parmesh->myrank ) nowned--;
  }

  /* Compute offsets on each proc */
  PMMG_CALLOC(parmesh,offsets,parmesh->nprocs+1,int,"offsets",return 0);
  MPI_Allgather( &nowned,1,MPI_INT,
                 &offsets[1],1,MPI_INT,parmesh->comm );
  for( i = 1; i <= parmesh->nprocs; i++ )
    offsets[i] += offsets[i-1];

#ifndef NDEBUG
  for( ip = 1; ip <= mesh->np; ip++ ) {
    mesh->point[ip].tmp = PMMG_UNSET;
  }
#endif

  /** Step 1: Pack */
  counter = 0;
  for( ip = 1; ip <= mesh->np; ip++ ) {
    ppt = &mesh->point[ip];
    if( ppt->flag != parmesh->myrank ) continue;
    ppt->tmp = ++counter+offsets[parmesh->myrank];
    assert(ppt->tmp);
  }
  assert( counter == nowned );


  /** Step 2: Communicate global numbering */

  /* Store numbering in the internal communicator */
  for( i = 0; i < grp->nitem_int_node_comm; i++ ){
    ip   = grp->node2int_node_comm_index1[i];
    idx  = grp->node2int_node_comm_index2[i];
    intvalues[idx] = mesh->point[ip].tmp;
    assert(intvalues[idx]);
  }

  /* Send-recv */
  for( icomm = 0; icomm < parmesh->next_node_comm; icomm++ ) {
    ext_node_comm = &parmesh->ext_node_comm[icomm];
    color = ext_node_comm->color_out;
    nitem = ext_node_comm->nitem;

    PMMG_CALLOC(parmesh,ext_node_comm->itosend,nitem,int,"itosend",return 0);
    PMMG_CALLOC(parmesh,ext_node_comm->itorecv,nitem,int,"itorecv",return 0);
    itosend = ext_node_comm->itosend;
    itorecv = ext_node_comm->itorecv;

    src = MG_MAX(parmesh->myrank,color);
    dst = MG_MIN(parmesh->myrank,color);
    tag = parmesh->nprocs*src+dst;

    if( parmesh->myrank == src ) {
      /* Fill send buffer from internal communicator */
      for( i = 0; i < nitem; i++ ) {
        idx = ext_node_comm->int_comm_index[i];
        itosend[i] = intvalues[idx];
        assert(itosend[i]);
      }
      MPI_CHECK( MPI_Isend(itosend,nitem,MPI_INT,dst,tag,
                           parmesh->comm,&request),return 0 );
    }
    if ( parmesh->myrank == dst ) {
      MPI_CHECK( MPI_Recv(itorecv,nitem,MPI_INT,src,tag,
                          parmesh->comm,&status),return 0 );
    }
  }

  /* Store recv buffer in the internal communicator */
  for( iproc = parmesh->myrank+1; iproc < parmesh->nprocs; iproc++ ){
    icomm = iproc2comm[iproc];
    if( icomm == PMMG_UNSET ) continue;
    ext_node_comm = &parmesh->ext_node_comm[icomm];
    nitem = ext_node_comm->nitem;
    itorecv = ext_node_comm->itorecv;
    for( i = 0; i < nitem; i++ ) {
      idx = ext_node_comm->int_comm_index[i];
      intvalues[idx] = itorecv[i];
    }
  }

  /* Retrieve numbering from the internal communicator */
  for( i = 0; i < grp->nitem_int_node_comm; i++ ){
    ip  = grp->node2int_node_comm_index1[i];
    idx = grp->node2int_node_comm_index2[i];
    ppt = &mesh->point[ip];
    if( ppt->flag > parmesh->myrank ){
      ppt->tmp = intvalues[idx];
    }
  }

#ifndef NDEBUG
  for( ip = 1; ip <= mesh->np; ip++ ) {
    ppt = &mesh->point[ip];
    assert( (ppt->tmp > 0) && (ppt->tmp <= offsets[parmesh->nprocs]) );
  }
#endif

  /* Free arrays */
  for( icomm = 0; icomm < parmesh->next_node_comm; icomm++ ) {
    ext_node_comm = &parmesh->ext_node_comm[icomm];
    PMMG_DEL_MEM(parmesh,ext_node_comm->itosend,int,"itosend");
    PMMG_DEL_MEM(parmesh,ext_node_comm->itorecv,int,"itorecv");
  }
  PMMG_DEL_MEM(parmesh,offsets,int,"offsets");
  PMMG_DEL_MEM(parmesh,offsets,int,"iproc2comm");
  PMMG_DEL_MEM(parmesh,int_node_comm->intvalues,int,"intvalues");
  return 1;
}
/**
 * \param parmesh pointer toward the parmesh
 *
 * \return PMMG_SUCCESS if success, PMMG_LOWFAILURE if merge or boundary
 * reconstruction fail
 *
 * Mesh post-treatment:
 *  1. merge meshes if centralized output is asked;
 *  2. build boundary entities of merged or distributed meshes (triangles, edges...);
 *
 */
static inline
int PMMG_parmmglib_post(PMMG_pParMesh parmesh) {
  mytime        ctim[TIMEMAX];
  int           ier,iresult;
  int8_t        tim;
  char          stim[32];

  tminit(ctim,TIMEMAX);

  iresult = 1;

  switch ( parmesh->info.fmtout ) {
  case ( PMMG_UNSET ):
    /* No output */
    break;
  case ( MMG5_FMT_VtkPvtu ): case ( PMMG_FMT_Distributed ):
  case ( PMMG_FMT_DistributedMeditASCII ): case ( PMMG_FMT_DistributedMeditBinary ):
    /* Distributed Output */
    tim = 1;
    chrono(ON,&(ctim[tim]));
    if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
      fprintf( stdout,"\n   -- PHASE 3 : MESH PACKED UP\n" );
    }

    ier = PMMG_bdryBuild ( parmesh );

    MPI_Allreduce( &ier, &iresult, 1, MPI_INT, MPI_MIN, parmesh->comm );
    if ( !iresult ) {
      if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
        fprintf(stdout,"\n\n\n  -- IMPOSSIBLE TO BUILD THE BOUNDARY MESH\n\n\n");
      }
      return PMMG_LOWFAILURE;
    }


    if( parmesh->info.globalNum ) {
      size_t myavailable,oldMemMax;
      PMMG_TRANSFER_AVMEM_TO_PARMESH(parmesh,myavailable,oldMemMax);

      ier = PMMG_Compute_verticesGloNum( parmesh );
      if( !ier ) {
        if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
          fprintf(stdout,"\n\n\n  -- WARNING: IMPOSSIBLE TO COMPUTE NODE GLOBAL NUMBERING\n\n\n");
        }
      }

      ier = PMMG_Compute_trianglesGloNum( parmesh );
      if( !ier ) {
        if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
          fprintf(stdout,"\n\n\n  -- WARNING: IMPOSSIBLE TO COMPUTE TRIANGLE GLOBAL NUMBERING\n\n\n");
        }
      }

      PMMG_TRANSFER_AVMEM_TO_MESHES(parmesh);
    }


    chrono(OFF,&(ctim[tim]));
    if (  parmesh->info.imprim >  PMMG_VERB_VERSION ) {
      printim(ctim[tim].gdif,stim);
      fprintf( stdout,"   -- PHASE 3 COMPLETED.     %s\n",stim );
    }

    break;
  default:
    /* Centralized Output */
    /** Merge all the meshes on the proc 0 */
    tim = 1;
    chrono(ON,&(ctim[tim]));
    if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
      fprintf( stdout,"\n   -- PHASE 3 : MERGE MESHES OVER PROCESSORS\n" );
    }

    ier = PMMG_merge_parmesh( parmesh );
    MPI_Allreduce( &ier, &iresult, 1, MPI_INT, MPI_MIN, parmesh->comm );

    if ( !iresult ) {
      /* Try to save at parallel format */
      if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
        fprintf(stdout,"\n\n\n  -- IMPOSSIBLE TO CENTRALIZE MESHES..."
                " TRY TO SAVE DISTRIBUTED MESHES\n\n\n");
      }
    }

    chrono(OFF,&(ctim[tim]));
    if ( parmesh->info.imprim >  PMMG_VERB_VERSION  ) {
      printim(ctim[tim].gdif,stim);
      fprintf( stdout,"   -- PHASE 3 COMPLETED.     %s\n",stim );
    }

    /** Boundaries reconstruction: by all the procs if the merge has failed,
     * only by proc 0 if the merge has succeed. */
    tim = 2;
    chrono(ON,&(ctim[tim]));
    if (  parmesh->info.imprim > PMMG_VERB_VERSION ) {
      fprintf( stdout,"\n   -- PHASE 4 : MESH PACKED UP\n" );
    }

    if ( (!iresult) || (!parmesh->myrank) ) {
      ier = PMMG_bdryBuild ( parmesh );

      if ( !iresult ) {
        /* Centralization has failed: needs to reduce the error value and to
         * build the communicators */
        MPI_Allreduce( &ier, &iresult, 1, MPI_INT, MPI_MIN, parmesh->comm );
        ier = iresult;
      }
      /* Check the bdy reconstruction */
      if ( !ier ) {
        if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
          fprintf(stdout,"\n\n\n  -- IMPOSSIBLE TO BUILD THE BOUNDARY MESH\n\n\n");
        }
        return PMMG_LOWFAILURE;
      }

      chrono(OFF,&(ctim[tim]));
      if (  parmesh->info.imprim >  PMMG_VERB_VERSION ) {
        printim(ctim[tim].gdif,stim);
        fprintf( stdout,"   -- PHASE 4 COMPLETED.     %s\n",stim );
      }
    }
  }

  return PMMG_SUCCESS;
}

int PMMG_parmmglib_centralized(PMMG_pParMesh parmesh) {
  PMMG_pGrp     grp;
  MMG5_pMesh    mesh;
  MMG5_pSol     met;
  int           ier;
  int           ierlib;
  mytime        ctim[TIMEMAX];
  int8_t        tim;
  char          stim[32];

 if ( parmesh->info.imprim > PMMG_VERB_NO ) {
    fprintf(stdout,"\n  %s\n   MODULE PARMMGLIB_CENTRALIZED: IMB-LJLL : "
            "%s (%s)\n  %s\n",PMMG_STR,PMMG_VERSION_RELEASE,PMMG_RELEASE_DATE,PMMG_STR);
    fprintf(stdout,"     git branch: %s\n",PMMG_GIT_BRANCH);
    fprintf(stdout,"     git commit: %s\n",PMMG_GIT_COMMIT);
    fprintf(stdout,"     git date:   %s\n\n",PMMG_GIT_DATE);
  }

  tminit(ctim,TIMEMAX);
  chrono(ON,&(ctim[0]));

  /* I/O check: if an input field name is provided but the output one is not,
   compute automatically an output solution field name. */
  if ( parmesh->fieldin &&  *parmesh->fieldin ) {
    ier = PMMG_Set_outputSolsName(parmesh,NULL);
    if ( !ier ) {
      fprintf(stdout,"  ## Warning: %s: rank %d: an input field name is"
              " provided without an output one.\n"
              "            : the saving process may fail.\n",
              __func__,parmesh->myrank);
    }
  }

  /* Distribute the mesh */
  ier = PMMG_distributeMesh_centralized_timers( parmesh, ctim );
  if( ier != PMMG_SUCCESS ) return ier;

  grp    = &parmesh->listgrp[0];
  mesh   = grp->mesh;
  met    = grp->met;

  /** Remeshing */
  tim = 3;
  chrono(ON,&(ctim[tim]));
  if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
    fprintf( stdout,"\n  -- PHASE 2 : %s MESHING\n",
             met->size < 6 ? "ISOTROPIC" : "ANISOTROPIC" );
  }

  ier = PMMG_parmmglib1(parmesh);
  MPI_Allreduce( &ier, &ierlib, 1, MPI_INT, MPI_MAX, parmesh->comm );

  chrono(OFF,&(ctim[tim]));
  printim(ctim[tim].gdif,stim);
  if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
    fprintf(stdout,"  -- PHASE 2 COMPLETED.     %s\n",stim);
  }
  if ( ierlib == PMMG_STRONGFAILURE ) {
    return ierlib;
  }

  ier = PMMG_parmmglib_post(parmesh);
  ierlib = MG_MAX ( ier, ierlib );

  chrono(OFF,&ctim[0]);
  printim(ctim[0].gdif,stim);
  if ( parmesh->info.imprim >= PMMG_VERB_VERSION ) {
    fprintf(stdout,"\n   PARMMGLIB_CENTRALIZED: ELAPSED TIME  %s\n",stim);
    fprintf(stdout,"\n  %s\n   END OF MODULE PARMMGLIB_CENTRALIZED: IMB-LJLL \n  %s\n",
            PMMG_STR,PMMG_STR);
  }

  PMMG_CLEAN_AND_RETURN(parmesh,ierlib);
}

int PMMG_parmmglib_distributed(PMMG_pParMesh parmesh) {
  MMG5_pMesh       mesh;
  MMG5_pSol        met;
  int              ier,iresult,ierlib;
  mytime           ctim[TIMEMAX];
  int8_t           tim;
  char             stim[32];


  if ( parmesh->info.imprim >= PMMG_VERB_VERSION ) {
    fprintf(stdout,"\n  %s\n   MODULE PARMMGLIB_DISTRIBUTED: IMB-LJLL : "
            "%s (%s)\n  %s\n",PMMG_STR,PMMG_VERSION_RELEASE,PMMG_RELEASE_DATE,PMMG_STR);
    fprintf(stdout,"     git branch: %s\n",PMMG_GIT_BRANCH);
    fprintf(stdout,"     git commit: %s\n",PMMG_GIT_COMMIT);
    fprintf(stdout,"     git date:   %s\n\n",PMMG_GIT_DATE);
  }

  tminit(ctim,TIMEMAX);
  chrono(ON,&(ctim[0]));

  /** Check input data */
  tim = 1;
  chrono(ON,&(ctim[tim]));

  if ( parmesh->info.fmtout == PMMG_FMT_Unknown ) {
    parmesh->info.fmtout = PMMG_FMT_Distributed;
  }

  /* I/O check: if an input field name is provided but the output one is not,
   compute automatically an output solution field name. */
  if ( parmesh->fieldin &&  *parmesh->fieldin ) {
    ier = PMMG_Set_outputSolsName(parmesh,NULL);
    if ( !ier ) {
      fprintf(stdout,"  ## Warning: %s: rank %d: an input field name is"
              " provided without an output one.\n"
              "            : the saving process may fail.\n",
              __func__,parmesh->myrank);
    }
  }

  ier = PMMG_check_inputData( parmesh );
  MPI_CHECK( MPI_Allreduce( &ier, &iresult, 1, MPI_INT, MPI_MIN, parmesh->comm ),
             return PMMG_LOWFAILURE);
  if ( !iresult ) return PMMG_LOWFAILURE;

  chrono(OFF,&(ctim[tim]));
  printim(ctim[tim].gdif,stim);
  if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
    fprintf(stdout,"  -- CHECK INPUT DATA COMPLETED.     %s\n",stim);
  }

  tim = 2;
  chrono(ON,&(ctim[tim]));
  if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
    fprintf(stdout,"\n  -- PHASE 1 : ANALYSIS\n");
  }

  assert ( parmesh->ngrp < 2 );
  if ( parmesh->ngrp ) {
    /** Mesh preprocessing: set function pointers, scale mesh, perform mesh
     * analysis and display length and quality histos. */
    ier  = PMMG_preprocessMesh_distributed( parmesh );
    mesh = parmesh->listgrp[0].mesh;
    met  = parmesh->listgrp[0].met;
    if ( (ier==PMMG_STRONGFAILURE) && MMG5_unscaleMesh( mesh, met, NULL ) ) {
      ier = PMMG_LOWFAILURE;
    }
  }
  else { ier = PMMG_SUCCESS; }

  MPI_Allreduce( &ier, &iresult, 1, MPI_INT, MPI_MAX, parmesh->comm );
  if ( iresult!=PMMG_SUCCESS ) {
    return iresult;
  }

  chrono(OFF,&(ctim[tim]));
  if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
    printim(ctim[tim].gdif,stim);
    fprintf(stdout,"   -- PHASE 1 COMPLETED.     %s\n",stim);
  }

  /** Remeshing */
  tim = 3;
  chrono(ON,&(ctim[tim]));
  if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
    fprintf( stdout,"\n  -- PHASE 2 : %s MESHING\n",
             met->size < 6 ? "ISOTROPIC" : "ANISOTROPIC" );
  }

  ier = PMMG_parmmglib1(parmesh);
  MPI_Allreduce( &ier, &ierlib, 1, MPI_INT, MPI_MAX, parmesh->comm );

  chrono(OFF,&(ctim[tim]));
  printim(ctim[tim].gdif,stim);
  if ( parmesh->info.imprim > PMMG_VERB_VERSION ) {
    fprintf(stdout,"  -- PHASE 2 COMPLETED.     %s\n",stim);
  }
  if ( ierlib == PMMG_STRONGFAILURE ) {
    return ierlib;
  }

  ier = PMMG_parmmglib_post(parmesh);
  ierlib = MG_MAX ( ier, ierlib );

  chrono(OFF,&ctim[0]);
  printim(ctim[0].gdif,stim);
  if ( parmesh->info.imprim >= PMMG_VERB_VERSION ) {
    fprintf(stdout,"\n   PARMMGLIB_DISTRIBUTED: ELAPSED TIME  %s\n",stim);
    fprintf(stdout,"\n  %s\n   END OF MODULE PARMMGLIB_DISTRIBUTED: IMB-LJLL \n  %s\n",
            PMMG_STR,PMMG_STR);
  }

  PMMG_CLEAN_AND_RETURN(parmesh,ierlib);
}

int PMMG_distributeMesh_centralized( PMMG_pParMesh parmesh ) {
  MMG5_pMesh mesh;
  MMG5_pSol  met;
  int ier,iresult;

  /** Check input data */
  ier = PMMG_check_inputData( parmesh );
  MPI_Allreduce( &ier, &iresult, 1, MPI_INT, MPI_MIN, parmesh->comm );
  if ( !iresult ) return PMMG_LOWFAILURE;

  /** Mesh preprocessing: set function pointers, scale mesh, perform mesh
   * analysis and display length and quality histos. */
  if( parmesh->myrank == parmesh->info.root ) {
    if ( parmesh->info.imprim >= PMMG_VERB_STEPS ) {
      fprintf(stdout,"\n  -- ANALYSIS" );
    }
    ier = PMMG_preprocessMesh( parmesh );
    if ( parmesh->info.imprim >= PMMG_VERB_STEPS ) {
      fprintf(stdout,"\n  -- ANALYSIS COMPLETED\n");
    }

    mesh = parmesh->listgrp[0].mesh;
    met  = parmesh->listgrp[0].met;
    if ( (ier==PMMG_STRONGFAILURE) && MMG5_unscaleMesh( mesh, met, NULL ) ) {
      ier = PMMG_LOWFAILURE;
    }

    /* Memory repartition */
    if ( !PMMG_parmesh_updateMemMax( parmesh,50,1 ) ) ier = 3;

  } else {
    ier = PMMG_SUCCESS;
  }

  MPI_Allreduce( &ier, &iresult, 1, MPI_INT, MPI_MAX, parmesh->comm );
  if ( iresult!=PMMG_SUCCESS ) {
    return iresult;
  }

  /** Send mesh partionning to other procs */
  if ( parmesh->info.imprim >= PMMG_VERB_STEPS ) {
    fprintf(stdout,"\n  -- PARTITIONING" );
  }
  if ( !PMMG_distribute_mesh( parmesh ) ) {
    PMMG_CLEAN_AND_RETURN(parmesh,PMMG_LOWFAILURE);
  }
  if ( parmesh->info.imprim >= PMMG_VERB_STEPS ) {
    fprintf(stdout,"\n  -- PARTITIONING COMPLETED\n");
  }

  /** Function setters (must be assigned before quality computation) */
  if( parmesh->myrank != parmesh->info.root ) {
    mesh = parmesh->listgrp[0].mesh;
    met  = parmesh->listgrp[0].met;
    MMG3D_Set_commonFunc();
    MMG3D_setfunc(mesh,met);
  }

  /* Memory repartition */
  if ( !PMMG_parmesh_updateMemMax( parmesh,50,1 ) ) return 3;

  iresult = PMMG_SUCCESS;
  return iresult;
}
