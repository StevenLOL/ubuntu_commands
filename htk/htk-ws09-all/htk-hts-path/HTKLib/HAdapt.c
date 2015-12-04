/* ----------------------------------------------------------- */
/*                                                             */
/*                          ___                                */
/*                       |_| | |_/   SPEECH                    */
/*                       | | | | \   RECOGNITION               */
/*                       =========   SOFTWARE                  */ 
/*                                                             */
/*                                                             */
/* ----------------------------------------------------------- */
/* developed at:                                               */
/*                                                             */
/*      Speech Vision and Robotics group                       */
/*      Cambridge University Engineering Department            */
/*      http://svr-www.eng.cam.ac.uk/                          */
/*                                                             */
/* ----------------------------------------------------------- */
/*         Copyright:                                          */
/*                                                             */
/*              2003  M.J.F. Gales and                         */
/*                    Cambridge University                     */
/*                    Engineering Department                   */
/*                                                             */
/*   Use of this software is governed by a License Agreement   */
/*    ** See the file License for the Conditions of Use  **    */
/*    **     This banner notice must not be removed      **    */
/*                                                             */
/* ----------------------------------------------------------- */
/*         File: HAdapt.c      Adaptation Library module       */
/* ----------------------------------------------------------- */

/*  *** THIS IS A MODIFIED VERSION OF HTK ***                        */
/* ----------------------------------------------------------------- */
/*           The HMM-Based Speech Synthesis System (HTS)             */
/*           developed by HTS Working Group                          */
/*           http://hts.sp.nitech.ac.jp/                             */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2001-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/*                2001-2008  Tokyo Institute of Technology           */
/*                           Interdisciplinary Graduate School of    */
/*                           Science and Engineering                 */
/*                                                                   */
/*                2008-2009  University of Edinburgh                 */
/*                           Centre for Speech Technology Research   */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the HTS working group nor the names of its  */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

char *hadapt_version = "!HVER!HAdapt:   3.4.1  [CUED 12/03/09]";
char *hadapt_vc_id =  "$Id: HAdapt.c,v 1.64 2011/06/16 04:15:50 uratec Exp $";

#include <stdio.h>      /* Standard C Libraries */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "HShell.h"     /* HMM ToolKit Modules */
#include "HMem.h"
#include "HMath.h"
#include "HSigP.h"
#include "HWave.h"
#include "HAudio.h"
#include "HParm.h"
#include "HLabel.h"
#include "HModel.h"
#include "HTrain.h"
#include "HUtil.h"
#include "HAdapt.h"
#include "HFB.h"

/* trace flags */
#define T_TOP   00001    /* Top level tracing */
#define T_ADT   00002    /* Trace number of adapted components  */
#define T_ACC   00004    /* Trace number of accumulates generated  */
#define T_TRE   00010    /* Trace use of regression class tree  */
#define T_XFM   00020    /* Trace generation of xforms  */
#define T_SXF   00040    /* Speaker xform updates */
#define T_OBC   00100    /* Trace observation cache */
#define T_SWP   00200    /* Trace transform manipulation */
#define T_FRS   00400    /* Trace fisher ratio selection */

/* -------------- Structures to store adaptation info -------------------- */

typedef struct {
   XFormKind xkind;
   Boolean useBias;
   int dim;
   double occ;  
   IntVec blockSize;
   IntVec bandWidth;
   DVector *K, D;
   DMatrix *G;
} AccStruct;

typedef struct _AInfo {
   int baseClass;
   int level;
   struct _AInfo *next;            /* next external file name in list */
} AInfo;

typedef struct {
   SVector mean;
   CovKind ckind;
   Covariance cov;
   float gConst;
} MInfo;

typedef struct {
   double  occ;
   DVector spSum;
   DVector spSumSq;
   DTriMat *bTriMat;
   DTriMat *bDiagMat;
   DVector bVector;
} RegAcc;

typedef struct {
   AInfo *aInfo;         /* current transform information */
   MInfo *mInfo;         /* any original model information */
   AInfo *paInfo;        /* parent transform information */
   RegAcc *regAcc;       /* accumulate information for generating transform */
   ObsCache *oc;         /* observation cache for input transform */
   ObsCache *paoc;       /* observation cache for parent transform */
   AccCache *paac;       /* accummulator cache for parent transform */
} XFormInfo;

/* General variables */
static ConfParam *cParm[MAXGLOBS];      /* config parameters */
static int nParm = 0;
static int trace = 0;                   /* trace info */

/* Local stack to allow storage of internal structrures */
static MemHeap infoStack;

/* Global variables */
/* also have the option of storing a model set for each of the speakers */
static Boolean saveSpkrModels = FALSE;

/* The xform config variable information */
static float minOccThresh = 0.0;       /* minimum occupancy to accumulate stats to estimate xform */
static Boolean storeMInfo = TRUE;      /* whether original model information  is to be stored */
static Boolean keepXFormDistinct = TRUE;
static Boolean swapXForms = FALSE;     /* swap the transforms around after generating transform */
static Boolean mllrCov2CMLLR= FALSE;   /* apply mllrcov transforms as cmllr transform */ 
static Boolean mllrDiagCov = FALSE;    /* perform diagonal covariance adaptation */
static Boolean useSMAPcriterion = FALSE;  /* perform SMAPLR and CSMAPLR adaptation */
static Boolean SaveAllSMAPXForms = FALSE; /* Save intermediate matrices estimated in SMAP */
static float   sigmascale = 1.0;          /* prior parameter for SMAP creterion*/

static IntVec enableBlockAdapt = NULL;

static int maxXFormIter = 10; /* something big, for CMLLR */ 

/* new variables to support semi-tied transforms */
static Vector semiTiedFloorScale = NULL;  /* stream-dependent flooring scales */
static int maxSemiTiedIter = 10;
static char *semiTiedMacro = NULL;
static Boolean semiTied2InputXForm = FALSE;
static Boolean semiTiedVFloor = TRUE;
static int numNuisanceDim = 0;
static TriMat *avCov = NULL;
static Boolean staticSemiTied = FALSE;
static Boolean initNuisanceFR = TRUE;
static Boolean initNuisanceFRIdent = FALSE;
static Boolean saveSemiTiedBinary = FALSE;

/*------------------------------------------------------------------------*/
/*    Support Routines for determining internal structures required       */
/*    Note: these only act on the transform NOT any parents.              */
/*------------------------------------------------------------------------*/

static Boolean AccAdaptMean(AdaptXForm *xform)
{
   /* Currently always true */
   if (xform->xformSet->xkind == SEMIT) 
      return (FALSE);
   else
      return (TRUE);
}

static Boolean AccAdaptVar(AdaptXForm *xform)
{
  XFormKind xkind = xform->xformSet->xkind;
  if ( (xkind == CMLLR) || (xkind == MLLRCOV) || (mllrDiagCov)) 
    return (TRUE);
  else
    return (FALSE);
}

static Boolean AccAdaptBaseTriMat(AdaptXForm *xform)
{
  XFormKind xkind = xform->xformSet->xkind;
  if ( (xkind == CMLLR) || (xkind == MLLRCOV)) 
    return (TRUE);
  else
    return (FALSE);
}

Boolean HardAssign(AdaptXForm *xform)
{
   AdaptKind akind = xform->akind;
   if ((akind == TREE) || (akind == BASE))
      return TRUE;
   else
      return FALSE;
}

static Boolean StoreObsCache(AdaptXForm *xform)
{
   XFormKind xkind = xform->xformSet->xkind;
   if ((xkind == CMLLR)  || (xkind == MLLRCOV) || (xkind == SEMIT))
      return TRUE;
   else
      return FALSE;
}

static Boolean StoreAdaptMean(AdaptXForm *xform)
{
   XFormKind xkind = xform->xformSet->xkind;
   if ((xkind == MLLRMEAN) || (xkind == MLLRCOV) || (xkind == CMLLR) || (xkind == SEMIT))
      return TRUE;
   else
      return FALSE;
}

static Boolean StoreAdaptCov(AdaptXForm *xform)
{
   XFormKind xkind = xform->xformSet->xkind;
   if ((xkind == MLLRVAR)  || (xkind == MLLRCOV) || (xkind == CMLLR) || (xkind == SEMIT))
      return TRUE;
   else
      return FALSE;
}

/*------------------------------------------------------------------------*/
/*            Initialisations and general structures allocation           */
/*------------------------------------------------------------------------*/

static void CheckAdaptOptions(XFInfo *xfinfo)
{
   XFormKind xKind = xfinfo->outXKind;
   Boolean useBias = xfinfo->useBias;
   
   if ((!keepXFormDistinct) && (swapXForms))
      HError(999,"Cannot save swapped XForms in a TMF");
   if (((xKind == MLLRCOV)||(xKind == SEMIT)) && (useBias))
      HError(999,"Cannot have a Bias with a Full variance transform");
   if ((mllrDiagCov) && (xKind != MLLRMEAN))
      HError(999,"Cannot have mllrDiagCov and not have MLLRMEAN");
   if ((!swapXForms) && (mllrCov2CMLLR))
      HError(999,"Cannot save mllrCov as CMLLR");
   if (strmProj && ((xKind != SEMIT) || (numNuisanceDim>0)))
      HError(999,"Can only have strmProj with SEMIT");
   if (numNuisanceDim > 0) {
      if (xKind != SEMIT) 
         HError(999,"Can only use HLDA with global SEMIT transforms");
   }
   if (staticSemiTied) {
      if (xKind != SEMIT) 
         HError(999,"Can only use static semiTied with SEMIT transforms");
   }
   if (((mllrDiagCov)|| (xKind == MLLRCOV)||(xKind == SEMIT)) && (useSMAPcriterion))
     HError(999,"Can use only SMAP with MLLRMEAN and CMLLR currently");
   if ((xfinfo->xformAdaptKind == BASE) && (useSMAPcriterion))
     HError(999,"Can use only SMAP with TREE currently");
}

/* EXPORT->CheckAdaptSetUp: check adapt setting */
void CheckAdaptSetUp (HMMSet *hset, XFInfo *xfinfo)
{
   int i,s,vsize;
   
   XFormKind xKind = xfinfo->outXKind;
   IntVec *xformBlockSize     = xfinfo->xformBlockSize;
   IntVec *mllrMeanBlockSize  = xfinfo->mllrMeanBlockSize;
   IntVec *mllrCovBlockSize   = xfinfo->mllrCovBlockSize;
   IntVec *cmllrBlockSize     = xfinfo->cmllrBlockSize;
   IntVec *xformBandWidth     = xfinfo->xformBandWidth;
   IntVec *mllrMeanBandWidth  = xfinfo->mllrMeanBandWidth;
   IntVec *mllrCovBandWidth   = xfinfo->mllrCovBandWidth;
   IntVec *cmllrBandWidth     = xfinfo->cmllrBandWidth;
   Vector xformSplitThresh    = xfinfo->xformSplitThresh;
   Vector mllrMeanSplitThresh = xfinfo->mllrMeanSplitThresh;
   Vector mllrCovSplitThresh  = xfinfo->mllrCovSplitThresh;
   Vector cmllrSplitThresh    = xfinfo->cmllrSplitThresh;
      
   /* check block size */
   for (s=1; s<=hset->swidth[0]; s++) {
      if (xformBlockSize[s] != NULL) {
         vsize = 0;
         for (i=1; i<=IntVecSize(xformBlockSize[s]); i++)
            vsize += xformBlockSize[s][i];
         if (vsize!=hset->swidth[s])
            HError(9999,"CheckAdaptSetup: %d-th stream width (%d) and transform size (%d) is inconsistent",s,hset->swidth[s],vsize);
      }
      else {
         switch(xKind) {
         case MLLRMEAN:
            if (mllrMeanBlockSize[s]!=NULL) {
               vsize = 0;
               for (i=1; i<=IntVecSize(mllrMeanBlockSize[s]); i++) 
                  vsize += mllrMeanBlockSize[s][i];
               if (vsize!=hset->swidth[s])
                  HError(9999,"CheckAdaptSetup: %d-th stream width (%d) and transform size (%d) is inconsistent",s,hset->swidth[s],vsize);
            }
            break;
         case MLLRCOV:
            if (mllrCovBlockSize[s]!=NULL) {
               vsize = 0;
               for (i=1; i<=IntVecSize(mllrCovBlockSize[s]); i++)
                  vsize += mllrCovBlockSize[s][i];
               if (vsize!=hset->swidth[s])
                  HError(9999,"CheckAdaptSetup: %d-th stream width (%d) and transform size (%d) is inconsistent",s,hset->swidth[s],vsize);
            }
            break;
         case CMLLR:
            if (cmllrBlockSize[s]!=NULL) {
               vsize = 0;
               for (i=1; i<=IntVecSize(cmllrBlockSize[s]); i++)
                  vsize += cmllrBlockSize[s][i];
               if (vsize!=hset->swidth[s])
                  HError(9999,"CheckAdaptSetup: %d-th stream width (%d) and transform size (%d) is inconsistent",s,hset->swidth[s],vsize);
            }
            break;
         case SEMIT:
         default:
            break;
         }
   }
}

   /* check band width */
   for (s=1; s<=hset->swidth[0]; s++) {
      if (xformBandWidth[s] != NULL) {
         for (i=1; i<=IntVecSize(xformBandWidth[s]); i++)
            if (xformBandWidth[s][i]>xformBlockSize[s][i])
               HError(9999,"CheckAdaptSetup: band width (%d) at %d-th block in %d-th stream of transform is larger than block size (%d)",
                      xformBandWidth[s][i],i,s,xformBlockSize[s][i]);
      }
      else {
         switch(xKind) {
         case MLLRMEAN:
            if (mllrMeanBlockSize[s]!=NULL) {
               for (i=1; i<=IntVecSize(mllrMeanBandWidth[s]); i++) 
                  if (mllrMeanBandWidth[s][i]>mllrMeanBlockSize[s][i])
                     HError(9999,"CheckAdaptSetup: band width (%d) at %d-th block in %d-th stream of MLLRMEAN transform is larger than block size (%d)",
                            mllrMeanBandWidth[s][i],i,s,mllrMeanBlockSize[s][i]);
            }
            break;
         case MLLRCOV:
            if (mllrCovBlockSize[s]!=NULL) {
               for (i=1; i<=IntVecSize(mllrCovBandWidth[s]); i++) 
                  if (mllrCovBandWidth[s][i]>mllrCovBlockSize[s][i])
                     HError(9999,"CheckAdaptSetup: band width (%d) at %d-th block in %d-th stream of MLLRCOV transform is larger than block size (%d)",
                            mllrCovBandWidth[s][i],i,s,mllrCovBlockSize[s][i]);
            }
            break;
         case CMLLR:
            if (cmllrBlockSize[s]!=NULL) {
               for (i=1; i<=IntVecSize(cmllrBandWidth[s]); i++) 
                  if (cmllrBandWidth[s][i]>cmllrBlockSize[s][i])
                     HError(9999,"CheckAdaptSetup: band width (%d) at %d-th block in %d-th stream of CMLLR transform is larger than block size (%d)",
                            cmllrBandWidth[s][i],i,s,cmllrBlockSize[s][i]);
            }
            break;
         case SEMIT:
         default:
            break;
         }
      }
   }

   /* check threshold */
   if (xformSplitThresh==NULL) {
      xfinfo->xformSplitThresh = xformSplitThresh = CreateVector(&infoStack, hset->swidth[0]);
      for (s=1; s<=hset->swidth[0]; s++)
         xformSplitThresh[s] = -1000.0;
      }
   switch (xKind) {
   case MLLRMEAN: 
      if (mllrMeanSplitThresh==NULL) {
         xfinfo->mllrMeanSplitThresh = mllrMeanSplitThresh = CreateVector(&infoStack, hset->swidth[0]);
         for (s=1; s<=hset->swidth[0]; s++)
            mllrMeanSplitThresh[s] = 1000.0;
      }
      break;
   case MLLRCOV:
      if (mllrCovSplitThresh==NULL) {
         xfinfo->mllrCovSplitThresh = mllrCovSplitThresh = CreateVector(&infoStack, hset->swidth[0]);
         for (s=1; s<=hset->swidth[0]; s++)
            mllrCovSplitThresh[s] = 1000.0;
      }
      break;
   case CMLLR:
      if (cmllrSplitThresh==NULL) {
         xfinfo->cmllrSplitThresh = cmllrSplitThresh = CreateVector(&infoStack, hset->swidth[0]);
         for (s=1; s<=hset->swidth[0]; s++)
            cmllrSplitThresh[s] = 1000.0;
      }
      break; 
   case SEMIT:
   default:
      break;
   }
   
   /* check variance flooring scales for semi-tied covariance */
   if (semiTiedFloorScale==NULL) {
      semiTiedFloorScale = CreateVector(&infoStack, hset->swidth[0]);
      for (s=1; s<=hset->swidth[0]; s++)
         semiTiedFloorScale[s] = 0.1;
   }
   
   return;
}

/* InitXFInfo: initialise XFInfo */
static void InitXFInfo (XFInfo *xfinfo)
{
   int s;

   xfinfo->outSpkrPat = "*.%%%";
   xfinfo->inSpkrPat = NULL;
   xfinfo->paSpkrPat = NULL;
   xfinfo->outXFormExt = NULL;
   xfinfo->inXFormExt = NULL;
   xfinfo->paXFormExt = NULL;
   xfinfo->al_inXFormExt = NULL;
   xfinfo->al_paXFormExt = NULL;
   xfinfo->outXFormDir = NULL;
   xfinfo->paXFormDir = NULL;
   xfinfo->al_inXFormDir = NULL;
   xfinfo->al_paXFormDir = NULL;
   xfinfo->useOutXForm = FALSE;
   xfinfo->useInXForm = FALSE;
   xfinfo->usePaXForm = FALSE;
   xfinfo->use_alInXForm = FALSE;
   xfinfo->use_alPaXForm = FALSE;
   xfinfo->outFullC = FALSE;
   xfinfo->inFullC  = FALSE;
   xfinfo->xformTMF = NULL;
   xfinfo->inXForm = NULL;
   xfinfo->outXForm = NULL;
   xfinfo->paXForm = NULL;
   xfinfo->diagCovXForm = NULL;
   xfinfo->al_hset = NULL;
   xfinfo->al_inXForm = NULL;
   xfinfo->al_paXForm = NULL;
         
   xfinfo->useBias = TRUE;
   xfinfo->outXKind = MLLRMEAN;

   xfinfo->xformSplitThresh    = NULL;
   xfinfo->mllrMeanSplitThresh = NULL;
   xfinfo->mllrCovSplitThresh  = NULL;
   xfinfo->cmllrSplitThresh    = NULL;

   xfinfo->xformAdaptKind    = BASE;
   xfinfo->mllrMeanAdaptKind = BASE;
   xfinfo->mllrCovAdaptKind  = BASE;
   xfinfo->cmllrAdaptKind    = BASE;

   xfinfo->xformRegTree    = NULL;
   xfinfo->mllrMeanRegTree = NULL;
   xfinfo->mllrCovRegTree  = NULL;
   xfinfo->cmllrRegTree    = NULL;

   xfinfo->xformBaseClass    = NULL;
   xfinfo->mllrMeanBaseClass = NULL;
   xfinfo->mllrCovBaseClass  = NULL;
   xfinfo->cmllrBaseClass    = NULL;

   /* initialize blocksize vector and occ threshold for each stream */
   for (s=0; s<SMAX; s++) {
      xfinfo->xformBlockSize  [s] = xfinfo->mllrMeanBlockSize[s] = NULL;
      xfinfo->mllrCovBlockSize[s] = xfinfo->cmllrBlockSize   [s] = NULL;
      xfinfo->xformBandWidth  [s] = xfinfo->mllrMeanBandWidth[s] = NULL;
      xfinfo->mllrCovBandWidth[s] = xfinfo->cmllrBandWidth   [s] = NULL;
   }
   
   for (s=0; s<MAXSTRLEN; s++)
     xfinfo->coutspkr[s] = xfinfo->cinspkr[s] = xfinfo->cpaspkr[s] = '\0';
   xfinfo->nspkr = 0;
   
   xfinfo->baseTriMatTime = -1;
   xfinfo->headpoc = NULL;
   xfinfo->headboc = NULL;
   xfinfo->headac = NULL;

   CreateHeap(&xfinfo->acccaStack,"AccStore", MSTAK, 1, 1.0, 50000, 500000);
   CreateHeap(&xfinfo->bobcaStack,"baseObsStore", MSTAK, 1, 1.0, 50000, 500000);
   CreateHeap(&xfinfo->pobcaStack,"parentObsStore", MSTAK, 1, 1.0, 50000, 500000);
}

