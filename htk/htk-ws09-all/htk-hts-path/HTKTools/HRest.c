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
/*         File: HRest.c: HMM initialisation program           */
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

char *hrest_version = "!HVER!HRest:   3.4.1 [CUED 12/03/09]";
char *hrest_vc_id = "$Id: HRest.c,v 1.13 2011/06/16 04:18:30 uratec Exp $";

/*
   This program is used to estimate the transition parameters,
   means, covariances and mixture weights of a
   hidden Markov model using Baum-Welch reestimation.
   It handles multiple streams and tying but it ignores
   stream weights by assuming that they are all unity.
*/

/* Trace Flags */
#define T_TOP    0001    /* Top level tracing */
#define T_LD0    0002    /* File Loading */
#define T_LD1    0004    /* + segments within each file */
#define T_OTP    0010    /* Observation Probabilities */
#define T_ALF    0020    /* Alpha matrices */
#define T_BET    0040    /* Beta matrices */
#define T_OCC    0100    /* Occupation Counters */
#define T_TAC    0200    /* Transition Counters */
#define T_MAC    0400    /* Mean Counters */
#define T_VAC   01000    /* Variance Counters */
#define T_WAC   02000    /* MixWeight Counters */
#define T_TRE   04000    /* Reestimated transition matrix */
#define T_WRE  010000    /* Reestimated mixture weights */
#define T_MRE  020000    /* Reestimated means */
#define T_VRE  040000    /* Reestimated variances */
#define T_LGP 0100000    /* Compare LogP via alpha and beta */


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


/* Global Settings */
static char * segLab = NULL;     /* segment label if any */
static LabId  segId  = NULL;     /* and its index */
static char * labDir = NULL;     /* label file directory */
static char * labExt = "lab";    /* label file extension */
static char * outDir = NULL;     /* output macro file directory, if any */
static char * durFN  = NULL;     /* output duration file */
static int  maxIter  = 20;       /* max iterations in parameter estimation */
static float epsilon = 1.0E-4;   /* convergence criterion */
static int minSeg    = 3;        /* min segments to train a model */
static Boolean firstTime = TRUE; /* Flag used to enable InitSegStore */
static Boolean saveBinary = FALSE;  /* save output in binary  */
static Boolean calcDuration = FALSE; /* save duration */
static FileFormat dff=UNDEFF;    /* data file format */
static FileFormat lff=UNDEFF;    /* label file format */
static float minVar  = 0.0;      /* minimum variance */
static float mixWeightFloor=0.0; /* Floor for mixture weights */
static float tMPruneThresh = 10.0;    /* tied mix prune threshold */
static char *hmmfn;              /* HMM definition file name */
static char *outfn=NULL;         /* output definition file name */
static UPDSet uFlags = (UPDSet) (UPMEANS|UPVARS|UPTRANS|UPMIXES);     /* update flags */
static int  trace    = 0;        /* Trace level */
static ConfParam *cParm[MAXGLOBS];   /* configuration parameters */
static int nParm = 0;               /* total num params */
static Boolean segReject = TRUE; /* Enable short train segment rejection */


/* Global Data Structures */
static HMMSet hset;        /* The current unitary hmm set */
static HLink hmm;          /* link to the hmm itself */
static int nStates;        /* numStates of hmm */
static int nStreams;       /* numStreams of hmm */
static HSetKind hsKind;          /* kind of the HMM system */
static int maxMixes;       /* max num mixtures across all streams */
static int maxMixInS[SMAX];/* array[1..swidth[0]] of max mixes */
static int nSeg;           /* num training segments */
static int nTokUsed;       /* actual number of tokens used */
static int maxT,minT,T;    /* max,min and current segment lengths */
static DMatrix alpha;      /* array[1..nStates][1..maxT] of forward prob */
static DMatrix beta;       /* array[1..nStates][1..maxT] of backward prob */
static DVector durOcc;     /* array[1..nStates] of duration counter (occ) */
static DVector durSum;     /* array[1..nStates] of duration counter (sum) */
static DVector durSqr;     /* array[1..nStates] of duration counter (sqr) */
static Matrix outprob;     /* array[2..nStates-1][1..maxT] of output prob */
static Vector **stroutp;   /* array[1..maxT][2..nStates-1][1..nStreams] ...*/
                           /* ... of streamprob */
static Matrix **mixoutp;   /* array[2..nStates-1][1..maxT][1..nStreams]
                              [1..maxMixes] of mixprob */
static Vector occr;        /* array[1..nStates-1] of occ count for cur time */
static Vector zot;         /* temp storage for zero mean obs vector */
static Vector vFloor[SMAX];      /* variance floor - default is all zero */
static float vDefunct=0.0;       /* variance below which mixture defunct */

static SegStore segStore;        /* Storage for data segments */
static MemHeap segmentStack;     /* Used by segStore */
static MemHeap alphaBetaStack;   /* For storage of alpha and beta probs */
static MemHeap accsStack;        /* For storage of accumulators */
static MemHeap transStack;       /* For storage of transcription */
static MemHeap bufferStack;      /* For storage of buffer */
static ParmBuf pbuf;             /* Currently input parm buffer */

   
/* ------------------ Process Command Line ------------------------- */

/* SetConfParms: set conf parms relevant to HRest  */
void SetConfParms(void)
{
   int i;
   double d;
   Boolean b;

   nParm = GetConfig("HREST", TRUE, cParm, MAXGLOBS);
   if (nParm>0) {
      if (GetConfInt(cParm,nParm,"TRACE",&i)) trace = i;
      if (GetConfBool(cParm,nParm,"SAVEBINARY",&b)) saveBinary = b;
      if (GetConfFlt(cParm,nParm,"VDEFUNCT",&d)) vDefunct = d;
   }
}

void ReportUsage(void)
{
   printf("\nModified for HTS\n");
   printf("\nUSAGE: HRest [options] hmmFile trainFiles...\n\n");
   printf(" Option                                       Default\n\n");
   printf(" -e f    Set convergence factor epsilon       1.0E-4\n");
   printf(" -g s    output duration model to file s                   none\n");
   printf(" -i N    Set max iterations to N              20\n");
   printf(" -l s    Set segment label to s               none\n");
   printf(" -m N    Set min segments needed              3\n");
   printf(" -o fn   Store new hmm def in fn (name only)               outDir/srcfn\n");
   printf(" -t      Disable short segment rejection      on\n");
   printf(" -u tmvw Update t)rans m)eans v)ars w)ghts    tmvw\n");
   printf(" -v f    Set minimum variance to f            0.0\n");
   printf(" -c f    Tied Mixture pruning threshold       10.0\n");
   printf(" -w f    Set mix wt floor to f x MINMIX       0.0\n");
   PrintStdOpts("BFGHILMSTX");
   printf("\n\n");
}

