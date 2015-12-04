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
/*      Entropic Cambridge Research Laboratory                 */
/*      (now part of Microsoft)                                */
/*                                                             */
/* ----------------------------------------------------------- */
/*         Copyright: Microsoft Corporation                    */
/*          1995-2000 Redmond, Washington USA                  */
/*                    http://www.microsoft.com                 */
/*                                                             */
/*              2002  Cambridge University                     */
/*                    Engineering Department                   */
/*                                                             */
/*   Use of this software is governed by a License Agreement   */
/*    ** See the file License for the Conditions of Use  **    */
/*    **     This banner notice must not be removed      **    */
/*                                                             */
/* ----------------------------------------------------------- */
/*         File: HFB.c: Forward Backward routines module       */
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

char *hfb_version = "!HVER!HFB:   3.4.1 [CUED 12/03/09]";
char *hfb_vc_id = "$Id: HFB.c,v 1.51 2011/06/16 04:18:28 uratec Exp $";

#include "HShell.h"     /* HMM ToolKit Modules */
#include "HMem.h"
#include "HMath.h"
#include "HSigP.h"
#include "HAudio.h"
#include "HWave.h"
#include "HVQ.h"
#include "HParm.h"
#include "HLabel.h"
#include "HModel.h"
#include "HTrain.h"
#include "HUtil.h"
#include "HAdapt.h"
#include "HFB.h"


/* ------------------- Trace Information ------------------------------ */
/* Trace Flags */
#define T_TOP   0001    /* Top level tracing */
#define T_OPT   0002    /* Option trace */
#define T_PRU   0004    /* pruning */
#define T_ALF   0010    /* Alpha/Beta matrices */
#define T_OCC   0020    /* Occupation Counters */
#define T_TRA   0040    /* Transition Counters */
#define T_MIX   0100    /* Mixture Weights */
#define T_OUT   0200    /* Output Probabilities */
#define T_UPD   0400    /* Model updates */
#define T_TMX  01000    /* Tied Mixture Usage */
#define T_TIM  02000    /* Time elapsed in FBUtt */

static int trace         =  0;
static int skipstartInit = -1;
static int skipendInit   = -1;
static Boolean alCompLevel = FALSE;   /* align model at component level */

extern Boolean keepOccm; /* keep mixture occ (used in parameter generation) */

static ConfParam *cParm[MAXGLOBS];      /* config parameters */
static int nParm = 0;

static struct { 

   LogDouble pruneInit;      /* pruning threshold initially */
   LogDouble pruneInc;       /* pruning threshold increment */
   LogDouble pruneLim;       /* pruning threshold limit */
   float minFrwdP;           /* mix prune threshold */

} pruneSetting = { NOPRUNE, 0.0, NOPRUNE, 10.0 };

static Boolean pde = FALSE;  /* partial distance elimination */
static Boolean sharedMix = FALSE; /* true if shared mixtures */
static Boolean sharedStream = FALSE; /* true if shared streams */

static Boolean semiMarkov = FALSE;  /* HMM or HSMM */
static int maxstdDevCoef = 10;      /* max duration */
static int minDur = 5;              /* min duration */
static MemHeap dprobStack;

/* ------------------------- Min HMM Duration -------------------------- */

/* Recusively calculate topological order for transition matrix */
void FindStateOrder(HLink hmm,IntVec so,int s,int *d)
{
   int p;

   so[s]=0; /* GRAY */
   for (p=1;p<hmm->numStates;p++) { 
      if (hmm->transP[p][s]>LSMALL && p!=s)
         if (so[p]<0) /* WHITE */
            FindStateOrder(hmm,so,p,d);
   }
   so[s]=++(*d); /* BLACK */
}
   

/* SetMinDurs: Set minDur field in each TrAcc */
void SetMinDurs(HMMSet *hset)
{
   HMMScanState hss;
   HLink hmm;
   TrAcc *ta;
   IntVec md,so;
   int d,nDS,i,j,k;

   NewHMMScan(hset,&hss);
   do {
      hmm = hss.hmm;
      ta = (TrAcc *)GetHook(hmm->transP);
      md = CreateIntVec(&gstack,hmm->numStates);
      so = CreateIntVec(&gstack,hmm->numStates);
      for (i=1,nDS=0;i<=hmm->numStates;i++) so[i]=md[i]=-1;
      /* Find topological order for states so that we can */
      /*  find minimum duration in single ordered pass */
      FindStateOrder(hmm,md,hmm->numStates,&nDS);
      for (i=1;i<=nDS;i++) so[md[i]]=i;
      for (i=1;i<=hmm->numStates;i++) md[i]=hmm->numStates;
      for (k=1,md[1]=0;k<=nDS;k++) {
         i=so[k];
         if (i<1 || i>hmm->numStates)  continue;
         /* Find minimum duration to state i */
         for (j=1;j<hmm->numStates;j++)
            if (hmm->transP[j][i]>LSMALL) {
               d=md[j]+((i==hmm->numStates)?0:1);
               if (d<md[i]) md[i]=d;
            }
      }
      if (nDS!=hmm->numStates) {
         char buf[8192]="";
         for (j=1;j<=hmm->numStates && strlen(buf)<4096;j++) 
            if (md[j]>=hmm->numStates)
               sprintf(buf+strlen(buf),"%d ",j);
         HError(-7332,"SetMinDurs: HMM-%s with %d/%d unreachable states ( %s)",
                HMMPhysName(hset,hmm),hmm->numStates-nDS,hmm->numStates,buf);
      }
      if (md[hmm->numStates]<0 || md[hmm->numStates]>=hmm->numStates) {
         /* Should really be an error */
         HError(-7333,"SetMinDurs: Transition matrix with discontinuity");
         ta->minDur = (hmm->transP[1][hmm->numStates]>LSMALL ? 
                       0 : 1 /*hmm->numStates-2*/ ); /* Under estimate */
      }
      else
         ta->minDur = md[hmm->numStates];
      FreeIntVec(&gstack,so); FreeIntVec(&gstack,md);
   } while (GoNextHMM(&hss));
   EndHMMScan(&hss);
}

/* ----------------------------------------------------------------------- */

/* CreateTrAcc: create an accumulator for transition counts */
static TrAcc *CreateTrAcc(MemHeap *x, int numStates)
{
   TrAcc *ta;
  
   ta = (TrAcc *) New(x,sizeof(TrAcc));
   ta->tran = CreateMatrix(x,numStates,numStates);
   ZeroMatrix(ta->tran);
   ta->occ = CreateVector(x,numStates);
   ZeroVector(ta->occ);
  
   return ta;
}

/* CreateWtAcc: create an accumulator for mixture weights */
static WtAcc *CreateWtAcc(MemHeap *x, int nMix)
{
   WtAcc *wa;
   
   wa = (WtAcc *) New(x,sizeof(WtAcc));
   wa->c = CreateSVector(x,nMix);
   ZeroVector(wa->c);
   wa->occ = 0.0;
   wa->time = -1; wa->prob = NULL;
   return wa;
}

/* AttachWtTrAccs: attach weight and transition accumulators to hset */
static void AttachWtTrAccs(HMMSet *hset, MemHeap *x)
{
   HMMScanState hss;
   StreamInfo *sti;
   HLink hmm;
  
   NewHMMScan(hset,&hss);
   do {
      hmm = hss.hmm;
      hmm->hook = (void *)0;  /* used as numEg counter */
      if (!IsSeenV(hmm->transP)) {
         SetHook(hmm->transP, CreateTrAcc(x,hmm->numStates));
         TouchV(hmm->transP);       
      }
      while (GoNextState(&hss,TRUE)) {
         while (GoNextStream(&hss,TRUE)) {
            sti = hss.sti;
            sti->hook = CreateWtAcc(x,hss.M);
         }
      }
   } while (GoNextHMM(&hss));
   EndHMMScan(&hss);
}

/* -------------------- HMM -> HSMM conversion --------------------- */

static void FixTransHSMM (HMMSet *hset)
{
   int i,j;
   HMMScanState hss;
   LogDouble sum;
   SMatrix tr;
   
   /* fix trans prob */
   NewHMMScan(hset,&hss);
   do {
      if (!IsSeenV(hss.hmm->transP)) {
         tr = hss.hmm->transP;
         for (i=1; i<=hss.N; i++) {
            if (tr[i][i]>LSMALL) {
               sum = LZERO;
               for (j=1; j<=hss.N; j++)  
                  if (j!=i && tr[i][j]>LSMALL)
                     sum = LAdd(sum,tr[i][j]);
               for (j=1; j<=hss.N; j++)
                  tr[i][j] = (tr[i][j]>LSMALL) ? tr[i][j]-sum : LZERO;
               tr[i][i] = LZERO;
            }
         }
         TouchV(hss.hmm->transP);
      }
   } while (GoNextHMM(&hss));
   EndHMMScan(&hss);
}

/* -------------------------- Initialisation ----------------------- */

/* EXPORT->InitFB: initialise FB module */
void InitFB(void)
{
   int m;
   int i;
   double d;
   Boolean b;

   Register(hfb_version,hfb_vc_id);

   /* setup the local memory management */
   CreateHeap(&dprobStack,"duration prob store", MSTAK, 1, 1.0, 50000, 500000);
   
   for (m = 0; m < 2; ++m) {
      nParm = GetConfig(m==0 ? "HFWDBKWD" : "HFB", TRUE, cParm, MAXGLOBS);
      if (nParm>0){
         if (GetConfInt(cParm,nParm,"TRACE",&i)) trace = i;
         if (GetConfInt(cParm,nParm,"HSKIPSTART",&i)) skipstartInit = i;
         if (GetConfInt(cParm,nParm,"HSKIPEND",&i)) skipendInit = i;
         if (GetConfInt(cParm,nParm,"MAXSTDDEVCOEF",&i)) maxstdDevCoef = i;
         if (GetConfInt(cParm,nParm,"MINDUR",&i)) minDur = i;
         if (GetConfFlt(cParm,nParm,"PRUNEINIT", &d)) pruneSetting.pruneInit = d;
         if (GetConfFlt(cParm,nParm,"PRUNEINC", &d)) pruneSetting.pruneInc = d;
         if (GetConfFlt(cParm,nParm,"PRUNELIM", &d)) pruneSetting.pruneLim = d;
         if (GetConfFlt(cParm,nParm,"MINFORPROB", &d)) pruneSetting.minFrwdP = d;
         if (GetConfBool(cParm,nParm,"ALIGNCOMPLEVEL",&b)) alCompLevel = b;
         if (GetConfBool(cParm,nParm,"PDE",&b)) pde = b;
      }
   }
}

/* EXPORT->ResetFB: reset FB module */
void ResetFB(void)
{
   ResetHeap(&dprobStack);
   return;   /* do nothing */
}

/* EXPORT->SetTraceFB: Allow tools to enable top-level tracing in HFB. Only here for historical reasons */
void SetTraceFB(void)
{
   trace |= T_TOP;
}


/* EXPORT->InitialiseForBack: IniInitialise the forward backward memory stacks and make initialisations  */
void InitialiseForBack(FBInfo *fbInfo, MemHeap *x, HMMSet *hset, UPDSet uFlags_hmm, HMMSet *dset, UPDSet uFlags_dur, 
                       LogDouble pruneInit, LogDouble pruneInc, LogDouble pruneLim, 
                       float minFrwdP, Boolean useAlign, Boolean genDur)
{
   int s;
   AlphaBeta *ab;
  
   fbInfo->uFlags_hmm = uFlags_hmm;
   fbInfo->uFlags_dur = uFlags_dur;
   fbInfo->up_hset = fbInfo->al_hset = hset;
   fbInfo->up_dset = fbInfo->al_dset = dset;
   fbInfo->twoModels = FALSE;
   fbInfo->hsKind = hset->hsKind;
   /* Accumulators attached using AttachAccs() in HERest are overwritten
      by the following line. This is ugly and needs to be sorted out. Note:
      this function is called by HERest and HVite */
   AttachWtTrAccs(hset, x);
   SetMinDurs(hset);
   fbInfo->maxM = MaxMixInSet(hset);
   fbInfo->skipstart = skipstartInit;
   fbInfo->skipend   = skipendInit;
   fbInfo->useAlign = useAlign;
   fbInfo->inXForm_hmm = fbInfo->al_inXForm_hmm = fbInfo->paXForm_hmm = NULL;
   fbInfo->inXForm_dur = fbInfo->al_inXForm_dur = fbInfo->paXForm_dur = NULL;
   for (s=1;s<=hset->swidth[0];s++)
      fbInfo->maxMixInS[s] = MaxMixInSetS(hset, s);
   fbInfo->ab = (AlphaBeta *) New(x, sizeof(AlphaBeta));
   ab = fbInfo->ab;
   CreateHeap(&ab->abMem,  "AlphaBetaFB",  MSTAK, 1, 1.0, 100000, 5000000);

   if (pruneInit < NOPRUNE) {   /* cmd line takes precedence over config file */
      pruneSetting.pruneInit = pruneInit;
      pruneSetting.pruneInc  = pruneInc;
      pruneSetting.pruneLim  = pruneLim;
   }
   if (minFrwdP < NOPRUNE)
      pruneSetting.minFrwdP  = minFrwdP;

   if (pruneSetting.pruneInit < NOPRUNE) 
      if (pruneSetting.pruneInc != 0.0)
         printf("Pruning-On[%.1f %.1f %.1f]\n", pruneSetting.pruneInit, 
                pruneSetting.pruneInc, pruneSetting.pruneLim);
      else if (fbInfo->useAlign)
         printf("Pruning-On[Alignment]\n");
      else
         printf("Pruning-On[%.1f]\n", pruneSetting.pruneInit);
   else
      printf("Pruning-Off\n");
   if (hset->numSharedMix > 0)
      sharedMix = TRUE;
   if (hset->numSharedStreams > 0)
      sharedStream = TRUE;
   if (dset!=NULL) {
      if ((dset->hsKind!=PLAINHS) && (dset->hsKind!=SHAREDHS))
         HError(7392,"InitialiseForBack: Model kind not supported for duration models");
      if (MaxMixInSet(dset)>1)
         HError(9999,"InitialiseForBack: Only single Gaussian is supported for duration models");
      for (s=1; s<=dset->swidth[0]; s++)
         if (dset->swidth[s]!=1)
            HError(9999,"InitialiseForBack: Stream width must be 1");
      if (!genDur) {
         FixTransHSMM(hset);
         semiMarkov = TRUE;
      }
      AttachWtTrAccs(dset,x);
   }
   if (pde) {
      if (sharedMix)
	 HError(7399,"PDE is not compatible with shared mixtures");
      printf("Partial Distance Elimination on\n");
   }
   
   return;
}