/* EXPORT->InitAdapt: initialise configuration parameters */
void InitAdapt (XFInfo *xfinfo_hmm, XFInfo *xfinfo_dur) 
{
   int i,s;
   Boolean b;
   double d;
   char *c,buf[MAXSTRLEN],tmp[MAXSTRLEN];
  
   Register(hadapt_version,hadapt_vc_id);
   nParm = GetConfig("HADAPT", TRUE, cParm, MAXGLOBS);

   /* setup the local memory management - defaults sensible? */
   CreateHeap(&infoStack,"InfoStore", MSTAK, 1, 1.0, 50000, 500000);

   /* Initialise the XFInfo values */
   if (xfinfo_hmm!=NULL) InitXFInfo(xfinfo_hmm);
   if (xfinfo_dur!=NULL) InitXFInfo(xfinfo_dur);

   if (nParm>0){
      /* general adaptation config variables */
      if (GetConfInt(cParm,nParm,"TRACE",&i)) trace = i;
      if (GetConfFlt(cParm,nParm,"MINOCCTHRESH",&d)) minOccThresh = (float) d;
      if (GetConfBool(cParm,nParm,"STOREMINFO",&b)) storeMInfo = b;
      if (GetConfBool(cParm,nParm,"KEEPXFORMDISTINCT",&b)) keepXFormDistinct = b;
      if (GetConfBool(cParm,nParm,"SAVESPKRMODELS",&b)) saveSpkrModels = b;
      
      if (GetConfInt(cParm,nParm,"MAXXFORMITER",&i)) maxXFormIter = i;
      if (GetConfBool(cParm,nParm,"MLLRDIAGCOV",&b)) mllrDiagCov = b;      
      if (GetConfBool(cParm,nParm,"SWAPXFORMS",&b)) swapXForms = b;      
      if (GetConfBool(cParm,nParm,"MLLRCOV2CMLLR",&b)) mllrCov2CMLLR = b; 
      if (GetConfStr (cParm,nParm,"SEMITIEDFLOOR",buf))
         semiTiedFloorScale = ParseConfVector(&infoStack,buf,TRUE);
      if (GetConfInt(cParm,nParm,"MAXSEMITIEDITER",&i)) maxSemiTiedIter = i;
      if (GetConfStr (cParm,nParm,"SEMITIEDMACRO",buf)) semiTiedMacro = CopyString(&infoStack,buf);
      if (GetConfBool(cParm,nParm,"SEMITIED2INPUTXFORM",&b)) semiTied2InputXForm = b; 
      if (GetConfBool(cParm,nParm,"SEMITIEDVFLOOR",&b)) semiTiedVFloor = b; 
      if (GetConfInt(cParm,nParm,"NUMNUISANCEDIM",&i)) numNuisanceDim = i;
      if (GetConfBool(cParm,nParm,"STATICSEMITIED",&b)) staticSemiTied = b; 
      if (GetConfBool(cParm,nParm,"INITNUISANCEFR",&b)) initNuisanceFR = b; 
      if (GetConfBool(cParm,nParm,"INITNUISANCEFRIDENT",&b)) initNuisanceFRIdent = b; 
      if (GetConfBool(cParm,nParm,"SAVESEMITIEDBINARY",&b)) saveSemiTiedBinary = b; 

      /* Adaptation transformation set-up for HMMs */
      if (xfinfo_hmm!=NULL) {
         if (GetConfBool(cParm,nParm,"SAVEFULLC",&b)) xfinfo_hmm->outFullC = b;
         if (GetConfBool(cParm,nParm,"USESMAP",&b)) useSMAPcriterion = b;
         if (GetConfBool(cParm,nParm,"SAVEALLSMAPXFORM",&b)) SaveAllSMAPXForms = b;
         if (GetConfFlt(cParm,nParm,"SMAPSIGMA",&d)) sigmascale = (float) d;
         if (GetConfBool(cParm,nParm,"USEBIAS",&b)) xfinfo_hmm->useBias = b;
         if (GetConfStr (cParm,nParm,"SPLITTHRESH",buf)) 
            xfinfo_hmm->xformSplitThresh = ParseConfVector(&infoStack,buf,TRUE);
         if (GetConfStr (cParm,nParm,"TRANSKIND",buf)) 
            xfinfo_hmm->outXKind = Str2XFormKind(buf);
         if (GetConfStr (cParm,nParm,"BLOCKSIZE",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_hmm->xformBlockSize[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_hmm->xformBlockSize[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
         if (GetConfStr (cParm,nParm,"BANDWIDTH",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_hmm->xformBandWidth[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_hmm->xformBandWidth[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
         if (GetConfStr (cParm,nParm,"BASECLASS",buf)) 
            xfinfo_hmm->xformBaseClass = CopyString(&infoStack,buf);
         if (GetConfStr (cParm,nParm,"REGTREE",buf)) 
            xfinfo_hmm->xformRegTree = CopyString(&infoStack,buf);
         if (GetConfStr (cParm,nParm,"ADAPTKIND",buf)) 
            xfinfo_hmm->xformAdaptKind = Str2AdaptKind(buf);
         
      /* Backward compatibility with old configuration options */
      /* MLLRMEAN specification */
         if (GetConfStr (cParm,nParm,"MLLRMEANSPLITTHRESH",buf)) 
            xfinfo_hmm->mllrMeanSplitThresh = ParseConfVector(&infoStack,buf,TRUE);
         if (GetConfStr (cParm,nParm,"MLLRMEANBLOCKSIZE",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_hmm->mllrMeanBlockSize[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_hmm->mllrMeanBlockSize[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
         if (GetConfStr (cParm,nParm,"MLLRMEANBANDWIDTH",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_hmm->mllrMeanBandWidth[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_hmm->mllrMeanBandWidth[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
      if (GetConfStr (cParm,nParm,"MLLRMEANBASECLASS",buf)) 
            xfinfo_hmm->mllrMeanBaseClass = CopyString(&infoStack,buf);
      if (GetConfStr (cParm,nParm,"MLLRMEANREGTREE",buf)) 
            xfinfo_hmm->mllrMeanRegTree = CopyString(&infoStack,buf);
      if (GetConfStr (cParm,nParm,"MLLRMEANADAPTKIND",buf)) 
            xfinfo_hmm->mllrMeanAdaptKind = Str2AdaptKind(buf);
      /* MLLRCOV specification */      
         if (GetConfStr (cParm,nParm,"MLLRCOVSPLITTHRESH",buf))
            xfinfo_hmm->mllrCovSplitThresh = ParseConfVector(&infoStack,buf,TRUE);
         if (GetConfStr (cParm,nParm,"MLLRCOVBLOCKSIZE",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_hmm->mllrCovBlockSize[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_hmm->mllrCovBlockSize[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
         if (GetConfStr (cParm,nParm,"MLLRCOVBANDWIDTH",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_hmm->mllrCovBandWidth[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_hmm->mllrCovBandWidth[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
      if (GetConfStr (cParm,nParm,"MLLRCOVBASECLASS",buf)) 
            xfinfo_hmm->mllrCovBaseClass = CopyString(&infoStack,buf);
      if (GetConfStr (cParm,nParm,"MLLRCOVREGTREE",buf)) 
            xfinfo_hmm->mllrCovRegTree = CopyString(&infoStack,buf);
      if (GetConfStr (cParm,nParm,"MLLRCOVADAPTKIND",buf)) 
            xfinfo_hmm->mllrCovAdaptKind = Str2AdaptKind(buf);
      /* CMLLR specification */
         if (GetConfStr (cParm,nParm,"CMLLRSPLITTHRESH",buf))
            xfinfo_hmm->cmllrSplitThresh = ParseConfVector(&infoStack,buf,TRUE);
         if (GetConfStr (cParm,nParm,"CMLLRBLOCKSIZE",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_hmm->cmllrBlockSize[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_hmm->cmllrBlockSize[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
         if (GetConfStr (cParm,nParm,"CMLLRBANDWIDTH",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_hmm->cmllrBandWidth[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_hmm->cmllrBandWidth[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
      if (GetConfStr (cParm,nParm,"CMLLRBASECLASS",buf))
            xfinfo_hmm->cmllrBaseClass = CopyString(&infoStack,buf);
      if (GetConfStr (cParm,nParm,"CMLLRREGTREE",buf))
            xfinfo_hmm->cmllrRegTree = CopyString(&infoStack,buf);
      if (GetConfStr (cParm,nParm,"CMLLRADAPTKIND",buf))
            xfinfo_hmm->cmllrAdaptKind = Str2AdaptKind(buf);
         
         CheckAdaptOptions(xfinfo_hmm);
   }

      /* Adaptation transformation set-up for duration models */
      if (xfinfo_dur!=NULL) {
         if (GetConfBool(cParm,nParm,"DURUSEBIAS",&b)) xfinfo_dur->useBias = b;
         if (GetConfStr (cParm,nParm,"DURSPLITTHRESH",buf)) 
            xfinfo_dur->xformSplitThresh = ParseConfVector(&infoStack,buf,TRUE);
         if (GetConfStr (cParm,nParm,"DURTRANSKIND",buf)) 
            xfinfo_dur->outXKind = Str2XFormKind(buf);
         if (GetConfStr (cParm,nParm,"DURBLOCKSIZE",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_dur->xformBlockSize[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_dur->xformBlockSize[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
         if (GetConfStr (cParm,nParm,"DURBANDWIDTH",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_dur->xformBandWidth[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_dur->xformBandWidth[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
         if (GetConfStr (cParm,nParm,"DURBASECLASS",buf)) 
            xfinfo_dur->xformBaseClass = CopyString(&infoStack,buf);
         if (GetConfStr (cParm,nParm,"DURREGTREE",buf)) 
            xfinfo_dur->xformRegTree = CopyString(&infoStack,buf);
         if (GetConfStr (cParm,nParm,"DURADAPTKIND",buf)) 
            xfinfo_dur->xformAdaptKind = Str2AdaptKind(buf);

         /* Backward compatibility with old configuration options */
         /* MLLRMEAN specification */
         if (GetConfStr (cParm,nParm,"DURMLLRMEANSPLITTHRESH",buf)) 
            xfinfo_dur->mllrMeanSplitThresh = ParseConfVector(&infoStack,buf,TRUE);
         if (GetConfStr (cParm,nParm,"DURMLLRMEANBLOCKSIZE",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_dur->mllrMeanBlockSize[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_dur->mllrMeanBlockSize[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
         if (GetConfStr (cParm,nParm,"DURMLLRMEANBANDWIDTH",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_dur->mllrMeanBandWidth[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_dur->mllrMeanBandWidth[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
         if (GetConfStr (cParm,nParm,"DURMLLRMEANBASECLASS",buf)) 
            xfinfo_dur->mllrMeanBaseClass = CopyString(&infoStack,buf);
         if (GetConfStr (cParm,nParm,"DURMLLRMEANREGTREE",buf)) 
            xfinfo_dur->mllrMeanRegTree = CopyString(&infoStack,buf);
         if (GetConfStr (cParm,nParm,"DURMLLRMEANADAPTKIND",buf)) 
            xfinfo_dur->mllrMeanAdaptKind = Str2AdaptKind(buf);
         /* MLLRCOV specification */      
         if (GetConfStr (cParm,nParm,"DURMLLRCOVSPLITTHRESH",buf))
            xfinfo_dur->mllrCovSplitThresh = ParseConfVector(&infoStack,buf,TRUE);
         if (GetConfStr (cParm,nParm,"DURMLLRCOVBLOCKSIZE",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_dur->mllrCovBlockSize[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_dur->mllrCovBlockSize[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
         if (GetConfStr (cParm,nParm,"DURMLLRCOVBANDWIDTH",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_dur->mllrCovBandWidth[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_dur->mllrCovBandWidth[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
         if (GetConfStr (cParm,nParm,"DURMLLRCOVBASECLASS",buf)) 
            xfinfo_dur->mllrCovBaseClass = CopyString(&infoStack,buf);
         if (GetConfStr (cParm,nParm,"DURMLLRCOVREGTREE",buf)) 
            xfinfo_dur->mllrCovRegTree = CopyString(&infoStack,buf);
         if (GetConfStr (cParm,nParm,"DURMLLRCOVADAPTKIND",buf)) 
            xfinfo_dur->mllrCovAdaptKind = Str2AdaptKind(buf);
         /* CMLLR specification */
         if (GetConfStr (cParm,nParm,"DURCMLLRSPLITTHRESH",buf))
            xfinfo_dur->cmllrSplitThresh = ParseConfVector(&infoStack,buf,TRUE);
         if (GetConfStr (cParm,nParm,"DURCMLLRBLOCKSIZE",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_dur->cmllrBlockSize[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_dur->cmllrBlockSize[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
         if (GetConfStr (cParm,nParm,"DURCMLLRBANDWIDTH",buf)) {
            for (s=1,c=buf; s<SMAX && c!=NULL; s++) {
               xfinfo_dur->cmllrBandWidth[s] = ParseConfIntVec(&infoStack,c,FALSE);
               for (i=1; i<=IntVecSize(xfinfo_dur->cmllrBandWidth[s])+2; i++) 
                  c = ParseString(c,tmp);
               if (ParseString(c,tmp)==NULL)
                  break;
            }
         }
         if (GetConfStr (cParm,nParm,"DURCMLLRBASECLASS",buf))
            xfinfo_dur->cmllrBaseClass = CopyString(&infoStack,buf);
         if (GetConfStr (cParm,nParm,"DURCMLLRREGTREE",buf))
            xfinfo_dur->cmllrRegTree = CopyString(&infoStack,buf);
         if (GetConfStr (cParm,nParm,"DURCMLLRADAPTKIND",buf))
            xfinfo_dur->cmllrAdaptKind = Str2AdaptKind(buf);
         
         CheckAdaptOptions(xfinfo_dur);
      }
   }
}

/* EXPORT->ResetAdapt: reset adaptation module */
void ResetAdapt (XFInfo *xfinfo_hmm, XFInfo *xfinfo_dur) 
{
   ResetHeap(&infoStack);
   if (xfinfo_hmm!=NULL) {
      ResetHeap(&xfinfo_hmm->acccaStack);
      ResetHeap(&xfinfo_hmm->bobcaStack);
      ResetHeap(&xfinfo_hmm->pobcaStack);
   }
   if (xfinfo_dur!=NULL) {
      ResetHeap(&xfinfo_dur->acccaStack);
      ResetHeap(&xfinfo_dur->bobcaStack);
      ResetHeap(&xfinfo_dur->pobcaStack);
   }
   
   return;
}

/* Additional code to parse configs to for appropriate thresholds */

static float GetSplitThresh(XFInfo *xfinfo, float *thresh)
{
   int s;
   float min=0.0;

   AdaptXForm *xform          = xfinfo->outXForm;
   Vector xformSplitThresh    = xfinfo->xformSplitThresh;
   Vector mllrMeanSplitThresh = xfinfo->mllrMeanSplitThresh;
   Vector mllrCovSplitThresh  = xfinfo->mllrCovSplitThresh;
   Vector cmllrSplitThresh    = xfinfo->cmllrSplitThresh;

   for (s=1; s<=xform->hset->swidth[0]; s++) {
      thresh[s] = 0.0;
      if (xformSplitThresh[s] > 0.0) {
         thresh[s] = xformSplitThresh[s];
      } 
      else {
     switch(xform->xformSet->xkind) {
     case MLLRMEAN:
            thresh[s] = mllrMeanSplitThresh[s];
       break;
     case MLLRCOV:
            thresh[s] = mllrCovSplitThresh[s];
       break;
     case CMLLR:
            thresh[s] = cmllrSplitThresh[s];
       break;
     }
   }
      if (thresh[s]<min)
         min = thresh[s];
   }

   return min;
}

static AdaptKind GetAdaptKind(XFInfo *xfinfo)
{
   AdaptKind akind = BASE;

   AdaptKind xformAdaptKind    = xfinfo->xformAdaptKind;
   AdaptKind mllrMeanAdaptKind = xfinfo->mllrMeanAdaptKind;
   AdaptKind mllrCovAdaptKind  = xfinfo->mllrCovAdaptKind;
   AdaptKind cmllrAdaptKind    = xfinfo->cmllrAdaptKind;

   if (xformAdaptKind != BASE) {
     akind = xformAdaptKind;
   } 
   else {
      switch(xfinfo->outXKind) {
     case MLLRMEAN:
       akind = mllrMeanAdaptKind;
       break;
     case MLLRCOV:
       akind = mllrCovAdaptKind;
       break;
     case CMLLR:
       akind = cmllrAdaptKind;
       break;
     }
   }
   return akind;
}

static RegTree* GetRegTree(HMMSet *hset, XFInfo *xfinfo)
{
   char* basename = NULL;
   char macroname[MAXSTRLEN];

   AdaptXForm *xform     = xfinfo->outXForm;
   char *xformRegTree    = xfinfo->xformRegTree;
   char *mllrMeanRegTree = xfinfo->mllrMeanRegTree;
   char *mllrCovRegTree  = xfinfo->mllrCovRegTree;
   char *cmllrRegTree    = xfinfo->cmllrRegTree;
   
   if (xformRegTree != NULL) {
     basename = xformRegTree;
   } 
   else {
     switch(xform->xformSet->xkind) {
     case MLLRMEAN:
       basename = mllrMeanRegTree;
       break;
     case MLLRCOV:
       basename = mllrCovRegTree;
       break;
     case CMLLR:
       basename = cmllrRegTree;
       break;
     }
   }
   if (basename == NULL) {
      HError(9999, "GetRegTree: No regression tree macro name specified\n");
      return NULL;
   } 
   return LoadRegTree(hset,NameOf(basename,macroname),basename);  
}

static BaseClass* GetBaseClass(HMMSet *hset, XFInfo *xfinfo)
{
   char* basename = NULL;
   char macroname[MAXSTRLEN];

   AdaptXForm *xform       = xfinfo->outXForm;
   char *xformBaseClass    = xfinfo->xformBaseClass;
   char *mllrMeanBaseClass = xfinfo->mllrMeanBaseClass;
   char *mllrCovBaseClass  = xfinfo->mllrCovBaseClass;
   char *cmllrBaseClass    = xfinfo->cmllrBaseClass;
   
   if (xformBaseClass != NULL) {
     basename = xformBaseClass;
   } 
   else {
     switch(xform->xformSet->xkind) {
     case MLLRMEAN:
       basename = mllrMeanBaseClass;
       break;
     case MLLRCOV:
       basename = mllrCovBaseClass;
       break;
     case CMLLR:
       basename = cmllrBaseClass;
       break;
     }
   }
   if (basename == NULL) {
      HError(-1,"No baseclass macro name specified - global transform assumed");
      basename = "global";
   }
   /* name may be a complete path, or just the macroname */
   return LoadBaseClass(hset,NameOf(basename,macroname),basename);
}

static int GetBaseClassVSize(BaseClass *bclass, const int class)
{
   ILink i;
   MixtureElem *me;

   /* currently does not check consistency of vector sizes */
   i=bclass->ilist[class];
   me = (MixtureElem *)i->item;
   return VectorSize(me->mpdf->mean);
}

/* GetBlockSize: return block size vector for given xform */
static IntVec GetBlockSize(XFInfo *xfinfo, const int b)
{
   IntVec blockSize = NULL;
   int s;

   AdaptXForm *xform = xfinfo->outXForm;
   IntVec *xformBlockSize    = xfinfo->xformBlockSize;
   IntVec *mllrMeanBlockSize = xfinfo->mllrMeanBlockSize;
   IntVec *mllrCovBlockSize  = xfinfo->mllrCovBlockSize;
   IntVec *cmllrBlockSize    = xfinfo->cmllrBlockSize;
   
   /* stream index */
   s = xform->bclass->stream[b];
   
   if (xformBlockSize[s] != NULL) {
      blockSize = xformBlockSize[s];
   } 
   else {
     switch(xform->xformSet->xkind) {
     case MLLRMEAN:
         blockSize = mllrMeanBlockSize[s];
       break;
     case MLLRCOV:
         blockSize = mllrCovBlockSize[s];
       break;
     case CMLLR:
         blockSize = cmllrBlockSize[s];
         break;
      default:
       break;
     }
   }
   if (strmProj) {
      if (blockSize != NULL)
         HError(-1,"For stream projection only full transform used");
      blockSize = CreateIntVec(xform->mem,1);
      blockSize[1] = xform->hset->vecSize;
   }
   else if (blockSize == NULL) {
      blockSize = CreateIntVec(xform->mem,1);
      blockSize[1] = GetBaseClassVSize(xform->bclass,b);
   }
   return blockSize;  
}

/* GetBandWidth: return band width vector for given transform */
static IntVec GetBandWidth(XFInfo *xfinfo, const int b, IntVec blockSize)
{
   IntVec bandWidth = NULL;
   int i,s;
   
   AdaptXForm *xform         = xfinfo->outXForm;
   IntVec *xformBandWidth    = xfinfo->xformBandWidth;
   IntVec *mllrMeanBandWidth = xfinfo->mllrMeanBandWidth;
   IntVec *mllrCovBandWidth  = xfinfo->mllrCovBandWidth;
   IntVec *cmllrBandWidth    = xfinfo->cmllrBandWidth;
   
   /* stream index */
   s = xform->bclass->stream[b];
   
   if (xformBandWidth[s] != NULL) {
      bandWidth = xformBandWidth[s];
   } 
   else {
      switch(xform->xformSet->xkind) {
      case MLLRMEAN:
         bandWidth = mllrMeanBandWidth[s];
         break;
      case MLLRCOV:
         bandWidth = mllrCovBandWidth[s];
         break;
      case CMLLR:
         bandWidth = cmllrBandWidth[s];
         break;
      default:
         break;
      }
   }
   if (bandWidth == NULL) {
      bandWidth = CreateIntVec(xform->mem,IntVecSize(blockSize));
      for (i=1; i<=IntVecSize(blockSize); i++)
         bandWidth[i] = blockSize[i];
   }

   return bandWidth;  
}

/*------------------------------------------------------------------------*/
/*                      Internal Structure Support                        */
/*------------------------------------------------------------------------*/

/* ----------------- Access Routines for the structures  ---------------- */

static RegAcc *GetRegAcc(MixPDF *mp)
{
   return ((XFormInfo *)mp->info)->regAcc;
}

static AInfo *GetAInfo(MixPDF *mp)
{
   return ((XFormInfo *)mp->info)->aInfo;
}

static AInfo *GetPAInfo(MixPDF *mp)
{
   return ((XFormInfo *)mp->info)->paInfo;
}

static MInfo *GetMInfo(MixPDF *mp)
{
   return ((XFormInfo *)mp->info)->mInfo;
}

static ObsCache *GetObsCache(MixPDF *mp)
{
  return ((XFormInfo *)mp->info)->oc;
}

static ObsCache *GetPAObsCache(MixPDF *mp)
{
  return ((XFormInfo *)mp->info)->paoc;
}

static AccCache *GetPAAccCache(MixPDF *mp)
{
  return ((XFormInfo *)mp->info)->paac;
}


/* --------------- handling the MInfo structure ------------------ */

static MInfo *CreateMInfo(MemHeap *x, MixPDF *mp, AdaptXForm *xform)
{
   MInfo *mi;
   Boolean adaptMean, adaptCov;
   AdaptXForm *xf;
   int size;

   size = VectorSize(mp->mean);
   adaptMean = FALSE; adaptCov = FALSE;
   mi = (MInfo *)New(&infoStack,sizeof(MInfo));
   /* depending on the nature of the transform determines the 
      parameters to be stored */
   xf = xform;
   while (xf != NULL) {
      if (StoreAdaptMean(xf)) adaptMean = TRUE;
      if (StoreAdaptCov(xf)) adaptCov = TRUE;
      xf = xf->parentXForm;
   }
   if (adaptMean) {
      mi->mean = CreateSVector(x,size);
      CopyVector(mp->mean,mi->mean);
   } 
   else 
      mi->mean = NULL;
   if (adaptCov) {
      switch(mp->ckind){
      case DIAGC:
      case INVDIAGC:
         mi->cov.var = CreateSVector(x,size);
         CopyVector(mp->cov.var,mi->cov.var);
         mi->gConst = mp->gConst;
         mi->ckind  = mp->ckind;
         break;
      default:
         HError(999,"CreateMInfo: bad ckind %d",mp->ckind);
      }
   } 
   else {
      switch(mp->ckind){
      case DIAGC:
      case INVDIAGC:
         mi->cov.var = NULL;
         mi->gConst = 0.0;
         mi->ckind = NULLC;
         break;
      default:
         HError(999,"CreateMInfo: bad ckind %d",mp->ckind);
      }
   }
   return mi;
}

/* Function used to store the original model parameters. May
   be overriden using 
    HMODEL:STOREMINFO = FALSE
*/
static void SetMInfo(HMMSet *hset, AdaptXForm *xform)
{
   HMMScanState hss;
   MixPDF *mp;
   int nMInfo=0;
  
   NewHMMScan(hset,&hss);
   do {
      while (GoNextState(&hss,TRUE)) {
         while (GoNextStream(&hss,TRUE)) {            
            if (hss.isCont)                     /* PLAINHS or SHAREDHS */
               while (GoNextMix(&hss,TRUE)) {
                  mp = hss.mp;
                  ((XFormInfo *)mp->info)->mInfo = CreateMInfo(hset->hmem,mp,xform);
                  nMInfo++;
               }
         }
      }
   } while (GoNextHMM(&hss));
   EndHMMScan(&hss);
   if (trace&T_ACC) printf("Attached %d MInfo structures\n",nMInfo);
   hset->attMInfo = TRUE;
}

static Boolean CompareMInfo(HMMSet *hset, AdaptXForm *xform)
{
   Boolean adaptMean, adaptCov;
   AdaptXForm *xf;
   HMMScanState hss;
   MixPDF *mp;
   MInfo *mi;

   adaptMean = FALSE; adaptCov = FALSE;
   /* depending on the nature of the transform determines the 
      parameters to be stored */
   xf = xform;
   while (xf != NULL) {
      if (StoreAdaptMean(xf)) adaptMean = TRUE;
      if (StoreAdaptCov(xf)) adaptCov = TRUE;
      xf = xf->parentXForm;
   }
   /* now check to see what is currently stored */
   NewHMMScan(hset,&hss);
   mp = hss.mp;
   mi = GetMInfo(mp);
   if ((adaptMean) && (mi->mean == NULL)) {
      EndHMMScan(&hss);
      return FALSE;
   }
   if ((adaptCov) && (mi->cov.var == NULL)) {
      EndHMMScan(&hss);
      return FALSE;
   }
   EndHMMScan(&hss);
   return TRUE;
}

/* 
   If the model info has already been set need to check that there are 
   no new parameters that need to be stored - add additional 
   storage as required
*/
static void UpdateMInfo(HMMSet *hset, AdaptXForm *xform)
{
   Boolean adaptMean, adaptCov;
   AdaptXForm *xf;
   int size;
   HMMScanState hss;
   MixPDF *mp;
   MInfo *mi;
   int nMInfo=0;

   adaptMean = FALSE; adaptCov = FALSE;
   /* depending on the nature of the transform determines the 
      parameters to be stored */
   xf = xform;
   while (xf != NULL) {
      if (StoreAdaptMean(xf)) adaptMean = TRUE;
      if (StoreAdaptCov(xf)) adaptCov = TRUE;
      xf = xf->parentXForm;
   }
   /* now check to see what is currently stored */
   NewHMMScan(hset,&hss);
   do {
      while (GoNextState(&hss,TRUE)) {
         while (GoNextStream(&hss,TRUE)) {            
            if (hss.isCont)                     /* PLAINHS or SHAREDHS */
               while (GoNextMix(&hss,TRUE)) {
                  mp = hss.mp;
                  mi = GetMInfo(mp);
                  if ((adaptMean) && (mi->mean == NULL)) {
		     size = VectorSize(mp->mean);
                     mi->mean = CreateSVector(hset->hmem,size);
                     CopyVector(mp->mean,mi->mean);
                     nMInfo++;
                  } 
                  if (adaptCov) {
                     switch(mp->ckind){
                     case DIAGC:
                     case INVDIAGC:
                        if (mi->cov.var == NULL) {
			   size = VectorSize(mp->mean);
                           mi->cov.var = CreateSVector(hset->hmem,size);
                           CopyVector(mp->cov.var,mi->cov.var);
                           mi->gConst = mp->gConst;
                           mi->ckind  = mp->ckind; 
                        }
                        break;
                     default:
                        HError(999,"AccMixPDFStats: bad ckind %d",mp->ckind);
                     }
                     nMInfo++;
                  }
               }
         }
      }
   } while (GoNextHMM(&hss));
   EndHMMScan(&hss);
   if (trace&T_ACC) printf("Attached %d additional MInfo structures\n",nMInfo);
}

/* --------------- handling the XFormInfo structure ------------------ */

static XFormInfo *CreateXFormInfo(MemHeap *x)
{
   XFormInfo *info;

   info = (XFormInfo *)New(x,sizeof(XFormInfo));
   info->aInfo  = NULL;
   info->mInfo = NULL;
   info->paInfo = NULL;
   info->regAcc = NULL;
   info->paoc = NULL;
   info->oc = NULL;
   info->paac = NULL;
   return info;
}

void AttachXFormInfo(HMMSet *hset)
{
   HMMScanState hss;
   MixPDF *mp;
   int nXFormInfo=0;
  
   NewHMMScan(hset,&hss);
   do {
      while (GoNextState(&hss,TRUE)) {
         while (GoNextStream(&hss,TRUE)) {            
            if (hss.isCont)                     /* PLAINHS or SHAREDHS */
               while (GoNextMix(&hss,TRUE)) {
                  mp = hss.mp;
                  mp->info = (XFormInfo *)CreateXFormInfo(hset->hmem);
                  nXFormInfo++;
               }
            else
               HError(7450, "AttachXFormInfo: Adaptation only available for PLAIN or SHARED systems!");
         }
      }
   } while (GoNextHMM(&hss));
   EndHMMScan(&hss); 
   if (trace&T_ACC) printf("Attached %d XFormInfo structures\n",nXFormInfo);
   hset->attXFormInfo = TRUE;
}

static Boolean CompareXFormInfo(AdaptXForm *xform1, AdaptXForm *xform2)
{
   AdaptXForm *xf1, *xf2;
   Boolean valid=FALSE;

   if (xform1 == xform2) return TRUE;
   xf1 = xform1; xf2 = xform2;
   while ((xf1 != NULL) && (xf2 != NULL)) {
      if (xf1->bclass != xf2->bclass)
         return valid;
      xf1 = xf1->parentXForm;
      xf2 = xf2->parentXForm;
   }
   /* check that they are both the same length */
   if ((xf1 == NULL) && (xf2 == NULL))
      valid = TRUE;
   return valid;
}

/* --------------- handling the AInfo structure ------------------ */

static void SetAInfo(HMMSet *hset, AdaptXForm *xform, Boolean parent)
{
   BaseClass *bclass;
   MixPDF *mp;
   ILink i;
   int b, nlevel;
   AdaptXForm *xf;
   AInfo *ai;
   int nAInfo=0;

   if (!hset->attXFormInfo) AttachXFormInfo(hset);
   /* transform baseclasses differ reset internal information */
   /* ResetHeap(&infoStack); */
   if (xform != NULL) {
      /* setup the adptation information for each component 
         according to the baseclass information.
         Expect the itemlist to always specify components .... */
      nlevel = 0; xf = xform;
      while (xf != NULL) {
         nlevel++;
         bclass = xf->bclass;
         for (b = 1; b <= bclass->numClasses; b++) {
            for (i=bclass->ilist[b]; i!=NULL; i=i->next) {
               mp = ((MixtureElem *)i->item)->mpdf;
               if (parent) ai = ((XFormInfo *)mp->info)->paInfo;
               else ai = ((XFormInfo *)mp->info)->aInfo;
               if (ai == NULL) {
                  if (parent) 
                     ((XFormInfo *)mp->info)->paInfo = ai = (AInfo *)New(&infoStack,sizeof(AInfo));
                  else ((XFormInfo *)mp->info)->aInfo = ai = (AInfo *)New(&infoStack,sizeof(AInfo));
                  nAInfo++;
               } 
               else if (nlevel>1) { 
                  /* go the end of the chain and add adaptation info, but not at the first level */
                  /* a fix to tidy memory at this stage is required */
                  while (ai->next != NULL) ai=ai->next;
                  ai = ai->next = (AInfo *)New(&infoStack,sizeof(AInfo));
                  nAInfo++;
               }
               ai->baseClass = b; ai->level = nlevel; ai->next = NULL;
            }
         }
         xf = xf->parentXForm;
      }
   }
   else { /* This may be called during a reset so set all the ai's to NULL */
      HMMScanState hss;
      NewHMMScan(hset,&hss);
      do {
         while (GoNextState(&hss,TRUE)) {
            while (GoNextStream(&hss,TRUE)) {            
                  while (GoNextMix(&hss,TRUE)) {
                     ((XFormInfo *)hss.mp->info)->aInfo = NULL;
                  }
            }
         }
      } while (GoNextHMM(&hss));
      EndHMMScan(&hss);
   }
   if (trace&T_ACC) printf("Attached %d AInfo structures\n",nAInfo);
}

/* --------------- handling the RegAcc structure ------------------ */

/* Using stored information reset all the model parameters */
static void ResetComp(MixPDF *mp)
{
   MInfo *mi;

   mi = GetMInfo(mp); 
   if ( mi != NULL) { /* Initial model parameters have been stored */
      /* copy original mean */
      if (mi->mean != NULL) CopyVector(mi->mean,mp->mean);
      
      if (mi->cov.var != NULL) {
         /* transformed variance is stored in FULLC form */
         if (mp->ckind==FULLC)
            mp->cov.var = (SVector) GetHook(mp->cov.inv);
                  
         /* copy original var */
         switch (mi->ckind) {
      case DIAGC:
      case INVDIAGC:
         case FULLC:
            CopyVector(mi->cov.var,mp->cov.var);
            mp->gConst = mi->gConst;
            mp->ckind  = mi->ckind;
         break;
      default:
            HError(999,"ResetComp: bad ckind %d",mi->ckind);
         }
      }
   } 
}

static DTriMat *CreateBlockTriMat(MemHeap *x, IntVec blockSize)
{
   DTriMat *tm;
   int nblock, bsize, b;
   long *i;
  
  nblock = IntVecSize(blockSize);
   tm = (DTriMat *)New(x,sizeof(DTriMat)*(nblock+1));  
   i = (long *) tm; *i = (long) nblock;
  for (b=1;b<=nblock;b++) {
    bsize = blockSize[b];
      tm[b] = CreateDTriMat(x, bsize); 
      ZeroDTriMat(tm[b]);
  }
  return(tm);
}

static void ZeroBlockTriMat(DTriMat *bTriMat)
{
  int b;
  long *nblock;

  nblock = (long *) bTriMat;
  for (b=1; b<=*nblock; b++)
     ZeroDTriMat(bTriMat[b]);
}

static void ZeroBaseTriMat(DTriMat *bTriMat)
{
  int i;
   long *vsize;
   DTriMat tm;
 
   vsize = (long *) bTriMat;
  for (i=1;i<=*vsize;i++) {
    tm = bTriMat[i];
      ZeroDTriMat(tm);
  }  
}

static void CreateBaseTriMat(XFInfo *xfinfo, MemHeap *x, MixPDF *mp, int class)
{
   DTriMat *tm;
  int vsize = VectorSize(mp->mean);
   IntVec blockSize = GetBlockSize(xfinfo,class);
  RegAcc *regAcc, *ra;
  MixPDF *me;
  BaseClass *bclass;
  ILink i;
   int j, cntj, b, bsize;
   long *vsp;
   AdaptXForm *xform = xfinfo->outXForm;

  regAcc = GetRegAcc(mp);
   if (vsize>0 && xform->info->accBTriMat) { 
      /* create BaseTriMat and attach it to RegAcc */
    regAcc->bVector =  CreateDVector(x,vsize);
    ZeroDVector(regAcc->bVector);
    regAcc->bDiagMat = CreateBlockTriMat(x,blockSize); 
    ZeroBlockTriMat(regAcc->bDiagMat);
      tm = (DTriMat *)New(x,sizeof(DTriMat)*(vsize+1));
      vsp = (long *) tm; *vsp = (long) vsize;
    for (b=1,cntj=1;b<=IntVecSize(blockSize);b++) {
      bsize = blockSize[b];
      for (j=1;j<=bsize;j++,cntj++) {
            tm[cntj] =  CreateDTriMat(x, bsize);  
            ZeroDTriMat(tm[cntj]);
      }
    }
    regAcc->bTriMat = tm;    

      /* link BaseTriMat to models in the same class */
  bclass = xform->bclass;
  for (i=bclass->ilist[class]; i!=NULL; i=i->next) { 
      me = ((MixtureElem *)i->item)->mpdf;
      if( me != mp ) {
        ra = GetRegAcc(me);
        ra->bVector = regAcc->bVector;
        ra->bDiagMat = regAcc->bDiagMat;
        ra->bTriMat = regAcc->bTriMat;
      }
      }
  }      
}

void SetBaseAccsTime(XFInfo *xfinfo, const int t)
{
   xfinfo->baseTriMatTime = t;
}


void UpdateAccCache(double Lr, Vector svec, MixPDF *mp)
{
   AccCache *paac;
   DTriMat m;
   int vsize = VectorSize(mp->mean);
   Vector covar;
   int i, j, bstart, bsize;
   long bl, nblock;

   paac = GetPAAccCache(mp);    
   if (paac==NULL) return;
   if ((paac->bTriMat!=NULL) && (paac->bVector[1]==0.0) && (svec!=NULL)) {
      nblock = (long)(paac->bTriMat[0]);
	for (bl=1,bstart=0;bl<=nblock;bl++) {
	  m = paac->bTriMat[bl];
         bsize = DTriMatSize(m);
	  for (i=1;i<=bsize;i++) { /* Fill the accumulate stores */
	    for (j=1; j<=i; j++)
		m[i][j] = svec[i+bstart] * svec[j+bstart];
	  }
	  bstart += bsize;
	}
      covar = mp->cov.var;
      for (i=1;i<=vsize;i++) {
         if (mp->ckind==INVDIAGC)
           paac->bVector[i] += Lr * (double) covar[i];
         else
           paac->bVector[i] += Lr / (double) covar[i];
      }
   }
}

void UpdateBaseAccs(XFInfo *xfinfo, Vector svec, const int t, RegAcc *regAcc)
{
   int i,j,k,b,bsize;
   long nblock, bl;
   int cnt, cnti, cntj;
   DTriMat tm, m;
   RegAcc *ra;
   DVector acc;
   BaseClass *bclass;
   MixPDF *mp;
   AdaptXForm *outXForm = xfinfo->outXForm;
   
   /* store statistics */
   if (t != xfinfo->baseTriMatTime) {
   bclass = outXForm->bclass;
#pragma omp parallel for private(mp,ra,acc,nblock,bl,cnti,m,bsize,i,tm,j,k)
   for (b=1;b<=bclass->numClasses;b++) {
      mp = ((MixtureElem *)(bclass->ilist[b])->item)->mpdf;
      ra = GetRegAcc(mp);
         if ((ra->bTriMat!=NULL) && (ra->bVector[1]>0.0)) {    
         acc = ra->bVector;
            nblock = (long)(ra->bDiagMat[0]);
    for (bl=1,cnti=1;bl<=nblock;bl++) {
      m = ra->bDiagMat[bl];
               bsize = DTriMatSize(m);
      for (i=1;i<=bsize;i++,cnti++) { /* Fill the accumulate stores */
        tm = ra->bTriMat[cnti];
        for (j=1; j<=bsize; j++)
          for (k=1; k<=j; k++)
            tm[j][k] += m[j][k] * acc[cnti];
      }
         }
         ZeroDVector(ra->bVector);
      }
   }
}

   if (regAcc==NULL) return;
   if ((regAcc->bTriMat!=NULL) && (regAcc->bVector[1]==0.0) && (svec!=NULL)) {
      nblock = (long)(regAcc->bDiagMat[0]);
      for (bl=1,cnt=1; bl<=nblock; bl++) {
         bsize = DTriMatSize(regAcc->bDiagMat[bl]);
         m = regAcc->bDiagMat[bl];
         for (i=1,cnti=cnt; i<=bsize; i++,cnti++) {
            for (j=1,cntj=cnt; j<=i; j++,cntj++)
               m[i][j] = svec[cnti]*svec[cntj];
         }
         cnt +=bsize;
      }
   }
}

void UpdateBaseAccsWithPaac(XFInfo *xfinfo, const int t)
{
   int i,j,k,b,bsize;
   long nblock, bl;
   int cnti;
   DTriMat tm, m;
   RegAcc *ra;
   DVector acc;
   BaseClass *bclass;
   MixPDF *mp;
   AccCache *paac;
   AdaptXForm *outXForm = xfinfo->outXForm;

   /* store statistics */
   if (t != xfinfo->baseTriMatTime) {
   bclass = outXForm->bclass;
      for (b=bclass->numClasses;b>0;b--) {
      mp = ((MixtureElem *)(bclass->ilist[b])->item)->mpdf;
         paac = ((XFormInfo *)mp->info)->paac;
      ra = GetRegAcc(mp);
         if ((ra->bTriMat!=NULL) && (paac->bVector[1]>0.0)) {
            acc = paac->bVector;
            nblock = (long)(ra->bDiagMat[0]);
            for (bl=1,cnti=1;bl<=nblock;bl++) {
              m = paac->bTriMat[bl];
               bsize = DTriMatSize(m);
              for (i=1;i<=bsize;i++,cnti++) { /* Fill the accumulate stores */
                tm = ra->bTriMat[cnti];
                for (j=1; j<=bsize; j++)
                  for (k=1; k<=j; k++)
                    tm[j][k] += m[j][k] * acc[cnti];
              }
            }
          }
        }
      }
   }  

void ResetAccCache(XFInfo *xfinfo, const int t)
{
   AccCache *ac;
 
   if (t != xfinfo->baseTriMatTime) {
      if (xfinfo->headac != NULL) {
         for (ac=xfinfo->headac; ac!=NULL; ac=ac->next) {
        ZeroDVector(ac->bVector);
         }
      }
   }
}

static Boolean XFormModCovar(AdaptXForm *xform)
{
  Boolean isModified = FALSE;

  while (xform != NULL){
    if (StoreAdaptCov(xform)){
      isModified = TRUE;
      break;
    }
    xform = xform->parentXForm;
  }
  return isModified;
}

static void AccBaseTriMat(XFInfo *xfinfo, double Lr, Vector svec, MixPDF *mp, int t)
{
   int vsize, i;
   Vector covar;
   RegAcc *regAcc;
   Boolean resetMixPDF = FALSE;
   AdaptXForm *xform = xfinfo->outXForm;
   HMMSet *hset;
  
   if (xform != NULL) {
      hset = xform->hset;
   if (((hset->parentXForm == NULL) && (hset->curXForm == NULL) ) || (hset->parentXForm == hset->curXForm))  {
      /* There's nothing to be done as model set the same */
      resetMixPDF = FALSE;
      }
      else if (hset->parentXForm == NULL) {
      /* xform to be built on original parameters */
      /* reset the models if the inputXForm alters the variances */
         if(xfinfo->covarChanged){
	      ResetComp(mp);
         resetMixPDF = TRUE;
      }
      }
      else {
      /* xform to be built on a parent xform */
         if (xfinfo->covarPChanged || xfinfo->covarChanged){
            ApplyCompXForm(mp,hset->parentXForm,FALSE);
         resetMixPDF = TRUE;
      }
   }
   }
   
   regAcc = GetRegAcc(mp);
   covar = mp->cov.var;
   vsize = VectorSize(mp->mean);

     /* Check to see whether this is the very first frame */
   if (xfinfo->headac==NULL) 
      UpdateBaseAccs(xfinfo,svec,t,regAcc);
     else  {
      UpdateBaseAccsWithPaac(xfinfo,t); 
      ResetAccCache(xfinfo,t);   
   }
   SetBaseAccsTime(xfinfo,t);

   if  (xfinfo->headac==NULL) {    
     for (i=1;i<=vsize;i++) {
       if (mp->ckind==INVDIAGC)
            regAcc->bVector[i] += Lr * (double) covar[i];
       else
            regAcc->bVector[i] += Lr / (double) covar[i];
     }
   } 
   else  
     UpdateAccCache( Lr, svec, mp);

   if (resetMixPDF) {
      hset = xform->hset;
      ApplyCompXForm(mp,hset->curXForm,FALSE);
   }
}

static RegAcc *CreateRegAcc(MemHeap *x, MixPDF *mp, AdaptXForm *xform)
{
  RegAcc *regAcc;
  int vsize = VectorSize(mp->mean);
  
  regAcc = (RegAcc *)New(x,sizeof(RegAcc));
   regAcc->occ = 0.0;
  if (xform->info->accSum) {
      regAcc->spSum = CreateDVector(x,vsize);
      ZeroDVector(regAcc->spSum);
   } 
   else regAcc->spSum = NULL;
  if (xform->info->accSumSq) {
      regAcc->spSumSq = CreateDVector(x,vsize);
      ZeroDVector(regAcc->spSumSq);
   } 
   else regAcc->spSumSq = NULL;

  regAcc->bTriMat = NULL;   
   regAcc->bDiagMat = NULL;
   regAcc->bVector = NULL;   

  return regAcc;
}

static void AttachRegAccs(HMMSet *hset, XFInfo *xfinfo)
{  
  MixPDF *mp = NULL;
  int nRegAcc=0, b;
  BaseClass *bclass;
  ILink i;
   AdaptXForm *xform = xfinfo->outXForm;

  /* RegAccs stored on the Info structure */
  if (!hset->attXFormInfo) AttachXFormInfo(hset);
  bclass = xform->bclass;
  for (b=1;b<=bclass->numClasses;b++) {
    for (i=bclass->ilist[b]; i!=NULL; i=i->next) {
      mp = ((MixtureElem *)i->item)->mpdf;
      ((XFormInfo *)(mp->info))->regAcc = (RegAcc *)CreateRegAcc(hset->hmem,mp,xform);
      nRegAcc++;   
    }

    /* Use last component of the baseclass to access baseclass stats */
      CreateBaseTriMat(xfinfo,hset->hmem,mp,b); 
  }

  if (trace&T_ACC) printf("Attached %d RegAcc structures\n",nRegAcc);
  hset->attRegAccs = TRUE;
}

/* --------------- handling the AccStruct structure ------------------ */

static AccStruct *CreateAccStruct(MemHeap *x, XFInfo *xfinfo, int vsize, 
                                  IntVec blockSize, IntVec bandWidth)
{
  AccStruct *accs;
  int dim,i,cnti;
  int b,bsize; 

   Boolean useBias = xfinfo->useBias;
   AdaptXForm *xform = xfinfo->outXForm;

  accs = (AccStruct *)New(x,sizeof(AccStruct));
   accs->occ = 0.0;
  accs->dim = vsize;
   accs->useBias = useBias;
  accs->blockSize = blockSize;
   accs->bandWidth = bandWidth;
  accs->K = NULL;
  accs->G = NULL;
  accs->D = NULL;
  /* Depending on the form of transform initialise appropriate
     structure elements */
  accs->xkind = xform->xformSet->xkind;
  switch (accs->xkind) {
  case MLLRMEAN:
    accs->K = (DVector *)New(x,(vsize+1)*sizeof(DVector));
    accs->G = (DMatrix *)New(x,(vsize+1)*sizeof(DMatrix));
    for (b=1,cnti=1;b<=IntVecSize(blockSize);b++) {
      bsize = blockSize[b];
      if (useBias) dim = bsize+1;
      else dim = bsize;
      for (i=1;i<=bsize;i++,cnti++) {
	accs->K[cnti] = CreateDVector(x,dim);
	ZeroDVector(accs->K[cnti]);
	accs->G[cnti] = CreateDMatrix(x,dim,dim);
	ZeroDMatrix(accs->G[cnti]);
      }
    }
    if (mllrDiagCov) {
      accs->D = CreateDVector(x,vsize);
      ZeroDVector(accs->D);
    }
    break;
  case MLLRCOV:
    accs->G = (DMatrix *)New(x,(vsize+1)*sizeof(DMatrix));
    for (b=1,cnti=1;b<=IntVecSize(blockSize);b++) {
      bsize = blockSize[b];
      for (i=1;i<=bsize;i++,cnti++) {
        accs->G[cnti] = CreateDMatrix(x,bsize,bsize);
        ZeroDMatrix(accs->G[cnti]);
      }
    }
    break;
  case SEMIT:
    accs->G = (DMatrix *)New(x,(vsize+1)*sizeof(DMatrix));
    /* For stream projections CANNOT have multiple blocks */
    if (strmProj) {
       for (i=1;i<=vsize;i++) {
          accs->G[i] = CreateDMatrix(x,vsize,vsize);
          ZeroDMatrix(accs->G[i]);
       }
      } 
      else {
       for (b=1,cnti=1;b<=IntVecSize(blockSize);b++) {
          bsize = blockSize[b];
          for (i=1;i<=bsize;i++,cnti++) {
             if ((staticSemiTied) && (b>1)){ 
                /* link all accumulates into one */
                accs->G[cnti] = accs->G[i];
               } 
               else {
                accs->G[cnti] = CreateDMatrix(x,bsize,bsize);
                ZeroDMatrix(accs->G[cnti]);
             }
          }
       }
    }
    break;
  case CMLLR:
    accs->K = (DVector *)New(x,(vsize+1)*sizeof(DVector));
    accs->G = (DMatrix *)New(x,(vsize+1)*sizeof(DMatrix));
    for (b=1,cnti=1;b<=IntVecSize(blockSize);b++) {
      bsize = blockSize[b];
      if (useBias) dim = bsize+1;
      else dim = bsize;
      for (i=1;i<=bsize;i++,cnti++) {
        accs->K[cnti] = CreateDVector(x,dim);
        ZeroDVector(accs->K[cnti]);
        accs->G[cnti] = CreateDMatrix(x,dim,dim);
        ZeroDMatrix(accs->G[cnti]);
      }
    }
    break;
  default :
    HError(999,"Transform kind not currently supported");
    break;
  }
  return accs;
}


/* 
   A separate global floor is set for each of the streams.
   For systems with mixed streams for a baseclass no 
   variance floors is applied.
*/
static void SetSemiTiedAvCov(HMMSet *hset)
{
   HMMScanState hss;
   StateInfo *si   ;
   StreamInfo *sti;
   MixPDF *mp;
   STriMat inv;
   DVector mean;
   DMatrix *mat, *smat, cov;
   DVector *svec,occAcc;
   double occ, tmp;
   int k,kk,s,S;
   MuAcc *ma;
   VaAcc *va;
   int vsize,m;

   /* allocate accumulators */
   S= hset->swidth[0];
   mat = (DMatrix *)New(&gstack,(S+1)*sizeof(DMatrix));
   smat = (DMatrix *)New(&gstack,(S+1)*sizeof(DMatrix));
   svec = (DVector *)New(&gstack,(S+1)*sizeof(DVector));
   occAcc = CreateDVector(&gstack, S);
   avCov = (TriMat *)New(hset->hmem,(S+1)*sizeof(TriMat));

   for (s=1;s<=S;s++) {
      if (strmProj) 
         vsize = hset->vecSize;
      else 
         vsize = hset->swidth[s];
           
      mat[s] = CreateDMatrix(&gstack,vsize,vsize);
      smat[s] = CreateDMatrix(&gstack,vsize,vsize);
      svec[s] = CreateDVector(&gstack,vsize);
      avCov[s] = CreateTriMat(hset->hmem,vsize);  /* create on HMMSet memory heap */
      ZeroDMatrix(mat[s]);
   }
   ZeroDVector(occAcc);
   
   NewHMMScan(hset,&hss);
   while(GoNextState(&hss,FALSE)) {
      si = hss.si;
      while (GoNextStream(&hss,TRUE)) {
         s = hss.s;
         cov = smat[s]; ZeroDMatrix(cov);
         mean = svec[s]; ZeroDVector(mean);
         sti = hss.sti;
         occ = 0;
         for (m=1; m<= hss.M; m++) {
            mp = sti->spdf.cpdf[m].mpdf;
            if (strmProj) 
               vsize = hset->vecSize;
            else 
               vsize = VectorSize(mp->mean);
               
            if (vsize==hset->swidth[s]) {  /* MSD check */
            ma = (MuAcc *)GetHook(mp->mean);
            va = (VaAcc *)GetHook(mp->cov.var);
               if (ma==NULL || va==NULL)
                  HError(9999,"SetSemiTiedAvCov: mean/variance accumulators are NULL");
            if (fabs(ma->occ-va->occ) > 0.1)
               HError(999,"State-clustered systems only for semitied updates");
               if (va->occ > 0.0) {
               inv = va->cov.inv;
               occ += va->occ;
               for (k=1;k<=vsize;k++) { 
                  mean[k] += ma->mu[k];
                  for (kk=1;kk<=k;kk++) 
                     cov[k][kk] += inv[k][kk];
               }
            }
         }
         }
         /* accumulate in */
         if (occ > 0.0) {
            for (k=1; k<=DVectorSize(mean); k++) {
               tmp = mean[k]/occ;
               for (kk=1;kk<=k;kk++) 
                  mat[s][k][kk] += cov[k][kk] - tmp*mean[kk];
            }
         }
         occAcc[s] += occ;
      }
   }
   EndHMMScan(&hss);
   
   /* normalisation and put in avCov */
   for (s=1;s<=S;s++) {
      if (strmProj) vsize = hset->vecSize;
      else vsize = hset->swidth[s];
      for (k=1;k<=vsize;k++)
         for (kk=1;kk<=k;kk++)
            avCov[s][k][kk] = mat[s][k][kk]/occAcc[s];
   }
   Dispose(&gstack,mat);
}

static void FixSemiTiedStats(HMMSet *hset)
{
   /* 
      This uses the standard HMM stats and sets the stream number for each 
      component (future support for multiple stream systems).
   */
   HMMScanState hss;
   MixPDF *mp;
   int k,kk;
   float tmp;
   VaAcc *va;
   MuAcc *ma;
   Vector mu;
   STriMat inv;

   NewHMMScan(hset,&hss);
   do {
      while (GoNextState(&hss,TRUE)) {
         while (GoNextStream(&hss,TRUE)) {            
            if (hss.isCont)                     /* PLAINHS or SHAREDHS */
               while (GoNextMix(&hss,TRUE)) {
                  mp = hss.mp; 
                  mp->stream = hss.s;
                  /* 
                     fix the statistics to be zero-mean out-product 
                  */
                  va = (VaAcc *) GetHook(mp->cov.var);
                  if (va->occ>0.0) {
                     ma = (MuAcc *) GetHook(mp->mean);
                     inv = va->cov.inv;
                     mu = ma->mu;
                     for (k=1;k<=VectorSize(mu);k++) {
                        tmp = mu[k]/va->occ;
                        for (kk=1;kk<=k;kk++)
                           inv[k][kk] -= tmp*mu[kk];
                     }
                  }
               }
         }
      }
   } while (GoNextHMM(&hss));
   EndHMMScan(&hss);
}

/*------------------------------------------------------------------------*/
/*                       Regression Tree Parsing                          */
/*------------------------------------------------------------------------*/

static double SetNodeOcc (RegNode *node, BaseClass *bclass)
{
   int c, b, stream, vsize;
   ILink i;
   MixPDF *mp=NULL;
   double occ;

   node->nodeOcc = 0.0;
   if (node->numChild>0) {
      for (c=1;c<=node->numChild;c++) {
         /* Check vector sizes of child nodes are consistent */
         occ = SetNodeOcc(node->child[c], bclass);
         stream = (node->child[c])->stream;
         vsize  = (node->child[c])->vsize;
         
         /* check consistency of stream and vector size */
         if ( ((node->stream>-1) && (stream != node->stream)) || 
              ((node->vsize >-1) && (vsize  != node->vsize )) )
            node->valid = FALSE;
         else {
            node->stream = stream;
            node->vsize  = vsize;
            node->nodeOcc += occ;
      }
      }
   } 
   else {
      for (b=1;b<=IntVecSize(node->baseClasses);b++) {
         node->stream = bclass->stream[node->baseClasses[b]];
         for (i=bclass->ilist[node->baseClasses[b]]; i!=NULL; i=i->next) {
            mp = ((MixtureElem *)i->item)->mpdf;

            /* Baseclass definition ensures that vector sizes are correct within a baseclass */
            vsize = VectorSize(mp->mean);
            if (node->vsize>-1) {
               if (vsize != node->vsize)
                  HError(999,"Inconsistent vector size in baseclasses (%d %d)", vsize, node->vsize);
         }
            else
                node->vsize = vsize;

            node->nodeOcc += (GetRegAcc(mp))->occ;
      }
   }
   }

   return node->nodeOcc;
}


static void SearchLeafNodes (RegNode *node, IntVec classes)
{
   int b,c;
   
   if (node->numChild>0) { /* Not a terminal node */
     for (c=1;c<=node->numChild;c++)
       SearchLeafNodes(node->child[c], classes);
   } 
   else { /* Mark baseclasses for adaptation */
     for (b=1;b<=IntVecSize(node->baseClasses);b++) 
       classes[node->baseClasses[b]] = 1;
   }
}


static Boolean ParseNode (XFInfo *xfinfo, RegNode *node,  
                          RegTree *rtree, IntVec classes, int xfindex)
{
   int b,c,size,parentxfindex=0;
   Boolean genXForm;
   IntVec lclasses;

   Boolean GenXForm(RegNode *node, XFInfo *xfinfo, IntVec classes, int xfindex);

   genXForm = FALSE;
   if (trace&T_TRE) printf("Node %d (stream=%d, vsize=%d, occ=%f)\n",
                           node->nodeIndex,node->stream,node->vsize,node->nodeOcc);

   if(useSMAPcriterion){

     if (node->nodeOcc > rtree->thresh[node->stream]) {
       size = IntVecSize(classes);
       lclasses = CreateIntVec(&gstack,size); /* temporal */
       ZeroIntVec(lclasses);
       SearchLeafNodes(node, lclasses);
       if (trace&T_TRE) 
         printf("Prior Transform index: %d\n",xfindex);
       if(GenXForm(node,xfinfo,lclasses,xfindex)){
         parentxfindex = xfinfo->outXForm->xformSet->numXForms;
       }
       FreeIntVec(&gstack,lclasses);
     } 
     if (node->numChild>0) { /* Not a terminal node */
       for (c=1;c<=node->numChild;c++)
         ParseNode(xfinfo, node->child[c], rtree, classes, parentxfindex);
     } 
     genXForm = TRUE;

   }else{

     if (node->nodeOcc > rtree->thresh[node->stream]) {
      size = IntVecSize(classes);
      lclasses = CreateIntVec(&gstack,IntVecSize(classes));
      ZeroIntVec(lclasses);
      if (node->numChild>0) { /* Not a terminal node */
         for (c=1;c<=node->numChild;c++)
           if (ParseNode(xfinfo, node->child[c], rtree, lclasses, xfindex)) genXForm = TRUE;
         /* any of the children need a xform generate it */
         if (genXForm) GenXForm(node,xfinfo,lclasses,xfindex);
       } 
       else { /* Generate xform for this node */
         for (b=1;b<=IntVecSize(node->baseClasses);b++) lclasses[node->baseClasses[b]] = 1;
         GenXForm(node,xfinfo,lclasses,xfindex);
      }
      FreeIntVec(&gstack,lclasses);
      genXForm = FALSE;
     } 
     else {
      if (node->numChild>0) { /* Not a terminal node */
         for (c=1;c<=node->numChild;c++)
           ParseNode(xfinfo, node->child[c], rtree, classes, xfindex);
       } 
       else { /* Mark baseclasses for adaptation */
         for (b=1;b<=IntVecSize(node->baseClasses);b++) classes[node->baseClasses[b]] = 1;
      }
      genXForm = TRUE;
   }
   }

   return genXForm;
}

static void SearchOutputXforms(AdaptXForm *xform)
{

  int i, b, numxforms=0;
  Boolean USEXFORM = FALSE;
  LinXForm **outxforms;
  IntVec outassign=NULL;

  outxforms = (LinXForm **)New(xform->mem,(xform->bclass->numClasses+1)*sizeof(LinXForm *));
   if (HardAssign(xform)) {
     outassign = CreateIntVec(xform->mem,xform->bclass->numClasses);
     ZeroIntVec(outassign);
   }
   else 
      HError(999,"Not currently supported");

  for (i=1;i<=xform->xformSet->numXForms;i++) {
    USEXFORM = FALSE;
    for (b=1;b<=xform->bclass->numClasses;b++) {
      if (HardAssign(xform)){
        if(xform->xformWgts.assign[b] == i)
          USEXFORM = TRUE;
      }
      else 
        HError(999,"Not currently supported");
    }
    if(USEXFORM){
      numxforms++;
      outxforms[numxforms] = CopyLinXForm(xform->mem,xform->xformSet->xforms[i]); 
      for (b=1;b<=xform->bclass->numClasses;b++) {
        if (HardAssign(xform)){
          if(xform->xformWgts.assign[b] == i)
            outassign[b] = numxforms;
        }
        else 
          HError(999,"Not currently supported");
      }
    }
  }
  
  CopyIntVec(outassign, xform->xformWgts.assign);
  xform->xformSet->numXForms = numxforms;
  xform->xformSet->xforms = NULL;
  xform->xformSet->xforms = outxforms;
  
}

static Boolean ParseTree (XFInfo *xfinfo)
{
   IntVec classes;
   double occ,minthresh;
   
   AdaptXForm *xform = xfinfo->outXForm;
   RegTree *rtree = xform->rtree;

   /* First set the correct threshold for this tree */
   minthresh = GetSplitThresh(xfinfo,rtree->thresh);
   occ = SetNodeOcc(rtree->root, rtree->bclass);

   if (occ<minthresh) /* not enough data to generate transforms */
      return FALSE;
   /* reset the number of transforms */
   xform->xformSet->numXForms = 0;
   if (xform->xformSet->xkind == SEMIT) 
      HError(999,"No support for semi-tied transforms with regression class trees");
   classes = CreateIntVec(&gstack,rtree->bclass->numClasses);

         ZeroIntVec(classes);
   ParseNode(xfinfo, rtree->root, rtree, classes, 0);
   FreeIntVec(&gstack,classes);

   if(useSMAPcriterion && (!SaveAllSMAPXForms))
     SearchOutputXforms(xform);

   if (xform->xformSet->numXForms == 0) return FALSE;
   else return TRUE;
}

/*------------------------------------------------------------------------*/
/*                 Accumulation of statistics from Tree                   */
/*------------------------------------------------------------------------*/

static void Tri2DMat (DMatrix m1, DMatrix m2)
{
  int i,j,nrows,ncols;

  nrows = NumDRows(m2); ncols = NumDCols(m2);
  if (nrows != ncols)
    HError(5270,"Tri2Mat: target matrix not square %d vs %d",
	   nrows,ncols);   
  /* if (ncols != TriMatSize(m1)) 
    HError(5270,"Tri2Mat: sizes differ %d vs %d",
	   TriMatSize(m1),ncols);
  */
  if (ncols != NumDRows(m1))
    HError(5270,"Tri2Mat: sizes differ %d vs %d",
	   NumDRows(m1),ncols);
  for (i=1; i<=nrows; i++)
    for (j=1; j<=i; j++) {
      m2[i][j] = m1[i][j];
      if (i!=j) m2[j][i] = m1[i][j];
    }
}

static void AccCMLLRBaseStats(MixPDF *mp, AccStruct *accs)
{
  /* update for the accumulates at the base level */
  RegAcc *ra;
  int i,j,k;
  int cnti,b,bsize;
   DTriMat tm;
 
  ra = GetRegAcc(mp);

   if (ra->bTriMat==NULL)
      return;

  for (b=1,cnti=1;b<=IntVecSize(accs->blockSize);b++) {
    bsize = accs->blockSize[b];
    for (i=1;i<=bsize;i++,cnti++) {
      tm = ra->bTriMat[cnti];
      for (j=1;j<=bsize;j++) {
         for (k=1;k<=j;k++) {
            accs->G[cnti][j][k] += tm[j][k];
         }      
      }
    }
  }
   
   return;
}
 
/* AccCMLLRPDFStats: accumulate pdf-dependent statistics to estimate CMLLR transform */
static void AccCMLLRPDFStats(MixPDF *mp,  AccStruct *accs)
{
  RegAcc *ra;
  int i,j;
   double icov=0.0,scale;
  int cnt,cnti,cntj,b,bsize;
   Boolean useBias = accs->useBias;
  Vector mean;
  Covariance cov;
 
  ra = GetRegAcc(mp);
  mean = mp->mean;
  cov = mp->cov;
   
  for (b=1,cnti=1,cnt=1;b<=IntVecSize(accs->blockSize);b++) {
    bsize = accs->blockSize[b];
    for (i=1;i<=bsize;i++,cnti++) {
      switch(mp->ckind){
      case INVDIAGC:
        icov = cov.var[cnti];
        break;
      case DIAGC:
            icov = 1.0/cov.var[cnti];
        break;
      default:
            HError(999,"AccCMLLRPDFStats: bad ckind %d",mp->ckind);
      }
      scale = ra->occ * icov;
      for (j=1,cntj=cnt;j<=bsize;j++,cntj++) {
        accs->K[cnti][j] += ra->spSum[cntj] * mean[cnti] * icov;
        if (useBias)
          accs->G[cnti][bsize+1][j] += icov * ra->spSum[cntj];
      }
      if (useBias) {
        accs->K[cnti][bsize+1] += scale * mean[cnti];
        accs->G[cnti][bsize+1][bsize+1] += scale;
      }
    }
    cnt += bsize;
  }
}

static void AccMLLRPDFStats(MixPDF *mp,  AccStruct *accs)
{
   RegAcc *ra;
   int i,j,k;
   double icov=0.0,scale;
   int cnt,cnti,cntj,cntk,b,bsize;
   Boolean useBias = accs->useBias;
   Vector mean;
   Covariance cov;

   ra = GetRegAcc(mp);
   mean = mp->mean;
   cov = mp->cov;
   for (b=1,cnti=1,cnt=1;b<=IntVecSize(accs->blockSize);b++) {
      bsize = accs->blockSize[b];
      for (i=1;i<=bsize;i++,cnti++) {
         switch(mp->ckind){
         case INVDIAGC:
            icov = cov.var[cnti]; 
            break;
         case DIAGC:
            icov = 1.0/cov.var[cnti]; 
            break;
         default:
            HError(999,"AccMLLRPDFStats: bad ckind %d",mp->ckind);
         }
         scale = ra->occ * icov;
         for (j=1,cntj=cnt;j<=bsize;j++,cntj++) {
            accs->K[cnti][j] += ra->spSum[cnti] * mean[cntj] * icov;
            for (k=1,cntk=cnt;k<=j;k++,cntk++)
               accs->G[cnti][j][k] += scale * mean[cntj] * mean[cntk];
            if (useBias)
               accs->G[cnti][bsize+1][j] += scale * mean[cntj];
         }
         if (useBias) {
            accs->K[cnti][bsize+1] += ra->spSum[cnti] *icov;
            accs->G[cnti][bsize+1][bsize+1] += scale;
         }
	 if (mllrDiagCov) {
	   accs->D[cnti] += icov * ra->spSumSq[cnti];
	 }
      }
      cnt += bsize;
   }
}

static void AccMLLRCOVPDFStats(MixPDF *mp,  AccStruct *accs)
{
  RegAcc *ra;
  int i,j,k;
   double icov=0.0,scale, c1, c2, c3;
  int cnt,cnti,cntj,cntk,b,bsize;
  Vector mean;
  Covariance cov;

  ra = GetRegAcc(mp);
  mean = mp->mean;
  cov = mp->cov;
  for (b=1,cnti=1,cnt=1;b<=IntVecSize(accs->blockSize);b++) {
    bsize = accs->blockSize[b];
    for (i=1;i<=bsize;i++,cnti++) {
      switch(mp->ckind){
      case INVDIAGC:
	icov = cov.var[cnti]; 
	break;
      case DIAGC:
	icov = 1/cov.var[cnti]; 
	break;
      default:
            HError(999,"AccMLLRCOVPDFStats: bad ckind %d",mp->ckind);
      }
      scale = ra->occ * icov;
      for (j=1,cntj=cnt;j<=bsize;j++,cntj++) {
         for (k=1,cntk=cnt;k<=j;k++,cntk++) {
            c1 = scale * mean[cntj] * mean[cntk] ;
            c2 = ra->spSum[cntj] * mean[cntk] * icov;
            c3 = ra->spSum[cntk] * mean[cntj] * icov;  
            accs->G[cnti][j][k] += (c1 - c2 -c3);
         }	
      }    
    }
    cnt += bsize;
  }
}

static void AccMixPDFStats(HMMSet *hset, MixPDF *mp, AccStruct *accs)
{
  RegAcc *ra;

  ra = GetRegAcc(mp);
  if (ra->occ > minOccThresh) {
    accs->occ += ra->occ;
    if (((hset->parentXForm == NULL) && (hset->curXForm == NULL) ) || (hset->parentXForm == hset->curXForm))  {
      /* There's nothing to be done as model set the same */
      } 
      else if (hset->parentXForm == NULL) { 
      /* xform to be built on original parameters */
      ResetComp(mp);
      } 
      else {
      /* xform to be built on a parent xform */
         ApplyCompXForm(mp,hset->parentXForm,FALSE);
    }
    switch (accs->xkind) {
    case MLLRMEAN:
      AccMLLRPDFStats(mp,accs);
      break;
    case MLLRCOV:
      AccMLLRCOVPDFStats(mp,accs);           
      break;
    case CMLLR:
      AccCMLLRPDFStats(mp,accs);
      break;
    case SEMIT:
     /* The accstructure is not used for semi-tied estimation */
       break;
    default :
      HError(999,"Transform kind not currently supported");
      break;
    }
  }
}

static void AccBaseClassStats(MixPDF *mp, AccStruct *accs)
{
	/* RegAcc *ra; */

  /* 
     Accumulate the statistics for the base classes. The 
     parent transforms have been already sorted.
  */
   
  switch (accs->xkind) {
  case MLLRCOV:
  case CMLLR:
    AccCMLLRBaseStats(mp,accs);
    break;
  default :
    HError(999,"Transform kind not currently supported");
    break;
  }
}

static void AccNodeStats(RegNode *node, AccStruct *accs, AdaptXForm *xform, IntVec classes)
{
  BaseClass *bclass;
  ILink i;
  int b,c;
  MixPDF *mp = NULL;
  

  if (node->numChild>0) {
    for (c=1;c<=node->numChild;c++)
      AccNodeStats(node->child[c],accs,xform,classes);
   } 
   else {
    bclass = xform->bclass;
    for (b=1;b<=IntVecSize(node->baseClasses);b++) {
      for (i=bclass->ilist[node->baseClasses[b]]; i!=NULL; i=i->next) {
	mp = ((MixtureElem *)i->item)->mpdf;
	AccMixPDFStats(xform->hset,mp,accs);
      }
      /* Use last component of the baseclass to access baseclass stats */
         if (AccAdaptBaseTriMat(xform))  
            AccBaseClassStats(mp,accs);
      }
   }
}

/* Add SMAP Prior to the accumulates */ 
static void AddXFPrior(AccStruct *accs, AdaptXForm *xform, int xfindex)
{
   int i,j;
   int cnt,cnti,b,bsize;
   Boolean useBias = accs->useBias;
   LinXForm *pxf;
   XFormSet *xformSet;

   xformSet = xform->xformSet;
   pxf = xformSet->xforms[xfindex]; 
   if(IntVecSize(accs->blockSize) != IntVecSize(pxf->blockSize)){
     HError(999,"AddXFPrior (%d) does not match block number for transform (%d)",
        IntVecSize(accs->blockSize),IntVecSize(pxf->blockSize));
   }
   if(accs->dim != pxf->vecSize){
     HError(999,"AddXFPrior (%d) does not match vector size for transform (%d)",
        accs->dim,pxf->vecSize);
   }

   for (b=1,cnti=1,cnt=1; b<=IntVecSize(accs->blockSize); b++) {
      bsize = accs->blockSize[b];
      if(bsize != pxf->blockSize[b]){
         HError(999,"AddXFPrior (%d) does not match block size for transform (%d)",
            accs->blockSize[b],pxf->blockSize[b]);
      }
      for (i=1; i<=bsize; i++,cnti++) {
         for (j=1; j<=bsize; j++) {
            accs->K[cnti][j] += pxf->xform[b][i][j]*sigmascale;
            accs->G[cnti][j][j] += sigmascale;
         }
         if (useBias) {
            accs->K[cnti][bsize+1] += pxf->bias[cnti]*sigmascale;
            accs->G[cnti][bsize+1][bsize+1] += sigmascale;
         }
    }
      cnt += bsize;
   }
}

/* Add Piror to the statistics Currently MLLRMEAN and CMLLR are supported */
static void AddPrior(AccStruct *accs, AdaptXForm *xform, int xfindex)
{
  
   switch (accs->xkind) {
   case MLLRMEAN:
   case CMLLR:
     AddXFPrior(accs,xform,xfindex);
     break;
   default :
     HError(999,"Transform kind not currently supported");
     break;
  }

}

/* Feature-Space adaptation */
static void FixDet(LinXForm *xf)
{
   int ind,nblock;
   double scale, bdet;
   double det;
 
   nblock = IntVecSize(xf->blockSize);
   if ( nblock == xf->vecSize) {   
      det=0.0;
      for (ind=1;ind<=xf->vecSize;ind++) {
         scale = xf->xform[ind][1][1];
         det += log(scale*scale);
      }
      xf->det = (float) det;
   } 
   else {   
      det=0.0; 
      for (ind=1;ind<=nblock;ind++) {
         bdet = MatDet(xf->xform[ind]);
         det += 2*log(fabs(bdet));
      }
      xf->det = (float) det;
   }
}
/*------------------------------------------------------------------------*/
/*     Accummulator Cache for application of parent XForms                */
/*------------------------------------------------------------------------*/
static AccCache *CreateAccCache(XFInfo *xfinfo, const int b)
{
   AccCache *ac;
   IntVec size;
   int vsize, bl;

   size = GetBlockSize(xfinfo,b),
   
   vsize = 0;
   for (bl=1;bl<=IntVecSize(size);bl++) vsize += size[bl];
   
   ac = (AccCache *)New(&xfinfo->acccaStack,sizeof(AccCache));
   ac->baseclass = b;
   ac->bVector  = CreateDVector(&xfinfo->acccaStack,vsize); 
   ZeroDVector(ac->bVector);
   ac->bTriMat = CreateBlockTriMat(&xfinfo->acccaStack,size);    
   ZeroBlockTriMat(ac->bTriMat);
   ac->next = xfinfo->headac;
   xfinfo->headac = ac;
   return(ac);
}

static void SetAccCache(XFInfo *xfinfo)
{
   MixPDF *mp;
   BaseClass *bclass;
   int b;
   ILink i;
   AccCache **ac = NULL;
   int nxflevel = 0, nxfcomb = 1, numXf = 0, nxf, ind;
   int nCache = 0;
   AInfo *ai;
   AdaptXForm *xf;
   XFormSet  *xformSet;
   HMMSet *hset;
   
   AdaptXForm *xform = xfinfo->outXForm;
   
   if ((xform != NULL) && (AccAdaptBaseTriMat(xform))) {
      hset = xform->hset;
      if (hset->parentXForm != NULL) {
        xform->parentXForm = hset->parentXForm;
        xform->parentXForm->nUse++;
      } 
      else
        xform->parentXForm = NULL;    

      nxflevel = 1; 
      nxfcomb *= (xform->bclass->numClasses + 1);
      xf = xform->parentXForm;
      /* Count the number of levels and combinations */
      while ( xf != NULL ) {
         if (StoreObsCache(xf)) {
            nxfcomb *= (xf->xformSet->numXForms + 1);
            nxflevel++;
         }
         xf = xf->parentXForm;
      }
 
      if (nxflevel>0) {
	ac = (AccCache **)New(&gstack,sizeof(AccCache *)*(nxfcomb+1));
         for ( ind = 0; ind <= nxfcomb; ind++)
            ac[ind] = NULL;
      }
 
      bclass = xform->bclass;
      for (b = 1; b <= bclass->numClasses; b++) {
         for (i=bclass->ilist[b]; i!=NULL; i=i->next) {
            mp = ((MixtureElem *)i->item)->mpdf;
            if (nxflevel  == 0)    ((XFormInfo *)mp->info)->paac = NULL;
            else {
               ai = GetPAInfo(mp);
               nxf = xform->bclass->numClasses + 1;
               ind = b;
               xf = xform->parentXForm;
               while ( xf != NULL ){
                  xformSet = xf->xformSet;
                  if (StoreObsCache(xf)) {
                     if (HardAssign(xform))
                        numXf = xf->xformWgts.assign[ai->baseClass];
                     else
                        HError(999,"Not currently supported");
                     ind += numXf * nxf;
                     nxf *= (xformSet->numXForms + 1);
                  }
                  xf = xf->parentXForm;
                  if ( xf != NULL )  ai = ai->next;
               }
               if (nxflevel > 0) {
                  if (ind > 0) { /* support no transform has been generated */
                    if ( ac[ind] == NULL )  {
                        ac[ind] = CreateAccCache(xfinfo,b);
                      nCache++;
                    }
                  }
                  ((XFormInfo *)mp->info)->paac = ac[ind];
               }
            }
         }
      }
      if (ac != NULL) Dispose(&gstack,ac);
      ac = NULL;
   }
   if (trace&T_TOP)
     printf("Created %d AccCaches (of %d possible)\n",nCache,nxfcomb);
}

/*------------------------------------------------------------------------*/
/*      Observation Cache for application of feature-space transform      */
/*------------------------------------------------------------------------*/

static ObsCache *CreateObsCache(MemHeap *heap, ObsCache **headoc, const int size)
{
   ObsCache  *oc;

   oc = (ObsCache *)New(heap,sizeof(ObsCache));
   oc->time = -1;
   oc->obs = CreateVector(heap,size);
   ZeroVector(oc->obs);
   oc->det =0;  
 
   oc->next = *headoc;
   *headoc = oc;

   return(oc);
}
  

static void SetObsCache(XFInfo *xfinfo, AdaptXForm *xform, Boolean parent)
{
   MixPDF *mp;
   BaseClass *bclass;
   int b, size, s;
   ILink i;
   ObsCache **headoc, **oc = NULL;
   int nxflevel = 0, nxfcomb = 1, numXf = 0, nxf, ind;
   int nCache = 0;
   AInfo *ai;
   AdaptXForm *xf;
   XFormSet *xformSet;
   MemHeap *heap;

   if (xform != NULL) {

    if (parent) {
        ResetHeap(&xfinfo->pobcaStack);
        heap = &xfinfo->pobcaStack;
        xfinfo->headpoc = NULL;
        headoc = &(xfinfo->headpoc);
    }
    else {
        ResetHeap(&xfinfo->bobcaStack);
        heap = &xfinfo->bobcaStack;
        xfinfo->headboc = NULL;
        headoc = &(xfinfo->headboc);
    }

    xf = xform;
    /* Count the number of levels and combinations */
    while ( xf != NULL ) {
      if (StoreObsCache(xf)) {
   nxfcomb *= (xf->xformSet->numXForms + 1);
   nxflevel++;
      }
      xf = xf->parentXForm;
    }
    
    if (nxflevel>0) {
      oc = (ObsCache **)New(&gstack,sizeof(ObsCache *)*(nxfcomb+1));
      for ( ind = 0; ind <= nxfcomb; ind++) {
   oc[ind] = NULL;
      }
    }
       
    bclass = xform->bclass;
    for (b = 1; b <= bclass->numClasses; b++) {
         s = bclass->stream[b];
      for (i=bclass->ilist[b]; i!=NULL; i=i->next) {
   mp = ((MixtureElem *)i->item)->mpdf;
   if (nxflevel  == 0) {
     if (parent) {
       ((XFormInfo *)mp->info)->paoc = NULL;
     }
     else ((XFormInfo *)mp->info)->oc = NULL;
            } 
            else {
     if (parent) ai = GetPAInfo(mp);
     else  ai = GetAInfo(mp);

     xf = xform; nxf = 1; 
     ind = 0;  
     while ( xf != NULL ){
       xformSet = xf->xformSet;
       if (StoreObsCache(xf)) {
         if (HardAssign(xform)) 
      numXf = xf->xformWgts.assign[ai->baseClass];
         else
      HError(999,"Not currently supported");
         ind += numXf * nxf;
         nxf *= (xformSet->numXForms + 1);
       } 
       xf = xf->parentXForm;
       ai = ai->next;
     }   
     if (nxflevel > 0) {
       if (ind > 0) { /* support no transform has been generated */
                     size = xform->hset->swidth[s];
         if ( oc[ind] == NULL )  {
      oc[ind] = CreateObsCache(heap, headoc, size);
      nCache++;
         }
       }
       if (parent) ((XFormInfo *)mp->info)->paoc = oc[ind];
       else ((XFormInfo *)mp->info)->oc = oc[ind];
     }
   }
      } 
    }
    if (oc != NULL) Dispose(&gstack,oc);
    oc = NULL;
   }
   if (trace&T_TOP)
     printf("Created %d ObsCaches (of %d possible)\n",nCache,nxfcomb);
}

static void UpdateObsCache( ObsCache *oc, Vector svec, LogFloat det, int t)  
{
   int i;

   if (oc != NULL ) {  
      if (t != oc->time) {
         oc->time = t;
         for (i=1; i<=VectorSize(oc->obs); i++)
            oc->obs[i] = (i<=VectorSize(svec)) ? svec[i] : ReturnIgnoreValue();
         oc->det = det;
      } 
   }
}

void ResetObsCache(XFInfo *xfinfo)
{
   ObsCache *oc;
      
   if(xfinfo != NULL){
      if (xfinfo->headboc!=NULL) {
         for (oc=xfinfo->headboc; oc!= NULL; oc=oc->next) {
       oc->time = -1;
       ZeroVector(oc->obs);
            oc->det = 0.0;
     }
   }
      if (xfinfo->headpoc!=NULL) {
         for (oc=xfinfo->headpoc; oc!= NULL; oc=oc->next) {
       oc->time = -1;
       ZeroVector(oc->obs);
            oc->det = 0.0;
         }
     }
   }
}

/*------------------------------------------------------------------------*/
/*                          Adaptation Application                         */
/*------------------------------------------------------------------------*/

/* ApplyXForm2Vector: apply xform to a given vector (mean' = A*mean+b) */
static void ApplyXForm2Vector(LinXForm *linXForm, Vector mean)
{  
   Vector vec, bias;
   int b,bsize;
   Matrix A;
   double tmp;
   int i,j;
   int cnt,cnti,cntj;

   /* Check dimensions */
   const int size = linXForm->vecSize;
   if (size != VectorSize(mean))
      HError(999,"Transform dimension (%d) does not match mean dimension (%d)",
             size,VectorSize(mean));
   vec = CreateVector(&gstack,size);
   CopyVector(mean,vec); /* ZeroVector(mean); */
   /* Transform mean */
   for (b=1,cnti=1,cnt=1;b<=IntVecSize(linXForm->blockSize);b++) {
      bsize = linXForm->blockSize[b];
      A = linXForm->xform[b];
      for (i=1;i<=bsize;i++,cnti++) {
         tmp = 0.0;
         for (j=1,cntj=cnt;j<=bsize;j++,cntj++)
            tmp += A[i][j] * vec[cntj];
         mean[cnti] = tmp;
      }
      cnt += bsize;
   }
   /* Apply bias if required */
   bias = linXForm->bias;
   if (bias != NULL) {
      for (i=1;i<=size;i++)
         mean[i] += bias[i];
   }
   FreeVector(&gstack,vec);
}

/* ApplyCMLLRXForm2Vector: apply CMLLR XForm to mean vector (mean' = A^{-1}*(mean-b)) */
static void ApplyCMLLRXForm2Vector(LinXForm *linXForm, Vector mean)
{  
   Vector vec, bias;
   int b,bsize;
   Matrix A;
   double tmp;
   int i,j;
   int cnt,cnti,cntj;

   /* Check dimensions */
   const int size = linXForm->vecSize;
   if (size != VectorSize(mean))
      HError(999,"Transform dimension (%d) does not match mean dimension (%d)",
             size,VectorSize(mean));
   vec = CreateVector(&gstack,size);
   CopyVector(mean,vec); ZeroVector(mean);

   /* First apply bias (if required) */
   bias = linXForm->bias;
   if (bias != NULL) {
      for (i=1;i<=size;i++)
         vec[i] -= bias[i];
   }

   /* Transform mean */
   for (b=1,cnti=1,cnt=1;b<=IntVecSize(linXForm->blockSize);b++) {
      bsize = linXForm->blockSize[b];
      A = CreateMatrix(&gstack, bsize, bsize);
      MatInvert(linXForm->xform[b], A);
      for (i=1;i<=bsize;i++,cnti++) {
         tmp = 0.0;
         for (j=1,cntj=cnt;j<=bsize;j++,cntj++)
            tmp += A[i][j] * vec[cntj];
         mean[cnti] = tmp;
      }
      cnt += bsize;
      FreeMatrix(&gstack, A);
   }
   
   FreeVector(&gstack,vec);
}

/* ApplyXForm2Cov: apply XForm to a given covariance matrix (cov' = A'*cov*A) */
static void ApplyXForm2Cov(LinXForm *linXForm, Covariance *cov, CovKind ckind)
{
   int b,bsize;
   Matrix A,A1,A2,mat;
   int i,j;
   int cnt,cnti,cntj;

   /* Check dimensions */
   const int size = TriMatSize(cov->inv);
   if (size != linXForm->vecSize)
      HError(999,"ApplyXForm2Cov: Transform dimension (%d) does not match covariance matrix dimension (%d)",
             linXForm->vecSize,size);
   
   /* prepare matrices */
   mat = CreateMatrix(&gstack,size,size);
   A1  = CreateMatrix(&gstack,size,size); ZeroMatrix(A1);
   A2  = CreateMatrix(&gstack,size,size); ZeroMatrix(A2);
   
   /* copy inverse covariance to mat */
   Tri2Mat(cov->inv,mat);
    
   /* copy transformation matrices to A1 and A2 */
   for (b=1,cnti=1,cnt=1;b<=IntVecSize(linXForm->blockSize);b++) {
      bsize = linXForm->blockSize[b];
      A = linXForm->xform[b];
      for (i=1;i<=bsize;i++,cnti++)
         for (j=1,cntj=cnt;j<=bsize;j++,cntj++)
            A1[cntj][cnti] = A2[cnti][cntj] = A[i][j];
      cnt += bsize;
   }
      
   /* compute A1*mat*A2 */
   MatrixMult(A1, mat, mat);
   MatrixMult(mat, A2, mat);

   /* copy transformed covariance to cov->inv */
   Mat2Tri(mat, cov->inv);
   
   FreeMatrix(&gstack,mat);
}

/* ApplyXForm2TriMat: Apply XForm to a given triangular matrix (m = A*t) */ 
static void ApplyXForm2TriMat (LinXForm *linXForm, TriMat t, Matrix m)
{
   int size,b,bsize;
   Matrix A,mat;
   float tmp;
   int i,j,k;
   int cnt,cnti,cntj,cntk;

   /* Check dimensions */
   size = linXForm->vecSize;
   if (size != TriMatSize(t))
      HError(999,"Transform dimension (%d) does not match matrix dimension (%d)",
             size,TriMatSize(t));
   mat = CreateMatrix(&gstack,size,size);
   Tri2Mat(t,mat); 
   /* Transform mean */
   for (b=1,cnti=1,cnt=1;b<=IntVecSize(linXForm->blockSize);b++) {
      bsize = linXForm->blockSize[b];
      A = linXForm->xform[b];
      for (i=1;i<=bsize;i++,cnti++) {
         for (j=1,cntj=cnt;j<=bsize;j++,cntj++) {
            tmp=0;
            for (k=1,cntk=cnt;k<=bsize;k++,cntk++)
               tmp += A[i][k] * mat[cntk][cntj];
            m[cnti][cntj] = tmp;
         }
      }
      cnt += bsize;
   }
   if (linXForm->bias != NULL) HError(999,"Assumes there is no bias in transform");  
   FreeMatrix(&gstack,mat);
}

static void DiagApplyMat2TXForm(LinXForm *linXForm, Matrix m, Vector v)
{  
   int size,b,bsize;
   Matrix A;
   float tmp;
   int i,j;
   int cnt,cnti,cntj;

   /* Check dimensions */
   size = linXForm->vecSize;
   if (size != NumRows(m))
      HError(999,"Transform dimension (%d) does not match matrix dimension (%d)",
             size,NumRows(m));
   for (b=1,cnti=1,cnt=1;b<=IntVecSize(linXForm->blockSize);b++) {
      bsize = linXForm->blockSize[b];
      A = linXForm->xform[b];
      for (i=1;i<=bsize;i++,cnti++) {
         tmp=0;
         for (j=1,cntj=cnt;j<=bsize;j++,cntj++) {
            tmp +=  m[cnti][cntj] * A[i][j];
         }
         v[cnti] = tmp;
      }
      cnt += bsize;
   }
   if (linXForm->bias != NULL) HError(999,"Assumes there is no bias in transform");  
}

/* ConvFullCov: store diag covariance in full covariance form */
static void ConvFullCov (HMMSet *hset, MixPDF *mp)
{
   int i,j,vsize;
   SVector var;
   STriMat inv;
   MInfo *mi;
   
   mi = GetMInfo(mp);
   var = mp->cov.var;
   vsize = VectorSize(var);
   
   /* create full cov matrix and hook var and inv each other */
   if(mi == NULL) {
      inv = CreateSTriMat(hset->hmem, vsize);
      SetHook(inv, var);
   }
   else if ((inv=GetHook(mi->cov.var))==NULL) {
      inv = CreateSTriMat(hset->hmem,vsize);
      SetHook(mi->cov.var,inv);
      SetHook(inv,var);
   }

   switch(mp->ckind) {
   case DIAGC:
      for (i=1; i<=vsize; i++) {
         for (j=1; j<i; j++)
            inv[i][j] = 0.0;
         inv[i][i] = 1.0/var[i];
      }
      mp->cov.inv = inv;
      mp->ckind   = FULLC;
      break;
   case INVDIAGC:
      for (i=1; i<=vsize; i++) {
         for (j=1; j<i; j++)
            inv[i][j] = 0.0;
         inv[i][i] = var[i];
      }
      mp->cov.inv = inv;
      mp->ckind   = FULLC;
      break;
   case FULLC:
   default:
      break;
   }
   
   return;
}
      
/* CompFXForm: Feature-Space adaptation */
static Vector CompFXForm(MixPDF *mp, Vector svec, AdaptXForm *xform, AInfo *ai, LogFloat *det)
{
  Vector vec;
  XFormSet *xformSet;
  int numXf = 0;

  if (ai->next != NULL) { /* There's a parent transform */
    vec = CompFXForm(mp,svec,xform->parentXForm,ai->next,det);
   } 
   else {
      *det = 0.0;
     vec = svec;
  }
   
   if (mp->ckind==FULLC)
      return svec;
   
  /* Check the kind of the adaptation */
  if ((xform->akind != BASE) && (xform->akind != TREE))
    HError(999,"Only BASE and TREE adaptation currently supported");
  if (HardAssign(xform))
    numXf = xform->xformWgts.assign[ai->baseClass];
  else 
    HError(999,"Not currently supported");
  /* Apply linear transformations to the parameters */
  if (numXf > 0) { /* Allows support when no transforms have been generated */
    xformSet = xform->xformSet;
    switch (xformSet->xkind) {
    case CMLLR: 
    case MLLRCOV:
    case SEMIT:
       ApplyXForm2Vector(xformSet->xforms[numXf],svec);
       *det += 0.5* (xformSet->xforms[numXf]->det);
       break;
    default:
      /* nothing is done */
      break;
    } /* No other options currently supported */
   } 
   else {
    /* no transforms equates to an identity transform */
    svec = vec;
  }
  return svec;
}

/* CompXForm: Model space adaptation */
static void CompXForm(MixPDF *mp, AdaptXForm *xform, AInfo *ai, Boolean full)
{
  XFormSet *xformSet;
   int numXf, i, size;
   Vector var;

   if (ai->next != NULL) { /* There are parent transforms */
      CompXForm(mp,xform->parentXForm,ai->next,full);
   } 
   else { /* set up model parameters for adptation */
    ResetComp(mp);
      if (full)
         ConvFullCov(xform->hset, mp);
  }
  /* Check the kind of the adaptation */
  if ((xform->akind != BASE) && (xform->akind != TREE))
    HError(999,"Only BASE and TREE adaptation currently supported");
  numXf = xform->xformWgts.assign[ai->baseClass];
  /* Apply linear transformations to the parameters */
  if (numXf > 0) { /* Allows support when no transforms have been generated */
    xformSet = xform->xformSet;
    switch (xformSet->xkind) {
    case MLLRMEAN:
      ApplyXForm2Vector(xformSet->xforms[numXf],mp->mean);
      break;
    case MLLRCOV:
         if (full) {  
            /* store transformed variance in full covariance form */
            ApplyXForm2Cov(xformSet->xforms[numXf],&mp->cov,mp->ckind);
            /* mp->gConst -= xformSet->xforms[numXf]->det; */
            FixFullGConst(mp,-CovDet(mp->cov.inv));
         }
         else
      ApplyXForm2Vector(xformSet->xforms[numXf],mp->mean);
      break;
    case MLLRVAR:    
      switch(mp->ckind){
      case DIAGC:
        ApplyXForm2Vector(xformSet->xforms[numXf], mp->cov.var);
        FixDiagGConst(mp);
        break;
      case INVDIAGC:    
            size = VectorSize(mp->mean);
            var  = CreateVector(&gstack,size);
            /* inv var -> var */
        for (i=1;i<=size;i++)
               var[i] = 1.0/mp->cov.var[i];
            /* apply xform to var */
            ApplyXForm2Vector(xformSet->xforms[numXf], var);
            /* store transformed var in inv var form */
        for (i=1;i<=size;i++)
               mp->cov.var[i] = 1.0/var[i];
            /* fix gConst */
        FixInvDiagGConst(mp);
            FreeVector(&gstack, var);
        break;
      default:
        HError(999,"CompXForm: bad ckind %d",mp->ckind);
      }
      break;
      case SEMIT:
      case CMLLR:
         if (full) { 
            ApplyXForm2Cov(xformSet->xforms[numXf],&mp->cov,mp->ckind);
            /* FixFullGConst(mp,-CovDet(mp->cov.inv)); */
            mp->gConst -= xformSet->xforms[numXf]->det;
            ApplyCMLLRXForm2Vector(xformSet->xforms[numXf],mp->mean);
         }
         break;
    default:
      /* nothing is done */
      break;
    } /* No other options currently supported */
  } 
}

/*------------------------------------------------------------------------*/
/*                 Transform Initialisation and Estimation                */
/*------------------------------------------------------------------------*/

static LinXForm *CreateLinXForm(MemHeap *x,int vsize,IntVec blockSize,Boolean useBias)
{
   LinXForm *xf;
   int b,bsize,size;

   xf = (LinXForm *)New(x,sizeof(LinXForm));
   xf->det = 0;
   xf->nUse = 0;
   xf->vecSize = vsize;
   xf->blockSize = blockSize;    
   xf->vFloor = NULL;
   xf->xform = (SMatrix *)New(x,(IntVecSize(blockSize)+1)*sizeof(Matrix));
   size = 0;
   for (b=1;b<=IntVecSize(blockSize);b++) {
      bsize = blockSize[b];
      xf->xform[b] = CreateSMatrix(x,bsize,bsize);
      size += bsize;
   }
   if (size != vsize)
      HError(999,"Incompatable xform sizes %d and %d (block)",vsize,size);
   if (useBias) xf->bias = CreateSVector(x,size);
   else xf->bias = NULL;
   return xf;
}

/* EstMLLRDiagCovXForm: estimate MLLR diagonal variance transform */
static void EstMLLRDiagCovXForm(AccStruct *accs, LinXForm *xf, LinXForm *dxf)
{
   int b, cnti,dim, bsize;
   int i,j,k;  
   double tmu, tvar;
   DVector tvec;
   Matrix A;
   DMatrix G;
  
   for (b=1,cnti=1;b<=IntVecSize(accs->blockSize);b++) {
      if ((enableBlockAdapt == NULL) || (enableBlockAdapt[b] == 1)) {
         bsize = accs->blockSize[b];
         if (xf->bias == NULL) dim = bsize;
         else dim = bsize+1;
         A = xf->xform[b]; 
         tvec = CreateDVector(&gstack,dim);
         G = CreateDMatrix(&gstack,dim,dim);
         for (i=1;i<=bsize;i++,cnti++) {
            tmu = 0; tvar = 0;
            ZeroDVector(tvec);
            DTri2DMat(accs->G[cnti],G);
            for (j=1;j<=bsize;j++) {
               tmu += A[i][j] * accs->K[cnti][j];
               for (k=1;k<=bsize;k++)
                  tvec[j] += G[j][k] * A[i][k];
               if (xf->bias != NULL) 
                  tvec[j] += G[j][dim] * xf->bias[cnti];
            }
            if (xf->bias != NULL) {
               for (k=1;k<=bsize;k++)
                  tvec[dim] += G[dim][k] * A[i][k];
               tvec[dim] += xf->bias[cnti] * G[dim][dim];
            }
            for (j=1;j<=bsize;j++)
               tvar += A[i][j] * tvec[j];
            if (xf->bias != NULL) {
               tmu += xf->bias[cnti] * accs->K[cnti][dim];
               tvar += xf->bias[cnti] * tvec[dim];
            }
            dxf->xform[cnti][1][1] =  (accs->D[cnti] - 2*tmu + tvar)/accs->occ;
         }
         FreeDVector(&gstack,tvec);
      } 
      else {
         bsize = accs->blockSize[b];         
         for (i=1;i<=bsize;i++,cnti++) {
            dxf->xform[cnti][1][1] = 1.0;
         }
      }
   }
}

/* InvertG: invert matrix G (band or full structure) */
static void InvertG (DMatrix G, DMatrix invG, const int c, const int bsize, const int bandw, const Boolean uBias)
{
   DMatrix inv, u, v;
   DVector w;
   int i,j,cnti,cntj,st,en,dim,size;

   if (bandw+1<bsize) {
      /* matrix size */
      for (st=c-bandw; st<1; st++);
      for (en=c+bandw; en>bsize; en--);
      dim  = (uBias) ? bsize+1 : bsize;
      size = (uBias) ? en-st+2 : en-st+1; 

      /* matrices for inversion */
      w   = CreateDVector(&gstack,size);
      u   = CreateDMatrix(&gstack,size,size);
      v   = CreateDMatrix(&gstack,size,size);
      inv = CreateDMatrix(&gstack,size,size);

      /* shrink G */
      ZeroDMatrix(inv);
      for (i=cnti=1; i<=dim; i++) {
         if ((st<=i && i<=en) || (uBias && i==dim)) {
            for (j=cntj=1; j<=dim; j++) {
               if ((st<=j && j<=en) || (uBias && j==dim)) {
                  inv[cnti][cntj] = G[i][j];
                  cntj++;
               }
            }
            cnti++;
         }
      }

      /* inversion */
      InvSVD(inv,u,w,v,inv);

      /* store inv matrix to invG */
      ZeroDMatrix(invG);
      for (i=cnti=1; i<=dim; i++) {
         if ((st<=i && i<=en) || (uBias && i==dim)) {
            for (j=cntj=1; j<=dim; j++) {
               if ((st<=j && j<=en) || (uBias && j==dim)) {
                  invG[i][j] = inv[cnti][cntj];
                  cntj++;
               }
            }
            cnti++;
         }
      }
   }
   else {
      /* matrices for inversion */
      dim  = (uBias) ? bsize+1 : bsize;
      w = CreateDVector(&gstack,dim);
      u = CreateDMatrix(&gstack,dim,dim);
      v = CreateDMatrix(&gstack,dim,dim);
      InvSVD(G,u,w,v,invG);
   }

   FreeDVector(&gstack,w);

   return;
}

/* EstMLLRMeanXForm: estimate MLLR mean transform */
static void EstMLLRMeanXForm(AccStruct *accs, LinXForm *xf)
{
   DMatrix invG;
   SMatrix A;
   SVector bias;
   int i,j,k,dim;
   int cnti,b,bsize;
   Boolean uBias;

   bias = xf->bias; 
   if (bias==NULL) uBias = FALSE;
   else uBias = TRUE;
   for (b=1,cnti=1;b<=IntVecSize(accs->blockSize);b++) {
      if ((enableBlockAdapt == NULL) || (enableBlockAdapt[b] == 1)) {
         bsize = accs->blockSize[b];
         if (uBias) dim = bsize+1;
         else dim = bsize;
         /* set up the matrices for the inversion and the transforms to be estimated */
         invG = CreateDMatrix(&gstack,dim,dim);
         A = xf->xform[b]; 
         ZeroMatrix(A); 
         for (i=1;i<=bsize;i++,cnti++) {
            Tri2DMat(accs->G[cnti],invG);
            InvertG(invG, invG, i, bsize, accs->bandWidth[b], uBias);
            for (j=1; j<=bsize; j++) {
               for (k=1;k<=dim;k++)
                  A[i][j] += invG[j][k] * accs->K[cnti][k];
            }
            if (uBias) {
               bias[cnti]=0;
               for (k=1;k<=dim;k++)
                  bias[cnti] += invG[dim][k] * accs->K[cnti][k];
            }
         }
         FreeDMatrix(&gstack,invG);
      } 
      else {
         bsize = accs->blockSize[b];         
         A = xf->xform[b]; 
         ZeroMatrix(A); 
         for (i=1;i<=bsize;i++,cnti++) {
            A[i][i] = 1.0;
            if (uBias) bias[cnti] = 0.0;
         }
      }
   }
}

static double GetAlphaLike(double a, double b, double c, double alpha)
{
  return (-c*log(fabs(alpha*a+b))-(alpha*alpha*a)/2);
}

static double GetAlpha(DMatrix invgmat,DVector kmat,double occ, DVector cofact)
{
  int bsize, dim, i ,j;
  DVector tvec;
  double a, b, c, tmp;
  double alpha1, alpha2, like1, like2;
 
  bsize= DVectorSize(cofact); 
  dim = DVectorSize(kmat);
  tvec = CreateDVector(&gstack,dim);
  ZeroDVector(tvec);
  for (i=1;i<=dim;i++)
    for (j=1;j<=bsize;j++)
      tvec[i] += cofact[j]*invgmat[i][j];
  /* Now set up the quadratic equation */
   a=0.0;b=0.0;c=-occ;
  for (i=1;i<=bsize;i++) {
    a += tvec[i]*cofact[i];
    b += tvec[i] * kmat[i];
  }
  if(bsize != dim)  b += tvec[dim] * kmat[dim];
  /* Must by definition be real */
   tmp = (b*b-4.0*a*c);
   if (tmp<0.0) {
    HError(-1,"WARNING: accumulates incorrect (%f < 0) - resetting",tmp);
      tmp=0.0;
  }
  
  tmp = sqrt(tmp);
  /* Now get the possible values of alpha */
   alpha1 = (-b+tmp)/(2.0*a);
   alpha2 = (-b-tmp)/(2.0*a);
  like1 = GetAlphaLike(a,b,c,alpha1);
  like2 = GetAlphaLike(a,b,c,alpha2);
 
  if (like2>like1)
    return alpha2;
  else
    return alpha1;
}

static double GetRowLike(DMatrix gmat,DVector kmat, DVector cofact, double occ, DVector w)
{
  double rowLike, det;
  int i, j, size,size2;
  DVector tvec;
  DMatrix tmat;
 
  size = DVectorSize(w);
  size2 = DVectorSize(cofact);
  tvec = CreateDVector(&gstack,size);
  tmat = CreateDMatrix(&gstack,size,size);
  Tri2DMat(gmat,tmat);
  ZeroDVector(tvec);
  for (i=1;i<=size;i++)
    for (j=1;j<=size;j++)
      tvec[i] += w[j]*tmat[i][j];
  rowLike = 0;
  for (i=1;i<=size;i++)
    rowLike += (tvec[i] - 2*kmat[i])*w[i];
  det=0;
  for (i=1;i<=size2;i++)
    det += cofact[i]*w[i];
  rowLike = log(fabs(det))*occ - rowLike/2;
  FreeDVector(&gstack,tvec);
  return rowLike;
}

static double GetSemiTiedLike(AccStruct *accs, LinXForm *xf)
{
   double loglike,tmp;
   int bsize,b,k,kk,cnt,nblocks,i;
   DMatrix G;
   Matrix A;
   Vector w;

   /* this is the complete dimensionality of the xform */
   loglike = 0;
   if (staticSemiTied) nblocks=1;
   else nblocks = IntVecSize(accs->blockSize);
   if (xf == NULL) {
      /* implies an identity XForm */
      for (b=1,cnt=1;b<=nblocks;b++) {
         bsize = accs->blockSize[b];
         for (k=1;k<=bsize;k++,cnt++)
            loglike += accs->G[cnt][k][k];
      }      
      loglike = -loglike/2;
   } 
   else {
      for (b=1,cnt=1;b<=nblocks;b++) {
         bsize = accs->blockSize[b];
         A = xf->xform[b];
         for (i=1;i<=bsize;i++,cnt++) {
            w = A[i]; 
            G = accs->G[cnt];
            for (k=1; k<= bsize; k++) {
               tmp = 0;
               for (kk=1; kk<=bsize; kk++)
                  tmp += w[kk] * G [k][kk];
               loglike += w [k] * tmp;
            }
         }
      }
      if (staticSemiTied) 
         loglike = (xf->det*accs->occ/IntVecSize(accs->blockSize) - loglike)/2;
      else 
         loglike = (xf->det*accs->occ - loglike)/2;
   }
   return loglike;
}

static void InitCMLLRXForm(AccStruct *accs, DVector W, DVector bias)
{
  DMatrix invG,u,v,lG;
  int i,k,dim,ldim;
  int cnt, cnti,b,bsize;
  Boolean uBias;
  double alpha, likeNew, likeOld;
  DVector tvec,tW,w,iW,lK;
  DVector cofact;
  
  if (bias==NULL) uBias = FALSE;
  else uBias = TRUE;
  cofact = CreateDVector(&gstack,1);
  if (uBias) ldim = 2;
  else ldim = 1;
  /* set up the matrices for the inversion */
  lG = CreateDMatrix(&gstack,ldim,ldim);
  invG = CreateDMatrix(&gstack,ldim,ldim);
  u = CreateDMatrix(&gstack, ldim, ldim);
  v = CreateDMatrix(&gstack, ldim, ldim);
  w = CreateDVector(&gstack, ldim);
  tW = CreateDVector(&gstack, ldim);
  tvec = CreateDVector(&gstack, ldim);
  lK = CreateDVector(&gstack, ldim);
  iW = CreateDVector(&gstack, ldim);
  /* identity xform for log-likelihood check */
  iW[1]=1; iW[2]=0;
  for (b=1,cnt=1;b<=IntVecSize(accs->blockSize);b++) {
    bsize = accs->blockSize[b];
    if (uBias) dim = bsize+1;
    else dim = bsize;
    /* and the transforms to be estimated */
    for (i=1,cnti=cnt;i<=bsize;i++,cnti++) {
      /* Copy appropriate elements from the accumlates */
      if (uBias) {
	lG[1][1] = accs->G[cnti][i][i]; lG[1][2] = accs->G[cnti][dim][i];
	lG[2][1] = lG[1][2]; lG[2][2] = accs->G[cnti][dim][dim];
	lK[1] = accs->K[cnti][i]; lK[2] = accs->K[cnti][dim];
         } 
         else {
	lG[1][1] = accs->G[cnti][i][i];
	lK[1] = accs->K[cnti][i];
      }
      /* For diag case the cofactors are independent */
      cofact[1]=1;
      InvSVD(lG, u, w, v, invG);
      alpha = GetAlpha(invG,lK,accs->occ,cofact);
      tvec[1] = alpha * cofact[1] + lK[1];
      if (uBias) tvec[2] = lK[2];
      ZeroDVector(tW);
      for (k=1;k<=ldim;k++)
	tW[1] += invG[1][k] * tvec[k];
      if (uBias) {
	tW[ldim]=0;
	for (k=1;k<=ldim;k++)
	  tW[ldim] += invG[ldim][k] * tvec[k];
      }
      likeNew = GetRowLike(lG,lK,cofact,accs->occ,tW);
      /* compare to identity transform */
      likeOld = GetRowLike(lG,lK,cofact,accs->occ,iW);
      if (likeNew<likeOld) {
	if (likeOld/likeNew>1.00001) /* put a threshold on this! */
	  printf(" Issue in intialising row %d of block %d (%f->%f)\n",
		 i,b,likeNew/accs->occ,likeOld/accs->occ);
	W[cnti] = iW[1];
	if (uBias) bias[cnti] = iW[2];
            } 
            else {
	W[cnti] = tW[1];
	if (uBias) bias[cnti] = tW[2];
      }
    }     
    cnt += bsize;
  }
  FreeDVector(&gstack,cofact);
}

static void EstCMLLRXForm(AccStruct *accs, LinXForm *xf)
{
   DMatrix *InvG,invG;
  DMatrix A;
  DVector bias;
  int i,j,k,dim;
  int iter;
  int cnt, cnti,b,bsize;
  Boolean uBias;
  double alpha, likeNew, likeOld;
  double det=0.0,tdet;
  DVector W, iniW, tvec, iniA;
  DVector cofact;
  
  iniA = CreateDVector(&gstack, xf->vecSize);
  if (xf->bias == NULL) {
     uBias = FALSE;
     bias = NULL;
   }
   else {
     uBias = TRUE;
     bias = CreateDVector(&gstack,xf->vecSize);
  } 

  InitCMLLRXForm(accs, iniA , bias);
  InvG = (DMatrix *)New(&gstack,sizeof(DMatrix)*(accs->dim+1)); 
  tdet = 0;
  
  for (b=1,cnt=1;b<=IntVecSize(accs->blockSize);b++) {
    bsize = accs->blockSize[b];
    cofact = CreateDVector(&gstack,bsize);
    if (uBias) dim = bsize+1;
    else dim = bsize;
    /* and the transforms to be estimated */
    A = CreateDMatrix(&gstack, bsize,bsize);
    W = CreateDVector(&gstack,dim);
    iniW = CreateDVector(&gstack,dim);
    tvec = CreateDVector(&gstack,dim);
    ZeroDMatrix(A); 
    for (i=1,cnti=cnt;i<=bsize;i++,cnti++) {
      A[i][i] = iniA[cnti];   
      InvG[cnti] = CreateDMatrix(&gstack,dim,dim);
         DTri2DMat(accs->G[cnti],InvG[cnti]);
         InvertG(InvG[cnti], InvG[cnti], i, bsize, accs->bandWidth[b], uBias);
    }
    for (iter=1;iter<=maxXFormIter;iter++) {
      ZeroDVector(iniW);
      for (i=1,cnti=cnt;i<=bsize;i++,cnti++) {
        for (j=1;j<=bsize;j++)      iniW[j] = A[i][j];
        if (uBias)  iniW[dim] = bias[cnti];
        det = DMatCofact(A,i,cofact);        
	invG = InvG[cnti];    
        alpha = GetAlpha(invG,accs->K[cnti],accs->occ,cofact);
        ZeroDVector(W);
        for (j=1;j<=bsize;j++)
          tvec[j] = alpha * cofact[j] + accs->K[cnti][j];
        if (uBias)  tvec[dim] = accs->K[cnti][dim];
        for (j=1;j<=bsize;j++)
          for (k=1;k<=dim;k++)
            W[j] += invG[j][k] * tvec[k];
        if (uBias) {
          W[dim]=0;
          for (k=1;k<=dim;k++)
            W[dim] += invG[dim][k] * tvec[k];
        }      
        likeNew = GetRowLike(accs->G[cnti],accs->K[cnti],cofact,accs->occ,W);
        likeOld = GetRowLike(accs->G[cnti],accs->K[cnti],cofact,accs->occ,iniW);
            if (trace&T_XFM)
               printf("Iteration %d (row %d): Old=%e, New=%e (diff=%e)\n",iter,cnti,likeOld,likeNew,likeNew-likeOld);
        if (likeNew>likeOld) {
           det = 0; 
           for (j=1;j<=bsize;j++) {
              A[i][j] = W[j];
              det += cofact[j]*W[j];
           }
           if (uBias) {
              bias[cnti] = 0;
              bias[cnti] += W[dim];
           }
            } 
            else {
               if (trace&T_XFM && (likeOld/likeNew>1.00001)) /* put a threshold on this! */
	      printf("  Not updating transform (Block: %d Row: %d Iter: %d (%f %f))\n",
		     b,i,iter,likeNew/accs->occ,likeOld/accs->occ);
	 }
      }
    }
    cnt += bsize;
    tdet += log(fabs(det));
    /* Copy the transform into single precision for storage */
    for (i=1;i<=bsize;i++)
       for (j=1;j<=bsize;j++)
          xf->xform[b][i][j] = A[i][j];
    FreeDVector(&gstack,cofact);
  }
  /* Copy the bias transform and determinant (stored single precision) */
  if (uBias) {
     for (i=1;i<=xf->vecSize;i++) xf->bias[i] = bias[i];
  }
  xf->det = tdet*2;
  FreeDVector(&gstack, iniA);
}

static void AccMixPDFSemiTiedStats(HMMSet *hset,MixPDF *mp, AccStruct *accs)
{
   VaAcc *va;
   int b,k,kk,bsize,s,strm;
   int i,cnti,bstart;
   DMatrix G;
   float ivar;
   STriMat inv;

   va = (VaAcc *) GetHook(mp->cov.var);
   if (va->occ>0.0) {
      strm = mp->stream;
      inv = va->cov.inv;
      if (strmProj) {
         /* 
            Only accumulate observations for the first stream.
            Otherwise the occupancy will be too large - the 
            sum for each stream of the state must be the same
         */
         if (strm == 1) accs->occ += va->occ;
         /* First extract the correct accumulate to add */
         for (s=1,cnti=1;s<strm;s++) cnti += hset->swidth[s];
         for (i=1;i<=hset->swidth[strm];i++,cnti++) {
            if (mp->ckind == INVDIAGC) ivar=mp->cov.var[i];
            else ivar = 1.0/mp->cov.var[i];
            G = accs->G[cnti]; 
            for (k=1;k<=hset->vecSize;k++) 
               for (kk=1;kk<=k;kk++) 
                  G[k][kk] += ivar*inv[k][kk];
         }
      } 
      else {
         /* the G accumulates may have been tied ... */
         if (staticSemiTied) accs->occ += va->occ * IntVecSize(accs->blockSize);
         else accs->occ += va->occ;
         for (b=1,cnti=1,bstart=0;b<=IntVecSize(accs->blockSize);b++) {
            bsize = accs->blockSize[b]; 
            /* 
               numNuisanceDim can ONLY be non-zero for single block
               systems.
            */
            for (i=1;i<=bsize-numNuisanceDim;i++,cnti++) {
               if (mp->ckind == INVDIAGC) ivar=mp->cov.var[cnti];
               else ivar = 1.0/mp->cov.var[cnti];
               G = accs->G[cnti]; 
               for (k=1;k<=bsize;k++) 
                  for (kk=1;kk<=k;kk++) 
                     G[k][kk] += ivar*inv[bstart+k][bstart+kk];
            }
            bstart += bsize;
         }
      }
   }
}

/* 
   returns -1 if the stream associated with the transform is not consistent
   and the list of all streams used. Wasteful ...
*/
static int GetXFormStream(AdaptXForm *xform, IntVec classes, IntVec streams)
{
   BaseClass *bclass;
   int b,stream=0;
   ILink il;
   MixPDF *mp = NULL;

   bclass = xform->bclass;
   ZeroIntVec(streams);
   for (b=1;b<=bclass->numClasses;b++) {
      if (classes[b] == 1) {
         for (il=bclass->ilist[b]; il!=NULL; il=il->next) {
            mp = ((MixtureElem *)il->item)->mpdf;
            streams[mp->stream] = 1;
            if (stream == 0) stream = mp->stream;
            else if (stream != mp->stream) stream = -1;
         }
      }
   }
   return stream;
}

/* extract the dimensions with the greatest between to within ratio (same as greatest total 
   to within given occupancy weighted counts. modify the transform so that this change is reflected
   in ixf
*/
static void InitSemiTiedFR(AdaptXForm *xform, LinXForm *xf, IntVec classes, TriMat totCov)
{
   BaseClass *bclass;
   DVector withinCov, fisherRatio;
   double occ;
   int size,i,j,k,b,max;
   double vmax;
   Matrix A,inv;
   ILink il;
   MixPDF *mp = NULL;
   VaAcc *va;

   size = xf->vecSize;
   withinCov = CreateDVector(&gstack,size);
   fisherRatio = CreateDVector(&gstack,size);
   ZeroDVector(withinCov); occ = 0;
   bclass = xform->bclass;
   for (b=1;b<=bclass->numClasses;b++) {
      if (classes[b] == 1) {
         for (il=bclass->ilist[b]; il!=NULL; il=il->next) {
            mp = ((MixtureElem *)il->item)->mpdf;
            va = (VaAcc *)GetHook(mp->cov.var);
            if (va->occ > 0.0) {
               occ += va->occ;
               inv = va->cov.inv;
               for (i=1;i<=size;i++)
                  withinCov[i] += inv[i][i];
            }
         }
      }
   }
   /* map withinCov to hold the ratio */
   for (i=1;i<=size;i++) fisherRatio[i] = totCov[i][i]*(occ/withinCov[i]) -1;
   /* perform simple select process - only single block for HLDA */
   A = xf->xform[1];
   ZeroMatrix(A);
   /* naive search, swap initialise as bottom dimensions projected out */
   if (trace & T_FRS) ShowDVector("Initial",fisherRatio,size);
   for (i=1;i<=size;i++) {
      max = 0; vmax = -1;
      for (j=1;j<=size;j++) {
         if (fisherRatio[j]>vmax) {
            max=j;
            vmax=fisherRatio[j];
         }
      }
      /* can either order by ranking .. or ident */
      if (!initNuisanceFRIdent) A[i][max] = 1.0;
      fisherRatio[max] = (double) -i;
   }
   for (i=1;i<=size;i++) fisherRatio[i] = -fisherRatio[i];
   if (initNuisanceFRIdent) {
      for (i=1,j=1,k=size-numNuisanceDim+1;i<=size;i++) {
         if (fisherRatio[i] <= size-numNuisanceDim) {
            A[j][i] = 1;
            j++;
         } else {
            A[k][i] = 1;
            k++;
         }
      }
   }
   if (trace & T_FRS) {
      ShowDVector("Selection Order",fisherRatio,size);
   }
   xf->det=0;
   FreeDVector(&gstack,withinCov);
}


static void UpdateSemiTiedAccs(AdaptXForm *xform, AccStruct *accs, IntVec classes, TriMat totCov)
{
   BaseClass *bclass;
   int b,i,k,kk,bsize,vsize;
   int cnti;
   ILink il;
   MixPDF *mp = NULL;
   DMatrix G;
   float ivar;

   /* zero the accumulates */
   accs->occ = 0;
   if (strmProj) vsize = xform->hset->vecSize;
   else vsize = accs->dim;
   for (i=1;i<=vsize;i++) ZeroDMatrix(accs->G[i]);
   bclass = xform->bclass;
   for (b=1;b<=bclass->numClasses;b++) {
      if (classes[b] == 1) {
         for (il=bclass->ilist[b]; il!=NULL; il=il->next) {
            mp = ((MixtureElem *)il->item)->mpdf;
            AccMixPDFSemiTiedStats(xform->hset,mp,accs);
         }
      }
   }
   /* make the accumulates symmetric (saves thinking later) */
   if (strmProj) {
      for (i=1;i<=vsize;i++) {
         G = accs->G[i]; 
         for (k=1;k<=vsize;k++) 
            for (kk=1;kk<k;kk++) 
               G[kk][k] = G[k][kk];
      }
   } else {
      /* handle the nuisance dimension attributes */
      for (i=vsize-numNuisanceDim+1;i<=vsize;i++) {
         /* mp is the last component of this baseclass */
         if (mp->ckind == INVDIAGC) ivar=mp->cov.var[i];
         else ivar = 1.0/mp->cov.var[i];
         /* scale the inverse to reflect counts */
         ivar *= accs->occ;
         G = accs->G[i];
         for (k=1;k<=vsize;k++) 
            for (kk=1;kk<=k;kk++) 
               G[k][kk] += ivar*totCov[k][kk];
      }
      for (b=1,cnti=1;b<=IntVecSize(accs->blockSize);b++) {
         bsize = accs->blockSize[b]; 
         for (i=1;i<=bsize;i++,cnti++) {
            G = accs->G[cnti]; 
            for (k=1;k<=bsize;k++) 
               for (kk=1;kk<k;kk++) 
                  G[kk][k] = G[k][kk];
         }
      }
   }
}

static void InitSemiTiedVars(AdaptXForm *xform, IntVec classes, TriMat totCov)
{
   int b,j,size;
   ILink i;
   MixPDF *mp = NULL;
   Vector tcov;
   HMMSet *hset;
   BaseClass *bclass;

   hset = xform->hset;
   bclass = xform->bclass;
   size = hset->vecSize; 
   tcov = CreateVector(&gstack,size);
   for (j=1;j<=size;j++)
      tcov[j] = totCov[j][j];
   for (b=1;b<=bclass->numClasses;b++) {
      if (classes[b] == 1) {
         for (i=bclass->ilist[b]; i!=NULL; i=i->next) {
            mp = ((MixtureElem *)i->item)->mpdf;
            for (j=size-numNuisanceDim+1;j<=size;j++) {
               /* no flooring necessary ... */
               mp->cov.var[j] = tcov[j];
            }
            if (mp->ckind == INVDIAGC) 
               for (j=size-numNuisanceDim+1;j<=size;j++)
                  mp->cov.var[j] = 1.0/mp->cov.var[j];
         }
      }
   }
   FreeVector(&gstack,tcov);
}

/*
  Updates the variance and returns the occupancy weighted summed log determinant
  for use in the likelihood calculation.
*/
static double UpdateSemiTiedVars(HMMSet *hset, LinXForm *xf, BaseClass *bclass, IntVec classes, Vector vFloor, TriMat totCov)
{
   int b,j,size,strm,s,cnt;
   int nFloor;
   ILink i;
   MixPDF *mp = NULL;
   Matrix mat;
   VaAcc *va;
   STriMat inv;
   Vector vec,tcov=NULL;
   double logdet;

   size = xf->vecSize; nFloor = 0; logdet=0;
   mat = CreateMatrix(&gstack,size,size);
   if (strmProj) vec = CreateVector(&gstack,size);
   else vec = NULL;
   if (numNuisanceDim > 0) {
      tcov = CreateVector(&gstack,size);
      ApplyXForm2TriMat(xf,totCov,mat);
      DiagApplyMat2TXForm(xf,mat,tcov);
   }
   for (b=1;b<=bclass->numClasses;b++) {
      if (classes[b] == 1) {
         for (i=bclass->ilist[b]; i!=NULL; i=i->next) {
            mp = ((MixtureElem *)i->item)->mpdf;
            va = (VaAcc *) GetHook(mp->cov.var);
            if (va->occ>0) {
               inv = va->cov.inv;
               ApplyXForm2TriMat(xf,inv,mat);
               if (strmProj) {
                  strm = mp->stream;
                  /* This is very wasteful - possibly fix later */
                  DiagApplyMat2TXForm(xf,mat,vec);
                  for (s=1,cnt=1;s<strm;s++)  cnt += hset->swidth[s];
                  for (j=1;j<=hset->swidth[strm];j++,cnt++) {
                     mp->cov.var[j] = vec[cnt]/va->occ;
                     if (mp->cov.var[j]<vFloor[cnt])
                        mp->cov.var[j] = vFloor[cnt];
                  }
                  if (mp->ckind == INVDIAGC) 
                     for (j=1;j<=hset->swidth[strm];j++)
                        mp->cov.var[j] = 1.0/mp->cov.var[j];
                  if (mp->ckind == INVDIAGC) {
                     for (j=1;j<=hset->swidth[strm];j++)
                        logdet += va->occ * log(mp->cov.var[j]);
                  } else {
                     for (j=1;j<=hset->swidth[strm];j++)
                        logdet -= va->occ * log(mp->cov.var[j]);
               }
               } else { 
                  DiagApplyMat2TXForm(xf,mat,mp->cov.var);
                  /* apply variance floor and normalise */
                  for (j=1;j<=size-numNuisanceDim;j++) {
                     mp->cov.var[j] /= va->occ;
                     if (mp->cov.var[j]<vFloor[j]) {
                        mp->cov.var[j] = vFloor[j];
                        nFloor++;
                     }
                  }
                  if (numNuisanceDim > 0) {
                     for (j=size-numNuisanceDim+1;j<=size;j++) {
                        /* no flooring necessary ... */
                        mp->cov.var[j] = tcov[j];
                     }
                  }
                  if (mp->ckind == INVDIAGC) 
                     for (j=1;j<=size;j++)
                        mp->cov.var[j] = 1.0/mp->cov.var[j];
                  /* get the information for the likelihood calculation */
                  if (mp->ckind == INVDIAGC) {
                     for (j=1;j<=size;j++)
                        logdet += va->occ * log(mp->cov.var[j]);
                  } else {
                     for (j=1;j<=size;j++)
                        logdet -= va->occ * log(mp->cov.var[j]);
                        }

               }
            }
         }
      }
   }
   if (nFloor>0)
      printf("  Floored %d component elements\n",nFloor);
   FreeMatrix(&gstack,mat);
   return logdet/2;
}

static double InitSemiTiedDet(BaseClass *bclass, IntVec classes)
{
   int b,j,size;
   ILink i;
   MixPDF *mp = NULL;
   VaAcc *va;
   double logdet;

   logdet = 0;
   for (b=1;b<=bclass->numClasses;b++) {
      if (classes[b] == 1) {
         for (i=bclass->ilist[b]; i!=NULL; i=i->next) {
            mp = ((MixtureElem *)i->item)->mpdf;
            va = (VaAcc *)GetHook(mp->cov.var);
            if (va->occ > 0) {
               size = VectorSize(mp->mean);
               if (mp->ckind == INVDIAGC) {
                  for (j=1;j<=size;j++)
                     logdet += va->occ * log(mp->cov.var[j]);
               } else {
                  for (j=1;j<=size;j++)
                     logdet -= va->occ * log(mp->cov.var[j]);
               }
            }
         }
      }
   }
   return logdet/2;
}

static void UpdateSemiTiedMeans(HMMSet *hset, LinXForm *xf, BaseClass *bclass, IntVec classes, Vector totMean)
{
   int b,j,size,vsize,s,strm,cnt;
   ILink i;
   MixPDF *mp = NULL;
   Vector vec=NULL,tvec;
   MuAcc *ma;

   size = xf->vecSize; 
   if (numNuisanceDim > 0) {
      tvec = CreateVector(&gstack,size);
      CopyVector(totMean,tvec);
      ApplyXForm2Vector(xf,tvec);
   } 
   else {
      tvec = NULL;
   }
   
   for (b=1;b<=bclass->numClasses;b++) {
      if (classes[b] == 1) {
         for (i=bclass->ilist[b]; i!=NULL; i=i->next) {
            mp = ((MixtureElem *)i->item)->mpdf;
            ma = (MuAcc *) GetHook(mp->mean);
            if (ma->occ>0.0) {
               vsize = VectorSize(mp->mean);
               vec = CreateVector(&gstack,vsize);
               CopyVector(ma->mu,vec);
               ApplyXForm2Vector(xf,vec);
               if (strmProj) {
                  strm = mp->stream;
                  for (s=1,cnt=1;s<strm;s++)  cnt += hset->swidth[s];
                  for (j=1;j<=hset->swidth[strm];j++,cnt++) 
                     mp->mean[j] = vec[cnt]/ma->occ;
               } 
               else {
                  for (j=1;j<=vsize-numNuisanceDim;j++)
                     mp->mean[j] = vec[j]/ma->occ;
               }
               /* update the global "tied" means */
               if (numNuisanceDim > 0) {
                  for (j=size-numNuisanceDim+1;j<=size;j++)
                     mp->mean[j] = tvec[j];
               }
               FreeVector(&gstack,vec);
            }
         }
      }
   }
}

static void GetSemiTiedTotCov(AdaptXForm *xform, IntVec classes, TriMat totCov, Vector totMean)
{
   int b,k,kk;
   int size,cnt;
   ILink i;
   MixPDF *mp=NULL;
   DMatrix mat=NULL, tmat=NULL;
   DVector tvec=NULL;
   VaAcc *va;
   MuAcc *ma;
   double totOcc;
   STriMat inv;
   Vector mu;
   BaseClass *bclass;
   float tmp;

   cnt = 0;
   bclass = xform->bclass;
   size = 0; mat = NULL; totOcc = 0;
   for (b=1;b<=bclass->numClasses;b++) {
      if (classes[b] == 1) {
         for (i=bclass->ilist[b]; i!=NULL; i=i->next) {
            cnt++;
            mp = ((MixtureElem *)i->item)->mpdf;
            if (mat == NULL) { /* need to get size for this transform */
               if (strmProj) size = xform->hset->vecSize; 
               else size = VectorSize(mp->mean);
               mat = CreateDMatrix(&gstack,size,size);
               tmat = CreateDMatrix(&gstack,size,size);
               tvec = CreateDVector(&gstack,size);
               totOcc = 0; ZeroDMatrix(mat);
               ZeroDMatrix(tmat); ZeroDVector(tvec);
            }
            va = (VaAcc *) GetHook(mp->cov.var);
            totOcc += va->occ;
            inv = va->cov.inv;
            if (va->occ>0) {
               ma = (MuAcc *) GetHook(mp->mean);
               mu = ma->mu;
               for (k=1;k<=size;k++) {
                  tmp = mu[k];
                  tvec[k] += tmp;
                  for (kk=1;kk<=k;kk++) 
                     tmat[k][kk] += inv[k][kk] + tmp*mu[kk]/va->occ;
               }
            }
         }
      }
   }
   for (k=1;k<=size;k++) {
      totMean[k] = tvec[k]/totOcc;
      for (kk=1;kk<=k;kk++) 
         totCov[k][kk] = tmat[k][kk]/totOcc - totMean[k]*totMean[kk];   
   }
   FreeDMatrix(&gstack,mat);
}

static void UpdateSemiTiedVFloor (HMMSet *hset, LinXForm *xf, TriMat avCov, SVector vFloor, const int s)
{
   Matrix mat;
   int size,i;

   if (semiTiedVFloor) {
      size = xf->vecSize;
      mat = CreateMatrix(&gstack,size,size);
      ApplyXForm2TriMat(xf,avCov,mat);
      DiagApplyMat2TXForm(xf,mat,vFloor);
      for (i=1;i<=size;i++) vFloor[i] *= semiTiedFloorScale[s];
   } 
   else
      ZeroVector(vFloor);
}

void UpdateHLDAModel(HMMSet *hset)
{
   HMMScanState hss;
   MixPDF *mp;
   SVector mu;
   SVector var;
   int size, hldasize, i;

   size = hset->vecSize;
   /* support HHEd projection work */
   if (hset->projSize > 0) hldasize = size-hset->projSize;
   else hldasize = size-numNuisanceDim;
   NewHMMScan(hset,&hss);
   do {
      while (GoNextState(&hss,TRUE)) {
         while (GoNextStream(&hss,TRUE)) {            
            if (hss.isCont)                     /* PLAINHS or SHAREDHS */
               while (GoNextMix(&hss,TRUE)) {
                  mp = hss.mp;
                  mu = CreateSVector(hset->hmem,hldasize);
                  var = CreateSVector(hset->hmem,hldasize);
                  for (i=1;i<=hldasize;i++) {
                     mu[i] = mp->mean[i];
                     var[i] = mp->cov.var[i];
                  }
                  /* set the hooks correctly so no by-products */
                  SetHook(mu,GetHook(mp->mean));
                  SetHook(var,GetHook(mp->cov.var));
                  mp->mean = mu;
                  mp->cov.var = var;
               }
         }
      }
   } while (GoNextHMM(&hss));
   EndHMMScan(&hss);
   /* now tidy the global attributes */
   hset->vecSize = hldasize;
   hset->swidth[1] = hldasize;
   hset->projSize = 0;
}

static void UpdateSemiTiedXForm(AccStruct *accs, LinXForm *xf, LinXForm *ixf)
{
   DMatrix invG;
   DMatrix *InvG;
   DVector w;
   DMatrix A;
   int i,j,k,dim;
   int iter;
   int cnt, cnti,b,bsize;
   double det=0.0,tdet=0.0;
   double beta, likeNew, likeOld;
   DVector W, iniW;
   DVector cofact;
  
   /* Reset xform  to identity matrix */
  
   InvG = (DMatrix *)New(&gstack,sizeof(DMatrix)*(accs->dim+1)); 
   for (b=1,cnt=1;b<=IntVecSize(accs->blockSize);b++) {
     bsize = accs->blockSize[b];
     dim = bsize;
     cofact = CreateDVector(&gstack,bsize);
     ZeroDVector(cofact); 
      /* set up the transforms to be estimated */
     A = CreateDMatrix(&gstack,bsize,bsize);
     ZeroDMatrix(A); 
     if (ixf == NULL) {
        /* initialise with the diagonal transform */
        for (i=1,cnti=cnt;i<=bsize;i++, cnti++) {
           A[i][i] = sqrt(accs->G[cnti][i][i]/accs->occ); 
            A[i][i] = 1.0/A[i][i];
           InvG[cnti] = CreateDMatrix(&gstack,dim,dim);
           Tri2DMat(accs->G[cnti],InvG[cnti]);
            InvertG(InvG[cnti], InvG[cnti], i, bsize, accs->bandWidth[b], FALSE);
        }
      } 
      else {
        for (i=1,cnti=cnt;i<=bsize;i++, cnti++) {
           InvG[cnti] = CreateDMatrix(&gstack,dim,dim);
           Tri2DMat(accs->G[cnti],InvG[cnti]);
            InvertG(InvG[cnti], InvG[cnti], i, bsize, accs->bandWidth[b], FALSE);
           for (j=1;j<=bsize;j++)
              A[i][j] = ixf->xform[b][i][j];
        }
     }
      w = CreateDVector(&gstack,dim);
     W = CreateDVector(&gstack,dim);
     iniW = CreateDVector(&gstack,dim);
     for (iter=1;iter<=maxXFormIter;iter++) {
       ZeroDVector(iniW);
       for (i=1,cnti=cnt;i<=bsize;i++,cnti++) {
            for (j=1;j<=bsize;j++)
               iniW[j] = A[i][j];
         invG = InvG[cnti];    
         det = DMatCofact(A,i,cofact);     
         beta = 0;
         for(j=1;j<=bsize;j++){
            for(k=1;k<=bsize;k++)
               beta += cofact[j]*invG[j][k]*cofact[k];
         }
         beta = sqrt(accs->occ/beta);
            if (isnan(beta)) {
                HError(-9999,"UpdateSemiTiedXForm: beta becomes NaN, possibly too few adaptation data");
            }
         ZeroDVector(W);
         for(j=1;j<=bsize;j++){
            for(k=1;k<=bsize;k++)
               W[j] += cofact[k]*invG[j][k];
            W[j] *= beta;          
         }
         ZeroDVector(w);
         likeNew = GetRowLike(accs->G[cnti],w,cofact,accs->occ,W);
         likeOld = GetRowLike(accs->G[cnti],w,cofact,accs->occ,iniW);
            if (trace&T_XFM)
               printf("Iteration %d (row %d): Old=%e, New=%e (diff=%e)\n",iter,cnti,likeOld,likeNew,likeNew-likeOld);
         if (likeNew>likeOld) {
               det = 0;
               for(j=1;j<=bsize;j++){
                  A[i][j] = W[j];
                  det += W[j]*cofact[j];
               }
            } 
            else {
               if (trace&T_XFM && (likeOld/likeNew>1.00001)) /* put a threshold on this! */
               printf("  Not updating transform (Block: %d Row: %d Iter: %d (%f %f))\n",
		      b,i,iter,likeNew/accs->occ,likeOld/accs->occ);
	 }
       }
     }
     cnt += bsize;
     tdet += log(fabs(det));
     /* Copy the transform into single precision for storage */
     for (i=1;i<=bsize;i++)
        for (j=1;j<=bsize;j++)
           xf->xform[b][i][j] = A[i][j];
     FreeDVector(&gstack, cofact);
   }
   xf->det = tdet*2;
   Dispose(&gstack,InvG);
}

static void CheckSemiTiedOptions(AdaptXForm *xform, AccStruct *accs)
{
   int s,b,bsize;
   Boolean msd;

   /* MSD flag */
   for (s=1,msd=FALSE; s<=xform->hset->swidth[0]; s++) {
      if (xform->hset->msdflag[s]!=0) {
         msd = TRUE;
         break;
      }
   }

   if (strmProj) { /* confirm that strmProj valid */
      if (xform->bclass->numClasses != 1)
         HError(999,"Can only have strmProj with global transform");
      if (xform->hset->swidth[0] == 1) 
         HError(999,"strmProj with a single stream is not valid");
      if (msd)
         HError(999,"strmProj with MSD HMM is not valid");
   } 
   else if (numNuisanceDim>0) { /* check the nuisance dimension options */
      if (xform->hset->swidth[0] != 1)
         HError(999,"Can only have Nuisance Dimensions with single streams");
      if (msd)
         HError(999,"Can only have Nuisance Dimensions with non-MSD HMMs");
   }

   if (semiTied2InputXForm) {
      if (xform->bclass->numClasses != 1)
         HError(999,"Can only store as input XForm with global transform");
      if (xform->hset->swidth[0] != 1) 
         HError(999,"Can only store as input XForm with single stream");
      if (xform->hset->xf != NULL)
         HError(999,"Can not store as input XForm if HMMSet already has an input XForm");
      if (xform->hset->semiTied != NULL)
         HError(999,"Can not store as input XForm if HMMSet already has an semitied XForm");
      if (IntVecSize(accs->blockSize) > 1)
         HError(999,"Can not store as input XForm if full XForm used");
   }   
   if (numNuisanceDim > 0) {
      if (IntVecSize(accs->blockSize) != 1)
         HError(999,"Can only use HLDA projections with a single block");
   }
   if (staticSemiTied) {
      bsize = accs->blockSize[1];
      for (b=2;b<=IntVecSize(accs->blockSize);b++)
         if (accs->blockSize[b] != bsize)
            HError(999,"Inconsistent block size [1 %d] and [%d %d]",bsize,b,accs->blockSize[b]);
   }
}

static void EstSemiTXForm(AdaptXForm *xform, AccStruct *accs, LinXForm *xf, IntVec classes)
{
   int iter,size=0,stream;
   TriMat totCov;
   Vector totMean;
   SVector vFloor;
   HMMSet *hset;
   IntVec streams;
   double logdet,tocc;
   LinXForm *ixf;

   hset = xform->hset; 
   CheckSemiTiedOptions(xform,accs);
   if (strmProj) { 
      size =  hset->vecSize;
   } 
   else
      size = accs->dim;
   
   ixf = NULL;
   vFloor = CreateSVector(xform->mem,size);
   streams = CreateIntVec(&gstack,hset->swidth[0]);
   totCov = CreateTriMat(&gstack,size);
   totMean = CreateVector(&gstack,size);
   
   if (strmProj) stream = 1;
   else stream = GetXFormStream(xform,classes,streams);
   
   /* handle initialisation for HLDA */
   /* get the global covariance matrix */
   GetSemiTiedTotCov(xform,classes,totCov,totMean);
   if (numNuisanceDim > 0) {
      if (initNuisanceFR) {
         /* compute Fisher ratios to initialise nuisance dimensions */
         InitSemiTiedFR(xform,xf,classes,totCov);
         ixf = xf;
         if (stream>0) { 
            /* 
               if there are multiple baseclases and a semitied transform already being used
               swap to a fraction of the global covariance matrix and warn the user
            */               
            if ((xform->bclass->numClasses>1) && (hset->semiTied != NULL)) {
               printf("WARNINING: using total baseclass variance, not average state variance\n");
               UpdateSemiTiedVFloor(hset,xf,totCov,vFloor,stream);
            } else
               UpdateSemiTiedVFloor(hset,xf,avCov[stream],vFloor,stream);
         }
         else 
            ZeroVector(vFloor);
         logdet = UpdateSemiTiedVars(hset,xf,xform->bclass,classes,vFloor,totCov);         
      } 
      else {
         InitSemiTiedVars(xform,classes,totCov);
         logdet = InitSemiTiedDet(xform->bclass,classes);
      }
   } 
   else
      logdet = InitSemiTiedDet(xform->bclass,classes);
   
   for (iter=1;iter<=maxSemiTiedIter;iter++) {
      UpdateSemiTiedAccs(xform,accs,classes,totCov);
      if (staticSemiTied) tocc = accs->occ/(IntVecSize(accs->blockSize));
      else tocc = accs->occ;
      if (iter == 1) {
         logdet += GetSemiTiedLike(accs,ixf);
         printf("Initial LogLikelihood = %f\n",logdet/tocc);
         UpdateSemiTiedXForm(accs, xf, ixf);
      }
      else 
         UpdateSemiTiedXForm(accs, xf, xf);
         
      printf("Iteration %d\n",iter); fflush(stdout);
      
      if (stream>0) {
         /* 
            if there are multiple baseclases and a semitied transform already being used
            swap to a fraction of the global covariance matrix and warn the user
         */               
         if ((xform->bclass->numClasses>1) && (hset->semiTied != NULL)) {
            printf("WARNINING: using total baseclass variance, not average state variance\n");
            UpdateSemiTiedVFloor(hset,xf,totCov,vFloor,stream);
         } else
            UpdateSemiTiedVFloor(hset,xf,avCov[stream],vFloor,stream);
      } else 
         ZeroVector(vFloor);

      logdet = UpdateSemiTiedVars(hset,xf,xform->bclass,classes,vFloor,totCov);
      logdet += GetSemiTiedLike(accs,xf);
      printf("  LogLikelihood = %f\n",logdet/tocc);
   }
   
   /* Tidy Stats */
   xf->vFloor = vFloor;
   UpdateSemiTiedMeans(hset,xf,xform->bclass,classes,totMean);
   FreeIntVec(&gstack,streams);
}

/* EstMLLRCovXForm: estimate MLLR covariance transform */
static void EstMLLRCovXForm(AccStruct *accs, LinXForm *xf)
{
   DMatrix invG;
  DMatrix *InvG;
  DVector w;
  DMatrix A;
  int i,j,k,dim;
  int iter;
  int cnt, cnti,b,bsize;
  double det=0.0,tdet=0.0;
  double beta, likeNew, likeOld;
  DVector W, iniW;
  DVector cofact;
  
   /* Reset xform  to identity matrix */
  
   InvG = (DMatrix *)New(&gstack,sizeof(DMatrix)*(accs->dim+1)); 
   for (b=1,cnt=1;b<=IntVecSize(accs->blockSize);b++) {
     bsize = accs->blockSize[b];
     dim = bsize;
     cofact = CreateDVector(&gstack,bsize);
     ZeroDVector(cofact); 
      /* set up the transforms to be estimated */
     A = CreateDMatrix(&gstack,bsize,bsize);
     ZeroDMatrix(A); 
      w = CreateDVector(&gstack,dim);
     W = CreateDVector(&gstack,dim);
     iniW = CreateDVector(&gstack,dim);
     /* initialise with the diagonal transform */
     for (i=1,cnti=cnt;i<=bsize;i++, cnti++) {
        A[i][i] = sqrt(accs->G[cnti][i][i]/accs->occ); 
         A[i][i] = 1.0/A[i][i];
        InvG[cnti] = CreateDMatrix(&gstack,dim,dim);
         DTri2DMat(accs->G[cnti],InvG[cnti]);
         InvertG(InvG[cnti], InvG[cnti], i, bsize, accs->bandWidth[b], FALSE);
     }
     for (iter=1;iter<=maxXFormIter;iter++) {
       ZeroDVector(iniW);
       for (i=1,cnti=cnt;i<=bsize;i++,cnti++) {
         for (j=1;j<=bsize;j++)      iniW[j] = A[i][j];
         invG = InvG[cnti];    
         det = DMatCofact(A,i,cofact);     
         beta = 0;
         for(j=1;j<=bsize;j++){
            for(k=1;k<=bsize;k++)
               beta += cofact[j]*invG[j][k]*cofact[k];
         }
         beta = sqrt(accs->occ/beta);
            if (isnan(beta)) {
                HError(-9999,"EstMLLRCovXForm: beta becomes NaN, possibly too few adaptation data");
            }
         ZeroDVector(W);
         for(j=1;j<=bsize;j++){
            for(k=1;k<=bsize;k++)
               W[j] += cofact[k]*invG[j][k];
            W[j] *= beta;          
         }
         ZeroDVector(w);
         likeNew = GetRowLike(accs->G[cnti],w,cofact,accs->occ,W);
         likeOld = GetRowLike(accs->G[cnti],w,cofact,accs->occ,iniW);
            if (trace&T_XFM) 
               printf("Iteration %d (row %d): Old=%e, New=%e (diff=%e)\n",iter,cnti,likeOld,likeNew,likeNew-likeOld);
         if (likeNew>likeOld) {
               det = 0;
               for(j=1;j<=bsize;j++){
                  A[i][j] = W[j];
                  det += W[j]*cofact[j];
               }
            } 
            else {
               if (trace&T_XFM && (likeOld/likeNew>1.00001)) /* put a threshold on this! */
               printf("  Not updating transform (Block: %d Row: %d Iter: %d (%f %f))\n",
		      b,i,iter,likeNew/accs->occ,likeOld/accs->occ);
	 }
       }
     }
     cnt += bsize;
     tdet += log(fabs(det));
     /* Copy the transform into single precision for storage */
     for (i=1;i<=bsize;i++)
        for (j=1;j<=bsize;j++)
           xf->xform[b][i][j] = A[i][j];
     FreeDVector(&gstack, cofact);
   }
   xf->det = tdet*2;
   Dispose(&gstack,InvG);
}

static void EstXForm(AccStruct *accs, XFInfo *xfinfo, IntVec classes)
{
  XFormSet *xformSet;
  LinXForm *xf, *dxf;
  IntVec diagBlockSize;
  int i;
  
   AdaptXForm *xform = xfinfo->outXForm; 
   AdaptXForm *diagCovXForm = xfinfo->diagCovXForm;
  
  xformSet = xform->xformSet;
   (xformSet->numXForms)++;
  xf = xformSet->xforms[xformSet->numXForms] 
      = CreateLinXForm(xform->mem,accs->dim,accs->blockSize,accs->useBias); 

  switch (accs->xkind) {
  case MLLRMEAN:    
    EstMLLRMeanXForm(accs, xf);
    if (mllrDiagCov) { /* additional code to allow efficient diagonal cov */
      diagBlockSize = CreateIntVec(xform->mem,accs->dim);
      for (i=1;i<=accs->dim;i++) diagBlockSize[i] = 1;
      xformSet = diagCovXForm->xformSet;
      xformSet->numXForms++;
      dxf = xformSet->xforms[xformSet->numXForms] 
             = CreateLinXForm(xform->mem,accs->dim,diagBlockSize,FALSE);
      dxf->bias = NULL;
      EstMLLRDiagCovXForm(accs,xf,dxf);
      FixDet(dxf);
    }
    break;
  case MLLRCOV:    
    EstMLLRCovXForm(accs, xf);
    break;
  case CMLLR: 
    EstCMLLRXForm(accs, xf);
    break;
  case SEMIT:
     EstSemiTXForm(xform,accs,xf,classes);
     break;
  default :
    HError(999,"Transform kind not currently supported");
    break;
  }
  if (trace&T_XFM)
    printf("Estimated XForm %d using %f observations\n",xformSet->numXForms,accs->occ);
}

Boolean GenXForm(RegNode *node, XFInfo *xfinfo, IntVec classes, int xfindex)
{
   AccStruct *accs;
   int class=1,b;
   IntVec blockSize,bandWidth;
   
   AdaptXForm *xform = xfinfo->outXForm;
   AdaptXForm *diagCovXForm = xfinfo->diagCovXForm;
  
   if (node->valid && node->vsize>0) {
   /* First get dimension of data associated with this set of transforms */
   if (trace&T_TRE) {
         printf("Using node %d (occ=%f, stream=%d, vsize=%d) to adapt baseclasses: \n",
                node->nodeIndex, node->nodeOcc, node->stream, node->vsize);
      for (b=1;b<=IntVecSize(classes);b++)
         if (classes[b] == 1) printf(" %d",b);
      printf("\n");
   }
   while (classes[class] == 0) class++;
      blockSize = GetBlockSize(xfinfo,class);
      bandWidth = GetBandWidth(xfinfo,class,blockSize);      
   if (strmProj)
         accs = CreateAccStruct(&gstack,xfinfo,xform->hset->vecSize,blockSize,bandWidth);
   else 
         accs = CreateAccStruct(&gstack,xfinfo,node->vsize,blockSize,bandWidth);
   AccNodeStats(node,accs,xform,classes);
      if(xfindex>0)
         AddPrior(accs,xform, xfindex);
      EstXForm(accs,xfinfo,classes);
   for (b=1;b<=IntVecSize(classes);b++)
      if (classes[b] == 1) {
	if (HardAssign(xform)) {
	  xform->xformWgts.assign[b] = xform->xformSet->numXForms;
	  if (mllrDiagCov) 
	    diagCovXForm->xformWgts.assign[b] = diagCovXForm->xformSet->numXForms;
	}
         else 
            HError(999,"Not currently supported");
      }
      
   Dispose(&gstack,accs);
      return TRUE;
   }else{
      return FALSE;
   }
}

static Boolean GenClassXForm(XFInfo *xfinfo)
{
  AccStruct *accs;
   int s,b;
   float thresh[SMAX];
  ILink i;
  MixPDF *mp = NULL;
   IntVec classes,blockSize,bandWidth;
   
   AdaptXForm *xform = xfinfo->outXForm;
   BaseClass *bclass = xform->bclass;
   AdaptXForm *diagCovXForm = xfinfo->diagCovXForm;

  /* reset the number of transforms */
  xform->xformSet->numXForms = 0;
   /* 
      SEMIT is NOT possible in incremental mode and the stats are stored
      in a non-zero-mean form. Fix statistics once to be of appropriate form.
      Prior to fixing stats sort out the covariance matrix for variance
      flooring.
      
   */
  if (xform->xformSet->xkind == SEMIT) {
     SetSemiTiedAvCov(xform->hset);
     FixSemiTiedStats(xform->hset);
  }
  classes = CreateIntVec(&gstack,bclass->numClasses);
  for (b=1;b<=bclass->numClasses;b++) {
      if (GetBaseClassVSize(bclass,b)>0) {
     ZeroIntVec(classes); classes[b] = 1;
    /* Accumulate structure regenerated each time as this will handle
       streams of different sizes simply */
         blockSize = GetBlockSize(xfinfo,b);
         bandWidth = GetBandWidth(xfinfo,b,blockSize);
     if (strmProj)
            accs = CreateAccStruct(&gstack,xfinfo,xform->hset->vecSize,blockSize,bandWidth);
     else     
            accs = CreateAccStruct(&gstack,xfinfo,GetBaseClassVSize(bclass,b),blockSize,bandWidth);
    for (i=bclass->ilist[b]; i!=NULL; i=i->next) {
      mp = ((MixtureElem *)i->item)->mpdf;
      AccMixPDFStats(xform->hset,mp,accs);
    }
    /* Use last component of the baseclass to access baseclass stats */
    if (AccAdaptBaseTriMat(xform))  AccBaseClassStats(mp,accs);
   
         /* get threshold for this base class */
         s = bclass->stream[b];
         GetSplitThresh(xfinfo,thresh);
         
         printf("Class %d (stream=%d, vsize=%d", b, s, accs->dim);
         if (xform->xformSet->xkind!=SEMIT)
            printf(",occ=%f)\n", accs->occ);
         else
            printf(")\n"); 
   
         if ((accs->dim>0) && ((xform->xformSet->xkind==SEMIT) || (accs->occ > thresh[s]))) {
            EstXForm(accs,xfinfo,classes);
      xform->xformWgts.assign[b] = xform->xformSet->numXForms;
      if (mllrDiagCov) 
	diagCovXForm->xformWgts.assign[b] = diagCovXForm->xformSet->numXForms;
         } 
         else {
       xform->xformWgts.assign[b] = 0;
    }
    Dispose(&gstack,accs);
  }
   }
  return TRUE;
}

InputXForm *AdaptXForm2InputXForm (HMMSet *hset, AdaptXForm *xform)
{
   InputXForm *ixform;
   int s,i,j,cnt,hldasize;
   char mac[MAXSTRLEN], num[MAXSTRLEN];
   LabId id;
   MLink m;
   SVector v;
   SMatrix mat;
   
   if (xform->bclass->numClasses != 1)
      HError(999,"Can only use Stream Projections with single base classes");
   ixform = (InputXForm *)New(hset->hmem,sizeof(InputXForm));
   ixform->xformName = CopyString(hset->hmem,xform->xformName);
   ixform->mmfIdMask = xform->bclass->mmfIdMask;
   ixform->fname = NULL;
   ixform->pkind = xform->hset->pkind;
   ixform->preQual = FALSE;
   ixform->xform = xform->xformSet->xforms[1];
   if ((numNuisanceDim > 0) || (hset->projSize>0)) { /* adjust size to remove HLDA */
      if (numNuisanceDim>0) hldasize = hset->vecSize-numNuisanceDim;
      else hldasize = hset->vecSize-hset->projSize;
      mat = CreateSMatrix(hset->hmem,hldasize,hset->vecSize);
      for (i=1; i<=hldasize; i++)
         for (j=1; j<=hset->vecSize; j++)
            mat[i][j] = ixform->xform->xform[1][i][j];
      ixform->xform->xform[1] = mat;
   }
   /*
      Update the HMMSet vFloor and then zero xf vFloor. For input
      xforms the HMMSet vFloor always  used.
   */
   if ((numNuisanceDim > 0) || (hset->projSize>0)) {
      strcpy(mac,"varFloor1");
      id = GetLabId(mac,FALSE);
      if (id != NULL  && (m=FindMacroName(hset,'v',id)) != NULL){
         /* found the varFloor to update */
         if (numNuisanceDim>0) hldasize = hset->vecSize-numNuisanceDim;
         else hldasize = hset->vecSize-hset->projSize;
         v = CreateSVector(hset->hmem,hldasize);
         for (i=1; i<=hldasize; i++)
            v[i] = ixform->xform->vFloor[i];
         m->structure = v;
      }
   } 
   else {
      for (s=1,cnt=1; s<=hset->swidth[0]; s++) {
         strcpy(mac,"varFloor");
         sprintf(num,"%d",s); strcat(mac,num);
         id = GetLabId(mac,FALSE);
         if (id != NULL  && (m=FindMacroName(hset,'v',id)) != NULL){
            /* found the varFloor to update */
            v = (SVector)m->structure;
            for (i=1; i<=hset->swidth[s]; i++,cnt++)
               v[i] = ixform->xform->vFloor[cnt];
         }
      }
   }
   ixform->xform->vFloor = NULL;
   ixform->nUse = 0;

   return ixform;
}

/* The transform generated from this can be applied to a model set */
static AdaptXForm *CreateBaseAdaptXForm(HMMSet *hset, XFInfo *xfinfo, char *xformName)
{
   AdaptXForm *xform;
   XFormAccInfo *info;
   XFormSet *xformSet;
   int b;
   char buf[MAXSTRLEN];

   XFormKind xKind = xfinfo->outXKind;

   xform = (AdaptXForm *)New(hset->hmem,sizeof(AdaptXForm));
   xform->xformName = CopyString(hset->hmem,xformName);
   xform->mem = hset->hmem;
   xform->hset = hset;
   xform->nUse = 0;
   xform->swapXForm = NULL;
   /* setup the baseclass */
   /* define a baseline xformset with no transforms */
   xform->xformSet = xformSet = (XFormSet *)New(hset->hmem,sizeof(XFormSet));
   xformSet->numXForms = 0;
   xformSet->nUse = 0;
   /* setup default values from config variables */
   xformSet->xkind = xKind; 
   xform->akind = GetAdaptKind(xfinfo);
   /* Now sort out the correct set of baseclasses */
   switch (xform->akind) {
   case BASE:
      xform->bclass = GetBaseClass(hset,xfinfo);
      break;
   case TREE:
      xform->rtree = GetRegTree(hset,xfinfo);
      xform->bclass = xform->rtree->bclass;
      break;
   default:
      HError(999,"Transform kind not current supported");
   }
   /* confirm that the current model set is valid with this baseclass */
   if ((hset->hmmSetId != NULL) && (!MaskMatch(xform->bclass->mmfIdMask,buf,hset->hmmSetId)))
      HError(999,"HMM set ID %s is not compatible with base class mask %s",
	     hset->hmmSetId,xform->bclass->mmfIdMask);
   /* Create space for the weight vectors */
   if (HardAssign(xform)) {
      xform->xformWgts.assign = CreateIntVec(hset->hmem,xform->bclass->numClasses);
      for (b=1;b<=xform->bclass->numClasses;b++) xform->xformWgts.assign[b] = 0;
   } 
   else 
      HError(999,"Not currently supported");
   /* create space for maximum number of transforms */
   if(useSMAPcriterion){
      xformSet->xforms = 
         (LinXForm **)New(hset->hmem,(xform->rtree->numNodes+xform->rtree->numTNodes+1)*sizeof(LinXForm *));
   }else{
   xformSet->xforms = 
      (LinXForm **)New(hset->hmem,(xform->bclass->numClasses+1)*sizeof(LinXForm *));
   }
   /* setup the xform accumulation information */
   xform->info = info = (XFormAccInfo *)New(hset->hmem,sizeof(XFormAccInfo));
   info->accSum = AccAdaptMean(xform);
   info->accSumSq = AccAdaptVar(xform);
   info->accBTriMat = AccAdaptBaseTriMat(xform);
   if (hset->parentXForm != NULL) {
      xform->parentXForm = hset->parentXForm;
      xform->parentXForm->nUse++;
   } 
   else 
      xform->parentXForm = NULL;     
   
   return xform;
} 

/*------------------------------------------------------------------------*/
/*                  Manipulation of Adaptation Transforms                 */
/*------------------------------------------------------------------------*/

static Boolean CompBlockSizes(IntVec blocks1, IntVec blocks2)
{
   int nblock1, nblock2;
   int i;

   nblock1 = IntVecSize(blocks1);
   nblock2 = IntVecSize(blocks2);
   if (nblock1 == nblock2) {
      for (i=1;i<=nblock1;i++) 
         if (blocks1[i] != blocks2[i])
            return FALSE;
   } 
   else 
      return(FALSE);
   
   return (TRUE);
}

static void MultCovMeanLinXForms(LinXForm *xf1, LinXForm *xf2, LinXForm *xf)
{
   /* 
      Needs to handle general case: xf may be xf1, or xf2
   */
   int i,j;
   int cnt, cnti, cntj, bl, bsize;
   Matrix mat,imat,res;
   Vector bres,bias;
   
   bres = CreateVector(&gstack,xf->vecSize);
   ZeroVector(bres);
   if (CompBlockSizes(xf1->blockSize,xf2->blockSize)) {
      /* simplest case treat each block independently */
      for (bl=1,cnti=1,cnt=1;bl<=IntVecSize(xf1->blockSize);bl++) {
         bsize = xf1->blockSize[bl];
         mat = xf1->xform[bl];
         imat = CreateMatrix(&gstack,bsize,bsize);
         res = CreateMatrix(&gstack,bsize,bsize);
         MatInvert(mat,imat);
         MatrixMult(mat,xf2->xform[bl],res);
         if (mllrCov2CMLLR) {
            CopyMatrix(res,xf2->xform[bl]);
         } 
         else {
            MatrixMult(res,imat,xf2->xform[bl]);
         }
         if (xf2->bias != NULL) {
            bias = xf2->bias;
            for (i=1;i<=bsize;i++,cnti++)
               for (j=1,cntj=cnt;j<=bsize;j++,cntj++) 
                  bres[cnti] += mat[i][j] * bias[cntj];
         }
         cnt += bsize;
      }
      if (xf2->bias != NULL) 
         CopyVector(bres,xf2->bias);
   } 
   else {
      HError(999,"Not currently supported");
   }
   FreeVector(&gstack,bres);
}

static void SwapAdaptXForms(AdaptXForm *xform, AdaptXForm *paxform)
{
   AdaptXForm *txform;
   /* char buf[MAXSTRLEN]; */
   
   txform = paxform->parentXForm;
   paxform->parentXForm = xform;
   xform->parentXForm = txform;
   /* 
      Now correct the xformNames. Need to swap over the extension (if 
      any), of the current xform and the parent xform.
      MakeFN(paxform->xformName,NULL,"swap",buf);
      paxform->xformName= CopyString(paxform->mem,buf);
   */
}


static void SwapMLLRCovMLLRMean(AdaptXForm *xform, AdaptXForm *paxform)
{
   XFormSet *xfset, *paxfset;
   int i;

   xfset = xform->xformSet;
   paxfset = paxform->xformSet;
   if (xfset->numXForms == 1) { /* simplest case */
      /* The MLLRCov transform is not altered */
      for (i=1;i<=paxfset->numXForms;i++) {
         MultCovMeanLinXForms(xfset->xforms[1],paxfset->xforms[i],paxfset->xforms[i]);
      }
   }
   if (mllrCov2CMLLR)   xfset->xkind = CMLLR;       
   SwapAdaptXForms(xform, paxform);
   xform->swapXForm = paxform;
}

/* swaps the transform and it's parent around */
static void SwapXForm(HMMSet *hset, AdaptXForm *xform)
{
   AdaptXForm *paxform;
   XFormKind xkind1, xkind2;
   char buf1[MAXSTRLEN], buf2[MAXSTRLEN];

   if (xform->parentXForm == NULL) { /* no transform to swap */
      if (trace&T_SWP)
         printf("No transform to swap for %s\n",xform->xformName);
      return;
   }
   paxform = CopyAdaptXForm(hset->hmem,xform->parentXForm);

   /* 
      Currently check that the baseclasses are not going to be an issue 
      Need to alter code at some stage for general xform support.
   */
   if ((xform->bclass == paxform->bclass) || 
       (xform->bclass->numClasses == 1) ||
       (paxform->bclass->numClasses == 1)) {
      xkind1=xform->xformSet->xkind;
      xkind2=paxform->xformSet->xkind;
      if ((xkind2==MLLRMEAN) && (xkind1==MLLRCOV))
         SwapMLLRCovMLLRMean(xform,paxform);
      else {
         if (trace&T_SWP) {
            HError(999,"Inappropriate combination of transform kinds %s %s\n",
                   XFormKind2Str(xkind1,buf1),XFormKind2Str(xkind2,buf2));
            HError(999,"General transform swapping not currently supported\n");
         }
      }
   } 
   else {
      if (trace&T_SWP) {
         printf("Inappropriate combination  of  baseclasses %s %s\n",
                xform->bclass->fname,paxform->bclass->fname);
         printf("General case not currently supported\n");
      }
   }
}

/*------------------------------------------------------------------------*/
/*                  External Routines for Adaptation code                 */
/*------------------------------------------------------------------------*/

/* ---------------- Accumulation Control Functions ---------------------- */

void AccAdaptFrame(XFInfo *xfinfo, double Lr, Vector svec, MixPDF *mp, int t)
{
   RegAcc *ra;
   int i, vsize;

   vsize = VectorSize(mp->mean);
   if (vsize!=SpaceOrder(svec))  /* check MSD */
      return;
      
   ra = GetRegAcc(mp);
   ra->occ += Lr;
   if (ra->spSum != NULL)
      for (i=1;i<=vsize;i++)
         ra->spSum[i] += Lr*svec[i];
   if (ra->spSumSq != NULL)
      for (i=1;i<=vsize;i++)
         ra->spSumSq[i] += Lr*svec[i]*svec[i];
   if (ra->bTriMat != NULL) 
      AccBaseTriMat(xfinfo,Lr,svec,mp,t);
}

void ZeroAdaptAccs(HMMSet *hset, AdaptXForm *xform)
{
   int b;
   BaseClass *bclass;
   MixPDF *mp;
   RegAcc *ra = NULL;
   ILink i;
  
   if (hset->attRegAccs) {
      bclass = xform->bclass;
      for (b=1;b<=bclass->numClasses;b++) {
         for (i=bclass->ilist[b]; i!=NULL; i=i->next) {
            mp = ((MixtureElem *)i->item)->mpdf;
            ra = GetRegAcc(mp);
            ra->occ = 0.0;
            if (ra->spSum != NULL) ZeroDVector(ra->spSum);
            if (ra->spSumSq != NULL) ZeroDVector(ra->spSumSq);
         }
         /* Use last component of the baseclass to access baseclass stats */
         if (ra->bTriMat != NULL) ZeroBaseTriMat(ra->bTriMat);
      }
   }
}

/* ---------------- Applying Transform Functions ------------------------ */

/* EXPORT->SetXForm: set current xform for given hset */
void SetXForm(HMMSet *hset, XFInfo *xfinfo, AdaptXForm *xform)
{
   if (!(CompareXFormInfo(hset->curXForm, xform))) {
      SetAInfo(hset,xform,FALSE);
   } 
   SetObsCache(xfinfo,xform,FALSE);
   if (storeMInfo) {
      if (hset->attMInfo) {
         if (!CompareMInfo(hset,xform))
            UpdateMInfo(hset,xform);
      } 
      else {
         SetMInfo(hset,xform);
      }
   }
   /* set the flag whether input transform changes covariance or not */
   xfinfo->covarChanged = XFormModCovar(xform);
   hset->curXForm = xform;
}

/* EXPORT->SetParentXForm: set parent xform for given hset */
void SetParentXForm(HMMSet *hset, XFInfo *xfinfo, AdaptXForm *xform)
{
   if (!(CompareXFormInfo(hset->parentXForm, xform))) {
      SetAInfo(hset,xform,TRUE);
   }
   SetObsCache(xfinfo, xform, TRUE);
   if (storeMInfo) {
      if (hset->attMInfo) {
         if (!CompareMInfo(hset,xform))
            UpdateMInfo(hset,xform);
      } 
      else {
         SetMInfo(hset,xform);
      }
   }
   /* set the flag whether parent transform changes covariance or not */
   xfinfo->covarPChanged = XFormModCovar(xform);
   hset->parentXForm = xform;
}

/* EXPORT->ApplyCompXForm: apply component-specific xform to MixPDF */
void ApplyCompXForm(MixPDF *mp, AdaptXForm *xform, Boolean full)
{
   AInfo *ai = NULL;
   HMMSet *hset;
   AdaptXForm *ax;

   /* Store covariance matrices in full covariance form or not */
   if (full) {      
      for (ax=xform,full=FALSE; (ax!=NULL) && (full==FALSE); ax=ax->parentXForm) {
         full |= ((ax->xformSet->xkind==MLLRCOV) 
               || (ax->xformSet->xkind==CMLLR) 
               || (ax->xformSet->xkind==SEMIT));
      }
   }

   if (xform != NULL) {
      hset = xform->hset;
      if (mp->info == NULL)
         HError(999,"No adaptation information for component");
      if (xform == hset->curXForm)  /* use adapt information from current Xform */
         ai = GetAInfo(mp);
      else if (xform == hset->parentXForm) {
         ai = GetPAInfo(mp);
      } 
      else 
         HError(999,"Can only apply parent and current transform");
      CompXForm(mp,xform,ai,full);
   }
}

/* EXPORT->ApplyCompFXForm: apply component-specific xform to given observation */
Vector ApplyCompFXForm(MixPDF *mp, Vector svec, AdaptXForm *xform, LogFloat *det, const int t)
{
  AInfo *ai = NULL;
  ObsCache *oc = NULL;
  Vector vec;
  HMMSet *hset;
   int vSize;

   vSize=VectorSize(mp->mean);
   if (xform==NULL || SpaceOrder(svec)==0 || SpaceOrder(svec)!=vSize) {
     *det = 0;
     return svec;
   }  
   else {
    hset = xform->hset;
    if (mp->info == NULL)
      HError(999,"No adaptation information for component");
    if (xform == hset->curXForm)  { /* use adapt information from current Xform */
      ai = GetAInfo(mp);
      oc = GetObsCache(mp);
    }
    else if (xform == hset->parentXForm) {
      ai = GetPAInfo(mp);
      oc = GetPAObsCache(mp);
      } 
      else 
      HError(999,"Can only apply parent and current transform");

      *det = 0.0;
    if ( oc != NULL ) {
      if (oc->time != t ) {
            vec = CreateVector(&gstack,vSize);
            CopyRVector(svec,vec,vSize);
        CompFXForm(mp,vec,xform,ai,det);
        UpdateObsCache(oc, vec, *det,t);  
	FreeVector(&gstack,vec); 
	if (trace&T_OBC) printf("Updated ObsCache %p at time %d\n",oc,t);
      } 
      vec = oc->obs;
      *det = oc->det;
      return vec;
      } 
      else {
         *det = 0.0;
       return svec;
    }
  }
}

/* EXPORT->ApplyHMMSetXForm: apply given xform to hset */
void ApplyHMMSetXForm(HMMSet *hset, AdaptXForm *xform, Boolean full)
{
  HMMScanState hss;
   AdaptXForm *ax;
  int nAdpt=0;

   /* Store covariance matrices in full covariance form or not */
   if (full) {      
      for (ax=xform,full=FALSE; (ax!=NULL) && (full==FALSE); ax=ax->parentXForm) {
         full |= ((ax->xformSet->xkind==MLLRCOV) 
               || (ax->xformSet->xkind==CMLLR) 
               || (ax->xformSet->xkind==SEMIT));
      }
   }

  /* Only created XForm and parent Xform strutures */
  if ((xform != hset->curXForm) && (xform != hset->parentXForm))
    HError(999,"Can only apply parent and current transform");
      
  NewHMMScan(hset,&hss);
  do {
    while (GoNextState(&hss,TRUE)) {
      while (GoNextStream(&hss,TRUE)) {            
	if (hss.isCont)                     /* PLAINHS or SHAREDHS */
	  while (GoNextMix(&hss,TRUE)) {
                  ApplyCompXForm(hss.mp,xform,full);
	    nAdpt++;
	  }
      }
    }
  } while (GoNextHMM(&hss));
  EndHMMScan(&hss);
   
   if (full)
      hset->ckind = FULLC;
      
  if (trace&T_ADT) printf("Adapted %d components\n",nAdpt);
}

/* EXPORT->ResetXFormHMMSet: reset applied xform */
void ResetXFormHMMSet(HMMSet *hset)
{
   HMMScanState hss;

   NewHMMScan(hset,&hss);
   do {
      while (GoNextState(&hss,TRUE)) {
         while (GoNextStream(&hss,TRUE)) {            
            if (hss.isCont)                     /* PLAINHS or SHAREDHS */
               while (GoNextMix(&hss,TRUE)) {
                  ResetComp(hss.mp);
               }
         }
      }
   } while (GoNextHMM(&hss));
   EndHMMScan(&hss);

   if (hset->ckind==FULLC)
      hset->ckind = DIAGC;
}

/* ---------------  Transform Copying Functions  ----------------------- */

LinXForm *CopyLinXForm(MemHeap *x, LinXForm *xf)
{
   LinXForm *nxf;
   int bl,bsize;

   nxf = (LinXForm *)New(x,sizeof(LinXForm));
   nxf->vecSize = xf->vecSize;
   nxf->blockSize = CreateIntVec(x,IntVecSize(xf->blockSize));
   CopyIntVec(xf->blockSize,nxf->blockSize);
   nxf->det = xf->det;
   nxf->nUse = 0; /* new linXForm */
   if (xf->bias != NULL) {
      nxf->bias = CreateSVector(x,xf->vecSize);
      CopyVector(xf->bias,nxf->bias);
   } 
   else
      nxf->bias = NULL;

   if (xf->vFloor != NULL) {
      nxf->vFloor = CreateSVector(x,VectorSize(xf->vFloor));
      CopyVector(xf->vFloor,nxf->vFloor);
   }
   else
      nxf->vFloor = NULL;
   nxf->xform = (Matrix *)New(x,(IntVecSize(xf->blockSize)+1)*sizeof(Matrix));
   for (bl=1;bl<=IntVecSize(xf->blockSize);bl++) {
      bsize = xf->blockSize[bl];
      nxf->xform[bl] = CreateSMatrix(x,bsize,bsize);
      CopyMatrix(xf->xform[bl],nxf->xform[bl]);
   }
   return nxf;
}

XFormSet *CopyXFormSet(MemHeap *x, XFormSet *xfset)
{
   XFormSet *nxfset;
   int i;

   nxfset = (XFormSet *)New(x,sizeof(XFormSet));
   nxfset->numXForms = xfset->numXForms;
   nxfset->xkind = xfset->xkind;
   nxfset->nUse = 0; /* new XFormSet */
   nxfset->xforms = (LinXForm **)New(x,(xfset->numXForms+1)*sizeof(LinXForm *));
   for (i=1;i<=xfset->numXForms;i++) {
      if (xfset->xforms[i]->nUse == 0)
         nxfset->xforms[i] = CopyLinXForm(x,xfset->xforms[i]);
      else 
         nxfset->xforms[i] = xfset->xforms[i];
   }
   return nxfset;
}

AdaptXForm *CopyAdaptXForm(MemHeap *x, AdaptXForm *xform)
{
   AdaptXForm *nxform;

   nxform = (AdaptXForm *)New(x,sizeof(AdaptXForm));
   nxform->xformName = CopyString(x,xform->xformName);
   nxform->fname = CopyString(x,xform->fname);
   nxform->mem  = x;
   nxform->akind = xform->akind;
   nxform->swapXForm = NULL;
   nxform->bclass = xform->bclass;
   nxform->rtree = xform->rtree;
   nxform->nUse = 0; /* This is a new transform */
   nxform->info = xform->info;
   nxform->parentXForm = xform->parentXForm;
   nxform->hset = xform->hset;
   if (xform->xformSet->nUse == 0) 
      nxform->xformSet = CopyXFormSet(x,xform->xformSet);
   else
      nxform->xformSet = xform->xformSet;
   if (HardAssign(xform)) {
      nxform->xformWgts.assign = CreateIntVec(x,IntVecSize(xform->xformWgts.assign));
      CopyIntVec(xform->xformWgts.assign,nxform->xformWgts.assign);
   } 
   else {
      HError(999,"Not currently supported");
   }
   
   return nxform;
}

/* ---------------  Transform Estimation Functions ----------------------- */

AdaptXForm *CreateAdaptXForm(HMMSet *hset, XFInfo *xfinfo, char *xformName)
{
   AdaptXForm *xform,*diagCovXForm;

   /* The macroname is not defined at this stage, to avoid
      over-writing the old version */
   xfinfo->outXForm = xform = CreateBaseAdaptXForm(hset,xfinfo,xformName);
   if (mllrDiagCov) { /* additional code for efficient diag Cov */     
      xfinfo->diagCovXForm = diagCovXForm = CreateBaseAdaptXForm(hset,xfinfo,xformName);
     /* fix the parent xform and attributes */
     if (xform->parentXForm != NULL) xform->parentXForm->nUse--;
     diagCovXForm->parentXForm = xform;
     diagCovXForm->xformSet->xkind = MLLRVAR;
     diagCovXForm->info = xform->info;
   }
   if (!hset->attRegAccs) AttachRegAccs(hset,xfinfo);
   xfinfo->outXForm = xform;
   return xform;
}

Boolean GenAdaptXForm(HMMSet *hset, XFInfo *xfinfo)
{
   AdaptKind akind;
   Boolean genXForm = FALSE;
   AdaptXForm *xform = xfinfo->outXForm;

   akind = xform->akind;
   switch(akind) {
   case TREE: 
      genXForm = ParseTree(xfinfo);
      break;
   case BASE:
      genXForm = GenClassXForm(xfinfo);
      break;
   default:
      HError(999,"Only TREE and BASE adaptation kinds currently supported");
   }
   /* support ability to generate and swap transforms */
   if (swapXForms) SwapXForm(hset, xform);
   return genXForm;
}

void TidyBaseAccs(XFInfo *xfinfo)
{
   SetBaseAccsTime(xfinfo,-2);
   if (xfinfo->headac==NULL)
      UpdateBaseAccs(xfinfo,NULL,-1,NULL);
   else { 
      UpdateBaseAccsWithPaac(xfinfo,-1);
      ResetAccCache(xfinfo,-1);
   }
}

AdaptXForm *GetMLLRDiagCov(XFInfo *xfinfo, AdaptXForm *xform)
{
   AdaptXForm *diagCovXForm = xfinfo->diagCovXForm;
   
   if (diagCovXForm == NULL)
      return xform;
   else 
      return diagCovXForm;
}

/* 
   UpdateSpkrStats: monitor speaker changes and generate transforms
   at each speaker boundary, returns TRUE when the output speaker
   has changed
*/
Boolean UpdateSpkrStats(HMMSet *hset, XFInfo *xfinfo, char *datafn)
{
   char newFn[MAXSTRLEN];
   char newMn[MAXSTRLEN];
   char spkr[MAXSTRLEN];
   char paspkr[MAXSTRLEN];
   Boolean resetHMMSet = FALSE, maskMatch;
   Boolean spkrChange = FALSE;
   int s;

   if (!((hset->hsKind == PLAINHS) || (hset->hsKind == SHAREDHS))
       && (xfinfo->useOutXForm || xfinfo->useInXForm || xfinfo->usePaXForm )) {
      HError(999,"Adaptation only supported for PLAINHS and SHAREDHS systems");
   }
   /* First: handle output transform generation */
   if (xfinfo->useOutXForm) { /* if there is an output transform to be generated */
      maskMatch = MaskMatch(xfinfo->outSpkrPat,spkr,datafn);
      if ((!maskMatch) && (datafn != NULL))
         HError(999,"Output xform mask %s does not match filename %s",xfinfo->outSpkrPat,datafn);
      if ((datafn == NULL) || (strcmp(spkr,xfinfo->coutspkr))) {
         /* end of current speaker, so complete his/her transform */
         if (xfinfo->nspkr>0) { /* nothing to generate if first speaker */
            if (trace&T_SXF)
               printf("Generating transform %s (%i)\n",xfinfo->coutspkr,xfinfo->nspkr);
            /* Tidy the statistics of the last frame */
            SetBaseAccsTime(xfinfo,-2);
            if (xfinfo->headac==NULL)
               UpdateBaseAccs(xfinfo,NULL,-1,NULL);
            else { 
               UpdateBaseAccsWithPaac(xfinfo,-1);
               ResetAccCache(xfinfo,-1);
            }
            /* Generate the new transform */
            MakeFN(xfinfo->coutspkr,NULL,xfinfo->outXFormExt, newMn);
            xfinfo->outXForm = CreateAdaptXForm(hset, xfinfo, newMn);
            GenAdaptXForm(hset,xfinfo);
            if (mllrDiagCov) xfinfo->outXForm = xfinfo->diagCovXForm;
            /* After generating a transform need to reset parameters */
            resetHMMSet = TRUE;
            if (keepXFormDistinct) {  /* Output individual transform */
               MakeFN(xfinfo->coutspkr,xfinfo->outXFormDir,xfinfo->outXFormExt,newFn);
               SaveOneXForm(hset,xfinfo->outXForm,newFn,xfinfo->saveBinary);
            } 
            else { /* Create macro from the masked speaker name and extension */
               MakeFN(xfinfo->coutspkr,NULL,xfinfo->outXFormExt,newMn);
               CreateXFormMacro(hset,xfinfo->outXForm,newMn);
            }
            if (saveSpkrModels) { 
               /* 
                  output distinct model for each speaker. The macro name
                  including extension is used to distinguish models.
                  First set the adaptation up.
               */
               SetXForm(hset,xfinfo,xfinfo->outXForm);
               ApplyHMMSetXForm(hset, xfinfo->outXForm, xfinfo->outFullC);
               ForceDiagC(hset);
               SaveHMMSet(hset,xfinfo->outXFormDir,newMn,newMn,xfinfo->saveBinary);
               ConvDiagC(hset,TRUE);
               /* remembering to reset the transform */
               if (xfinfo->useInXForm) 
                  SetXForm(hset,xfinfo,xfinfo->inXForm);
               else
                  SetXForm(hset,xfinfo,NULL);
            }
            spkrChange = TRUE;
            ZeroAdaptAccs(hset,xfinfo->outXForm);
            if ((xfinfo->usePaXForm || xfinfo->use_alPaXForm) && (datafn != NULL)) {
               maskMatch = MaskMatch(xfinfo->paSpkrPat,paspkr,datafn);
               if (!maskMatch)
                  HError(999,"Parent xform mask %s does not match filename %s",xfinfo->paSpkrPat,datafn);
               /* parent transform changed and not the last file? */
               if (strcmp(paspkr,xfinfo->cpaspkr)) { 
                  strcpy(xfinfo->cpaspkr,paspkr);
                  if (xfinfo->usePaXForm) {
                     MakeFN(xfinfo->cpaspkr,xfinfo->paXFormDir,xfinfo->paXFormExt,newFn);
                     MakeFN(xfinfo->cpaspkr,NULL,xfinfo->paXFormExt,newMn);
                  xfinfo->paXForm = LoadOneXForm(hset,newMn,newFn);
                     SetParentXForm(hset,xfinfo,xfinfo->paXForm);
                     SetAccCache(xfinfo);
                  }
                  if (xfinfo->al_hset!=NULL && xfinfo->use_alPaXForm) {
                     MakeFN(xfinfo->cpaspkr,xfinfo->al_paXFormDir,xfinfo->al_paXFormExt,newFn);
                     MakeFN(xfinfo->cpaspkr,NULL,xfinfo->al_paXFormExt,newMn);
                     xfinfo->al_paXForm = LoadOneXForm(xfinfo->al_hset,newMn,newFn);
                     SetParentXForm(xfinfo->al_hset,xfinfo,xfinfo->al_paXForm);
                     SetAccCache(xfinfo);
                  }
                  else
                     xfinfo->al_paXForm = xfinfo->paXForm;
               }
               }
            }
         else if (xfinfo->usePaXForm || xfinfo->use_alPaXForm) { /* set-up the initial parent transform information */
            maskMatch = MaskMatch(xfinfo->paSpkrPat,paspkr,datafn);
            if (!maskMatch)
               HError(999,"Parent xform mask %s does not match filename %s",xfinfo->paSpkrPat,datafn);
            strcpy(xfinfo->cpaspkr,paspkr);
            if (xfinfo->usePaXForm) {
               MakeFN(xfinfo->cpaspkr,xfinfo->paXFormDir,xfinfo->paXFormExt,newFn);
               MakeFN(xfinfo->cpaspkr,NULL,xfinfo->paXFormExt,newMn);
            xfinfo->paXForm = LoadOneXForm(hset,newMn,newFn);
               SetParentXForm(hset,xfinfo,xfinfo->paXForm);
               SetAccCache(xfinfo);
            }
            if (xfinfo->al_hset!=NULL && xfinfo->use_alPaXForm) {
               MakeFN(xfinfo->cpaspkr,xfinfo->al_paXFormDir,xfinfo->al_paXFormExt,newFn);
               MakeFN(xfinfo->cpaspkr,NULL,xfinfo->al_paXFormExt,newMn);
               xfinfo->al_paXForm = LoadOneXForm(xfinfo->al_hset,newMn,newFn);
               SetParentXForm(xfinfo->al_hset,xfinfo,xfinfo->al_paXForm);
               SetAccCache(xfinfo);
         }
            else
               xfinfo->al_paXForm = xfinfo->paXForm;
         }
         
         if (datafn!=NULL) {
            xfinfo->nspkr++;
            strcpy(xfinfo->coutspkr,spkr);
         }      
      } 
      else if (xfinfo->usePaXForm || xfinfo->use_alPaXForm) { 
         /* check to see whether the parent transform changes */
         /* this should not happen */
         MaskMatch(xfinfo->paSpkrPat,paspkr,datafn);
         if (strcmp(paspkr,xfinfo->cpaspkr)) 
            HError(999,"Changing parent transform out of sync with output transform (%s %s)",
                   paspkr,xfinfo->cpaspkr);
      }
      }
   else if ((xfinfo->usePaXForm || xfinfo->use_alPaXForm) && (datafn != NULL)) {
      /* Parent transform specified with no output transform */
      maskMatch = MaskMatch(xfinfo->paSpkrPat,paspkr,datafn);
      if (!maskMatch)
         HError(999,"Parent xform mask %s does not match filename %s",xfinfo->paSpkrPat,datafn);
      /* parent transform changed and not the last file? */
      if (strcmp(paspkr,xfinfo->cpaspkr)) { 
         strcpy(xfinfo->cpaspkr,paspkr);
         if (xfinfo->usePaXForm) {  /* parent xform for hset */
            MakeFN(xfinfo->cpaspkr,xfinfo->paXFormDir,xfinfo->paXFormExt,newFn);
            MakeFN(xfinfo->cpaspkr,NULL,xfinfo->paXFormExt,newMn);
         xfinfo->paXForm = LoadOneXForm(hset,newMn,newFn);
            SetParentXForm(hset,xfinfo,xfinfo->paXForm);
            SetAccCache(xfinfo);
      }
         else if (hset->semiTied != NULL) { /* model set uses a semitied transform */
      if (xfinfo->paXForm == NULL) { /* semi-tied transform is for all files */
         xfinfo->paXForm = hset->semiTied;
               SetParentXForm(hset,xfinfo,xfinfo->paXForm);
               SetAccCache(xfinfo);
      }
         }
         if (xfinfo->al_hset!=NULL) {
            if (xfinfo->use_alPaXForm) {  /* parent xform for al_hset */
               MakeFN(xfinfo->cpaspkr,xfinfo->al_paXFormDir,xfinfo->al_paXFormExt,newFn);
               MakeFN(xfinfo->cpaspkr,NULL,xfinfo->al_paXFormExt,newMn);
               xfinfo->al_paXForm = LoadOneXForm(xfinfo->al_hset,newMn,newFn);
               SetParentXForm(xfinfo->al_hset,xfinfo,xfinfo->al_paXForm);
               SetAccCache(xfinfo);
            }
            else if (xfinfo->al_hset->semiTied != NULL) { /* model set uses a semitied transform */
               if (xfinfo->al_paXForm == NULL) { /* semi-tied transform is for all files */
                  xfinfo->al_paXForm = xfinfo->al_hset->semiTied;
                  SetParentXForm(xfinfo->al_hset,xfinfo,xfinfo->al_paXForm);
                  SetAccCache(xfinfo);
               }
            }
         }
         else 
            xfinfo->al_paXForm = xfinfo->paXForm;

      spkrChange=TRUE;
      }
   } 

   /* Second: handle input transform */
   if ((xfinfo->useInXForm || xfinfo->use_alInXForm) && (datafn != NULL)) {
      maskMatch = MaskMatch(xfinfo->inSpkrPat,spkr,datafn);
      if (!maskMatch)
         HError(999,"Input xform mask %s does not match filename %s",xfinfo->inSpkrPat,datafn);
      /* if a transform has been changed the input transform must be 
         reapplied */
      if (strcmp(spkr,xfinfo->cinspkr) || resetHMMSet) {
         if (trace&T_SXF)
            printf("Using input transform %s\n",spkr);
         strcpy(xfinfo->cinspkr,spkr);      
         if (xfinfo->useInXForm) {  /* input xform for hset */
            MakeFN(xfinfo->cinspkr,NULL,xfinfo->inXFormExt,newMn);
         xfinfo->inXForm = LoadOneXForm(hset,newMn,NULL);
            SetXForm(hset,xfinfo,xfinfo->inXForm);
            ApplyHMMSetXForm(hset,xfinfo->inXForm,xfinfo->inFullC);
         }
         if (xfinfo->al_hset!=NULL && xfinfo->use_alInXForm) {  /* input xform for al_hset */
            MakeFN(xfinfo->cinspkr,xfinfo->al_inXFormDir,xfinfo->al_inXFormExt,newFn);
            MakeFN(xfinfo->cinspkr,NULL,xfinfo->al_inXFormExt,newMn);
            xfinfo->al_inXForm = LoadOneXForm(xfinfo->al_hset,newMn,newFn);
            SetXForm(xfinfo->al_hset,xfinfo,xfinfo->al_inXForm);
            ApplyHMMSetXForm(xfinfo->al_hset,xfinfo->al_inXForm,FALSE);
         } 
         else 
            xfinfo->al_inXForm = xfinfo->inXForm;

         spkrChange = TRUE;
         }
      }
   else if (resetHMMSet && (xfinfo->usePaXForm || (xfinfo->inXForm != NULL) || saveSpkrModels)) { 
      /* 
         Reset model parameters as transform generated using 
         a parent transform - it is possible to be more efficient 
         if the nature of the transform is also considered 
      */
      ResetXFormHMMSet(hset);
   }
   else { /* handle the fact that alignment and update HMMSet may use semi-tied transforms */
      if ((xfinfo->inXForm == NULL) && (xfinfo->al_inXForm == NULL)) { /* semi-tied transform is for all files */
         if (hset->semiTied != NULL) { /* model set uses a semitied transform */
            xfinfo->inXForm = hset->semiTied;
            SetXForm(hset,xfinfo,xfinfo->inXForm);
         }
         if (xfinfo->al_hset != NULL) {
            if (xfinfo->al_hset->semiTied != NULL) { /* alignment model set uses a semitied transform */
               xfinfo->al_inXForm = xfinfo->al_hset->semiTied;
               SetXForm(xfinfo->al_hset,xfinfo,xfinfo->al_inXForm);
            }
         } else {
            xfinfo->al_inXForm = xfinfo->inXForm;
         }
      }
   }

   /* All the files have been handled - store xforms */
   if (datafn == NULL) { 
      if (!keepXFormDistinct) {
         if (xfinfo->xformTMF == NULL) {
            MakeFN("TMF",xfinfo->outXFormDir,NULL,newFn);
         } 
         else {
            MakeFN(xfinfo->xformTMF,xfinfo->outXFormDir,NULL,newFn);
         }
            SaveAllXForms(hset,newFn,xfinfo->saveBinary);
      }
      
      /* reset static variables for speaker name */
      for (s=0; s<MAXSTRLEN; s++){
         xfinfo->coutspkr[s] = '\0';
         xfinfo->cpaspkr[s] = '\0';
         xfinfo->cinspkr[s] = '\0';
      }
   }

   return spkrChange;
}

void UpdateSemiTiedModels(HMMSet *hset, XFInfo *xfinfo)
{
   char newFn[MAXSTRLEN];
   char *newMn;
   InputXForm *ixform;

   if (semiTiedMacro == NULL) {
      if (numNuisanceDim > 0) newMn = "HLDA";
      else newMn = "SEMITIED";
   } 
   else 
      newMn = semiTiedMacro;
   xfinfo->outXForm = CreateAdaptXForm(hset, xfinfo, newMn);
   GenAdaptXForm(hset,xfinfo);
   CreateXFormMacro(hset,xfinfo->outXForm,newMn);
   MakeFN(newMn,xfinfo->outXFormDir,xfinfo->outXFormExt,newFn);
   if ((strmProj) || (semiTied2InputXForm)) {
      hset->semiTied = NULL;
      ixform = AdaptXForm2InputXForm(hset,xfinfo->outXForm);
      hset->xf = ixform;
      if (numNuisanceDim>0) UpdateHLDAModel(hset);
      SaveInputXForm(hset,ixform,newFn,saveSemiTiedBinary);
   } 
   else {
      hset->projSize = numNuisanceDim;
      hset->semiTied = xfinfo->outXForm;
      SaveOneXForm(hset,xfinfo->outXForm,newFn,saveSemiTiedBinary);
   }
   /* All parameters updated - now do GConst */
   FixAllGConsts(hset);
}

void UpdateProjectModels(HMMSet *hset, char *dir)
{
   char newFn[MAXSTRLEN];
   char *newMn;
   InputXForm *ixform;
   AdaptXForm* xform;

   if (semiTiedMacro == NULL) newMn = hset->semiTiedMacro;
   else newMn = semiTiedMacro;
   xform = hset->semiTied;
   ixform = AdaptXForm2InputXForm(hset,xform);
   hset->xf = ixform; hset->semiTied = NULL;
   UpdateHLDAModel(hset);
   MakeFN(newMn,dir,NULL,newFn);
   SaveInputXForm(hset,ixform,newFn,saveSemiTiedBinary);
   /* gconsts fixed in main script */
}

/* ApplyMapXForm: apply mapped xform to MixPDF */
void ApplyMapXForm(MixPDF * mp, MixPDF * mp_map, AdaptXForm * xform, Boolean full)
{
   AInfo *ai = NULL;
   HMMSet *hset;
   AdaptXForm *ax;

   /* Store covariance matrices in full covariance form or not */
   if (full)
      for (ax = xform, full = FALSE; (ax != NULL) && (full == FALSE); ax = ax->parentXForm)
         full |= ((ax->xformSet->xkind == MLLRCOV) || (ax->xformSet->xkind == CMLLR) || (ax->xformSet->xkind == SEMIT));

   if (xform != NULL) {
      hset = xform->hset;
      if (mp_map->info == NULL)
         HError(999, "ApplyMapXForm: No adaptation information for component");
      if (xform == hset->curXForm)      /* use adapt information from current Xform */
         ai = GetAInfo(mp_map);
      else if (xform == hset->parentXForm)
         ai = GetPAInfo(mp_map);
      else
         HError(999, "ApplyMapXForm: Can only apply parent and current transform");
      CompXForm(mp, xform, ai, full);
   }
}

LinXForm* GetRelateXForm(HMMSet *hset, MixPDF *mp, XFormKind xkind)
{
   AInfo *ai = NULL;
   AdaptXForm *xform;
   int numXf;

   xform = hset->curXForm;
   ai = GetAInfo(mp);

   while(ai) {
      if(xform->xformSet->xkind == xkind) {
         numXf = xform->xformWgts.assign[ai->baseClass];
         return xform->xformSet->xforms[numXf];
      }
      ai = ai->next;
      xform = xform->parentXForm;
   }

  return NULL;
}

/* ------------------------ End of HAdapt.c ------------------------ */