void SetuFlags(void)
{
   char *s;
   
   s=GetStrArg();
   uFlags=(UPDSet) 0;        
   while (*s != '\0')
      switch (*s++) {
      case 't': uFlags = (UPDSet) (uFlags+UPTRANS); break;
      case 'm': uFlags = (UPDSet) (uFlags+UPMEANS); break;
      case 'v': uFlags = (UPDSet) (uFlags+UPVARS); break;
      case 'w': uFlags = (UPDSet) (uFlags+UPMIXES); break;
      default: HError(2220,"SetuFlags: Unknown update flag %c",*s);
         break;
      }
}

int main(int argc, char *argv[])
{
   char *datafn, *s;
   void Initialise1(void);
   void Initialise2(void);
   void LoadFile(char *fn);
   void ReEstimateModel(void);
   void SaveModel(char *outfn);
 
   if(InitShell(argc,argv,hrest_version,hrest_vc_id)<SUCCESS)
      HError(2200,"HRest: InitShell failed");

   InitMem();   InitLabel();
   InitMath();  InitSigP();
   InitWave();  InitAudio();
   InitVQ();    InitModel();
   if(InitParm()<SUCCESS)  
      HError(2200,"HRest: InitParm failed");

   InitTrain(); InitUtil();

   if (!InfoPrinted() && NumArgs() == 0)
      ReportUsage();
   if (NumArgs() == 0) Exit(0);
   SetConfParms();
   CreateHMMSet(&hset,&gstack,FALSE);
   while (NextArg() == SWITCHARG) {
      s = GetSwtArg();
      if (strlen(s)!=1) 
         HError(2219,"HRest: Bad switch %s; must be single letter",s);
      switch(s[0]){      
      case 'e':
         epsilon = GetChkedFlt(0.0,1.0,s); break;
      case 'g':
         calcDuration = TRUE;
         if (NextArg()!=STRINGARG)
            HError(2219,"HRest: duration model file name expected");
         durFN = GetStrArg(); break;
      case 'i':
         maxIter = GetChkedInt(1,100,s); break;
      case 'l':
         if (NextArg() != STRINGARG)
            HError(2219,"HRest: Segment label expected");
         segLab = GetStrArg();
         break;
      case 'm':
         minSeg = GetChkedInt(1,1000,s); break;
      case 't':
         segReject = FALSE;
         break;
      case 'u':
         SetuFlags(); break;
      case 'v':
         minVar = GetChkedFlt(0.0,100.0,s); break;
      case 'c':
         tMPruneThresh = GetChkedFlt(0.0,1000.0,s); break;
      case 'o':
         outfn = GetStrArg();
         break;
      case 'w':
         mixWeightFloor = MINMIX * GetChkedFlt(0.0,10000.0,s); break;
      case 'B':
         saveBinary = TRUE;
         break;
      case 'F':
         if (NextArg() != STRINGARG)
            HError(2219,"HRest: Data File format expected");
         if((dff = Str2Format(GetStrArg())) == ALIEN)
            HError(-2289,"HRest: Warning ALIEN Data file format set");
         break;
      case 'G':
         if (NextArg() != STRINGARG)
            HError(2219,"HRest: Label File format expected");
         if((lff = Str2Format(GetStrArg())) == ALIEN)
            HError(-2289,"HRest: Warning ALIEN Label file format set");
         break;
      case 'H':
         if (NextArg() != STRINGARG)
            HError(2219,"HRest: HMM macro file name expected");
         AddMMF(&hset,GetStrArg());
         break;
      case 'I':
         if (NextArg() != STRINGARG)
            HError(2219,"HRest: MLF file name expected");
         LoadMasterFile(GetStrArg());
         break;
      case 'L':
         if (NextArg()!=STRINGARG)
            HError(2219,"HRest: Label file directory expected");
         labDir = GetStrArg(); break;
      case 'M':
         if (NextArg()!=STRINGARG)
            HError(2219,"HRest: Output macro file directory expected");
         outDir = GetStrArg(); break;  
      case 'T':
         trace = GetChkedInt(0,0100000,s); break;
      case 'X':
         if (NextArg()!=STRINGARG)
            HError(2219,"HRest: Label file extension expected");
         labExt = GetStrArg(); break;
      default:
         HError(2219,"HRest: Unknown switch %s",s);
      }
   }
   if (NextArg()!=STRINGARG)
      HError(2219,"HRest: source HMM file name expected");
   hmmfn = GetStrArg();
   if (outfn==NULL) outfn = hmmfn;
   Initialise1();
   do {
      if (NextArg()!=STRINGARG)
         HError(2219,"HRest: training data file name expected");
      datafn = GetStrArg();
      LoadFile(datafn);
   } while (NumArgs()>0);
   nSeg = NumSegs(segStore);
   if (nSeg < minSeg)
      HError(2221,"HRest: Too Few Training Examples [%d]",nSeg);
   Initialise2();
   if (trace&T_TOP) {
      printf("%d Examples loaded, Max length = %d, Min length = %d\n",
             nSeg, maxT,minT);
      fflush(stdout);
   }
   ReEstimateModel();

   if(SaveHMMSet(&hset,outDir,NULL,NULL,saveBinary)<SUCCESS)
      HError(2211,"HRest: SaveHMMSet failed");

   ResetUtil();
   ResetTrain();
   ResetParm();
   ResetModel();
   ResetVQ();
   ResetAudio();
   ResetWave();
   ResetSigP();
   ResetMath();
   ResetLabel();
   ResetMem();
   ResetShell();
   
   Exit(0);
   return (0);          /* never reached -- make compiler happy */
}

/* ------------------------ Initialisation ----------------------- */

/* PrintInitialInfo: print a header of program settings */
void PrintInitialInfo(void)
{   
   printf("Reestimating HMM %s . . . \n",hmmfn);
   PrintHMMProfile(stdout, hmm);
   if (segLab==NULL)
      printf(" SegLab   :  NONE\n");
   else
      printf(" SegLab   :  %s\n",segLab);
   printf(" MaxIter  :  %d\n",maxIter);
   printf(" Epsilon  :  %f\n",epsilon);
   printf(" Updating :  ");
   if (uFlags&UPTRANS) printf("Transitions "); 
   if (uFlags&UPMEANS) printf("Means "); 
   if (uFlags&UPVARS)  printf("Variances "); 
   if (uFlags&UPMIXES && maxMixes>1)  printf("MixWeights"); 
   printf("\n\n");
   printf(" - system is ");
   switch (hset.hsKind){
   case PLAINHS:  printf("PLAIN\n");  break;
   case SHAREDHS: printf("SHARED\n"); break;
   case TIEDHS:   printf("TIED\n");   break;
   case DISCRETEHS: printf("DISCRETE\n"); break;
   }   
   fflush(stdout);
}
   