/* Use a different model set for alignment */
void UseAlignHMMSet(FBInfo* fbInfo, MemHeap* x, HMMSet *al_hset, HMMSet *al_dset)
{
   int s,S;

   /* First check 2-model mode allowed for current up_hset */
   if ((fbInfo->hsKind != PLAINHS) && (fbInfo->hsKind != SHAREDHS))
      HError(7392,"Model kind not supported for fixed alignments");

   if (al_hset!=NULL) {
   if (al_hset->hsKind != fbInfo->hsKind)
      HRError(7392,"Different kinds in alignment and update HMM sets!");
   /* check stream compatibility */
   S = al_hset->swidth[0];
   if (S != fbInfo->up_hset->swidth[0])
      HError(7392,"Different num streams in alignment and update HMM sets!");
   for (s=1; s<=S; s++) 
      if (al_hset->swidth[s] != fbInfo->up_hset->swidth[s]) 
         HError(7392,"Stream %d widths differ in alignment and update HMM sets!",s);
   /* check update flags */
      if (fbInfo->uFlags_hmm&UPTRANS) {
      HRError(7392,"Don't update transitions on a 2-model alignment"); 
         fbInfo->uFlags_hmm = (UPDSet) (fbInfo->uFlags_hmm & ~UPTRANS);
   }
   /* check input Xforms */
   if ((al_hset->xf!=NULL) && (al_hset->xf != fbInfo->up_hset->xf))
      HError(7392,"Inconsistent input transforms for align HMMSet");
    
   /* update the global alignment set features */
   fbInfo->hsKind = al_hset->hsKind;      
   fbInfo->maxM = MaxMixInSet(al_hset);
       
   /* dummy accs to accomodate minDir */
   AttachWtTrAccs(al_hset, x);
   SetMinDurs(al_hset);
         
   /* precomps */
   if ( al_hset->hsKind == SHAREDHS)
      AttachPreComps(al_hset,al_hset->hmem);
    
      if (semiMarkov)
         FixTransHSMM(al_hset);
      
   fbInfo->al_hset = al_hset;  
   }

   if (fbInfo->up_dset!=NULL && al_dset!=NULL) {
      S = al_dset->swidth[0];
      if (S != fbInfo->up_dset->swidth[0])
         HError(7392,"UseAlignHMMSet: Different num streams in alignment and update duration model sets!");
      for (s=1; s<=S; s++) 
         if (al_dset->swidth[s] != fbInfo->up_dset->swidth[s]) 
            HError(7392,"UseAlignHMMSet: Stream %d widths differ in alignment and update duration model sets!",s);
      if ((al_dset->hsKind!=PLAINHS) && (al_dset->hsKind!=SHAREDHS))
         HError(7392,"UseAlignHMMSet: Model kind not supported for duration models");
      if (MaxMixInSet(al_dset)>1)
         HError(9999,"UseAlignHMMSet: Only single Gaussian is supported for duration models");
      /* check input Xforms */
      if ((al_dset->xf!=NULL) && (al_dset->xf != fbInfo->up_dset->xf))
         HError(7392,"UseAlignHMMSet: Inconsistent input transforms for align duration model set");
            
      if (fbInfo->uFlags_dur&(UPMEANS|UPVARS)) {
         HRError(7392,"UseAlignHMMSet: Don't update duration models on a 2-model alignment"); 
         fbInfo->uFlags_dur = (UPDSet) 0;
      }
      AttachWtTrAccs(al_dset, x);
      fbInfo->al_dset = al_dset;
   }
   fbInfo->twoModels = TRUE;
}

/* Initialise the utterance memory requirements */
void InitUttInfo( UttInfo *utt, Boolean twoFiles )
{
   CreateHeap(&utt->transStack,"transStore",MSTAK, 1, 0.5, 1000,  10000);
   CreateHeap(&utt->dataStack,"dataStore",MSTAK, 1, 0.5, 1000,  10000);
   if (twoFiles)
      CreateHeap(&utt->dataStack2,"dataStore2",MSTAK, 1, 0.5, 1000,  10000);
   utt->pbuf = NULL; utt->pbuf2 = NULL; utt->tr = NULL;
}

/* InitPruneStats: initialise pruning stats */
static void InitPruneStats(AlphaBeta *ab)
{
   PruneInfo *p;

   ab->pInfo = (PruneInfo *) New(&ab->abMem, sizeof(PruneInfo));
   p = ab->pInfo;
   p->maxBeamWidth = 0;
   p->maxAlphaBeta = LZERO;
   p->minAlphaBeta = 1.0;
}


/* -------------------------- Trace Support ----------------------- */

/* CreateTraceOcc: create the array of acc occ counts */
static void CreateTraceOcc(AlphaBeta *ab, UttInfo *utt)
{
   int q;
   Vector *occa;

   ab->occa=(Vector *)New(&ab->abMem, utt->Q*sizeof(Vector));
   occa = ab->occa;
   --occa;
   for (q=1;q<=utt->Q;q++){
      occa[q] = CreateVector(&ab->abMem, ab->al_qList[q]->numStates);
      ZeroVector(occa[q]);
   }
}

/* TraceOcc: print current accumulated occ counts for all models */
static void TraceOcc(AlphaBeta *ab, UttInfo *utt, int t)
{
   int Nq, q, i;
   Vector occaq;
   HLink hmm;
   float max;

   printf("Accumulated Occ Counts at time %d\n",t);
   for (q=1; q<=utt->Q; q++){
      occaq = ab->occa[q]; hmm = ab->al_qList[q]; Nq = hmm->numStates;
      max = 0.0;        /* ignore zero vectors */
      for (i=1;i<=Nq;i++)
         if (occaq[i]>max) max = occaq[i];
      if (max>0.0) {    /* not zero so print it */
         printf("  Q%2d: %5s", q,ab->qIds[q]->name);
         for (i=1;i<=Nq;i++) printf("%7.2f",occaq[i]);
         printf("\n");
      }
   }
}

/* SetOcct: set the global occupation count for given hmm */
static void SetOcct(HLink hmm, int q, Vector occt, Vector *occa, int *maxDur,
                    DVector *aqt, DVector *bqt, DVector *bq1t, LogDouble pr)
{
   int i,N,d;
   double x,y;
   Vector occaq;
   
   N=hmm->numStates;
   y=0.0;
   for (i=1;i<=N;i++) {
      x = LZERO;
      for (d=1; d<=maxDur[i]; d++)
         if (aqt[i][d]>LSMALL && bqt[i][d]>LSMALL)
            x = LAdd(x,aqt[i][d]+bqt[i][d]);
      if (i==1 && bq1t != NULL && ApplyDAEM(hmm->transP[1][N]) > LSMALL)
	 x = LAdd(x,aqt[1][1]+bq1t[1][1]+ApplyDAEM(hmm->transP[1][N]));
      x -= pr;
      occt[i] = (x>MINEARG) ? exp(x) : 0.0;
   }
   if (trace&T_OCC) {
      occaq = occa[q];
      for (i=1;i<=N;i++) occaq[i] += occt[i];
   }
}


/* NonSkipRegion: returns true if t is not in the skip region */
static Boolean NonSkipRegion(int skipstart, int skipend, int t)
{
   if (skipstart<1 || t<skipstart || t>skipend)
      return TRUE;
   else
      return FALSE;
}

/* PrLog: print a log value */
void PrLog(LogDouble x)
{
   if (x<LSMALL)
      printf("       LZERO");
   else
      printf("%12.5f",x);
}



/* -------------------------------------------------------------------*/

/* GetInputObs: Get input Observations for t */
void GetInputObs( UttInfo *utt, int t, HSetKind hsKind )
{

   if (utt->twoDataFiles)
      ReadAsTable(utt->pbuf2,t-1,&(utt->o2[t]));
   ReadAsTable(utt->pbuf,t-1,&(utt->o[t]));

   if (hsKind == TIEDHS)
      if (utt->twoDataFiles)
         ReadAsTable(utt->pbuf,t-1,&(utt->o2[t]));

}

/* --------------------------- Forward-Backward --------------------- */



/* CheckPruning: record peak alpha.beta product and position */
static void CheckPruning(AlphaBeta *ab, int t, int skipstart, int skipend)
{
   int i,q,Nq,bestq,besti,margin,d,*maxDur;
   PruneInfo *p;
   DVector *aq,*bq;
   HLink hmm;
   LogDouble lx,maxL;

   bestq = besti = 0;
   maxL = LZERO;
   p = ab->pInfo;
   for (q=p->qLo[t];q<=p->qHi[t];q++){
      hmm = ab->al_qList[q]; Nq = hmm->numStates;   
      aq = ab->alphat[q]; bq=ab->beta[t][q];
      maxDur = ab->maxDur[q];
      for (i=2;i<Nq;i++){
         lx = LZERO;
         for (d=1; d<=maxDur[i]; d++)
            if (aq[i][d]>LSMALL && bq[i][d]>LSMALL)
               lx = LAdd(lx,aq[i][d]+bq[i][d]);
         if (lx>maxL){
            bestq = q; besti = i; maxL=lx;
         }
      }
   }
   if (maxL > p->maxAlphaBeta) p->maxAlphaBeta = maxL;
   if (maxL < p->minAlphaBeta) p->minAlphaBeta = maxL;
   margin = p->qHi[t] - p->qLo[t]+1;
   if (margin>p->maxBeamWidth) p->maxBeamWidth = margin;
   if (NonSkipRegion(skipstart, skipend, t)){
      if (bestq == 0) 
         printf("%3d. No max found in alpha.beta\n",t);
      else
         printf("%3d. Max Alpha.Beta = %9.4f at q=%d i=%d [%s]\n",
                t,maxL,bestq,besti,ab->qIds[bestq]->name);
   }
}

/* SummarisePruning: print out pruning stats */
static void SummarisePruning(PruneInfo *p, int Q, int T)
{
   long totalQ=0;
   float e;
   int t;
   
   for (t=1;t<=T;t++)
      totalQ += p->qHi[t]-p->qLo[t]+1;
   e = (1.0 - (float) totalQ / ((float) T*Q)) * 100.0;
   printf(" Pruning %.1f%%; MaxBeamWidth %d; PeakShortFall %.2f\n",
          e,p->maxBeamWidth,p->maxAlphaBeta - p->minAlphaBeta);
   fflush(stdout);
}

/* CreateInsts: create array of hmm instances for current transcription */
static int CreateInsts(FBInfo *fbInfo, AlphaBeta *ab, int Q, Transcription *tr)
{
   int q,qt;
   LLink lab;
   MLink macroName;
   TrAcc *ta;
   LabId  *qIds;
   short *qDms;
   HLink *al_qList=NULL, *up_qList=NULL, *al_dList=NULL, *up_dList=NULL;
   HMMSet *al_hset, *up_hset;
   HMMSet *al_dset, *up_dset;

   al_hset=fbInfo->al_hset; up_hset=fbInfo->up_hset;
   al_dset=fbInfo->al_dset; up_dset=fbInfo->up_dset;

   /* init logical hmm & dur model list */
   up_qList=(HLink *)New(&ab->abMem, Q*sizeof(HLink)); 
   --up_qList;
   if (up_dset!=NULL) {
      up_dList=(HLink *)New(&ab->abMem, Q*sizeof(HLink)); 
      --up_dList;
   }

   /* 2-model re-estimation update models */
   if (fbInfo->twoModels) {
      al_qList=(HLink *)New(&ab->abMem, Q*sizeof(HLink));
      --al_qList;
      if (al_dset!=NULL) {
         al_dList=(HLink *)New(&ab->abMem, Q*sizeof(HLink));
         --al_dList;
      }
   }
   else {
      /* use same list for update and align */
      al_qList = up_qList;
      al_dList = up_dList;
   }

   qIds = (LabId *)New(&ab->abMem, Q*sizeof(LabId));
   --qIds;
   qDms = (short *)New(&ab->abMem, Q*sizeof(short));
   --qDms;

   qt=0;
   for (lab=tr->head->head->succ,q=1; lab->succ!= NULL; lab=lab->succ,q++){
      /* align models */
      if((macroName=FindMacroName(al_hset,'l',lab->labid))==NULL)
         HError(7321,"CreateInsts: Unknown label %s",lab->labid->name);
      al_qList[q] = (HLink)macroName->structure;
      if (al_dset!=NULL) {
         if ((macroName=FindMacroName(al_dset,'l',lab->labid))==NULL)
            HError(7321,"CreateInsts: Unknown label %s",lab->labid->name);
         al_dList[q] = (HLink)macroName->structure;
      }

      /* 2-model re-estimation update models */
      if (fbInfo->twoModels){ 
         if((macroName=FindMacroName(up_hset,'l',lab->labid))==NULL)
            HError(2321,"CreateInsts: Unknown update label %s",lab->labid->name);
         up_qList[q] = (HLink)macroName->structure;
         if (up_dset!=NULL) {
            if ((macroName=FindMacroName(up_dset,'l',lab->labid))==NULL)
               HError(2321,"CreateInsts: Unknown update label %s",lab->labid->name);
            up_dList[q] = (HLink)macroName->structure;
         }
         /* need equal num states */
         if ((al_qList[q])->numStates != (up_qList[q])->numStates)
            HError(999,"Num states differ in align and update models (%d %d)",
                   (al_qList[q])->numStates,(up_qList[q])->numStates);      
      }
      qIds[q] = macroName->id;
      ta = (TrAcc *)GetHook(al_qList[q]->transP);
      qt += (qDms[q] = ta->minDur);

      if (q>1 && qDms[q]==0 && qDms[q-1]==0)
         HError(7332,"CreateInsts: Cannot have successive Tee models");
      if (al_hset->hsKind==SHAREDHS)
         ResetHMMPreComps(al_qList[q],al_hset->swidth[0]);
      else if (al_hset->hsKind==PLAINHS)
         ResetHMMWtAccs(al_qList[q],al_hset->swidth[0]);         
   }
   if ((qDms[1]==0)||(qDms[Q]==0))
      HError(7332,"CreateInsts: Cannot have Tee models at start or end of transcription");

   /* store in struct*/
   ab->al_qList = al_qList; 
   ab->up_qList = up_qList;
   ab->al_dList = al_dList;
   ab->up_dList = up_dList;
   ab->qIds  = qIds;
   ab->qDms  = qDms;

   return(qt);
}