/* Initialise1: 1st phase of init prior to loading dbase */
void Initialise1(void)
{
   MLink link;
   LabId  hmmId;
   char base[MAXSTRLEN];
   char path[MAXSTRLEN];
   char ext[MAXSTRLEN];
   int s;

   /* Load HMM def */
   if(MakeOneHMM( &hset,BaseOf(hmmfn,base))<SUCCESS)
      HError(2128,"Initialise1: MakeOneHMM failed");
   if(LoadHMMSet( &hset,PathOf(hmmfn,path),ExtnOf(hmmfn,ext))<SUCCESS)
      HError(2128,"Initialise1: LoadHMMSet failed");
   SetParmHMMSet(&hset);
   if (hset.hsKind!=PLAINHS)
      uFlags = (UPDSet) (uFlags & (~(UPMEANS|UPVARS)));

   /* Get a pointer to the physical HMM and set related globals */
   hmmId = GetLabId(base,FALSE);
   link = FindMacroName(&hset,'h',hmmId);
   hmm = (HLink)link->structure;  
   nStates = hmm->numStates;
   nStreams = hset.swidth[0];
   hsKind = hset.hsKind;
   
   /* Stacks for global structures requiring memory allocation */
   CreateHeap(&segmentStack,"SegStore", MSTAK, 1, 0.0, 100000, LONG_MAX);
   CreateHeap(&alphaBetaStack,"AlphaBetaStore", MSTAK, 1, 0.0, 1000, 1000);
   CreateHeap(&accsStack,"AccsStore", MSTAK, 1, 0.0, 1000, 1000);
   CreateHeap(&transStack,"TransStore", MSTAK, 1, 0.0, 1000, 1000);
   CreateHeap(&bufferStack,"BufferStore", MSTAK, 1, 0.0, 1000, 1000);
   AttachAccs(&hset, &accsStack, uFlags);

   SetVFloor( &hset, vFloor, minVar);

   if(segLab != NULL)
      segId = GetLabId(segLab,TRUE);
   if(trace&T_TOP)
      PrintInitialInfo();

   maxMixes = MaxMixtures(hmm);
   for(s=1; s<=nStreams; s++)
      maxMixInS[s] = MaxMixInS(hmm, s);
   T = maxT = 0; minT = 100000;
}

/* Initialise2: 2nd phase of init after loading dbase */
void Initialise2(void)
{
   int t,j,m,s;

   alpha = CreateDMatrix(&alphaBetaStack,nStates,maxT);
   beta = CreateDMatrix(&alphaBetaStack,nStates,maxT);
   outprob = CreateMatrix(&alphaBetaStack,nStates-1,maxT); /* row 1 not used */
   ZeroMatrix(outprob);
   if (maxMixes>1){
      mixoutp = (Matrix**)New(&alphaBetaStack, (nStates-2)*sizeof(Matrix*));
      mixoutp -= 2;
      for (j=2;j<nStates;j++){
         mixoutp[j] = (Matrix*)New(&alphaBetaStack, maxT*sizeof(Matrix));
         --mixoutp[j];
         for (t=1;t<=maxT;t++){
            mixoutp[j][t] = CreateMatrix(&alphaBetaStack,nStreams,maxMixes);
            for (s=1;s<=nStreams;s++){
               for (m=1;m<=maxMixes;m++)
                  mixoutp[j][t][s][m]=LZERO;
            }
         }
      }
   }
   if (nStreams>1){
      stroutp = (Vector**)New(&alphaBetaStack, maxT*sizeof(Vector*));
      --stroutp;
      for (t=1;t<=maxT;t++){
         stroutp[t] = (Vector*)New(&alphaBetaStack,(nStates-2)*sizeof(Vector));
         stroutp[t] -= 2;
         for (j=2;j<nStates;j++)
            stroutp[t][j] = CreateVector(&alphaBetaStack,nStreams);
      }
   }
   occr = CreateVector(&gstack,nStates-1);
   zot = CreateVector(&gstack,hset.vecSize);
   
   if (calcDuration) {
      durOcc = CreateDVector(&accsStack, nStates);
      durSum = CreateDVector(&accsStack, nStates);
      durSqr = CreateDVector(&accsStack, nStates);
   }
}

/* ---------------------------- Load Data ------------------------- */


/* CheckData: check data file consistent with HMM definition */
void CheckData(char *fn, BufferInfo info) 
{
   char tpk[80];
   char mpk[80];
   
   if (info.tgtPK != hset.pkind)
      HError(2250,"CheckData: Parameterisation in %s[%s] is incompatible with hmm %s[%s]",
             fn,ParmKind2Str(info.tgtPK,tpk),hmmfn,ParmKind2Str(hset.pkind,mpk));

   if (info.tgtVecSize!=hset.vecSize)
      HError(2250,"CheckData: Vector size in %s[%d] is incompatible with hmm %s[%d]",
             fn,info.tgtVecSize,hmmfn,hset.vecSize);
}

/* InitSegStore : Initialise segStore for particular observation */
void InitSegStore(BufferInfo *info)
{
   Observation obs;
   Boolean eSep;

   SetStreamWidths(info->tgtPK,info->tgtVecSize,hset.swidth,&eSep);
   obs = MakeObservation(&gstack,hset.swidth,info->tgtPK,
                         ((hset.hsKind==DISCRETEHS) ? TRUE:FALSE),eSep);
   segStore = CreateSegStore(&segmentStack,obs,10);
   firstTime = FALSE;
}

/* LoadFile: load whole file or segments into segStore */
void LoadFile(char *fn)
{
   BufferInfo info;
   char labfn[MAXSTRLEN];
   Transcription *trans;
   long segStIdx,segEnIdx;
   static int segIdx=1;  /* Between call handle on latest seg in segStore */  
   static int prevSegIdx=1;
   HTime tStart, tEnd;
   int k,i,s,ncas,nObs,segLen;
   LLink p;
   Observation obs;

   if((pbuf=OpenBuffer(&bufferStack, fn, 10, dff, FALSE_dup, FALSE_dup))==NULL)
      HError(2250,"LoadFile: Config parameters invalid");
   GetBufferInfo(pbuf,&info);
   CheckData(fn,info);
   if (firstTime) InitSegStore(&info);

   if (segId == NULL)  {   /* load whole parameter file */
      nObs = ObsInBuffer(pbuf);
      tStart = 0.0;
      tEnd = (info.tgtSampRate * nObs);
      LoadSegment(segStore, tStart, tEnd, pbuf);
      if (nObs > maxT) 
         maxT=nObs; 
      if (nObs < minT)
         minT=nObs;      
      segIdx++;
   }
   else {                  /* load segment of parameter file */
      MakeFN(fn,labDir,labExt,labfn);
      trans = LOpen(&transStack,labfn,lff);
      ncas = NumCases(trans->head,segId);      
      nObs = 0;
      if ( ncas > 0) {
         for (i=1,nObs=0; i<=ncas; i++) {
            p = GetCase(trans->head,segId,i);
            segStIdx= (long) (p->start/info.tgtSampRate);
            segEnIdx  = (long) (p->end/info.tgtSampRate);
            if (segEnIdx >= ObsInBuffer(pbuf)) 
               segEnIdx = ObsInBuffer(pbuf)-1;
            if (((segEnIdx - segStIdx + 1 >= nStates-2) || !segReject) 
		&& (segStIdx <= segEnIdx)) {	/* skip short segments */
               LoadSegment(segStore, p->start, p->end, pbuf);
               if (trace&T_LD1)
                  printf("  loading seg %s %f[%ld]->%f[%ld]\n",segId->name,
                         p->start,segStIdx,p->end,segEnIdx);
               segLen = SegLength(segStore, segIdx);
               nObs += segLen;
               if (segLen > maxT) 
                  maxT=segLen; 
               if (segLen < minT)
                  minT=segLen;
               segIdx++;
            }else if (trace&T_LD1)
               printf("   seg %s %f->%f ignored\n",segId->name,
                      p->start,p->end);
         }        
      }   
   }
   if (hset.hsKind == DISCRETEHS){
      for (k=prevSegIdx; k<segIdx; k++){
         segLen = SegLength(segStore, k);
         for (i=1; i<=segLen; i++){
            obs = GetSegObs(segStore, k, i);
            for (s=1; s<=nStreams; s++){
               if( (obs.vq[s] < 1) || (obs.vq[s] > maxMixInS[s]))
                  HError(2250,"LoadFile: Discrete data value [ %d ] out of range in stream [ %d ] in file %s",obs.vq[s],s,fn);
            }
         }
      }
      prevSegIdx=segIdx;
   }

   if (trace&T_LD0)
      printf(" %d observations loaded from %s\n",nObs,fn);
   CloseBuffer(pbuf);
   ResetHeap(&transStack);
}


/* ------------------------ Trace Functions -------------------- */

/* ShowSegNum: if not already printed, print seg number */
void ShowSegNum(const int seg)
{
   static int lastseg = -1;
   
   if (seg != lastseg){
      printf("---- Training Segment %d [%3d frames] ----\n",seg,T);
      lastseg = seg;
   }
}
   
/* ------------------------- Alpha-Beta ------------------------ */

/* SetOutP: Set the output and mix prob matrices */                        
void SetOutP(const int seg)
{
   int i,t,m,mx,s,nMix=0;
   StreamElem *ste;
   StreamInfo *sti;
   MixtureElem *me;
   StateInfo *si;
   Matrix mixp;
   LogFloat x,prob,streamP;
   Vector strp = NULL;
   Observation obs;
   TMixRec *tmRec = NULL;
   float wght=0.0,tmp;
   MixPDF *mpdf=NULL;
   PreComp *pMix;
   
   for (t=1;t<=T;t++) {
      obs = GetSegObs(segStore, seg, t);
      if (hsKind == TIEDHS)
         PrecomputeTMix(&hset,&obs,tMPruneThresh,0);         
      if ((maxMixes>1) && (hsKind!=DISCRETEHS)){ /* Multiple Mix Case */
         for (i=2;i<nStates;i++) {
            prob = 0.0;
            si = hmm->svec[i].info;
            ste = si->pdf+1; 
            mixp = mixoutp[i][t];
            if (nStreams>1) strp = stroutp[t][i];
            for (s=1;s<=nStreams;s++,ste++){
               sti = ste->info;
               switch (hsKind){         /* Get nMix */
               case TIEDHS:
                  tmRec = &(hset.tmRecs[s]);
                  nMix = tmRec->nMix;
                  break;
               case PLAINHS:
               case SHAREDHS:
                  nMix = sti->nMix;
                  break;
               }
               streamP = LZERO;
               for (mx=1;mx<=nMix;mx++) {
                  m=(hsKind==TIEDHS)?tmRec->probs[mx].index:mx;
                  switch (hsKind){      /* Get wght and mpdf */
                  case TIEDHS:
                     wght=sti->spdf.tpdf[m];
                     mpdf=tmRec->mixes[m];
                     break;
                  case PLAINHS:
                  case SHAREDHS:
                     me = sti->spdf.cpdf+m;
                     wght=me->weight;
                     mpdf=me->mpdf;
                     break;
                  }
                  if (wght>MINMIX){
                     switch(hsKind) { /* Get mixture prob */
                     case TIEDHS:
                        tmp = tmRec->probs[mx].prob;
                        x = (tmp>=MINLARG)?log(tmp)+tmRec->maxP:LZERO;
                        break;
                     case SHAREDHS : 
                        pMix = (PreComp *)mpdf->hook;
                        if (pMix->time==t)
                           x = pMix->prob;
                        else {
                           x = MOutP(obs.fv[s],mpdf);
                           pMix->prob = x; pMix->time = t;
                        }
                        break;
                     case PLAINHS : 
                        x=MOutP(obs.fv[s],mpdf);
                        break;
                     default:
                        x=LZERO;
                        break;
                     }
                     mixp[s][m]=x;
                     streamP = LAdd(streamP,log(wght)+x);
                  } else
                     mixp[s][m]=LZERO;
               }               
               if (nStreams>1) {
                  strp[s]=si->weights[s]*streamP;
                  prob += si->weights[s]*streamP;
               }
               else {
                  prob += streamP;
               }
            }   
            outprob[i][t]=prob;
         }
      } else 
         if (nStreams>1) {      /* Single Mixture multiple stream */
            for (i=2;i<nStates;i++) {
               prob = 0.0;
               si = hmm->svec[i].info;
               ste = si->pdf+1;
               strp = stroutp[t][i];
               for (s=1;s<=nStreams;s++,ste++){
                  sti = ste->info;
                  streamP = SOutP(&hset,s,&obs,sti);
                  strp[s] = si->weights[s]*streamP;
                  prob += si->weights[s]*streamP; /* note stream weights ignored */
               }
               outprob[i][t]=prob;
            }
         } else                 /* Single Mixture - Single Stream */
            for (i=2;i<nStates;i++){
               si = hmm->svec[i].info;
               ste = si->pdf+1;
               if (hsKind==DISCRETEHS)
                  outprob[i][t]=SOutP(&hset,1,&obs,ste->info);
               else
                  outprob[i][t]=OutP(&obs,hmm,i);
            }
   }
   if (trace  & T_OTP) {
      ShowSegNum(seg);
      ShowMatrix("OutProb",outprob,10,12);
   }
}

/* SetAlpha: compute alpha matrix and return prob of given sequence */
LogDouble SetAlpha(const int seg)
{
   int i,j,t;
   LogDouble x,a;

   alpha[1][1] = 0.0;
   for (j=2;j<nStates;j++) {              /* col 1 from entry state */
      a=hmm->transP[1][j];
      if (a<LSMALL)
         alpha[j][1] = LZERO;
      else
         alpha[j][1] = a+outprob[j][1];
   }
   alpha[nStates][1] = LZERO;
   
   for (t=2;t<=T;t++) {             /* cols 2 to T */
      for (j=2;j<nStates;j++) {
         x=LZERO ;
         for (i=2;i<nStates;i++) {
            a=hmm->transP[i][j];
            if (a>LSMALL)
               x = LAdd(x,alpha[i][t-1]+a);
         }
         alpha[j][t]=x+outprob[j][t];
      }
      alpha[1][t] = alpha[nStates][t] = LZERO;
   }
   x = LZERO ;                      /* finally calc seg prob */
   for (i=2;i<nStates;i++) {
      a=hmm->transP[i][nStates];
      if (a>LSMALL)
         x=LAdd(x,alpha[i][T]+a); 
   }  
   alpha[nStates][T] = x;
   
   if (trace  & T_ALF) {
      ShowSegNum(seg);
      ShowDMatrix("Alpha",alpha,10,12); 
      printf("LogP= %10.3f\n\n",x);
   }
   return x;
}