/* CreateAlpha: allocate alpha columns */
static void CreateAlpha(AlphaBeta *ab, HMMSet *hset, int Q, int T)
{
   int q,Nq,i;
   DVector **alphat, **alphat1;
 
   /* Create Storage Space - two columns */
   alphat = (DVector **)New(&ab->abMem, Q*sizeof(DVector *));
   --alphat;
   for (q=1;q<=Q;q++) {
      Nq = (ab->al_qList[q])->numStates;
      alphat[q] = (DVector *)New(&ab->abMem, Nq*sizeof(DVector));
      --alphat[q];
      for (i=1; i<=Nq; i++)
         alphat[q][i] = CreateDVector(&ab->abMem, ab->maxDur[q][i]);
   }

   alphat1=(DVector **)New(&ab->abMem, Q*sizeof(DVector *));
   --alphat1;
   for (q=1;q<=Q;q++) {
      Nq = (ab->al_qList[q])->numStates;
      alphat1[q] = (DVector *)New(&ab->abMem, Nq*sizeof(DVector));
      --alphat1[q];
      for (i=1; i<=Nq; i++)
         alphat1[q][i] = CreateDVector(&ab->abMem, ab->maxDur[q][i]);
   }

   ab->occt = CreateVector(&ab->abMem,MaxStatesInSet(hset));
   ab->alphat  = alphat;
   ab->alphat1 = alphat1;
}


/* ZeroAlpha: zero alpha's of given models */
static void ZeroAlpha(AlphaBeta *ab, int qlo, int qhi)
{
   HLink hmm;
   int Nq,j,q,d,maxDur;
   DVector aqj;
   
   for (q=qlo;q<=qhi;q++) {   
      hmm = ab->al_qList[q]; 
      Nq = hmm->numStates; 
      for (j=1; j<=Nq; j++) {
         aqj = ab->alphat[q][j];  maxDur = ab->maxDur[q][j];
         for (d=1; d<=maxDur; d++)
            aqj[d] = LZERO;
      }
   }
}

/* InitAlpha: initialise alpha columns for time t=1 */
static void InitAlpha(AlphaBeta *ab, int *start, int *end, 
                      int Q, int skipstart, int skipend)
{
   int i,j,Nq,eq,q,d,*maxDur;
   PruneInfo *p;
   HLink hmm;
   DVector *aq;
   float ***outprob;
   SVector *durprob;
   LogDouble x,a,a1N=0.0;
   
   p = ab->pInfo;
   eq = p->qHi[1];
   for (q=1; q<=eq; q++){
      hmm = ab->al_qList[q]; Nq = hmm->numStates;
      aq = ab->alphat[q];
      durprob = ab->durprob[q];
      maxDur  = ab->maxDur [q];
      aq[1][1] = (q==1) ? 0.0 : ab->alphat[q-1][1][1]+a1N;
      if((outprob = ab->otprob[1][q]) == NULL)
         HError(7322,"InitAlpha: Outprob NULL in model %d in InitAlpha",q);
      for (j=2;j<Nq;j++) {
	 a = ApplyDAEM(hmm->transP[1][j]);
         aq[j][1] = (a>LSMALL) ? aq[1][1]+a+outprob[j][0][0] : LZERO;
         for (d=2; d<=maxDur[j]; d++)
            aq[j][d] = LZERO;
      }
      x = LZERO;
      for (i=2;i<Nq;i++) {
	 a = ApplyDAEM(hmm->transP[i][Nq]);
         if (a>LSMALL)
            x = LAdd(x,aq[i][1]+durprob[i][1]+a);
      }
      aq[Nq][1] = x;
      a1N = ApplyDAEM(hmm->transP[1][Nq]);
   }
   ZeroAlpha(ab,eq+1,Q);
   if (trace&T_PRU && p->pruneThresh < NOPRUNE)
      CheckPruning(ab,1,skipstart,skipend);
   *start = 1; *end = eq;
}

/* MaxModelProb: Calc max probability of being in model q at
   time t, return LZERO if cannot do so */
static LogDouble MaxModelProb(AlphaBeta *ab, int q, int t, int minq)
{
   DVector *aq,*bq,*bq1;
   LogDouble maxP,x;
   int Nq1,Nq,d,i,qx,qx1,*maxDur;
   HLink hmm;
   
   if (q==1)
      maxP = LZERO;
   else {
      bq1 = ab->beta[t][q-1]; Nq1 = ab->al_qList[q-1]->numStates;
      maxP = (bq1==NULL) ? LZERO : ab->alphat[q-1][Nq1][1] + bq1[Nq1][1];
      for (qx=q-1;qx>minq && ab->al_qList[qx]->transP[1][Nq1] > LSMALL;qx--){
         qx1 = qx-1;
         bq1 = ab->beta[t][qx1]; Nq1 = ab->al_qList[qx1]->numStates;
         x = (bq1==NULL) ? LZERO : ab->alphat[qx1][Nq1][1]+bq1[Nq1][1];
         if (x > maxP) maxP = x;
      }
   }
   hmm = ab->al_qList[q]; Nq = hmm->numStates;   
   bq=ab->beta[t][q];
   if (bq != NULL) {
      aq = ab->alphat[q]; 
      maxDur = ab->maxDur[q];
      for (i=1;i<Nq;i++) {
         x = LZERO;
         for (d=1; d<=maxDur[i]; d++)
            if (aq[i][d]>LSMALL && bq[i][d]>LSMALL)
               x = LAdd(x,aq[i][d]+bq[i][d]);
         if (x > maxP) maxP = x;
      }
   }
   return maxP;
}

/* StepAlpha: calculate alphat column for time t and return
   forward beam limits in startq and endq */
static void StepAlpha(AlphaBeta *ab, int t, int *start, int *end, 
                      int Q, int T, LogDouble pr, int skipstart, int skipend)
{
   DVector *aq,*laq,**tmp, **alphat,**alphat1;
   PruneInfo *p;
   float ***outprob;
   SVector *durprob;
   int sq,eq,i,j,d,q,Nq,lNq,*maxDur;
   LogDouble x=0.0,y,a,a1N=0.0;
   HLink hmm;
   
   alphat  = ab->alphat;
   alphat1 = ab->alphat1;

   /* First prune beta beam further to get alpha beam */
   p = ab->pInfo;
   sq = p->qLo[t-1];    /* start start-point at bottom of beta beam at t-1 */

   while (pr-MaxModelProb(ab,sq,t-1,sq)>pruneSetting.minFrwdP){
      ++sq;                /* raise start point */
      if (sq>p->qHi[t]) 
         HError(7390,"StepAlpha: Alpha prune failed sq(%d) > qHi(%d) at time %d",sq,p->qHi[t],t);
   }
   if (sq<p->qLo[t])       /* start-point below beta beam so pull it back */
      sq = p->qLo[t];
   
   eq = (p->qHi[t-1]<Q) ? p->qHi[t-1]+1 : p->qHi[t-1];
   /* start end-point at top of beta beam at t-1  */
   /* JJO : + 1 to allow for state q-1[N] -> q[1] */
   /*       + 1 for each tee model following eq.  */
   while (pr-MaxModelProb(ab,eq,t-1,sq)>pruneSetting.minFrwdP){
      --eq;             /* lower end-point */
      if (eq<sq) 
         HError(7390,"StepAlpha: Alpha prune failed eq(%d) < sq(%d) at time %d",eq,sq,t);
   }
   while (eq<Q && ab->qDms[eq]==0) eq++;
   if (eq>p->qHi[t])  /* end point above beta beam so pull it back */
      eq = p->qHi[t]; 
      
   if (trace&T_PRU && NonSkipRegion(skipstart,skipend,t)){
      printf("%d: Alpha Beam %d->%d \n",t,sq,eq);
      fflush(stdout);
   } 
   
   /* Now compute current alpha column */
   tmp = ab->alphat1; ab->alphat1 = ab->alphat; ab->alphat = tmp;
   alphat  = ab->alphat;
   alphat1 = ab->alphat1;

   if (sq>1) ZeroAlpha(ab,1,sq-1);

   Nq = (sq == 1) ? 0:ab->al_qList[sq-1]->numStates;

   for (q = sq; q <= eq; q++) {
      lNq = Nq; hmm = ab->al_qList[q]; Nq = hmm->numStates; 
      aq = alphat[q]; 
      laq = alphat1[q];
      durprob = ab->durprob[q];
      maxDur  = ab->maxDur[q];
      if (laq == NULL)
         HError(7322,"StepAlpha: laq gone wrong!");
      if((outprob = ab->otprob[t][q]) == NULL)
         HError(7322,"StepAlpha: Outprob NULL at time %d model %d in StepAlpha",t,q);
      if (q==1)
         aq[1][1] = LZERO;
      else{
         aq[1][1] = alphat1[q-1][lNq][1];
         if (q>sq && a1N>LSMALL) /* tee Model */
            aq[1][1] = LAdd(aq[1][1], alphat[q-1][1][1]+a1N);
      }
      for (j=2;j<Nq;j++) {
	 a = ApplyDAEM(hmm->transP[1][j]);
         x = (a>LSMALL) ? aq[1][1]+a : LZERO;
         for (i=2;i<Nq;i++){
	    a = ApplyDAEM(hmm->transP[i][j]);
            if (a>LSMALL)
               for (d=1; d<=maxDur[i]; d++) {
                  y = laq[i][d];
                  if (y>LSMALL)
                     x = LAdd(x,y+durprob[i][d]+a);
               }
         }
         aq[j][1] = x + outprob[j][0][0];

         for (d=2; d<=maxDur[j]; d++)
            aq[j][d] = laq[j][d-1] + outprob[j][0][0];
      }
      x = LZERO;
      for (i=2;i<Nq;i++){
	a = ApplyDAEM(hmm->transP[i][Nq]);
         if (a>LSMALL)
            for (d=1; d<=maxDur[i]; d++) {
               y = aq[i][d];
               if (y>LSMALL)
                  x = LAdd(x,y+durprob[i][d]+a);
      }
      }
      aq[Nq][1] = x; a1N = ApplyDAEM(hmm->transP[1][Nq]);
   }
   if (eq<Q) ZeroAlpha(ab,eq+1,Q);

   if (trace&T_PRU && p->pruneThresh < NOPRUNE)
      CheckPruning(ab,t,skipstart,skipend);
   if (t==T){
      if (fabs((x-pr)/T) > 0.001)
         HError(-7391,"StepAlpha: Forward/Backward Disagree %f/%f",x,pr);
      if (trace&T_PRU && p->pruneThresh < NOPRUNE) 
         SummarisePruning(p, Q, T);
   }

   *start=sq; *end=eq;

}


/* CreateBeta: create Q and T pointer arrays for beta */
static void CreateBeta(AlphaBeta *ab, int T)
{
   int t;
   PruneInfo *p;
   DVector ***beta;

   p = ab->pInfo;
   p->qHi = CreateShortVec(&ab->abMem, T); /* storage for min and max q vals */
   p->qLo = CreateShortVec(&ab->abMem, T);
   beta=(DVector ***)New(&ab->abMem, T*sizeof(DVector **));
   --beta;
   for (t=1;t<=T;t++){
      beta[t] = NULL;
   }

   ab->beta = beta;
}

/* CreateBetaQ: column of DVectors covering current beam */
static DVector **CreateBetaQ (MemHeap *x, int qLo, int qHi, int Q)
{
   int q;
   DVector **v;

   qLo--; qLo--;  if (qLo<1) qLo=1;
   qHi++;  if (qHi>Q) qHi=Q;
   v = (DVector **)New(x, (qHi-qLo+1)*sizeof(DVector *));
   v -= qLo;
   for (q=qLo;q<=qHi;q++) v[q] = NULL;
   return(v);
}
  

/* CreateOtprob: create T pointer arrays for Otprob */
static void CreateOtprob(AlphaBeta *ab, int T)
{
   int t;
   float *****otprob;
   
   otprob=(float *****)New(&ab->abMem, T*sizeof(float ****));
   --otprob;
   for (t=1;t<=T;t++){
      otprob[t] = NULL;
   }
   
   ab->otprob = otprob;

}

/* CreateOqprob: create Q pointer arrays for Otprob */
static float ****CreateOqprob(MemHeap *x, int qLo,int qHi)
{
   int q;
   float ****v;
   
   v=(float ****)New(x, (qHi-qLo+1)*sizeof(float ***));
   v-=qLo;
   for (q=qLo;q<=qHi;q++) v[q] = NULL;
   return(v);
}

/* NewBetaVec: create prob vector size 1..N */
static DVector *NewBetaVec(MemHeap *x, int N, int *maxDur)
{
   int n;
   DVector *v;
   
   v=(DVector *)New(x, N*sizeof(double));
   --v;
   for (n=1; n<=N; n++)
      v[n] = CreateDVector(x, maxDur[n]);

   return v;
}

/* CreateOjsprob: create [2..N-1][0..S] arrays for Otprob */
static float *** CreateOjsprob(MemHeap *x, int N, int S)
{
   float ***v;
   int SS,j,s;
   
   SS=(S==1)?1:S+1;
   v=(float ***)New(x, (N-2)*sizeof(float **));
   v -= 2;
   for (j=2;j<N;j++) {
      v[j]=(float **)New(x, SS*sizeof(float *));
      if (S==1)
	 v[j][0] = NULL;
      else {
	 v[j][0] = (float *)New(x, sizeof(float));
	 v[j][0][0] = LZERO;
	 for (s=1;s<SS;s++)
	    v[j][s] = NULL;
      }
   }
   return v;
}

/* NewOtprobVec: create prob vector size [0..M] */
static float * NewOtprobVec(MemHeap *x, int M)
{
   float *v;
   int MM,m;

   MM=(M==1)?1:M+1;
   v=(float *)New(x, MM*sizeof(float));
   v[0]=LZERO;
   if (M>1)
      for (m=1;m<=M;m++)
	 v[m] = LZERO;
   return v;
}

/* ShStrP: Stream Outp calculation exploiting sharing */
static float * ShStrP(HMMSet *hset, StreamInfo *sti, Vector v, const int t, 
		       AdaptXForm *xform, MemHeap *abmem)
{
   WtAcc *wa;
   MixtureElem *me;
   MixPDF *mp;
   float *outprobjs;
   int m,M;
   PreComp *pMix;
   LogFloat det,x,mixp,wt=0.0;
   Vector otvs;
   
   wa = (WtAcc *)sti->hook;
   if (wa->time==t)           /* seen this state before */
      outprobjs = wa->prob;
   else {
      M = sti->nMix;
      outprobjs = NewOtprobVec(abmem,M);
      me = sti->spdf.cpdf+1;
      if (M==1){                 /* Single Mix Case */
         mp = me->mpdf;
         pMix = (PreComp *)mp->hook;
         if ((pMix != NULL) && (pMix->time == t))
            x = pMix->prob;
         else {
            x = MOutP(ApplyCompFXForm(mp,v,xform,&det,t),mp);	 
            x += det;
            if (pMix != NULL) {
               pMix->prob = x; pMix->time = t;
            }
         }
         x = ApplyDAEM(x);
      } else if (sharedMix) { /* Multiple Mixture Case - general case */
         x = LZERO;
         for (m=1;m<=M;m++,me++) {
            wt = MixLogWeight(hset,me->weight);
            if (wt>LMINMIX){
               mp = me->mpdf;
               pMix = (PreComp *)mp->hook;
               if ((pMix != NULL) && (pMix->time == t))
                  mixp = pMix->prob;
               else {
                  mixp = MOutP(ApplyCompFXForm(mp,v,xform,&det,t),mp);
		  mixp += det;
                  if (pMix != NULL) {
                     pMix->prob = mixp; pMix->time = t;
                  }
               }
               mixp = ApplyDAEM(mixp);
               wt   = ApplyDAEM(wt);
               if (mixp>LSMALL && wt>LSMALL)
               x = LAdd(x,wt+mixp);
	       outprobjs[m] = mixp;
            }
         }
      } else if (!pde) { /* Multiple Mixture Case - no shared mix case */
         x = LZERO;
         for (m=1;m<=M;m++,me++) {
            wt = MixLogWeight(hset,me->weight);
            if (wt>LMINMIX){
               mp = me->mpdf;
	       mixp = MOutP(ApplyCompFXForm(mp,v,xform,&det,t),mp);
	       mixp += det;
               mixp = ApplyDAEM(mixp);
               wt   = ApplyDAEM(wt);
               if (mixp>LSMALL && wt>LSMALL)
               x = LAdd(x,wt+mixp);
	       outprobjs[m] = mixp;
            }
         }
      } else {    /* Partial distance elimination */
	 /* first Gaussian computed exactly in PDE */
	 wt = MixLogWeight(hset,me->weight);
	 mp = me->mpdf;
	 otvs = ApplyCompFXForm(mp,v,xform,&det,t);
	 mixp = IDOutP(otvs,VectorSize(otvs),mp); /* INVDIAGC assumed */
	 mixp += det;
         mixp = ApplyDAEM(mixp);
         wt   = ApplyDAEM(wt);
	 x = wt+mixp;
	 outprobjs[1] = mixp;
	 for (m=2,me=sti->spdf.cpdf+2;m<=M;m++,me++) {
            wt = MixLogWeight(hset,me->weight);
	    if (wt>LMINMIX){
	       mp = me->mpdf;
	       otvs = ApplyCompFXForm(mp,v,xform,&det,t);
	       if (PDEMOutP(otvs,mp,&mixp,x-wt-det) == TRUE) {
		  mixp += det;
                  mixp = ApplyDAEM(mixp);
                  wt   = ApplyDAEM(wt);
                  if (mixp>LSMALL && wt>LSMALL)
		  x = LAdd(x,wt+mixp);
	       }
	       outprobjs[m] = mixp; /* LZERO if PDEMOutP returns FALSE */
	    }
	 }
      }
      outprobjs[0] = x;
      wa->prob = outprobjs;
      wa->time = t;
   }
   return outprobjs;
}
   
/* Setotprob: allocate and calculate otprob matrix at time t */
static void Setotprob(AlphaBeta *ab, FBInfo *fbInfo, UttInfo *utt, int t, int S, int qHi, int qLo)
{
   int q,j,Nq,s;
   float ***outprob, **outprobj, *****otprob;
   StateInfo *si;
   StreamElem *ste;
   StreamInfo *sti;
   HLink hmm;
   LogFloat sum;
   PruneInfo *p;
   int skipstart, skipend;
   HMMSet *hset;
   Boolean seenStr=FALSE;
   
   hset = fbInfo->al_hset;
   skipstart = fbInfo->skipstart;
   skipend = fbInfo->skipend;
   p = ab->pInfo;
   otprob = ab->otprob;
   if (hset->hsKind == TIEDHS)
      PrecomputeTMix(hset,&utt->o[t],pruneSetting.minFrwdP,0);
   if (trace&T_OUT && NonSkipRegion(skipstart,skipend,t)) 
      printf(" Output Probs at time %d\n",t);
   if (qLo>1) --qLo;
   otprob[t] = CreateOqprob(&ab->abMem,qLo,qHi);
   for (q=qHi;q>=qLo;q--) {
      if (trace&T_OUT && NonSkipRegion(skipstart,skipend,t)) 
         printf(" Q%2d: ",q);
      hmm = ab->al_qList[q]; Nq = hmm->numStates;
      if (otprob[t][q] == NULL)
         {
            outprob = otprob[t][q] = CreateOjsprob(&ab->abMem,Nq,S);
            for (j=2;j<Nq;j++){
               si=hmm->svec[j].info;
               ste=si->pdf+1; sum = 0.0;
               outprobj = outprob[j];
               for (s=1;s<=S;s++,ste++){
                  sti = ste->info;
                  switch (hset->hsKind){
                  case TIEDHS:  /* SOutP deals with tied mix calculation */
                  case DISCRETEHS:
                     if (S==1) {
                        outprobj[0] = NewOtprobVec(&ab->abMem,1);
                        outprobj[0][0] = SOutP(hset,s,&utt->o[t],sti);
                     } else {
                        outprobj[s] = NewOtprobVec(&ab->abMem,1);
                        outprobj[s][0] = SOutP(hset,s,&utt->o[t],sti);
                     }
		     break;
    /* Check that PLAINHS is handled correctly this  way - efficient? */
                  case PLAINHS:  
                  case SHAREDHS: 
		     if (S==1)
                        outprobj[0] = ShStrP(hset,sti,utt->o[t].fv[s],t,fbInfo->al_inXForm_hmm,&ab->abMem);
		     else {
                        if (((WtAcc *)sti->hook)->time==t) seenStr=TRUE;
                        else seenStr=FALSE;
                        outprobj[s] = ShStrP(hset,sti,utt->o[t].fv[s],t,fbInfo->al_inXForm_hmm,&ab->abMem);
                     }
		    break;
                  default:
                     if (S==1)
		        outprobj[0] = NULL; 
		     else
		        outprobj[s] = NULL; 
                  }
                  if (S>1)
                     sum += si->weights[s] * outprobj[s][0];
               }
               if (S>1){
                     outprobj[0][0] = sum;
               }
               if (trace&T_OUT && NonSkipRegion(skipstart,skipend,t)) {
                  printf(" %d. ",j); PrLog(outprobj[0][0]);
                  if (S>1){
                     printf("[ ");
                     for (s=1; s<=S; s++) PrLog(outprobj[0][0] - si->weights[s]*outprobj[s][0]);
                     printf("]");
                  }
               }
            }
         }
      if (trace&T_OUT && NonSkipRegion(skipstart,skipend,t)) 
         printf("\n");
   }
}

/* EXPORT->ResetDMMPreComps: reset all pre-calculated durprob matrices */
void ResetDMMPreComps (HMMSet *dset)
{
   HMMScanState hss;
   WtAcc *wa;
   
   /* reset dprob hooks */
   NewHMMScan(dset,&hss);
   while (GoNextStream(&hss,FALSE)) {
      wa = (WtAcc *)hss.sti->hook;
      SetHook(wa->c,NULL);
   }
   EndHMMScan(&hss);
   
   /* reset all dprob matrices */ 
   ResetHeap(&dprobStack);
}

/* Setdurprob: allocate and calculate durprob matrix */
static void Setdurprob(AlphaBeta *ab, FBInfo *fbInfo, UttInfo *utt)
{
   int q,j,Nq,d,maxDur;
   double var=0.0;
   float stw;
   LogFloat det;
   Vector dur;
   SVector dprob;
   StateInfo *si;
   StreamInfo *sti;
   MixPDF *mp;
   WtAcc *wa;
   
   /* vector to be used to calculate duration prob */
   dur = CreateVector(&gstack,1);
   
   /* duration prob and max # of durations */
   ab->durprob = (SVector **) New(&ab->abMem,utt->Q*sizeof(SVector *));
   ab->maxDur  = (int **)     New(&ab->abMem,utt->Q*sizeof(int *));
   ab->durprob--;  ab->maxDur--;

   for (q=1; q<=utt->Q; q++) {
      Nq = ab->al_qList[q]->numStates;
      ab->durprob[q] = (SVector *) New(&ab->abMem, Nq*sizeof(SVector));
      ab->maxDur [q] = (int *)     New(&ab->abMem, Nq*sizeof(int));
      ab->durprob[q]--;  ab->maxDur[q]--;
      si = (semiMarkov) ? ab->al_dList[q]->svec[2].info : NULL;
      
      for (j=1; j<=Nq; j++) {
         if (!semiMarkov || j==1 || j==Nq) {
            ab->maxDur [q][j]    = 1;
            ab->durprob[q][j]    = CreateSVector(&ab->abMem, 1);
            ab->durprob[q][j][1] = 0.0;
         }
         else {
            /* currently only single Gaussian is supported */
            sti = ab->al_dList[q]->svec[2].info->pdf[j-1].info;
            stw = ab->al_dList[q]->svec[2].info->weights[j-1];   /* stream weight */
            wa = (WtAcc *)sti->hook;
            if (GetHook(wa->c)==NULL) {
               /* calculate max duration */
               mp = sti->spdf.cpdf[1].mpdf;
               ApplyCompXForm(mp, fbInfo->al_inXForm_dur, TRUE);  /* transform mp in full form */
               switch(mp->ckind) {
               case DIAGC:    var = mp->cov.var[1]; break;
               case INVDIAGC: var = 1.0 / mp->cov.var[1]; break;
               case FULLC:    var = 1.0 / mp->cov.inv[1][1]; break;
               }
               maxDur = (int)(mp->mean[1] + maxstdDevCoef*sqrt(var) + 0.5);
               maxDur = (maxDur<minDur) ? minDur : maxDur;
               ApplyCompXForm(mp, fbInfo->al_inXForm_dur, fbInfo->xfinfo_dur->inFullC);
               
               /* compute duration prob */
               dprob  = CreateSVector(&dprobStack,maxDur);
               for (d=1; d<=maxDur; d++) {
                  dur[1] = (float)d;
                  dprob[d] = MOutP(ApplyCompFXForm(mp,dur,fbInfo->al_inXForm_dur,&det,d),mp);
                  dprob[d] += det;
                  dprob[d] = ApplyDAEM(dprob[d]);
               }
               if (stw!=1.0) /* multiply stream weight */
                  for (d=1; d<=maxDur; d++) dprob[d] *= stw;
               SetHook(dprob,(Ptr)((long)(maxDur)));
               SetHook(wa->c,dprob);
            }
            else {
               dprob = (SVector) GetHook(wa->c);
               maxDur = (int)((long)GetHook(dprob));
            }
            ab->maxDur [q][j] = maxDur;
            ab->durprob[q][j] = dprob;
         }
      }
   }
   
   FreeVector(&gstack,dur);
}