/* SetBeta: compute beta matrix */
LogDouble SetBeta(const int seg)
{
   int i,j,t;
   LogDouble x,a;

   beta[nStates][T] = 0.0;
   for (i=2;i<nStates;i++)                /* Col T from exit state */
      beta[i][T]=hmm->transP[i][nStates];
   beta[1][T] = LZERO;
   for (t=T-1;t>=1;t--) {           /* Col t from col t+1 */
      for (i=1;i<=nStates;i++)
         beta[i][t]=LZERO ;
      for (j=2;j<nStates;j++) {
         x=outprob[j][t+1]+beta[j][t+1];
         for (i=2;i<nStates;i++) {
            a=hmm->transP[i][j];
            if (a>LSMALL)
               beta[i][t]=LAdd(beta[i][t],x+a);
         }
      }
   }
   x=LZERO ;
   for (j=2;j<nStates;j++) {
      a=hmm->transP[1][j];
      if (a>LSMALL)
         x=LAdd(x,beta[j][1]+a+outprob[j][1]); 
   }
   beta[1][1] = x;
   if (trace & T_BET) {
      ShowSegNum(seg);
      ShowDMatrix("Beta",beta,10,12); 
      printf("LogP=%10.3f\n\n",beta[1][1]);
   }
   return x;
}

/* --------------------- Record Statistics ---------------- */

/* SetOccr: set the global occupation counters occr for current seg */
void SetOccr(const LogDouble pr, const int seg)
{
   int i,t;
   DVector alpha_i,beta_i;
   Vector a_i;
   LogDouble x;
   
   occr[1] = 1.0;
   for (i=2;i<nStates;i++) {
      alpha_i = alpha[i]; beta_i = beta[i];
      a_i = hmm->transP[i];
      x=LZERO ;
      for (t=1;t<=T;t++)
         x=LAdd(x,alpha_i[t]+beta_i[t]);
      x -= pr;
      if (x>MINEARG) 
         occr[i] = exp(x);
      else
         occr[i] = 0.0;
   }
   if (trace & T_OCC){
      ShowSegNum(seg);
      ShowVector("OCC: ",occr,20);
   }
}

/* UpTranCounts: update the transition counters in ta */
void UpTranCounts(const LogDouble pr, const int seg)
{
   int i,j,t;
   Matrix tran;
   Vector tran_i,outprob_j,a_i,occ;
   DVector alpha_i,beta_j;
   LogDouble x,a_ij;
   double y;
   TrAcc *ta;
   
   ta = (TrAcc *) GetHook(hmm->transP);
   tran = ta->tran; occ = ta->occ;
   for (i=2; i<nStates; i++)
      occ[i] += occr[i];
   tran_i = tran[1];          /* transitions 1->j    1<j<nStates */
   a_i = hmm->transP[1];
   for (j=2;j<nStates;j++) {
      a_ij = a_i[j];
      if (a_ij>LSMALL) {
         x = a_ij + outprob[j][1] + beta[j][1] - pr;
         if (x>MINEARG) {
            y =  exp(x);
            tran_i[j] += y; occ[1] += y;
         }
      }
   }
   for (i=2;i<nStates;i++) {        /* transitions i->j    1<i,j<nStates */
      a_i = hmm->transP[i];
      alpha_i = alpha[i];
      tran_i = tran[i];
      for (j=2;j<nStates;j++) {
         a_ij=a_i[j];
         if (a_ij>LSMALL) {
            x=LZERO; beta_j=beta[j]; outprob_j=outprob[j];
            for (t=1;t<=T-1;t++)
               x=LAdd(x,alpha_i[t]+a_ij+outprob_j[t+1]+beta_j[t+1]);
            x -= pr;
            if (x>MINEARG)
               tran_i[j] += exp(x);
         }
      }
   }
   for (i=2; i<nStates; i++) {    /* transitions i->nStates    1<i<nStates */
      a_ij = hmm->transP[i][nStates];
      if (a_ij>LSMALL) {
         x = a_ij + alpha[i][T] - pr;
         if (x>MINEARG)
            tran[i][nStates] += exp(x);
      }     
   }
   if (trace & T_TAC){
      ShowSegNum(seg);
      ShowMatrix("TRAN: ",tran,10,10);
      ShowVector("TOCC: ",occ,10);
      fflush(stdout);
   }
}

/* UpStreamCounts: update mean, cov & mixweight counts for given stream */
void UpStreamCounts(const int j, const int s, StreamInfo *sti, 
                    int vSize, const LogDouble pr, const int seg,
                    DVector alphj, DVector betaj)
{
   int i,m,nMix=0,k,l,t,ss,idx;
   MixtureElem *me;
   MixPDF *mpdf=NULL;
   MuAcc *ma=NULL;
   WtAcc *wa;
   VaAcc *va=NULL;
   Matrix *mixp_j;
   Vector ot, strpt;
   LogFloat a_ij,w;
   LogDouble Lr;
   double y;
   Observation obs;
   TMixRec *tmRec = NULL;
   float wght=0.0;
   
   wa = (WtAcc *)sti->hook;
   switch (hsKind){       /* Get nMix */
   case TIEDHS:
      tmRec = &(hset.tmRecs[s]);
      nMix = tmRec->nMix;
      break;
   case PLAINHS:
   case SHAREDHS:
      nMix = sti->nMix;
      break;      
   case DISCRETEHS:
      nMix = 1;                /* Only one code selected per observation */
      break;
   }
   mixp_j = (maxMixes>1) ? mixoutp[j] : NULL;
   for (m=1; m<=nMix; m++) {
      switch (hsKind){            /* Get mpdf, wght */
      case TIEDHS:               
         wght=sti->spdf.tpdf[m];
         mpdf=tmRec->mixes[m];
         break;
      case DISCRETEHS:
         wght=1.0;                 /* weight for DISCRETEHS has to be 1 */
         mpdf=NULL;
         break;
      case PLAINHS:
      case SHAREDHS:
         me = sti->spdf.cpdf+m;
         wght=me->weight;
         mpdf=me->mpdf;
         if(hset.msdflag[s]) vSize = VectorSize(mpdf->mean);
         break;
      }
      if (hsKind!=DISCRETEHS){
         ma = (MuAcc *)GetHook(mpdf->mean);
         va = (VaAcc *)GetHook(mpdf->cov.var);
      }
      if (wght > MINMIX) {
         w = log(wght);
         for (t=1; t<=T; t++) {
         
            /* Get observation vec ot and zero mean zot */
            obs = GetSegObs(segStore, seg, t);
            ot = obs.fv[s];
            if (hsKind!=DISCRETEHS)
               for (k=1; k<=vSize; k++)
                  zot[k] = ot[k] - mpdf->mean[k];
                                 
            /* Compute state/mix occupation log likelihood */
            if (nMix==1 || (hsKind==DISCRETEHS)){
               Lr = alphj[t]+betaj[t] - pr;
            }
            else {
               if (t==1)
                  Lr = hmm->transP[1][j];
               else {
                  Lr = LZERO;
                  for (i=2; i<nStates; i++)
                     if ((a_ij=hmm->transP[i][j]) > LSMALL)
                        Lr = LAdd(Lr,alpha[i][t-1]+a_ij);
               }
               if (Lr>LSMALL) {
                  Lr += mixp_j[t][s][m] + w + betaj[t] - pr;
                  if (nStreams>1) { /* add contrib of parallel streams */
                     strpt = stroutp[t][j];
                     for (ss=1; ss<=nStreams; ss++)
                        if (ss!=s) Lr += strpt[ss];
                  }
               }
            }
            
            if (Lr > MINEARG) {
               y = exp(Lr);
                  
               /* Update Weight Counter */
               if (uFlags&UPMIXES) {   
                  idx = (hsKind==DISCRETEHS) ? obs.vq[s] : m;
                  wa->occ += y; wa->c[idx] += y;
               }
               
               /* Update Mean Counter */
               if (uFlags&UPMEANS){
                  ma->occ += y; 
                  for (k=1; k<=vSize; k++)
                     ma->mu[k] += zot[k]*y; /* sum zero mean */
               }
               
               /* Update Covariance Counter */
               if (uFlags&UPVARS){
                  va->occ += y;
                  if (mpdf->ckind==DIAGC){
                     for (k=1; k<=vSize; k++)
                        va->cov.var[k] += zot[k] * zot[k] * y;
                  } else{
                     for (k=1; k<=vSize; k++)
                        for (l=1; l<=k; l++)
                           va->cov.inv[k][l] += zot[k] * zot[l] * y;
                  }
               }  
            }
         }
      }
      if ((trace&(T_MAC|T_VAC))&&(hsKind!=DISCRETEHS)) {
         ShowSegNum(seg);
         printf("State %d, Stream %d, Mixture %d\n",j,s,m);
         if (trace&T_MAC){
            printf("MEAN OCC: %.2f\n",ma->occ);
            ShowVector("MEAN ACC: ",ma->mu,10);
         }
         if (trace&T_VAC) {
            if (mpdf->ckind==DIAGC){
               printf("VAR OCC: %.2f\n",va->occ);
               ShowVector("VAR ACC: ",va->cov.var,10);
            } else {
               printf("INV OCC: %.2f\n",va->occ);
               ShowMatrix("INV ACC: ",va->cov.inv,10,10);
            }
         }
      }
   }
   if (trace&T_WAC){
      ShowSegNum(seg);
      printf("State %d, Stream %d\n",j,s);
      printf("WT OCC: %.2f\n",wa->occ);
      ShowVector("WT ACC: ",wa->c,10);
   }     
}
   
/* UpPDFCounts: update output PDF counts for each stream of each state */
void UpPDFCounts(const LogDouble pr, const int seg)
{
   int j,s;
   StateInfo *si;
   StreamElem *ste;
   DVector alj,betj;

   for (j=2; j<nStates; j++) {
      si = hmm->svec[j].info;
      alj = alpha[j]; betj = beta[j];
      for (s=1,ste = si->pdf+1; s<=nStreams; s++,ste++)
         UpStreamCounts(j,s,ste->info,hset.swidth[s],pr,seg,alj,betj);
   }
}

/* UpDurCounts: update duration counts */
void UpDurCounts(const LogDouble pr, const int seq)
{
   int j,k,t0,t1;
   LogDouble x,x0,Sumx;
   
   for (j=2;j<nStates;j++) { 
      for (t0=1;t0<=T;t0++) {
         if (t0 == 1) x0 = hmm->transP[1][j]; 
         else {
            x0 = LZERO;
            for (k=2; k<nStates; k++)
               if (k!=j && hmm->transP[k][j]>LSMALL)
                  x0 = LAdd(x0, alpha[k][t0-1]+hmm->transP[k][j]);
         }
               
         Sumx = x0;
              
         /* from t0 to t1 */ 
         for (t1=t0; t1<=T; t1++) {  
            if (Sumx>LSMALL) {
               Sumx += outprob[j][t1];
               if (t1!=t0) 
                  Sumx += (double)hmm->transP[j][j];

               if (t1==T) { 
                  x = hmm->transP[j][nStates];
               }
               else {
                  x = LZERO;
                  for (k=2; k<nStates; k++)
                     if (k!=j && hmm->transP[j][k]>LSMALL)
                        x = LAdd(x, (double)hmm->transP[j][k]
                                   +(double)outprob[k][t1+1]+beta[k][t1+1]);
               } 
                  
               x = x+Sumx-pr;
                  
               /* update statistics */
               if (x>LSMALL) {
                  durSqr[j-1] = LAdd(durSqr[j-1], x+2*log(t1-t0+1)); 
                  durSum[j-1] = LAdd(durSum[j-1], x+log(t1-t0+1)); 
                  durOcc[j-1] = LAdd(durOcc[j-1], x);
               }      
            } 
            else 
               break; 
         }
      } 
   }  
} 
 

/* UpdateCounters: update the various counters */
void UpdateCounters(const LogDouble pr, const int seg)
{
   SetOccr(pr,seg);
   if (uFlags&UPTRANS) 
      UpTranCounts(pr,seg);
   if (uFlags&(UPMEANS|UPVARS|UPMIXES))
      UpPDFCounts(pr,seg);
}

/* ------------------------- Model Update ----------------------- */

/* RestTransP: reestimate transition probs */
void RestTransP(void)
{
   int i,j;
   float occi,x,sum;
   TrAcc *ta;

   ta = (TrAcc *) GetHook(hmm->transP);
   
   for (i=1;i<nStates;i++) {
      hmm->transP[i][1] = LZERO;
      occi = ta->occ[i];
      if (occi == 0.0)
         HError(2222,"RestTransP: Zero state %d occupation count",i);
      sum = 0.0;
      for (j=2;j<=nStates;j++) {
         x = ta->tran[i][j]/occi;
         hmm->transP[i][j] = x; sum += x;
      }
      for (j=2;j<=nStates;j++) {
         x = hmm->transP[i][j]/sum;
         hmm->transP[i][j] = (x<MINLARG) ? LZERO : log(x);
      }
   }
   if (trace & T_TRE)
      ShowMatrix("NEW TRANS: ",hmm->transP,10,10);
}

/* FloorMixes: apply floor to given mix set */
void FloorMixes(MixtureElem *mixes, const int M, const float floor)
{
   float sum,fsum,scale;
   MixtureElem *me;
   int m;
   
   sum = fsum = 0.0;
   for (m=1,me=mixes; m<=M; m++,me++) {
      if (me->weight>floor)
         sum += me->weight;
      else {
         fsum += floor; me->weight = floor;
      }
   }
   if (fsum>1.0)
      HError(2223,"FloorMixes: Floor sum too large");
   scale = (1.0-fsum)/sum;
   if (trace&T_WRE) printf("MIXW: ");
   for (m=1,me=mixes; m<=M; m++,me++){
      if (me->weight>floor)
         me->weight *= scale;
      if (trace&T_WRE) printf(" %.2f",me->weight);
   }
   if (trace&T_WRE) printf("\n");
}  