/* TraceAlphaBeta: print alpha/beta values at time t, also sum
         alpha/beta product across states at t-, t, and t+ */
static void TraceAlphaBeta(AlphaBeta *ab, int t, int startq, int endq, LogDouble pr)
{
   int i,q,Nq,d,*maxDur;
   DVector *aqt,*bqt;
   HLink hmm;
   double summ,sump,sum;
   
   printf("Alpha/Betas at time %d\n",t);
   summ = sump = sum = LZERO;
   for (q=startq; q<=endq; q++) {
      hmm = ab->al_qList[q]; Nq = hmm->numStates;
      printf("  Q%2d: %5s           alpha             beta\n",
             q,ab->qIds[q]->name);
      aqt = ab->alphat[q]; bqt = ab->beta[t][q];
      maxDur = ab->maxDur[q];
      for (i=1;i<=Nq;i++){
         for (d=1; d<=maxDur[i]; d++) {
            if (!semiMarkov) {
               printf("                "); PrLog(aqt[i][d]);
               printf("     ");        PrLog(bqt[i][d]);
            }
            else { 
               printf("   d%5d       ",d); PrLog(aqt[i][d]);
               printf("     ");        PrLog(bqt[i][d]);
            }
         printf("\n");
      }
      }
      summ = LAdd(summ,aqt[1][1]+bqt[1][1]);
      for (i=2;i<Nq;i++)
         for (d=1; d<=maxDur[i]; d++)
            if (aqt[i][d]>LSMALL && bqt[i][d]>LSMALL)
               sum = LAdd(sum,aqt[i][d]+bqt[i][d]);
      sump = LAdd(sump,aqt[Nq][1]+bqt[Nq][1]);
   }
   printf("  Sums of Products:  "); PrLog(summ-pr);
   printf("(-)   "); PrLog(sum-pr); 
   printf("    ");   PrLog(sump-pr);
   printf("(+)\n");
}
         
/* SetBeamTaper: set beam start and end points according to the minimum
           duration of the models in the current sequence */
static void SetBeamTaper(PruneInfo *p, short *qDms, int Q, int T)
{
   int q,dq,i,t;
   
   /* Set leading taper */
   q=1;dq=qDms[q];i=0;
   for (t=1;t<=T;t++) {
      while (i==dq) {
         i=0;
         if (q<Q) q++,dq=qDms[q];
         else dq=-1;
      }
      p->qHi[t]=q;
      i++;
   }
   q=Q;dq=qDms[q];i=0;
   for (t=T;t>=1;t--) {
      while (i==dq) {
         i=0;
         if (q>1) q--,dq=qDms[q];
         else dq=-1;
      }
      p->qLo[t]=q;
      i++;
   }
   /*    if (trace>1) 
         for (t=1;t<=T;t++)
         printf("%d: %d to %d\n",t,p->qLo[t],p->qHi[t]);
         exit(1);*/
}


/* SetAlign: set start and end frame according to given label */
void SetAlign (long *st, long *en, short *qDms, const int Q, const int T)
{
   int q, i, mindur;

   /* set segment boundaries which can be determined from given label */
   st[1] = 1;
   en[Q] = T;
   for (q=1; q<=Q; q++) {
      if (q>1 && st[q]<=0 && en[q-1]>0) st[q] = en[q-1]+1;
      if (q<Q && en[q]<=0 && st[q+1]>0) en[q] = st[q+1]-1;

      if (q>1 && st[q]>0 && en[q-1]>0 && st[q]-en[q-1]>1)
         st[q] = en[q-1] = (st[q]+en[q-1])/2; 
   }
   
   /* set segment boundaries which can be determined from given label and #HMM states */
   /* set segment end and start frames */
   for (q=Q-1; q>=1; q--) {
      if (en[q]<=0)
         en[q] = en[q+1]-qDms[q+1];
   }
   for (q=2; q<=Q; q++) {
      if (st[q]<=0)
         st[q] = st[q-1]+qDms[q-1];
   }

   /* check length of each segment and #HMM states */
   for (q=1; q<=Q; q++) {
      for (i=mindur=0; q+i<=Q; i++) {
         mindur += qDms[q+i];
         while (en[q+i]-st[q]+1<mindur) {
            if (st[q]<=1 && en[q+i]>=T) 
               HError(7399,"SetAlign: given data sequence is shorter than given sentence HMM");
            if (st[q]>1)
               st[q]--;
            if (en[q+i]<T)
               en[q+i]++;
         }
      }
   }

   /* check start and end */
   for (q=2; q<=Q; q++) {
      if (st[q]>en[q-1]+1)
         HError(-7399,"SetAlign: start[%d]=%d is larger than end[%d]=%d", q, st[q], q-1, en[q-1]);
   }
}

/* SetAlignBeam: set beam start and end points according to given model alignment */
static void SetAlignBeam (AlphaBeta *ab, PruneInfo *p, UttInfo *utt)
{
   LabList *lablist;
   LLink lab;
   long *st, *en;
   int t, q;
   Boolean hasAlign = FALSE;

   const int T = utt->T;
   const int Q = utt->Q;

   /* label list for current uttreance */
   lablist = utt->tr->head;
   
   /* allocate start and end */ 
   st = (long *) New(&ab->abMem, Q*sizeof(long));
   en = (long *) New(&ab->abMem, Q*sizeof(long));
   st--; en--;

   /* get start and end time for q-th model according to given model alignment */
   for (q=1; q<=Q; q++) {
      lab = GetLabN(lablist,q);  /* q-th label */
      /* read start and end time for current model */
      st[q] = (long)lab->start/utt->tgtSampRate + 1;
      en[q] = (long)lab->end  /utt->tgtSampRate;
      
      if (en[q]>T)
         en[q] = T;
         
      if (!hasAlign && ( (st[q]>1 && st[q]<T) || (en[q]>1 && en[q]<T)))
         hasAlign = TRUE;
   }

   if (hasAlign) {
      /* arrange alignment */ 
      SetAlign(st, en, ab->qDms, Q, T);
      
      /* set beam width (qLo and qHi) according to start and end time */
      for (t=2; t<T; t++) {
         /* set qHi */
         for (q=p->qHi[t]; q>0; q--) {
            if (st[q]<=t && t<=en[q]) 
               break;
         }
         p->qHi[t] = q;

         /* set qLo */
         for (q=p->qLo[t]; q<=Q; q++) {
            if (st[q]<=t && t<=en[q])
               break;
         }
         p->qLo[t] = q;
      }
      
      /* check qLo and qHi */
      for (t=1; t<=T; t++) {
         if (p->qLo[t]>p->qHi[t]) 
            HError(7399,"SetAlignBeam: beam is irregal, qLo[%d]=%d > qHi[%d]=%d", t, p->qLo[t], t, p->qHi[t]); 
      }
   }

   /* free start and end */
   st++; en++;
   Dispose(&ab->abMem, en);
   Dispose(&ab->abMem, st);
   
   return;
}

/* SetBeta: allocate and calculate beta and otprob matrices */
static LogDouble SetBeta(AlphaBeta *ab, FBInfo *fbInfo, UttInfo *utt)
{

   int i,j,t,q,Nq,lNq=0,q_at_gMax,startq,endq,d,*maxDur;
   int S, Q, T;
   DVector *bqt=NULL,*bqt1,*bq1t1,maxP,***beta;
   float ***outprob;
   SVector *durprob;
   LogDouble x,y,gMax,lMax,a,a1N=0.0;
   HLink hmm;
   PruneInfo *p;
   int skipstart, skipend;
   HMMSet *hset;
   
   hset = fbInfo->al_hset;
   skipstart = fbInfo->skipstart;
   skipend = fbInfo->skipend;

   S=utt->S;
   Q=utt->Q;
   T=utt->T;
   p=ab->pInfo;
   beta=ab->beta;

   maxP = CreateDVector(&gstack, Q);   /* for calculating beam width */
  
   /* Last Column t = T */
   p->qHi[T] = Q; endq = p->qLo[T];
   Setotprob(ab,fbInfo,utt,T,S,Q,endq);
   Setdurprob(ab,fbInfo,utt);
   beta[T] = CreateBetaQ(&ab->abMem,endq,Q,Q);
   gMax = LZERO;   q_at_gMax = 0;    /* max value of beta at time T */
   for (q=Q; q>=endq; q--){
      hmm = ab->al_qList[q]; Nq = hmm->numStates;
      durprob = ab->durprob[q];
      maxDur  = ab->maxDur[q];
      bqt = beta[T][q] = NewBetaVec(&ab->abMem,Nq,maxDur);
      bqt[Nq][1] = (q==Q) ? 0.0 : beta[T][q+1][lNq][1]+a1N;
      for (i=2;i<Nq;i++) 
         for (d=1; d<=maxDur[i]; d++)
	    bqt[i][d] = durprob[i][d] + ApplyDAEM(hmm->transP[i][Nq]) + bqt[Nq][1];
      outprob = ab->otprob[T][q];
      x = LZERO;
      for (j=2; j<Nq; j++){
	 a = ApplyDAEM(hmm->transP[1][j]); y = bqt[j][1];
         if (a>LSMALL && y > LSMALL)
            x = LAdd(x,a+outprob[j][0][0]+y);
      }
      bqt[1][1] = x;
      lNq = Nq; a1N = ApplyDAEM(hmm->transP[1][Nq]);
      if (x>gMax) {
         gMax = x; q_at_gMax = q;
      }
   }
   if (trace&T_PRU && NonSkipRegion(skipstart,skipend,T) && 
       p->pruneThresh < NOPRUNE)
      printf("%d: Beta Beam %d->%d; gMax=%f at %d\n",
             T,p->qLo[T],p->qHi[T],gMax,q_at_gMax);
   
   /* Columns T-1 -> 1 */
   for (t=T-1;t>=1;t--) {      

      gMax = LZERO;   q_at_gMax = 0;    /* max value of beta at time t */
      startq = p->qHi[t+1];
      endq = (p->qLo[t+1]==1)?1:((p->qLo[t]>=p->qLo[t+1])?p->qLo[t]:p->qLo[t+1]-1);
      while (endq>1 && ab->qDms[endq-1]==0) endq--;
      /* start end-point at top of beta beam at t+1  */
      /*  unless this is outside the beam taper.     */
      /*  + 1 to allow for state q+1[1] -> q[N]      */
      /*  + 1 for each tee model preceding endq.     */
      Setotprob(ab,fbInfo,utt,t,S,startq,endq);
      beta[t] = CreateBetaQ(&ab->abMem,endq,startq,Q);
      for (q=startq;q>=endq;q--) {
         lMax = LZERO;                 /* max value of beta in model q */
         hmm = ab->al_qList[q]; 
         Nq = hmm->numStates;
         durprob = ab->durprob[q];
         maxDur  = ab->maxDur[q];
         bqt = beta[t][q] = NewBetaVec(&ab->abMem,Nq,maxDur);
         bqt1 = beta[t+1][q];
         bq1t1 = (q==Q)?NULL:beta[t+1][q+1];
         outprob = ab->otprob[t+1][q];
         bqt[Nq][1] = (bq1t1==NULL) ? LZERO : bq1t1[1][1];
         if (q<startq && a1N>LSMALL)
            bqt[Nq][1] = LAdd(bqt[Nq][1], beta[t][q+1][lNq][1]+a1N);
         for (i=Nq-1;i>1;i--){
            for (d=1; d<=maxDur[i]; d++) {
	       x = durprob[i][d] + ApplyDAEM(hmm->transP[i][Nq]) + bqt[Nq][1];
               if (q>=p->qLo[t+1] && q<=p->qHi[t+1]) {
               for (j=2;j<Nq;j++) {
		     a = ApplyDAEM(hmm->transP[i][j]); y = bqt1[j][1];
                  if (a>LSMALL && y>LSMALL)
                        x = LAdd(x,durprob[i][d]+a+outprob[j][0][0]+y);
                  }
                  if (d<maxDur[i] && bqt1[i][d+1]>LSMALL)
                     x = LAdd(x,outprob[i][0][0]+bqt1[i][d+1]);
               }
               bqt[i][d] = x;
               }
            /* compute lMax and gMax only if pruning is on */
            if (p->pruneThresh < NOPRUNE) { 
               x = LZERO;
               for (d=1; d<=maxDur[i]; d++)
                  if (bqt[i][d]>LSMALL)
                     x = LAdd(x,bqt[i][d]);
            if (x>lMax) lMax = x;
            if (x>gMax) {
               gMax = x; q_at_gMax = q;
            }
         }
         }
         outprob = ab->otprob[t][q];
         x = LZERO;
         for (j=2; j<Nq; j++){
	    a = ApplyDAEM(hmm->transP[1][j]);
            y = bqt[j][1];
            if (a>LSMALL && y>LSMALL)
               x = LAdd(x,a+outprob[j][0][0]+y);
         }
         bqt[1][1] = x;
         maxP[q] = lMax;
         lNq = Nq; a1N = ApplyDAEM(hmm->transP[1][Nq]);
      }
      while (gMax-maxP[startq] > p->pruneThresh) {
         beta[t][startq] = NULL;
         --startq;                   /* lower startq till thresh reached */
         if (startq<1) HError(7323,"SetBeta: Beta prune failed sq < 1");
      }
      while(p->qHi[t]<startq) {        /* On taper */
         beta[t][startq] = NULL;
         --startq;                   /* lower startq till thresh reached */
         if (startq<1) HError(7323,"SetBeta: Beta prune failed on taper sq < 1");
      }
      p->qHi[t] = startq;
      while (gMax-maxP[endq]>p->pruneThresh){
         beta[t][endq] = NULL;
         ++endq;                   /* raise endq till thresh reached */
         if (endq>startq) {
            return(LZERO);
         }
      }
      p->qLo[t] = endq;
      if (trace&T_PRU && NonSkipRegion(skipstart,skipend,t) && 
          p->pruneThresh < NOPRUNE)
         printf("%d: Beta Beam %d->%d; gMax=%f at %d\n",
                t,p->qLo[t],p->qHi[t],gMax,q_at_gMax);
   }

   /* Finally, set total prob pr */
   utt->pr = bqt[1][1];

   if (utt->pr <= LSMALL) {
      return LZERO;
   }

   if (trace&T_TOP) {
     if (fbInfo->al_hset->xf != NULL) {
        /* take into account the determinant */
         printf(" Utterance prob per frame = %e\n",utt->pr/T + 0.5*fbInfo->al_hset->xf->xform->det);
     } else {
        printf(" Utterance prob per frame = %e\n",utt->pr/T);
     }
      fflush(stdout);
   }
   return utt->pr;
}