/* FloorTMMixes: apply floor to given tied mix set */
void FloorTMMixes(Vector mixes, const int M, const float floor)
{
   float sum,fsum,scale,fltWt;
   int m;
   
   sum = fsum = 0.0;
   for (m=1; m<=M; m++) {
      fltWt = mixes[m];
      if (fltWt>floor)
         sum += fltWt;
      else {
         fsum += floor;
         mixes[m] = floor;
      }
   }
   if (fsum>1.0) HError(2223,"FloorTMMixes: Floor sum too large");
   scale = (1.0-fsum)/sum;
   if (trace&T_WRE) printf("MIXW: ");
   for (m=1; m<=M; m++){
      fltWt = mixes[m];
      if (fltWt>floor)
         mixes[m] = fltWt*scale;
      if (trace&T_WRE) printf(" %.2f",fltWt);
   }
}

/* FloorDProbs: apply floor to given discrete prob set */
void FloorDProbs(ShortVec mixes, const int M, const float floor)
{
   float sum,fsum,scale,fltWt;
   int m;
   
   sum = fsum = 0.0;
   for (m=1; m<=M; m++) {
      fltWt = Short2DProb(mixes[m]);
      if (fltWt>floor)
         sum += fltWt;
      else {
         fsum += floor;
         mixes[m] = DProb2Short(floor);
      }
   }
   if (fsum>1.0) HError(2327,"FloorDProbs: Floor sum too large");
   if (fsum == 0.0) return;
   if (sum == 0.0) HError(2328,"FloorDProbs: No probabilities above floor");
   scale = (1.0-fsum)/sum;
   for (m=1; m<=M; m++){
      fltWt = Short2DProb(mixes[m]);
      if (fltWt>floor)
         mixes[m] = DProb2Short(fltWt*scale);
   }
}

/* RestMixWeights: reestimate the mixture weights */
void RestMixWeights(const int state, const int s, StreamInfo *sti)
{
   WtAcc *wa;
   int m,M=0;
   float x;
   MixtureElem *me;
   
   wa = (WtAcc *)sti->hook;
   if (wa->occ == 0.0)
      HError(2222,"RestMixWeights: Zero weight occupation count");
   switch (hsKind){
   case TIEDHS:
      M=hset.tmRecs[s].nMix;
      break;
   case PLAINHS:
   case SHAREDHS:
   case DISCRETEHS:
      M=sti->nMix;
      break;
   }
   for (m=1; m<=M; m++){
      x = wa->c[m]/wa->occ;
      if (x>1.0) 
         HError(2290,"RestMixWeights: Mix wt>1 in %d.%d.%d",state,s,m);
      switch (hsKind){
      case DISCRETEHS:
         sti->spdf.dpdf[m] = (x>MINMIX) ? DProb2Short(x) : DLOGZERO;
         break;
      case TIEDHS:
         sti->spdf.tpdf[m] = (x>MINMIX) ? x : 0.0;
         break;
      case PLAINHS:
      case SHAREDHS:
         me=sti->spdf.cpdf+m;
         me->weight = (x>MINMIX) ? x : 0.0;
         break;
      }      
   }
}

/* RestMean: reestimate the given mean vector */
void RestMean(Vector mean, const int vSize)
{
   int k;
   MuAcc *ma;
   float x;
   
   ma = (MuAcc *)GetHook(mean);
   if (ma->occ == 0.0)
      HError(2222,"RestMean: Zero mean occupation count");
   for (k=1; k<=vSize; k++){
      x = mean[k] + ma->mu[k]/ma->occ;
      ma->mu[k] = mean[k];  /* remember old mean */
      mean[k] = x;
   }
   if (trace&T_MRE)
      ShowVector("MEAN: ",mean,10);
}

/* RestCoVar: reestimate the given covariance and return FALSE
              if any diagonal component == 0.0 */
Boolean RestCoVar(MixPDF *mp, const int vSize, const Vector minV,
                  Vector oldMean, Vector newMean, const Boolean shared)
{
   int k,l;
   VaAcc *va;
   float x,z;
   float muDiffk,muDiffl;
   
   va = (VaAcc *)GetHook(mp->cov.var);
   if (va->occ == 0.0)
      HError(2222,"RestCoVar: Zero variance occupation count");
   if (mp->ckind==DIAGC){
      for (k=1; k<=vSize; k++){
         muDiffk = (shared)?0.0:newMean[k]-oldMean[k];
         x = va->cov.var[k] / va->occ - muDiffk*muDiffk;
         if (x<minV[k]) x = minV[k];
         if (x<vDefunct) return FALSE;
         mp->cov.var[k] = x;
      }
      FixDiagGConst(mp);
      if (trace&T_VRE)
         ShowVector("VARS: ",mp->cov.var,10);
   } else {
      for (k=1; k<=vSize; k++){
         muDiffk = (shared)?0.0:newMean[k]-oldMean[k];
         for (l=1; l<k; l++) {
            muDiffl = (shared)?0.0:newMean[l]-oldMean[l];
            x = va->cov.inv[k][l] / va->occ - muDiffk*muDiffl;
            mp->cov.inv[k][l] = x;
         }
         z = va->cov.inv[k][k]/va->occ - muDiffk*muDiffk;
         mp->cov.inv[k][k] = (z<minV[k])?minV[k]:z;
      }
      if (trace&T_VRE)
         ShowTriMat("COVM: ",mp->cov.inv,10,10);

      FixFullGConst(mp,CovInvert(mp->cov.inv,mp->cov.inv));
   }
   return TRUE;
}

/* RestStream: reestimate stream parameters */
void RestStream(const int state, const int s, StreamInfo *sti, int vSize)
{
   int m,M;
   MixtureElem *me;
   MixPDF *mp;
   MuAcc *ma;
   Boolean shared;
   float wght;

   if (trace&(T_WRE|T_MRE|T_VRE))
      printf("State %d, Stream %d\n",state,s);
   if (uFlags&UPMIXES)
      RestMixWeights(state,s,sti);
   if ((hsKind != DISCRETEHS)&&(hsKind != TIEDHS)){ /*wts only DI'ETE & TIED*/
      M=sti->nMix;
      for (m=1; m<=M; m++){
         me = sti->spdf.cpdf+m;
         wght=me->weight;
         mp=me->mpdf;
         if(hset.msdflag[s]) vSize = VectorSize(mp->mean);
         if (wght > MINMIX) {
            if (trace&(T_MRE|T_VRE) && M>1)
               printf("Mixture %d\n",m);
            if (uFlags&UPMEANS)
               RestMean(mp->mean,vSize);
            /* NB old mean left in ma->mu */
            if (uFlags&UPVARS){
               shared = (GetUse(mp->cov.var)>1) ? TRUE : FALSE;
               ma = (MuAcc *)GetHook(mp->mean);
               if ( !RestCoVar(mp,vSize,vFloor[s],ma->mu,mp->mean,shared)) {
                  if (M > 1) {
                     HError(-2225,"RestStream: Defunct Mix %d.%d.%d",state,s,m);
                     me->weight = 0.0;
                  } else
                     HError(2222,"RestStream: Zero Covariance in %d.%d",state,s);
               }
            }
         }
      }
   }
   if (hsKind == TIEDHS)
      M=hset.tmRecs[s].nMix;
   else
      M=sti->nMix;
   if (M>1){
      switch (hsKind){
      case DISCRETEHS:
         FloorDProbs(sti->spdf.dpdf,M,mixWeightFloor);
         break;
      case TIEDHS:
         FloorTMMixes(sti->spdf.tpdf,M,mixWeightFloor);
         break;
      case PLAINHS:
      case SHAREDHS:
         FloorMixes(sti->spdf.cpdf+1,M,mixWeightFloor);
         break;
      }     
   }
}