/* -------------------- Top Level of F-B Updating ---------------- */

/* CheckData: check data file consistent with HMM definition */
static void CheckData(HMMSet *hset, char *fn, BufferInfo *info, 
                      Boolean twoDataFiles) 
{
   if (info->tgtVecSize!=hset->vecSize)
      HError(7350,"CheckData: Vector size in %s[%d] is incompatible with hset [%d]",
             fn,info->tgtVecSize,hset->vecSize);
   if (!twoDataFiles){
      if (info->tgtPK != hset->pkind)
         HError(7350,"CheckData: Parameterisation in %s is incompatible with hset",
                fn);
   }
}

/* StepBack: Step utterance from T to 1 calculating Beta matrix*/
static Boolean StepBack(FBInfo *fbInfo, UttInfo *utt)
{
   LogDouble lbeta;
   LogDouble pruneThresh;
   AlphaBeta *ab;
   PruneInfo *p;
   int qt;
   
   ResetObsCache(fbInfo->xfinfo_hmm);
   if (fbInfo->xfinfo_dur != NULL)
     ResetObsCache(fbInfo->xfinfo_dur);
   ab = fbInfo->ab;
   pruneThresh=pruneSetting.pruneInit;
   do
      {
         ResetHeap(&ab->abMem);
         InitPruneStats(ab);  
         p = fbInfo->ab->pInfo;
         p->pruneThresh = pruneThresh;
         qt=CreateInsts(fbInfo,ab,utt->Q,utt->tr);
         if (qt>utt->T) {
            if (trace&T_TOP)
               printf(" Unable to traverse %d states in %d frames\n",qt,utt->T);
            HError(-7324,"StepBack: Bad data or over pruning\n");
            return FALSE;
         }
         CreateBeta(ab,utt->T);
         SetBeamTaper(p,ab->qDms,utt->Q,utt->T);
         if (fbInfo->useAlign)
            SetAlignBeam(ab, p, utt);
         CreateOtprob(ab,utt->T);
         lbeta=SetBeta(ab,fbInfo,utt);
         if (lbeta>LSMALL) break;
         pruneThresh+=pruneSetting.pruneInc;
         if (pruneThresh>pruneSetting.pruneLim || pruneSetting.pruneInc==0.0) {
            if (trace&T_TOP)
               printf(" No path found in beta pass\n");
            HError(-7324,"StepBack: Bad data or over pruning\n");
            return FALSE;
         }
         if (trace&T_TOP) {
            printf("Retrying Beta pass at %5.1f\n",pruneThresh);
         }
      }
   while(pruneThresh<=pruneSetting.pruneLim);
   
   if (lbeta<LSMALL)
      HError(7323,"StepBack: Beta prune error");
   return TRUE;
}

/* ---------------------- Statistics Accumulation -------------------- */

/* UpTranParms: update the transition counters of given hmm */
static void UpTranParms(FBInfo *fbInfo, HLink hmm, int t, int q,
                        DVector *aqt, DVector *bqt, DVector *bqt1, DVector *bq1t, 
                        LogDouble pr)
{
   int i,j,N,d,*maxDur;
   Vector ti,ai;
   float ***outprob,***outprob1;
   SVector *durprob;
   double sum,x;
   TrAcc *ta;
   AlphaBeta *ab;

   N = hmm->numStates;
   ab = fbInfo->ab;
   ta = (TrAcc *) GetHook(hmm->transP);
   outprob = ab->otprob[t][q]; 
   durprob = ab->durprob[q];
   maxDur  = ab->maxDur[q];
   if (bqt1!=NULL) outprob1 = ab->otprob[t+1][q];  /* Bug fix */
   else outprob1 = NULL;
   for (i=1;i<N;i++) {
      ti = ta->tran[i]; ai = hmm->transP[i];
      for (j=2;j<=N;j++) {
         if (i==1 && j<N) {                  /* entry transition */
	    x = aqt[1][1]+ApplyDAEM(ai[j])+outprob[j][0][0]+bqt[j][1]-pr;
         }
         else {
            if (i>1 && j<N && bqt1!=NULL) {     /* internal transition */
               x = LZERO;
               for (d=1; d<=maxDur[i]; d++)
                  if (aqt[i][d]>LSMALL)
                     x = LAdd(x,aqt[i][d]+durprob[i][d]);
               x += ApplyDAEM(ai[j])+outprob1[j][0][0]+bqt1[j][1]-pr;
            }
            else {
               if (i>1 && j==N) {                  /* exit transition */
                  x = LZERO;
                  for (d=1; d<=maxDur[i]; d++)
                     if (aqt[i][d]>LSMALL)
                        x = LAdd(x,aqt[i][d]+durprob[i][d]);
                  x += ApplyDAEM(ai[N])+bqt[N][1]-pr;
               }
               else
                  x = LZERO;
	    }
	 }
         if (x>MINEARG) {
            ti[j] += exp(x); ta->occ[i] += exp(x);
         }
         if (i==1 && j==N && ApplyDAEM(ai[N])>LSMALL && bq1t != NULL){ /* tee transition */
	    x = aqt[1][1]+ApplyDAEM(ai[N])+bq1t[1][1]-pr;
            if (x>MINEARG) {
               ti[N] += exp(x); ta->occ[i] += exp(x);
               }
         }
      }
   }
   if (trace&T_TRA && NonSkipRegion(fbInfo->skipstart,fbInfo->skipend,t)) {
      printf("Tran Counts at time %d, Model Q%d %s\n",t,q,ab->qIds[q]->name);
      for (i=1;i<=N;i++) {
         printf("  %d. Occ %8.2f: Trans ",i,ta->occ[i]);
         sum = 0.0;
         for (j=2; j<=N; j++) {
            x = ta->tran[i][j]; sum += x;
            printf("%7.2f ",x);
         }
         printf(" [%8.2f]\n",sum);
      }
   }
}

/* UpMixParms: update mu/va accs of given hmm  */
static void UpMixParms(FBInfo *fbInfo, int q, HLink hmm, HLink al_hmm,
                       Observation *o, Observation *o2, 
                       int t, DVector *aqt, DVector *aqt1, DVector *bqt, int S,
                       Boolean twoDataFiles, LogDouble pr)
{
   int i,s,j,k,kk,m=0,mx,M=0,N,vSize,order,d,*maxDur;
   Vector mu_jm,var,mean=NULL,invk,otvs;
   TMixRec *tmRec = NULL;
   float **outprob;
   SVector *durprob;
   Matrix inv;
   LogFloat c_jm,a,prob=0.0;
   LogDouble x,initx = LZERO;
   float zmean,zmeanlr,zmean2,tmp;
   double Lr,steSumLr;
   HMMSet *hset;
   HSetKind hsKind;
   AlphaBeta *ab;
   StateInfo *si;
   StreamElem *ste, *al_ste=NULL;
   StreamInfo *sti, *al_sti=NULL;
   MixtureElem *me;
   MixPDF *mp=NULL;
   MuAcc *ma;
   VaAcc *va;
   WtAcc *wa = NULL;
   Boolean mmix=FALSE;  /* TRUE if multiple mixture */
   float wght=0.0;
   /* variables for 2-model reestimation */
   Vector comp_prob=NULL;        /* array[1..M] of Component probability */
   float norm=0.0, stw;          /* total mixture prob */
   LogFloat det;
   AdaptXForm *inxform;
   Vector ovec=NULL;

   ab     = fbInfo->ab;
   hset   = fbInfo->up_hset;
   hsKind = fbInfo->hsKind;

   if (trace&T_MIX && fbInfo->uFlags_hmm&UPMIXES && 
       NonSkipRegion(fbInfo->skipstart,fbInfo->skipend,t)){
      printf("Mixture Weights at time %d, model Q%d %s\n",
             t,q,ab->qIds[q]->name);
   }
   
   /* allows a clean tidy of the space */
   comp_prob = CreateVector(&gstack,fbInfo->maxM);

   if (strmProj) { /* recreate full vector */
      ovec = CreateVector(&gstack,hset->vecSize);
      for (i=1,s=1;s<=S;s++)
         for (k=1;k<=hset->swidth[s];k++,i++)
            ovec[i] = o[t].fv[s][k];
   }

   N = hmm->numStates;
   maxDur = ab->maxDur[q];
   durprob = ab->durprob[q];
   
   if (keepOccm) {
      ab->occm[t][q] = (Vector **) New(&ab->abMem, N*sizeof(Vector *));
      ab->occm[t][q]--;
   }
         
   for (j=2;j<N;j++) {
      si = hmm->svec[j].info;
      if (fbInfo->maxM>1){
	 initx = ApplyDAEM(hmm->transP[1][j]) + aqt[1][1];
         if (t>1) {
            for (i=2;i<N;i++){
	       a = ApplyDAEM(hmm->transP[i][j]);
               if (a>LSMALL)
                  for (d=1; d<=maxDur[i]; d++)
                     if (aqt1[i][d]>LSMALL)
                        initx = LAdd(initx,aqt1[i][d]+durprob[i][d]+a);
            }
            initx += bqt[j][1];
            for (d=2; d<=maxDur[j]; d++)
               if (aqt1[j][d-1]>LSMALL && bqt[j][d]>LSMALL)
                  initx = LAdd(initx,aqt1[j][d-1]+bqt[j][d]);
            }
         else
            initx += bqt[j][1];
         initx -= pr;
      }
      if (trace&T_MIX && fbInfo->uFlags_hmm&UPMIXES && 
          NonSkipRegion(fbInfo->skipstart,fbInfo->skipend,t))
         printf("  State %d: ",j);
      if (keepOccm) {
         ab->occm[t][q][j] = (Vector *) New(&ab->abMem, S*sizeof(Vector));
         ab->occm[t][q][j]--;
      }
      ste = hmm->svec[j].info->pdf+1;
      outprob = ab->otprob[t][q][j];
      for (s=1;s<=S;s++,ste++){
         sti = ste->info;
         stw = (si->weights!=NULL) ? si->weights[s] : 1.0;
         /* Get observation vector for this state/stream */
         vSize = hset->swidth[s];
      
         switch (hsKind){
         case TIEDHS:             /* if tied mixtures then we only */
            tmRec = &(hset->tmRecs[s]); /* want to process the non-pruned */
            M = tmRec->topM;            /* components */
            mmix = TRUE;
            break;
         case DISCRETEHS:
            M = 1;
            mmix = FALSE;
            break;
         case PLAINHS:
         case SHAREDHS:
            M = sti->nMix;
            mmix = (M>1) ? TRUE : FALSE;
            break;
         }
         /* update weight occupation count */
         wa = (WtAcc *) sti->hook; steSumLr = 0.0;

         if (keepOccm) {
            ab->occm[t][q][j][s] = CreateVector(&ab->abMem, M);
            ZeroVector(ab->occm[t][q][j][s]);
         }
         if (fbInfo->twoModels || stw!=1.0) { /* component probs of update hmm */
            norm = LZERO;
            for (mx=1; mx<=M; mx++) {
               if (alCompLevel) {
                  al_ste = al_hmm->svec[j].info->pdf+1;
                  al_sti = al_ste->info;
                  if (al_sti->nMix != M)
                     HError(999,"Cannot align at the component level if number of components is different!");
               }
               if (alCompLevel) {
                  me = al_sti->spdf.cpdf+mx;
               } else {
                  me = sti->spdf.cpdf+mx;
               }
               inxform = fbInfo->inXForm_hmm;
               mp=me->mpdf;
               if (twoDataFiles){  
                  if (alCompLevel) {
                     otvs = ApplyCompFXForm(mp,o2[t].fv[s],fbInfo->al_inXForm_hmm,&det,t);
                  }
                  else {
                     otvs = ApplyCompFXForm(mp,o2[t].fv[s],inxform,&det,t);
                  }
               } else if (fbInfo->twoModels) {
                  otvs = ApplyCompFXForm(mp,o[t].fv[s],inxform,&det,t);
               } else
                  otvs = o[t].fv[s];
               order = SpaceOrder(otvs);
               wght = stw * ApplyDAEM(MixLogWeight(hset,me->weight));
               prob = (fbInfo->twoModels) ? ApplyDAEM(MOutP(otvs,mp)+det) : ((S==1) ? outprob[0][mx] : outprob[s][mx]);
               comp_prob[mx] = (!hset->msdflag[s] || (hset->msdflag[s] && order==VectorSize(mp->mean))) ? stw*prob : LZERO;  /* MSD check */
               if (wght+comp_prob[mx]>LSMALL)
                  norm = LAdd(norm,wght+comp_prob[mx]);
            }
         }

         for (mx=1;mx<=M;mx++) { 
            /* process mixtures */
            switch (hsKind){    /* Get wght and mpdf */
            case TIEDHS:
               m=tmRec->probs[mx].index;
               wght=MixLogWeight(hset,sti->spdf.tpdf[m]);
               mp=tmRec->mixes[m];
               break;
            case DISCRETEHS:
               if (twoDataFiles)
                  m=o2[t].vq[s];
               else
                  m=o[t].vq[s];
               wght = 0.0; /* This is the log-weight, just for consistency! */
               mp=NULL;
               break;
            case PLAINHS:
            case SHAREDHS:
               m = mx;
               me = sti->spdf.cpdf+m;
               wght = MixLogWeight(hset,me->weight);
               mp=me->mpdf;
               if (hset->msdflag[s]) vSize = VectorSize(mp->mean);
               break;
            }
            if (wght>LMINMIX){
              wght = ApplyDAEM(wght);
              /* compute mixture likelihood  */
              if (!mmix || (hsKind==DISCRETEHS)) {/* For DISCRETEHS calcs are same as single mix*//* note: only SHAREDHS or PLAINHS */
                  x = LZERO;
                  for (d=1; d<=maxDur[j]; d++)
                     if (aqt[j][d]>LSMALL && bqt[j][d]>LSMALL)
                        x = LAdd(x,aqt[j][d]+bqt[j][d]);   /* same as single mix*/
                  x -= pr;
              }
              else if (fbInfo->twoModels) {      
                  c_jm = stw * wght;
                  x = LZERO;
                  for (d=1; d<=maxDur[j]; d++)
                     if (aqt[j][d]>LSMALL && bqt[j][d]>LSMALL)
                        x = LAdd(x,aqt[j][d]+bqt[j][d]);
                  x += c_jm+comp_prob[m]-pr-norm;
              }
              else {
                 c_jm = stw * wght;
                 x = initx+c_jm;
                 switch(hsKind) {
                 case TIEDHS :
                    tmp = tmRec->probs[mx].prob;
                    prob = (tmp>=MINLARG)?log(tmp)+tmRec->maxP:LZERO;
                    prob = (prob>LSMALL) ? stw * prob : LZERO;
                    break;
                 case PLAINHS : 
                 case SHAREDHS:
                    if (stw==1.0)
                       prob = (S==1) ? outprob[0][mx] : outprob[s][mx];
		    else
                       prob = (S==1) ? stw * outprob[0][0] + comp_prob[mx] - norm
                                     : stw * outprob[s][0] + comp_prob[mx] - norm;
                    break;
                 default:
                    x=LZERO;
                    break;
                 }
                 x += prob;
                 if (S>1)      /* adjust for parallel streams */
                    x += outprob[0][0] - stw * outprob[s][0];
              }
              if (twoDataFiles){  /* switch to new data for mu & var est */
                 otvs = o2[t].fv[s];
              }
              if (-x<pruneSetting.minFrwdP) {
		 if (twoDataFiles){  /* switch to new data for mu & var est */
                   otvs = ApplyCompFXForm(mp,o2[t].fv[s],fbInfo->paXForm_hmm,&det,t);
		 } else {
                   otvs = ApplyCompFXForm(mp,o[t].fv[s],fbInfo->paXForm_hmm,&det,t);
		 }
                 Lr = exp(x);
                  /* More diagnostics */
                  /* if (Lr>0.000001 && ab->occt[j]>0.000001 &&
                     (Lr/ab->occt[j])>1.00001)
                     printf("Too big %d %d %s : %5.3f %10.2f %8.2f (%4.2f)\n",t,q,
                     ab->qIds[q]->name,Lr/ab->occt[j],Lr,ab->occt[j],prob); */
            
                  /* update occupation counts */
                  steSumLr += Lr;
                  /* update the adaptation statistic counts */
                  if (fbInfo->uFlags_hmm&UPXFORM)
                     AccAdaptFrame(fbInfo->xfinfo_hmm, Lr, otvs, mp, t);
                  /* 
                     update the semi-tied statistic counts
                     this accumulates "true" outer products to allow multiple streams
                  */ 
                  if (fbInfo->uFlags_hmm&UPSEMIT) {
                     ma = (MuAcc *) GetHook(mp->mean);
                     va = (VaAcc *) GetHook(mp->cov.var);
                     ma->occ += Lr;
                     va->occ += Lr;
                     mu_jm = ma->mu;
                     inv = va->cov.inv;
                     if (strmProj) {
                        /* 
                           accumulate over complete space for multiple stream support 
                           will only operate with no parent/input transform. Can 
                           therefore just look at original space.
                        */
                        for (k=1;k<=hset->vecSize;k++) {
                           invk = inv[k];
                           zmean=ovec[k];
                           zmeanlr=zmean*Lr;
                           mu_jm[k] += zmeanlr;
                           for (kk=1;kk<=k;kk++) {
                              invk[kk] += ovec[kk]*zmeanlr;
                           }
                        }
                     } else {
                        for (k=1;k<=vSize;k++) {
                           invk = inv[k];
                           zmean=otvs[k];
                           zmeanlr=zmean*Lr;
                           mu_jm[k] += zmeanlr;
                           for (kk=1;kk<=k;kk++) {
                              invk[kk] += otvs[kk]*zmeanlr;
                           }
                        }
                     }
                  } else {
                     /* update mean counts */
                     if ((fbInfo->uFlags_hmm&UPMEANS) || (fbInfo->uFlags_hmm&UPVARS))
                        mean = mp->mean; 
                     if ((fbInfo->uFlags_hmm&UPMEANS) && (fbInfo->uFlags_hmm&UPVARS)) {
                        ma = (MuAcc *) GetHook(mean);
                        va = (VaAcc *) GetHook(mp->cov.var);
                        ma->occ += Lr;
                        va->occ += Lr;
                        mu_jm = ma->mu;
                        if ((mp->ckind==DIAGC)||(mp->ckind==INVDIAGC)){
                           var = va->cov.var;
                           for (k=1;k<=vSize;k++) {
                              zmean=otvs[k]-mean[k];
                              zmeanlr=zmean*Lr;
                              mu_jm[k] += zmeanlr;
                              var[k] += zmean*zmeanlr;
                           }
                        } else {
                           inv = va->cov.inv;
                           for (k=1;k<=vSize;k++) {
                              invk = inv[k];
                              zmean=otvs[k]-mean[k];
                              zmeanlr=zmean*Lr;
                              mu_jm[k] += zmeanlr;
                              for (kk=1;kk<=k;kk++) {
                                 zmean2 = otvs[kk]-mean[kk];
                                 invk[kk] += zmean2*zmeanlr;
                              }
                           }
                        }
                     }
                     else if (fbInfo->uFlags_hmm&UPMEANS){
                        ma = (MuAcc *) GetHook(mean);
                        mu_jm = ma->mu;
                        ma->occ += Lr;
                        for (k=1;k<=vSize;k++)     /* sum zero mean */
                           mu_jm[k] += (otvs[k]-mean[k])*Lr;
                     }
                     else if (fbInfo->uFlags_hmm&UPVARS){
                        /* update covariance counts */
                        va = (VaAcc *) GetHook(mp->cov.var);
                        va->occ += Lr;
                        if ((mp->ckind==DIAGC)||(mp->ckind==INVDIAGC)){
                           var = va->cov.var;
                           for (k=1;k<=vSize;k++) {
                              zmean=otvs[k]-mean[k];
                              var[k] += zmean*zmean*Lr;
                           }
                        } else {
                           inv = va->cov.inv;
                           for (k=1;k<=vSize;k++) {
                              invk = inv[k];
                              zmean=otvs[k]-mean[k];               
                              for (kk=1;kk<=k;kk++) {
                                 zmean2 = otvs[kk]-mean[kk];
                                 invk[kk] += zmean*zmean2*Lr;
                              }
                           }
                        }
                     }
                  }
                  /* update mixture weight counts */
                  if (fbInfo->uFlags_hmm&UPMIXES) {
                     wa->c[m] +=Lr;
                     if (trace&T_MIX && NonSkipRegion(fbInfo->skipstart,fbInfo->skipend,t))
                        printf("%3d. %7.2f",m,wa->c[m]);
                  }
                  
                  if (keepOccm)
                     ab->occm[t][q][j][s][m] = Lr;
               }
            }
            if (twoDataFiles){ /* Switch back to old data for prob calc */
               otvs = o[t].fv[s];
            }    
         }
      
         wa->occ += steSumLr;
         if (trace&T_MIX && mmix && fbInfo->uFlags_hmm&UPMIXES && 
             NonSkipRegion(fbInfo->skipstart,fbInfo->skipend,t))
            printf("[%7.2f]\n",wa->occ);
      }
   }

   FreeVector(&gstack,comp_prob);
}

/* UpDurParms: update duration accs of given hmm */
static void UpDurParms(FBInfo *fbInfo, UttInfo *utt, HLink hmm, const int t, const int q)
{
   int i,j,m,k,kk,vSize,d,*maxDur;
   double Lr,zmean,zmean2,zmeanlr,steSumLr;
   SVector *durprob;
   LogDouble initx,x;
   LogFloat det;
   AlphaBeta *ab;
   PruneInfo *p;
   StreamInfo *sti;
   MixPDF *mp;
   Vector mean,var,mu_jm,invk,dur,xdur;
   DVector *aqt,*aqt1,*bqt,*bqt1;
   TriMat inv;
   MuAcc *ma=NULL;
   VaAcc *va=NULL;
   WtAcc *wa=NULL;

   const int T = utt->T;
   const int N = hmm->numStates;
   
   ab   = fbInfo->ab;
   p    = ab->pInfo;
   aqt = ab->alphat[q];
   bqt = ab->beta[t][q];
   bqt1 = (t==T) ? NULL : ab->beta[t+1][q];
   aqt1 = (t==1) ? NULL : ab->alphat1[q];
            
   maxDur  = ab->maxDur[q];
   durprob = ab->durprob[q];
   
   dur  = CreateVector(&gstack,1);
   xdur = CreateVector(&gstack,1);
         
   for (j=2; j<N; j++) {
      sti = ab->up_dList[q]->svec[2].info->pdf[j-1].info;
      wa = (WtAcc *) sti->hook; steSumLr = 0.0;

      /* initial term */
      if (!semiMarkov) {
         /* HMM */
	 initx = ApplyDAEM(hmm->transP[1][j]) + aqt[1][1];
         if (t>1) {
            for (i=2; i<N; i++)
	       if (i!=j && ApplyDAEM(hmm->transP[i][j])>LSMALL && aqt1[i][1]>LSMALL)
		  initx = LAdd(initx,aqt1[i][1]+ApplyDAEM(hmm->transP[i][j]));
         }
      }
      else {
         /* HSMM */
	 initx = ApplyDAEM(hmm->transP[j][N])+bqt[N][1];
         if (bqt1!=NULL)
            for (i=2; i<N; i++)
	       if (ApplyDAEM(hmm->transP[j][i])>LSMALL && bqt1[i][1]>LSMALL)
		  initx = LAdd(initx,ApplyDAEM(hmm->transP[j][i])+ab->otprob[t+1][q][i][0][0]+bqt1[i][1]);
      }

      for (m=1; m<=sti->nMix; m++) {
         /* accumulators */
         mp = sti->spdf.cpdf[m].mpdf;
         mean = mp->mean; vSize = VectorSize(mean);
         if (fbInfo->uFlags_dur&UPMEANS) ma = (MuAcc *) GetHook(mean);
         if (fbInfo->uFlags_dur&UPVARS)  va = (VaAcc *) GetHook(mp->cov.var);
      
         /* count statistics from t to t1 */
         for (d=1; (!semiMarkov && t+d-1<=T) || (semiMarkov && d<=maxDur[j]); d++) {
            /* acc statistics */
            if (initx>LSMALL && (semiMarkov || (!semiMarkov && p->qLo[t+d-1]<=q && q<=p->qHi[t+d-1])) ) {
               if (!semiMarkov) {
                  /* HMM */
                  initx += ab->otprob[t+d-1][q][j][0][0];
                  if (d>1) 
		     initx += ApplyDAEM(hmm->transP[j][j]);
   
                  x = ApplyDAEM(hmm->transP[j][N]);
                  if (t+d-1<T) {
                     x += (p->qLo[t+d]<=q+1 && q+1<=p->qHi[t+d]) ? ab->beta[t+d][q+1][1][1] : LZERO;
                     if (p->qLo[t+d]<=q && q<=p->qHi[t+d]) {
                        for (k=2; k<N; k++)
			   if (k!=j && ApplyDAEM(hmm->transP[j][k])>LSMALL && ab->beta[t+d][q][k][1]>LSMALL)
			      x = LAdd(x, ApplyDAEM(hmm->transP[j][k])+ab->otprob[t+d][q][k][0][0]+ab->beta[t+d][q][k][1]);
                     }
                  }
               }
               else {
                  /* HSMM */
                  x = aqt[j][d]+durprob[j][d]; 
               }
               x = initx+x-utt->pr;
                        
               /* update duration counts */
               Lr = L2F(x);
               if (Lr>0.0) {
                  /* duration observation */
                  dur[1] = (float)d;
                  xdur = ApplyCompFXForm(mp,dur,fbInfo->paXForm_dur,&det,d);

                  /* update the adaptation statistic counts */
                  if (fbInfo->uFlags_dur&UPXFORM)
                     AccAdaptFrame(fbInfo->xfinfo_dur,Lr,xdur,mp,d);
                   
                  /* acc statistics */
                  if ((fbInfo->uFlags_dur&UPMEANS) && (fbInfo->uFlags_dur&UPVARS)) {
                     ma->occ += Lr;
                     va->occ += Lr;
                     mu_jm = ma->mu;
                     if ((mp->ckind==DIAGC)||(mp->ckind==INVDIAGC)){
                        var = va->cov.var;
                        for (k=1; k<=vSize; k++) {
                           zmean=xdur[k]-mp->mean[k];
                           zmeanlr=zmean*Lr;
                           mu_jm[k] += zmeanlr;
                           var[k] += zmean*zmeanlr;
                        }
                     }
                     else {
                        inv = va->cov.inv;
                        for (k=1; k<=vSize; k++) {
                           invk = inv[k];
                           zmean=xdur[k]-mp->mean[k];
                           zmeanlr=zmean*Lr;
                           mu_jm[k] += zmeanlr;
                           for (kk=1; kk<=k; kk++) {
                              zmean2 = xdur[k]-mp->mean[kk];
                              invk[kk] += zmean2*zmeanlr;
                           }
                        }
                     }
                  }
                  else if (fbInfo->uFlags_dur&UPMEANS) {
                     mu_jm = ma->mu;
                     ma->occ += Lr;
                     for (k=1; k<=vSize; k++)     /* sum zero mean */
                        mu_jm[k] += (xdur[k]-mean[k])*Lr;
                  }
                  else if (fbInfo->uFlags_dur&UPVARS){
                      va->occ += Lr;
                      if ((mp->ckind==DIAGC)||(mp->ckind==INVDIAGC)) {
                         var = va->cov.var;
                         for (k=1; k<=vSize; k++) {
                            zmean=xdur[k]-mean[k];
                            var[k] += zmean*zmean*Lr;
                         }
                      }
                  }
                  /* update mixture weight counts */
                  if (fbInfo->uFlags_dur&UPMIXES) {
                     steSumLr += Lr;
                     wa->c[m] +=Lr;
                  }
               }  
            }
            else 
               break; 
         }
      }
      wa->occ += steSumLr;
   }
   
   FreeVector(&gstack,dur);
}