/* UpdateTheModel: use accumulated statistics to update model */
void UpdateTheModel(void)
{
   int j,s;
   StateInfo *si;
   StreamElem *ste;

   if (uFlags&UPTRANS)
      RestTransP();
   if (uFlags&(UPMEANS|UPVARS|UPMIXES))
      for (j=2; j<nStates; j++) {
         si = hmm->svec[j].info;
         for (s=1,ste = si->pdf+1; s<=nStreams; s++,ste++)
            RestStream(j,s,ste->info,hset.swidth[s]);
      }
   if (uFlags&UPVARS)
      FixAllGConsts(&hset);
}

/* SaveDuration: save duration distribution */ 
void SaveDuration(void) 
{ 
   int i; 
   double mean, var; 
   char base[MAXSTRLEN],buf[MAXSTRLEN];
   FILE *fp;
   LabId hmmId;
   
   /* Get HMM name */
   BaseOf(hmmfn,base);
   hmmId = GetLabId(base,FALSE);
   
   if ((fp=fopen(durFN,"w")) == NULL)
      HError(2260,"SaveDuration: Can not open duration model file %s.\n", durFN);

   /* ---- Output duration model ---- */
   /* output model definition */
   fprintf(fp,"~o\n");
   fprintf(fp,"<STREAMINFO> %d", nStates-2);
   for (i=1; i<=nStates-2; i++)
      fprintf(fp," 1");
   fprintf(fp,"\n<MSDINFO> %d", nStates-2);
   for (i=1; i<=nStates-2; i++)
      fprintf(fp," 0");
   fprintf(fp,"\n<VECSIZE> %d <NULLD><DIAGC><%s>\n",nStates-2,ParmKind2Str(hset.pkind,buf));
   fprintf(fp,"~h \"%s\"\n",hmmId->name);
   fprintf(fp,"<BEGINHMM>\n<NUMSTATES> 3\n<STATE> 2\n");
               
   /* output mean & variance */
   for (i=2; i<nStates; i++) {
      fprintf(fp,"<STREAM> %d\n", i-1);
      
      /* mean */
      if (durOcc[i-1]<=LSMALL) mean = 0;
      else mean = L2F(durSum[i-1]-durOcc[i-1]);
      fprintf(fp,"<MEAN> 1\n");
      fprintf(fp," %e\n",mean);
      
      /* variance */
      if ( (durOcc[i-1]<=LSMALL) 
           ||(durSqr[i-1]-durOcc[i-1]<2*(durSum[i-1]-durOcc[i-1])) )
         var = minVar;
      else {
         var = LSub(durSqr[i-1]-durOcc[i-1], 2*(durSum[i-1]-durOcc[i-1]));
         var = L2F(var);
      }
      /* floor variance */
      if (var<minVar) var = minVar;
      fprintf(fp,"<VARIANCE> 1\n");
      fprintf(fp," %e\n",var);
   }

   /* output dummy transP */
   fprintf(fp,"<TRANSP> 3\n");
   fprintf(fp,"0 1 0\n0 0 1\n0 0 0\n");
   fprintf(fp,"<ENDHMM>\n");

   fclose(fp);
   
   return;
}

/* ------------------------- Top Level Control ----------------------- */


/* ReEstimateModel: top level of algorithm */
void ReEstimateModel(void)
{
   LogFloat segProb,oldP,newP,delta;
   LogDouble ap,bp;
   int i,converged,iteration,seg;

   iteration=0; 
   oldP=LZERO;
   do {        /*main re-est loop*/   
      ZeroAccs(&hset, uFlags); newP = 0.0; ++iteration;
      nTokUsed = 0;
      if (calcDuration)
         for (i=1;i<DVectorSize(durOcc);i++) 
            durOcc[i] = durSum[i] = durSqr[i] = LZERO;
      
      for (seg=1;seg<=nSeg;seg++) {
         T=SegLength(segStore,seg);
         SetOutP(seg);
         if ((ap=SetAlpha(seg)) > LSMALL){
            bp = SetBeta(seg);
            if (trace & T_LGP)
               printf("%d.  Pa = %e, Pb = %e, Diff = %e\n",seg,ap,bp,ap-bp);
            segProb = (ap + bp) / 2.0;  /* reduce numeric error */
            newP += segProb; ++nTokUsed;
            UpdateCounters(segProb,seg);
         } else
            if (trace&T_TOP) 
               printf("Example %d skipped\n",seg);
      }
      if (nTokUsed==0)
         HError(2226,"ReEstimateModel: No Usable Training Examples");
      UpdateTheModel();
      newP /= nTokUsed;
      delta=newP-oldP; oldP=newP;
      converged=(fabs(delta)<epsilon); 
      if (trace&T_TOP) {
         printf("Ave LogProb at iter %d = %10.5f using %d examples",
                iteration,oldP,nTokUsed);
         if (iteration > 1)
            printf("  change = %10.5f",delta);
         printf("\n");
         fflush(stdout);
      }
   } while ((iteration < maxIter) && !converged);
   
   if (calcDuration) {
      nTokUsed = 0;
      for (i=1;i<DVectorSize(durOcc);i++) 
         durOcc[i] = durSum[i] = durSqr[i] = LZERO;
      
      for (seg=1;seg<=nSeg;seg++) {
         T=SegLength(segStore,seg);
         SetOutP(seg);
         if ((ap=SetAlpha(seg)) > LSMALL){
            bp = SetBeta(seg);
            if (trace & T_LGP)
               printf("%d.  Pa = %e, Pb = %e, Diff = %e\n",seg,ap,bp,ap-bp);
            segProb = (ap + bp) / 2.0;  /* reduce numeric error */
            ++nTokUsed;
            UpDurCounts(segProb,seg);
         } else
            if (trace&T_TOP) 
               printf("Example %d skipped\n",seg);
      }
      if (nTokUsed==0)
         HError(2226,"ReEstimateModel: No Usable Training Examples");

      SaveDuration();
   }
   
   if (trace&T_TOP) {
      if (converged)
         printf("Estimation converged at iteration %d\n",iteration);
      else
         printf("Estimation aborted at iteration %d\n",iteration);
      fflush(stdout);
   }
}
/* ----------------------------------------------------------- */
/*                      END:  HRest.c                          */
/* ----------------------------------------------------------- */