/* UpStateOcc: update state-level occupancy counts of given hmm */
static void UpStateOcc(FBInfo *fbInfo, int q, HLink hmm, DVector *aqt, DVector *bqt, LogDouble pr)
{
   int i,N,d,*maxDur;
   double x;
   float occ,tmp;
   SVector *durprob;
   StateInfo *si;

   N = hmm->numStates;
   maxDur = fbInfo->ab->maxDur[q];
   durprob = fbInfo->ab->durprob[q];
   
   for (i=2; i<N; i++) {
      x = LZERO;
      for (d=1; d<=maxDur[i]; d++)
         if (aqt[i][d]>LSMALL && bqt[i][d]>LSMALL)
            x = LAdd(x,aqt[i][d]+bqt[i][d]);
      x -= pr;
      occ = (x>MINEARG) ? exp(x) : 0.0;
      
      si = hmm->svec[i].info;
      if (si->hook==NULL)
         tmp = 0.0;
      else
         memcpy(&tmp,&(si->hook),sizeof(float));
      tmp += occ;
      memcpy(&(si->hook),&tmp,sizeof(float));
   }
   
   return;
}

/* -------------------- Top Level of F-B Updating ---------------- */


/* StepForward: Step from 1 to T calc'ing Alpha columns and 
   accumulating statistic */

static void StepForward(FBInfo *fbInfo, UttInfo *utt)
{
   int q,t,start,end;
   long negs;
   DVector *aqt,*aqt1,*bqt,*bqt1,*bq1t;
   HLink al_hmm, up_hmm;
   AlphaBeta *ab;
   PruneInfo *p;

   /* reset the memory heap for alpha for a new utterance */
   /* ResetHeap(&(fbMemInfo.alphaStack)); */
  
   ab = fbInfo->ab;
   p = ab->pInfo;
   CreateAlpha(ab,fbInfo->al_hset,utt->Q, utt->T); /* al_hset may be idential to up_hset */
   InitAlpha(ab,&start,&end,utt->Q,fbInfo->skipstart,fbInfo->skipend);
   ab->occa = NULL;
   if (trace&T_OCC) 
      CreateTraceOcc(ab,utt);
   if (keepOccm) {  /* create occm for parameter generation */
      ab->occm = (Vector ****) New (&ab->abMem, utt->T*sizeof(Vector ***));
      ab->occm--;
   }  
   for (q=1;q<=utt->Q;q++){             /* inc access counters */
      /* hmms */
      up_hmm = ab->up_qList[q];
      negs = (long)up_hmm->hook+1;
      up_hmm->hook = (void *)negs;
      
      /* duration models */
      if (fbInfo->up_dset!=NULL && fbInfo->uFlags_dur&(UPMEANS|UPVARS|UPMIXES)) {
         up_hmm = ab->up_dList[q];
         negs = (long)up_hmm->hook+1;
      up_hmm->hook = (void *)negs;
   }
   }

   ResetObsCache(fbInfo->xfinfo_hmm);
   if (fbInfo->xfinfo_dur != NULL)
      ResetObsCache(fbInfo->xfinfo_dur);

   for (t=1;t<=utt->T;t++) {

      if (fbInfo->hsKind == TIEDHS)
         PrecomputeTMix(fbInfo->al_hset,&(utt->o[t]),pruneSetting.minFrwdP,0);

      if (t>1)
         StepAlpha(ab,t,&start,&end,utt->Q,utt->T,utt->pr,
                   fbInfo->skipstart,fbInfo->skipend);
    
      if (trace&T_ALF && NonSkipRegion(fbInfo->skipstart,fbInfo->skipend,t)) 
         TraceAlphaBeta(ab,t,start,end,utt->pr);
    
      if (keepOccm) {
         ab->occm[t] = (Vector ***) New(&ab->abMem, (end-start+1)*sizeof(Vector **));
         ab->occm[t] -= start;
         p->qLo[t] = start;  
         p->qHi[t] = end;
      }
      
      for (q=start;q<=end;q++) { 
         /* increment accs for each active model */
         al_hmm = ab->al_qList[q];
         up_hmm = ab->up_qList[q];
         aqt = ab->alphat[q];
         bqt = ab->beta[t][q];
         bqt1 = (t==utt->T) ? NULL:ab->beta[t+1][q];
         aqt1 = (t==1)      ? NULL:ab->alphat1[q];
         bq1t = (q==utt->Q) ? NULL:ab->beta[t][q+1];
         if (trace&T_OCC)
            SetOcct(al_hmm,q,ab->occt,ab->occa,ab->maxDur[q],aqt,bqt,bq1t,utt->pr);
         /* accumulate the statistics */
         if (fbInfo->uFlags_hmm&(UPMEANS|UPVARS|UPMIXES|UPXFORM) || keepOccm)
            UpMixParms(fbInfo,q,up_hmm,al_hmm,utt->o,utt->o2,t,aqt,aqt1,bqt,
                       utt->S, utt->twoDataFiles, utt->pr);
         if (fbInfo->uFlags_hmm&UPTRANS)
            UpTranParms(fbInfo,up_hmm,t,q,aqt,bqt,bqt1,bq1t,utt->pr);
         if (fbInfo->uFlags_dur&(UPMEANS|UPVARS|UPMIXES|UPXFORM))
            UpDurParms(fbInfo,utt,al_hmm,t,q);
         if (fbInfo->up_hset->numSharedStreams>0)
            UpStateOcc(fbInfo,q,up_hmm,aqt,bqt,utt->pr);
      }
      if (trace&T_OCC && NonSkipRegion(fbInfo->skipstart,fbInfo->skipend,t)) 
         TraceOcc(ab,utt,t);
   }
}

/* load the labels into the UttInfo structure from file */
void LoadLabs(UttInfo *utt, FileFormat lff, char * datafn, 
              char *labDir, char *labExt)
{

   char labfn[MAXSTRLEN], buf1[MAXSTRLEN], buf2[MAXSTRLEN];

   /* reset the heap for a new transcription */
   ResetHeap(&utt->transStack);
  
   MakeFN(datafn,labDir,labExt,labfn);
   if (trace&T_TOP) {
      printf(" Processing Data: %s; Label %s\n",
             NameOf(datafn,buf1),NameOf(labfn,buf2));
      fflush(stdout);
   }

   utt->tr = LOpen(&utt->transStack,labfn,lff);
   utt->Q  = CountLabs(utt->tr->head);
   if (utt->Q==0)
      HError(-7325,"LoadUtterance: No labels in file %s",labfn);

}

/* load the data file(s) into the UttInfo structure */
void LoadData(HMMSet *hset, UttInfo *utt, FileFormat dff, 
              char * datafn, char * datafn2)
{
   BufferInfo info, info2;
   int T2=0;

   /* close any open buffers */
   if (utt->pbuf != NULL) {
      CloseBuffer(utt->pbuf);
      if (utt->twoDataFiles)
         CloseBuffer(utt->pbuf2);
   }

   /* reset the data stack for a new utterance */
   ResetHeap(&utt->dataStack);
   if (utt->twoDataFiles)
      ResetHeap(&utt->dataStack2);

   if (utt->twoDataFiles)
      if(SetChannel("HPARM1")<SUCCESS)
         HError(7350,"HFB: Channel parameters invalid");
   if((utt->pbuf=OpenBuffer(&utt->dataStack,datafn,0,dff,
                            FALSE_dup,FALSE_dup))==NULL)
      HError(7350,"HFB: Config parameters invalid");
   GetBufferInfo(utt->pbuf,&info);
   if (utt->twoDataFiles){
      if(SetChannel("HPARM2")<SUCCESS)
         HError(7350,"HFB: Channel parameters invalid");
     
      if((utt->pbuf2=OpenBuffer(&utt->dataStack2,datafn2,0,dff,
                                FALSE_dup,FALSE_dup))==NULL)
         HError(7350,"HFB: Config parameters invalid");
      GetBufferInfo(utt->pbuf2,&info2);
      CheckData(hset,datafn2,&info2,utt->twoDataFiles);
      T2 = ObsInBuffer(utt->pbuf2);
      utt->tgtSampRate2 = info2.tgtSampRate;
   }else
      CheckData(hset,datafn,&info,utt->twoDataFiles);

   utt->T = ObsInBuffer(utt->pbuf);
   utt->tgtSampRate = info.tgtSampRate;
   if (utt->twoDataFiles && (utt->T != T2))
      HError(7326,"HFB: Paired training files must be same length for single pass retraining");
  
  
}

/* EXPORT -> InitUttObservations: Initialise the observation structures within UttInfo */
void InitUttObservations(UttInfo *utt, HMMSet *al_hset, 
                         char * datafn, int * maxMixInS)
{

   BufferInfo info, info2;
   Boolean eSep;
   int s, t;

   if (utt->twoDataFiles)
      if(SetChannel("HPARM1")<SUCCESS)
         HError(7350,"HFB: Channel parameters invalid");

   GetBufferInfo(utt->pbuf,&info);
   if (utt->twoDataFiles){
      if(SetChannel("HPARM2")<SUCCESS)
         HError(7350,"HFB: Channel parameters invalid");
      GetBufferInfo(utt->pbuf2,&info2);
   }
  
   SetStreamWidths(info.tgtPK,info.tgtVecSize,al_hset->swidth,&eSep);
   
   /* prepare storage for observations */
   utt->o  = (Observation *) New(&gstack, utt->T*sizeof(Observation));
   utt->o--;
   if (utt->twoDataFiles) {
      utt->o2 = (Observation *) New(&gstack, utt->T*sizeof(Observation));
      utt->o2--;
   }
   
   for (t=1; t<=utt->T; t++) {
      utt->o[t] = MakeObservation(&gstack,al_hset->swidth,info.tgtPK,
                                  ((al_hset->hsKind==DISCRETEHS) ? TRUE:FALSE),eSep);
   if (utt->twoDataFiles)
         utt->o2[t] = MakeObservation(&gstack,al_hset->swidth,info2.tgtPK,
                                      ((al_hset->hsKind==DISCRETEHS) ? TRUE:FALSE),eSep);

      /* load observation and store it to utt->o and utt->o2 */
      GetInputObs(utt, t, al_hset->hsKind);
   
   if (al_hset->hsKind==DISCRETEHS){ 
         for (s=1; s<=utt->S; s++){
            if( (utt->o[t].vq[s] < 1) || (utt->o[t].vq[s] > maxMixInS[s]))
               HError(7350,"LoadFile: Discrete data value [ %d ] out of range in stream [ %d ] in file %s",
                      utt->o[t].vq[s],s,datafn);
         }
      }
   }
}


/* EXPORT -> ResetUttObservations: reset the observation structures within UttInfo */
void ResetUttObservations (UttInfo *utt, HMMSet *al_hset)
{
   BufferInfo info, info2;
   int t;
   
   /* load buffer informations */
   GetBufferInfo(utt->pbuf,&info);
   if (utt->twoDataFiles)
      GetBufferInfo(utt->pbuf2,&info2);
   
   /* reset each observation */
   for (t=utt->T; t>0; t--) {
      if (utt->twoDataFiles)
         ResetObservation(&gstack,&utt->o2[t],al_hset->swidth,info2.tgtPK);
      ResetObservation(&gstack,&utt->o[t],al_hset->swidth,info.tgtPK);
}

   /* reset observation storage */
   if (utt->twoDataFiles) {
      utt->o2++;
      Dispose(&gstack, utt->o2);
   }
   utt->o++;
   Dispose(&gstack, utt->o);
}

/* FBUtt: apply forward-backward to given utterance */
Boolean FBUtt (FBInfo *fbInfo, UttInfo *utt)
{
   Boolean success;

   if ((success = StepBack(fbInfo,utt)))
      StepForward(fbInfo,utt);

#ifdef PDE_STATS
   PrintPDEstats();
#endif

   return success;
}

/* ------------------------ End of HFB.c --------------------------- */
