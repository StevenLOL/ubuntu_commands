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
/*          2002-2004 Cambridge University                     */
/*                    Engineering Department                   */
/*                                                             */
/*   Use of this software is governed by a License Agreement   */
/*    ** See the file License for the Conditions of Use  **    */
/*    **     This banner notice must not be removed      **    */
/*                                                             */
/* ----------------------------------------------------------- */
/*         File: HERest.c: Embedded B-W ReEstimation           */
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

char *herest_version = "!HVER!HERest:   3.4.1 [CUED 12/03/09]";
char *herest_vc_id = "$Id: HERest.c,v 1.49 2011/06/16 04:18:29 uratec Exp $";

/*
   This program is used to perform a single reestimation of
   the parameters of a set of HMMs using Baum-Welch.  Training
   data consists of one or more utterances each of which has a 
   transcription in the form of a standard label file (segment
   boundaries are ignored).  For each training utterance, a
   composite model is effectively synthesised by concatenating
   the phoneme models given by the transcription.  Each phone
   model has the usual set of accumulators allocated to it,
   these are updated by performing a standard B-W pass over
   each training utterance using the composite model. This program
   supports arbitrary parameter tying and multiple data streams.
   
   Added in V1.4 - support for tee-Models ie HMMs with a non-
   zero transition from entry to exit states.

   In v2.2 most of the core functionality has been moved to the
   library module HFB
*/

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
#include "HMap.h"
#include "HFB.h"

/* Trace Flags */
#define T_TOP   0001    /* Top level tracing */
#define T_MAP   0002    /* logical/physical hmm map */
#define T_UPD   0004    /* Model updates */


/* possible values of updateMode */
#define UPMODE_DUMP 1
#define UPMODE_UPDATE 2
#define UPMODE_BOTH 3

/* Global Settings */

static char * labDir = NULL;     /* label (transcription) file directory */
static char * labExt = "lab";    /* label file extension */
static char * hmmDir = NULL;     /* directory to look for hmm def files */
static char * hmmExt = NULL;     /* hmm def file extension */
static char * newhmmDir = NULL;  /* directory to store new hmm def files */
static char * newhmmExt = NULL;  /* extension of new reestimated hmm files */
static char * statFN;            /* stats file, if any */
static float minVar  = 0.0;      /* minimum variance (diagonal only) */
static float mixWeightFloor=0.0; /* Floor for mixture weights */
static int minEgs    = 3;        /* min examples to train a model */
static UPDSet uFlags_hmm = (UPDSet) (UPMEANS|UPVARS|UPTRANS|UPMIXES); /* update flags for HMMs */
static UPDSet uFlags_dur = (UPDSet) 0; /* update flags for duration models */
static int parMode   = -1;       /* enable one of the // modes */
static Boolean stats = FALSE;    /* enable statistics reports */
static char * mmfFn  = NULL;     /* output MMF file, if any */
static int trace     = 0;        /* Trace level */
static Boolean saveBinary = FALSE;  /* save output in binary  */
static Boolean ldBinary = TRUE;        /* load/dump in binary */
static Boolean applyVFloor = FALSE;  /* variance flooring */
static Boolean useAlign = FALSE;     /* use model alignment from label */
static FileFormat dff=UNDEFF;       /* data file format */
static FileFormat lff=UNDEFF;       /* label file format */
static int updateMode = UPMODE_UPDATE; /* dump summed accs, update models or do both? */


static ConfParam *cParm[MAXGLOBS];   /* configuration parameters */
static int nParm = 0;               /* total num params */
Boolean keepOccm = FALSE;

static Boolean al_hmmUsed = FALSE;   /* Set for 2-model ReEstimation */
static char al_hmmDir[MAXFNAMELEN];  /* dir to look for alignment hmm defs */
static char al_hmmExt[MAXSTRLEN];  	 /* alignment hmm def file extension */
static char al_hmmMMF[MAXFNAMELEN];  /* alignment hmm MMF */
static char al_hmmLst[MAXFNAMELEN];  /* alignment hmm list */
static char up_hmmMMF[MAXFNAMELEN];  /* alignment hmm list */
static HMMSet al_hset;               /* Option 2nd set of HMMs for alignment */

/* Global Data Structures - valid for all training utterances */
static LogDouble pruneInit = NOPRUNE;    /* pruning threshold initially */
static LogDouble pruneInc = 0.0;         /* pruning threshold increment */
static LogDouble pruneLim = NOPRUNE;     /* pruning threshold limit */
static float minFrwdP = NOPRUNE;         /* mix prune threshold */

static Boolean twoDataFiles = FALSE; /* Enables creation of ot2 for FB
                                        training using two data files */
static int totalT=0;       /* total number of frames in training data */
static LogDouble totalPr=0;   /* total log prob upto current utterance */
static Vector vFloor[SMAX]; /* variance floor - default is all zero */

static MemHeap hmmStack;   /* For storage of all dynamic structures created... */
static MemHeap uttStack;
static MemHeap fbInfoStack;
static MemHeap accStack;

/* information about transforms */
static XFInfo xfInfo_hmm;
static int maxSpUtt = 0;
static float varFloorPercent = 0;

static char *labFileMask = NULL;

/* for duration model generation */
static char * durDir = NULL;     /* directory to look for duration model def files */
static char * durExt = NULL;     /* duration model def file extension */
static char * newdurDir = NULL;  /* directory to store new duration model def files */
static char * newdurExt = NULL;  /* extension of new reestimated duration model files */
static MemHeap durStack;   /* For storage of all dynamic structures created... */
static float durminVar = 0.0;           /* variance floor value */
static float durvarFloorPercent = 0.0;  /* variance flooring percent */
static float durmapTau = 0.0;           /* map tau for duration models */
static Boolean applydurVFloor = FALSE;  /* apply variance flooring */
static Boolean al_durUsed = FALSE;      /* Set for 2-model ReEstimation */
static Boolean up_durLoaded = FALSE;    /* Set semi-Markov training */
static char al_durDir[MAXFNAMELEN];     /* dir to look for alignment duration model defs */
static char al_durExt[MAXSTRLEN];       /* alignment duration model def file extension */
static char al_durMMF[MAXFNAMELEN];     /* alignment duration model MMF */
static char al_durLst[MAXFNAMELEN];     /* alignment duration model list */
static char up_durMMF[MAXFNAMELEN];     /* alignment duration model list */
static HMMSet al_dset;                  /* Option 2nd set of duration models for alignment */
static XFInfo xfInfo_dur;

/* ------------------ Process Command Line -------------------------- */
   
/* SetConfParms: set conf parms relevant to HCompV  */
void SetConfParms(void)
{
   int i;
   Boolean b;
   double f;
   char buf[MAXSTRLEN];
   
   nParm = GetConfig("HEREST", TRUE, cParm, MAXGLOBS);
   if (nParm>0) {
      if (GetConfInt(cParm,nParm,"TRACE",&i)) trace = i;
      if (GetConfFlt(cParm,nParm,"VARFLOORPERCENTILE",&f)) varFloorPercent = f;
      if (GetConfBool(cParm,nParm,"SAVEBINARY",&b)) saveBinary = b;
      if (GetConfBool(cParm,nParm,"BINARYACCFORMAT",&b)) ldBinary = b;
      if (GetConfBool(cParm,nParm,"APPLYVFLOOR", &b)) applyVFloor = b;  
      /* 2-model reestimation alignment model set */
      if (GetConfStr(cParm,nParm,"ALIGNMODELMMF",buf)) {
          strcpy(al_hmmMMF,buf); al_hmmUsed = TRUE;
      }
      if (GetConfStr(cParm,nParm,"ALIGNHMMLIST",buf)) {
          strcpy(al_hmmLst,buf); al_hmmUsed = TRUE;
      }
      /* allow multiple individual model files */
      if (GetConfStr(cParm,nParm,"ALIGNMODELDIR",buf)) {
          strcpy(al_hmmDir,buf); al_hmmUsed = TRUE;
      }
      if (GetConfStr(cParm,nParm,"ALIGNMODELEXT",buf)) {
          strcpy(al_hmmExt,buf); al_hmmUsed = TRUE;
      }
      if (GetConfStr(cParm,nParm,"ALIGNXFORMEXT",buf)) {
         xfInfo_hmm.al_inXFormExt = CopyString(&hmmStack,buf);
      }
      if (GetConfStr(cParm,nParm,"ALIGNXFORMDIR",buf)) {
         xfInfo_hmm.al_inXFormDir = CopyString(&hmmStack,buf);
         xfInfo_hmm.use_alInXForm = TRUE;
      }
      if (GetConfStr(cParm,nParm,"INXFORMMASK",buf)) {
         xfInfo_hmm.inSpkrPat = CopyString(&hmmStack,buf);
      }
      if (GetConfStr(cParm,nParm,"PAXFORMMASK",buf)) {
         xfInfo_hmm.paSpkrPat = CopyString(&hmmStack,buf);
      }
      if (GetConfStr(cParm,nParm,"LABFILEMASK",buf)) {
         labFileMask = (char*)malloc(strlen(buf)+1); 
         strcpy(labFileMask, buf);
      }
      /* duration model */
      if (GetConfFlt(cParm,nParm,"DURMINVAR",&f)) {
         durminVar = f;
      }
      if (GetConfFlt(cParm,nParm,"DURVARFLOORPERCENTILE",&f)) {
         durvarFloorPercent = f;
      }
      if (GetConfBool(cParm,nParm,"APPLYDURVARFLOOR",&b)) {
         applydurVFloor = b;
      }
      if (GetConfFlt(cParm,nParm,"DURMAPTAU",&f)) {
         durmapTau = f;
      }
      /* 2-model reestimation alignment model set */
      if (GetConfStr(cParm,nParm,"ALIGNDURMMF",buf)) {
         strcpy(al_durMMF,buf); al_durUsed = TRUE;
      }
      if (GetConfStr(cParm,nParm,"ALIGNDURLIST",buf)) {
         strcpy(al_durLst,buf); al_durUsed = TRUE;
      }
      /* allow multiple individual model files */
      if (GetConfStr(cParm,nParm,"ALIGNDURDIR",buf)) {
         strcpy(al_durDir,buf); al_durUsed = TRUE;
      }
      if (GetConfStr(cParm,nParm,"ALIGNDUREXT",buf)) {
         strcpy(al_durExt,buf); al_durUsed = TRUE;
      }
      if (GetConfStr(cParm,nParm,"ALIGNDURXFORMEXT",buf)) {
         xfInfo_dur.al_inXFormExt = CopyString(&durStack,buf);
      }
      if (GetConfStr(cParm,nParm,"ALIGNDURXFORMDIR",buf)) {
         xfInfo_dur.al_inXFormDir = CopyString(&durStack,buf);
         xfInfo_dur.use_alInXForm = TRUE;
      }
      if (GetConfStr(cParm,nParm,"DURINXFORMMASK",buf)) {
         xfInfo_dur.inSpkrPat = CopyString(&durStack,buf);
      }
      if (GetConfStr(cParm,nParm,"DURPAXFORMMASK",buf)) {
         xfInfo_dur.paSpkrPat = CopyString(&durStack,buf);
      }

      if (GetConfStr(cParm,nParm,"UPDATEMODE",buf)) {
         if (!strcmp (buf, "DUMP")) updateMode = UPMODE_DUMP;
         else if (!strcmp (buf, "UPDATE")) updateMode = UPMODE_UPDATE;
         else if (!strcmp (buf, "BOTH")) updateMode = UPMODE_BOTH;
         else HError(2319, "Unknown UPDATEMODE specified (must be DUMP, UPDATE or BOTH)");
      }
   }
}

void ReportUsage(void)
{
   printf("\nModified for HTS\n");
   printf("\nUSAGE: HERest [options] hmmList [durList] dataFiles...\n\n");
   printf(" Option                                       Default\n\n");
   printf(" -a      Use an input linear transform for hmms            off\n");
   printf(" -b      Use an input linear transform for dur models      off\n");
   printf(" -c f    Mixture pruning threshold            10.0\n");
   printf(" -d s    dir to find hmm definitions          current\n");
   printf(" -e      use model alignment from label for pruning        off\n");
   printf(" -f s    extension for new duration model files            as src\n");
   printf(" -g s    output duration model to file s                   none\n");
   printf(" -h s    set output speaker name pattern   *.%%%%%%\n");
   printf("         to s, optionally set input and parent patterns\n");
   printf(" -k f    set temperature parameter for DAEM training       1.0\n");
   printf(" -l N    set max files per speaker            off\n");
   printf(" -m N    set min examples needed per model    3\n");
   printf(" -n s    dir to find duration model definitions            current\n");
   printf(" -o s    extension for new hmm files          as src\n");
   printf(" -p N    set parallel mode to N               off\n");
   printf(" -q s    Save all xforms for duration to TMF file s        TMF\n");
   printf(" -r      Enable Single Pass Training...       \n");
   printf("         ...using two parameterisations       off\n");
   printf(" -s s    print statistics to file s           off\n");
   printf(" -t f [i l] set pruning to f [inc limit]      inf\n");
   printf(" -u tmvwapsd update t)rans m)eans v)ars w)ghts             tmvw\n");
   printf("                a)daptation xform p)rior used     \n");
   printf("                s)semi-tied xform                 \n");
   printf("                d) switch to duration model update flag    \n");
   printf(" -v f    set minimum variance to f            0.0\n");
   printf(" -w f    set mix weight floor to f*MINMIX     0.0\n");
   printf(" -y s    extension for duration model files                none\n");
   printf(" -x s    extension for hmm files              none\n");
   printf(" -z s    Save all xforms to TMF file s        TMF\n");
   PrintStdOpts("BEFGHIJKLMNRSTWXYZ");
   printf("\n\n");
}

void SetuFlags(void)
{
   char *s;
   UPDSet *uFlags=&uFlags_hmm;
   
   s=GetStrArg();
   *uFlags=(UPDSet) 0;        
   while (*s != '\0')
      switch (*s++) {
      case 't': *uFlags = (UPDSet) (*uFlags+UPTRANS); break;
      case 'm': *uFlags = (UPDSet) (*uFlags+UPMEANS); break;
      case 'v': *uFlags = (UPDSet) (*uFlags+UPVARS); break;
      case 'w': *uFlags = (UPDSet) (*uFlags+UPMIXES); break;
      case 'a': *uFlags = (UPDSet) (*uFlags+UPXFORM); break;
      case 'p': *uFlags = (UPDSet) (*uFlags+UPMAP); break;
      case 's': *uFlags = (UPDSet) (*uFlags+UPSEMIT); break;
      case 'd':  uFlags = &uFlags_dur; *uFlags=(UPDSet)0; break;
      default: HError(2320,"SetuFlags: Unknown update flag %c",*s);
         break;
      }
}

/* ScriptWord: return next word from script */
char *ScriptWord(FILE *script, char *scriptBuf)
{
   int ch,qch,i;
   
   i=0; ch=' ';
   while (isspace(ch)) ch = fgetc(script);
   if (ch==EOF) {
      scriptBuf=NULL;
      return NULL;
   }
   if (ch=='\'' || ch=='"'){
      qch = ch;
      ch = fgetc(script);
      while (ch != qch && ch != EOF) {
         scriptBuf[i++] = ch; 
         ch = fgetc(script);
      }
      if (ch==EOF)
         HError(5051,"ScriptWord: Closing quote missing in script file");
   } else {
      do {
         scriptBuf[i++] = ch; 
         ch = fgetc(script);
      }while (!isspace(ch) && ch != EOF);
   }
   scriptBuf[i] = '\0';

   return scriptBuf;
}

void CheckUpdateSetUp()
{
  AdaptXForm *xf;

   xf = xfInfo_hmm.paXForm;
   if ((xfInfo_hmm.paXForm != NULL) && !(uFlags_hmm&UPXFORM)) {
      while (xf != NULL) {
         if ((xf->xformSet->xkind != CMLLR) && (xf->xformSet->xkind != SEMIT))
            HError(999,"SAT only supported with SEMIT/CMLLR transforms");
         xf = xf->parentXForm;
      }
   }
   
   xf = xfInfo_dur.paXForm;
   if ((xfInfo_dur.paXForm != NULL) && !(uFlags_dur&UPXFORM)) {
    while (xf != NULL) {
       if ((xf->xformSet->xkind != CMLLR) && (xf->xformSet->xkind != SEMIT))
	HError(999,"SAT only supported with SEMIT/CMLLR transforms");
      xf = xf->parentXForm;
    }
  }
}

int main(int argc, char *argv[])
{
   char *datafn=NULL;
   char *datafn2=NULL;
   char *hmmListFn=NULL, *durListFn=NULL;
   char *s;
   char datafn1[MAXSTRLEN];
   char newFn[MAXSTRLEN];
   FILE *f;
   UttInfo *utt;            /* utterance information storage */
   FBInfo *fbInfo;          /* forward-backward information storage */
   HMMSet hset;             /* Set of HMMs to be re-estimated */
   HMMSet dset;             /* Set of duration models to be generated */
   Source src;
   float tmpFlt;
   int tmpInt;
   int numUtt,spUtt=0;

   void Initialise(FBInfo *fbInfo, MemHeap *x, HMMSet *hset, HMMSet *dset, char *hmmListFn, char *durListFn);
   void DoForwardBackward(FBInfo *fbInfo, UttInfo *utt, char *datafn, char *datafn2);
   void UpdateVFloors (HMMSet *hset, const double minVar, const double percent);
   void UpdateModels(HMMSet *hset, XFInfo *xfinfo, ParmBuf pbuf2, UPDSet uFlags);
   void StatReport(HMMSet *hset);
   
   if(InitShell(argc,argv,herest_version,herest_vc_id)<SUCCESS)
      HError(2300,"HERest: InitShell failed");
   InitMem();    InitMath();
   InitSigP();   InitAudio();
   InitWave();   InitVQ();
   InitLabel();  InitModel();
   if(InitParm()<SUCCESS)  
      HError(2300,"HERest: InitParm failed");
   InitTrain();
   InitUtil();   InitFB();
   InitAdapt(&xfInfo_hmm,&xfInfo_dur); InitMap();

   if (!InfoPrinted() && NumArgs() == 0)
      ReportUsage();
   if (NumArgs() == 0) Exit(0);
   al_hmmDir[0] = '\0'; al_hmmExt[0] = '\0'; 
   al_hmmMMF[0] = '\0'; al_hmmLst[0] = '\0'; 
   up_hmmMMF[0] = '\0';
   al_durDir[0] = '\0'; al_durExt[0] = '\0'; 
   al_durMMF[0] = '\0'; al_durLst[0] = '\0'; 
   up_durMMF[0] = '\0';
   CreateHeap(&hmmStack,"HmmStore", MSTAK, 1, 1.0, 50000, 500000);
   CreateHeap(&durStack,"DurStore", MSTAK, 1, 1.0, 50000, 500000);
   SetConfParms(); 
   CreateHMMSet(&hset,&hmmStack,TRUE);
   CreateHMMSet(&dset,&durStack,TRUE);
   CreateHeap(&uttStack,   "uttStore",    MSTAK, 1, 0.5, 100,   1000);
   utt = (UttInfo *) New(&uttStack, sizeof(UttInfo));
   CreateHeap(&fbInfoStack,   "FBInfoStore",  MSTAK, 1, 0.5, 100 ,  1000 );
   fbInfo = (FBInfo *) New(&fbInfoStack, sizeof(FBInfo));
   CreateHeap(&accStack,   "accStore",    MSTAK, 1, 1.0, 50000,   500000);

   while (NextArg() == SWITCHARG) {
      s = GetSwtArg();
      if (strlen(s)!=1) 
         HError(2319,"HERest: Bad switch %s; must be single letter",s);
      switch(s[0]){
      case 'c':
         minFrwdP = GetChkedFlt(0.0,100000.0,s);
         break;
      case 'd':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: HMM definition directory expected");
         hmmDir = GetStrArg(); break;   
      case 'e':
         useAlign = TRUE; break;
      case 'm':
         minEgs = GetChkedInt(0,1000,s); break;
      case 'o':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: HMM file extension expected");
         newhmmExt = GetStrArg(); break;
      case 'p':
         parMode = GetChkedInt(0,2000,s); break;
      case 'r':
         twoDataFiles = TRUE; break;
      case 's':
         stats = TRUE;
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: Stats file name expected");
         statFN = GetStrArg(); break;
      case 't':
         pruneInit =  GetChkedFlt(0.0,1.0E20,s);
         if (NextArg()==FLOATARG || NextArg()==INTARG)
            {
               pruneInc = GetChkedFlt(0.0,1.0E20,s);
               pruneLim = GetChkedFlt(0.0,1.0E20,s);
            }
         else
            {
               pruneInc = 0.0;
               pruneLim = pruneInit  ;
            }
         break;
      case 'u':
         SetuFlags(); break;
      case 'v':
         minVar = GetChkedFlt(0.0,10.0,s); applyVFloor = TRUE; break;
      case 'w':
         mixWeightFloor = MINMIX * GetChkedFlt(0.0,10000.0,s); 
         break;
      case 'x':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: HMM file extension expected");
         hmmExt = GetStrArg(); break;
      case 'B':
         saveBinary=TRUE;
         break;
      case 'F':
         if (NextArg() != STRINGARG)
            HError(2319,"HERest: Data File format expected");
         if((dff = Str2Format(GetStrArg())) == ALIEN)
            HError(-2389,"HERest: Warning ALIEN Data file format set");
         break;
      case 'G':
         if (NextArg() != STRINGARG)
            HError(2319,"HERest: Label File format expected");
         if((lff = Str2Format(GetStrArg())) == ALIEN)
            HError(-2389,"HERest: Warning ALIEN Label file format set");
         break;
      case 'H':
         if (NextArg() != STRINGARG)
            HError(2319,"HERest: HMM macro file name expected");
         strcpy(up_hmmMMF,GetStrArg());
         AddMMF(&hset,up_hmmMMF);
         break;     
      case 'I':
         if (NextArg() != STRINGARG)
            HError(2319,"HERest: MLF file name expected");
         LoadMasterFile(GetStrArg());
         break;
      case 'L':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: Label file directory expected");
         labDir = GetStrArg(); break;
      case 'M':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: Output macro file directory expected");
         newhmmDir = GetStrArg();
         break;     
      case 'T':
         trace = GetChkedInt(0,0100000,s);
         break;
      case 'X':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: Label file extension expected");
         labExt = GetStrArg(); break;
	 /* additional options for transform support */
      case 'a':
        xfInfo_hmm.useInXForm = TRUE; break;
      case 'h':
	if (NextArg()!=STRINGARG)
	  HError(1,"Speaker name pattern expected");
        xfInfo_hmm.outSpkrPat = xfInfo_dur.outSpkrPat = GetStrArg();
	break;
      case 'l':
         maxSpUtt = GetChkedInt(0,0100000,s);
         break;
      case 'E':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: parent transform directory expected");
         xfInfo_hmm.usePaXForm = TRUE;
         xfInfo_hmm.paXFormDir = GetStrArg(); 
         if (NextArg()==STRINGARG)
           xfInfo_hmm.paXFormExt = GetStrArg(); 
	 if (NextArg() != SWITCHARG)
	   HError(2319,"HERest: cannot have -E as the last option");	  
         break;              
      case 'J':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: input transform directory expected");
         AddInXFormDir(&hset,GetStrArg());
         if (NextArg()==STRINGARG) {
            if (xfInfo_hmm.inXFormExt == NULL)
               xfInfo_hmm.inXFormExt = GetStrArg(); 
            else
               HError(2319,"HERest: only one input transform extension may be specified");
         }
	 if (NextArg() != SWITCHARG)
	   HError(2319,"HERest: cannot have -J as the last option");	  
         break;              
      case 'K':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: output transform directory expected");
         xfInfo_hmm.outXFormDir = GetStrArg(); 
         if (NextArg()==STRINGARG)
           xfInfo_hmm.outXFormExt = GetStrArg(); 
	 if (NextArg() != SWITCHARG)
	   HError(2319,"HERest: cannot have -K as the last option");	  
         break;              
      case 'z':
         if (NextArg() != STRINGARG)
            HError(2319,"HERest: output TMF file expected");
         xfInfo_hmm.xformTMF = GetStrArg(); break;
      /* for state duration model */
      case 'b':
         xfInfo_dur.useInXForm = TRUE; break;
      case 'f':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: duration model file extension expected");
         newdurExt = GetStrArg(); break;
      case 'g':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: duration model file name expected");
         strcpy(up_durMMF,GetStrArg());
         AddMMF(&dset,up_durMMF);
         uFlags_dur = (UPDSet)(UPMEANS|UPVARS);
         break;
      case 'n':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: duration model definition directory expected");
         durDir = GetStrArg();
         up_durLoaded = TRUE;
         break;
      case 'y':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: duration model file extension expected");
         durExt = GetStrArg(); break;
      case 'N':
         if (NextArg() != STRINGARG)
            HError(2319,"HERest: duration model macro file name expected");
         strcpy(up_durMMF,GetStrArg());
         AddMMF(&dset,up_durMMF);
         up_durLoaded = TRUE;
         break;
      case 'R':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: Output duration macro file directory expected");
         newdurDir = GetStrArg();
         break;
      case 'W':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: parent duration transform directory expected");
         xfInfo_dur.usePaXForm = TRUE;
         xfInfo_dur.paXFormDir = GetStrArg(); 
         if (NextArg()==STRINGARG)
            xfInfo_dur.paXFormExt = GetStrArg(); 
         if (NextArg() != SWITCHARG)
            HError(2319,"HERest: cannot have -W as the last option");      
         break;
      case 'Y':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: input duration transform directory expected");
         AddInXFormDir(&dset,GetStrArg());
         if (NextArg()==STRINGARG) {
            if (xfInfo_dur.inXFormExt == NULL)
               xfInfo_dur.inXFormExt = GetStrArg(); 
            else
               HError(2319,"HERest: only one input duration transform extension may be specified");
         }
         if (NextArg() != SWITCHARG)
           HError(2319,"HERest: cannot have -Y as the last option");      
         break;              
      case 'Z':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: output duration transform directory expected");
         xfInfo_dur.outXFormDir = GetStrArg(); 
         if (NextArg()==STRINGARG)
            xfInfo_dur.outXFormExt = GetStrArg(); 
         if (NextArg() != SWITCHARG)
            HError(2319,"HERest: cannot have -Z as the last option");      
         break;
      case 'q':
         if (NextArg() != STRINGARG)
            HError(2319,"HERest: output duration TMF file expected");
         xfInfo_dur.xformTMF = GetStrArg(); break;
      case 'k':
         SetDAEMTemp(GetChkedFlt(0.0,1.0,s));
         break;
      default:
         HError(2319,"HERest: Unknown switch %s",s);
      }
   } 
   if (NextArg() != STRINGARG)
      HError(2319,"HERest: file name of vocabulary list expected");

   hmmListFn = GetStrArg(); durListFn = (up_durLoaded) ? GetStrArg() : NULL;
   Initialise(fbInfo, &fbInfoStack, &hset, &dset, hmmListFn, durListFn);
   InitUttInfo(utt, twoDataFiles);
   numUtt = 1;

   if (trace&T_TOP) 
      SetTraceFB(); /* allows HFB to do top-level tracing */

   do {
      if (NextArg()!=STRINGARG)
         HError(2319,"HERest: data file name expected");
      if (twoDataFiles && (parMode!=0)){
         if ((NumArgs() % 2) != 0)
            HError(2319,"HERest: Must be even num of training files for single pass training");
         strcpy(datafn1,GetStrArg());
         datafn = datafn1;
         datafn2 = GetStrArg();
      }
      else if (parMode==0 && up_durLoaded && uFlags_dur) {
         if ((NumArgs() % 2) != 0)
            HError(2319,"HERest: Must be even num of dump files for HSMM training");
         strcpy(datafn1,GetStrArg());
         datafn = datafn1;     
         datafn2 = GetStrArg();
         if (strstr(datafn,"hmm")==NULL)
            HError(-9999,"dump file for HMM include \"hmm\"",datafn);
         if (strstr(datafn2,"dur")==NULL)
            HError(-9999,"dump file for duration model include \"dur\"",datafn2);
      }
      else
         datafn = GetStrArg();
      if (parMode==0){
         src=LoadAccs(&hset, datafn, uFlags_hmm);
         ReadFloat(&src,&tmpFlt,1,ldBinary);
         totalPr += (LogDouble)tmpFlt;
         ReadInt(&src,&tmpInt,1,ldBinary);
         totalT += tmpInt;
         CloseSource( &src );
         if (up_durLoaded && uFlags_dur) {
            src=LoadAccs(&dset, datafn2, uFlags_dur);
            CloseSource( &src );
         }
      }
      else {
         /* track speakers */	 
         if (UpdateSpkrStats(&hset,&xfInfo_hmm,datafn)) {
            spUtt=0;
         }
         if (up_durLoaded) {
            if (UpdateSpkrStats(&dset,&xfInfo_dur,datafn) && xfInfo_dur.useInXForm)
               ResetDMMPreComps(&dset);   /* reset all pre-calculated duration probs */
         }
	 /* Check to see whether set-up is valid */
	 CheckUpdateSetUp();

         fbInfo->xfinfo_hmm     = &xfInfo_hmm;
         fbInfo->xfinfo_dur     = &xfInfo_dur;
         fbInfo->inXForm_hmm    = xfInfo_hmm.inXForm;
         fbInfo->inXForm_dur    = xfInfo_dur.inXForm;
         fbInfo->al_inXForm_hmm = xfInfo_hmm.al_inXForm;
         fbInfo->al_inXForm_dur = xfInfo_dur.al_inXForm;
         fbInfo->paXForm_hmm    = xfInfo_hmm.paXForm;
         fbInfo->paXForm_dur    = xfInfo_dur.paXForm;

         if ((maxSpUtt==0) || (spUtt<maxSpUtt))
            DoForwardBackward(fbInfo, utt, datafn, datafn2) ;
         numUtt++; spUtt++;
      }
   } while (NumArgs()>0);

   if (uFlags_hmm&UPXFORM || uFlags_dur&UPXFORM) {
      /* ensure final speaker correctly handled */
      if (uFlags_hmm&UPXFORM)  
         UpdateSpkrStats(&hset,&xfInfo_hmm,NULL);
      if (uFlags_dur&UPXFORM && up_durLoaded)
         UpdateSpkrStats(&dset,&xfInfo_dur,NULL);
      if (trace&T_TOP) {
         printf("Reestimation complete - average log prob per frame = %e (%d frames)\n",
                totalPr/totalT, totalT);
      }
   } else {
      if (parMode>0  || (parMode==0 && (updateMode&UPMODE_DUMP))){
         /* dump HMM accs */
         MakeFN("HER$.hmm.acc",newhmmDir,NULL,newFn);
         f=DumpAccs(&hset,newFn,uFlags_hmm,parMode);
         tmpFlt = (float)totalPr;
         WriteFloat(f,&tmpFlt,1,ldBinary);
         WriteInt(f,(int*)&totalT,1,ldBinary);
         fclose( f );

         /* dump dur accs */
         if (up_durLoaded && uFlags_dur) {
            MakeFN("HER$.dur.acc",newhmmDir,NULL,newFn);
            f=DumpAccs(&dset,newFn,uFlags_dur,parMode);
         tmpFlt = (float)totalPr;
         WriteFloat(f,&tmpFlt,1,ldBinary);
         WriteInt(f,(int*)&totalT,1,ldBinary);
         fclose( f );
      }
      }
      if (parMode <= 0) {
         if (stats) {
            StatReport(&hset);
         }
         /* update HMMs */
         if (updateMode&UPMODE_UPDATE) {
            UpdateModels(&hset,&xfInfo_hmm,utt->pbuf2, uFlags_hmm);
            if (trace&T_TOP) {
               if (mmfFn == NULL)
                  printf("Saving hmm's to dir %s\n",(newhmmDir==NULL)?"Current":newhmmDir); 
               else
                  printf("Saving hmm's to MMF %s\n",mmfFn);
               fflush(stdout);
            }
            SaveHMMSet(&hset,newhmmDir,newhmmExt,NULL,saveBinary);
         }
         /* update duration models */
         if (up_durLoaded && (updateMode&UPMODE_UPDATE) && uFlags_dur) {
            /* first estimate variance floor */
            if (durvarFloorPercent>0.0 && uFlags_dur&UPVARS)
               UpdateVFloors(&dset, durminVar, durvarFloorPercent);
            if (durmapTau>0.0)
               SetMapTau(durmapTau);
            if (durminVar>0.0)
               SetMinVar(durminVar);
            /* replace minVar & applyVFloor using these for duration models */
            minVar = durminVar; applyVFloor = applydurVFloor;
            /* estimate duration models */
            UpdateModels(&dset,&xfInfo_dur,utt->pbuf2,uFlags_dur);
            if (trace&T_TOP) {
               if (newdurDir!=NULL)
                  printf("Saving duration models to dir %s\n",(newdurDir==NULL)?"Current":newdurDir); 
               else
                  printf("Saving duration models to MMF %s\n",up_durMMF);
               fflush(stdout);
      }
            SaveHMMSet(&dset,newdurDir,newhmmExt,NULL,saveBinary);
   }
         
         if (trace&T_TOP) {
            printf("Reestimation complete - average log prob per frame = %e\n", totalPr/totalT);
            printf("     - total frames seen          = %e\n", (double)totalT);
         }
      }
   }

   ResetHeap(&accStack);
   Dispose(&fbInfoStack, fbInfo);
   ResetHeap(&fbInfoStack);
   Dispose(&uttStack, utt);
   ResetHeap(&uttStack);
   ResetHeap(&hmmStack);
   
   ResetMap();
   ResetAdapt(&xfInfo_hmm,&xfInfo_dur);
   ResetFB();
   ResetUtil();
   ResetTrain();
   ResetParm();
   ResetModel();
   ResetLabel();
   ResetVQ();
   ResetWave();
   ResetAudio();
   ResetSigP();
   ResetMath();
   ResetMem();
   ResetShell();

   Exit(0);
   return (0);          /* never reached -- make compiler happy */
}

/* -------------------------- Initialisation ----------------------- */

void Initialise(FBInfo *fbInfo, MemHeap *x, HMMSet *hset, HMMSet *dset, char *hmmListFn, char *durListFn)
{   
   HSetKind hsKind;
   int L,P,S,vSize,maxM; 

   void GenDurMMF (HMMSet *hset, char *durfn);

   /* Load HMMs and init HMMSet related global variables */
   if(MakeHMMSet( hset, hmmListFn )<SUCCESS)
      HError(2321,"Initialise: MakeHMMSet failed");
   if(LoadHMMSet( hset,hmmDir,hmmExt)<SUCCESS)
      HError(2321,"Initialise: LoadHMMSet failed");
   if (uFlags_hmm&UPSEMIT) uFlags_hmm = (UPDSet) (uFlags_hmm|UPMEANS|UPVARS);
   AttachAccs(hset, &accStack, uFlags_hmm);
   ZeroAccs(hset, uFlags_hmm);
   P = hset->numPhyHMM;
   L = hset->numLogHMM;
   vSize = hset->vecSize;
   S = hset->swidth[0];
   maxM = MaxMixInSet(hset);

   hsKind = hset->hsKind;
   if (hsKind==DISCRETEHS)
     uFlags_hmm = (UPDSet) (uFlags_hmm & (~(UPMEANS|UPVARS|UPXFORM|UPSEMIT)));

   if (parMode != 0) {
      ConvDiagC(hset,TRUE);
   }
   if (trace&T_TOP) {
      printf("HMM:\n");
      if (uFlags_hmm&UPMAP)  printf(" HERest  MAP Updating: ");
      else printf("HERest  ML Updating: ");
      if (uFlags_hmm&UPTRANS) printf("Transitions "); 
      if (uFlags_hmm&UPMEANS) printf("Means "); 
      if (uFlags_hmm&UPVARS)  printf("Variances ");
      if (uFlags_hmm&UPSEMIT) printf("SemiTied ");
      if (uFlags_hmm&UPXFORM) printf("XForms ");
      if (uFlags_hmm&UPMIXES && maxM>1)  printf("MixWeights ");
      printf("\n ");
    
      if (parMode>=0) printf("Parallel-Mode[%d] ",parMode);

      printf("System is ");
      switch (hsKind){
      case PLAINHS:  printf("PLAIN\n");  break;
      case SHAREDHS: printf("SHARED\n"); break;
      case TIEDHS:   printf("TIED\n"); break;
      case DISCRETEHS: printf("DISCRETE\n"); break;
      }

      printf("%d Logical/%d Physical Models Loaded, VecSize=%d\n",L,P,vSize);
      if (hset->numFiles>0)
         printf("%d MMF input files\n",hset->numFiles);
      if (mmfFn != NULL)
         printf("Output to MMF file:  %s\n",mmfFn); 
      fflush(stdout);
   }
   totalPr = 0.0;

   if (xfInfo_hmm.inSpkrPat == NULL) xfInfo_hmm.inSpkrPat = xfInfo_hmm.outSpkrPat; 
   if (xfInfo_hmm.paSpkrPat == NULL) xfInfo_hmm.paSpkrPat = xfInfo_hmm.outSpkrPat; 
   if (uFlags_hmm&UPXFORM) {
      if ((hsKind != PLAINHS) && (hsKind != SHAREDHS))
         HError(999,"Can only estimated transforms with PLAINHS and SHAREDHS!");
      if (uFlags_hmm != UPXFORM)
         HError(999,"Can only update linear transforms OR model parameters!");
      xfInfo_hmm.useOutXForm = TRUE;
      /* This initialises things - temporary hack - THINK!! */
      CreateAdaptXForm(hset, &xfInfo_hmm, "tmp");
   } 

   if ((uFlags_hmm&UPXFORM) || (uFlags_hmm&UPSEMIT))
      CheckAdaptSetUp(hset,&xfInfo_hmm); 

   /* duration model initialization */
   if (up_durLoaded || up_durMMF[0]!='\0') {
      if (!up_durLoaded)
         GenDurMMF(hset,up_durMMF);  /* generate duration model mmf according to hset */
      if (MakeHMMSet(dset,((!up_durLoaded)?hmmListFn:durListFn))<SUCCESS)
         HError(2321,"Initialise: MakeHMMSet failed");
      if (LoadHMMSet(dset,durDir,durExt)<SUCCESS)
         HError(2321,"Initialise: LoadHMMSet failed");
      AttachAccs(dset,&accStack,uFlags_dur);
      ZeroAccs(dset,uFlags_dur);
      
      uFlags_dur = (UPDSet) uFlags_dur & (~UPTRANS);  /* turn off transition update flag */ 
      if (dset->hsKind==DISCRETEHS)
        uFlags_dur = (UPDSet) (uFlags_dur & (~(UPMEANS|UPVARS|UPXFORM|UPSEMIT)));
      
      if (parMode != 0)
         ConvDiagC(dset,TRUE);

      if (trace&T_TOP) {
         printf("Duration model:\n");
         if (uFlags_dur&UPMAP)  printf(" HERest  MAP Updating: ");
         else printf(" HERest  ML Updating: ");
         if (uFlags_dur&UPTRANS) printf("Transitions "); 
         if (uFlags_dur&UPMEANS) printf("Means "); 
         if (uFlags_dur&UPVARS)  printf("Variances ");
         if (uFlags_dur&UPSEMIT) printf("SemiTied ");
         if (uFlags_dur&UPXFORM) printf("XForms ");
         if (uFlags_dur&UPMIXES && maxM>1)  printf("MixWeights ");
         printf("\n ");

         if (parMode>=0) printf("Parallel-Mode[%d] ",parMode);
         printf("System is ");
         switch (dset->hsKind){
         case PLAINHS:  printf("PLAIN\n");  break;
         case SHAREDHS: printf("SHARED\n"); break;
         case TIEDHS:   printf("TIED\n"); break;
         case DISCRETEHS: printf("DISCRETE\n"); break;
         }
         
         printf(" %d Logical/%d Physical Models Loaded, #States=%d\n",dset->numLogHMM,dset->numPhyHMM,dset->vecSize);
         if (dset->numFiles>0)
            printf(" %d MMF input files\n",dset->numFiles);
         if (mmfFn != NULL)
            printf(" Output to MMF file:  %s\n",up_durMMF);
         fflush(stdout);
      }
      
      if (xfInfo_dur.inSpkrPat == NULL) xfInfo_dur.inSpkrPat = xfInfo_dur.outSpkrPat; 
      if (xfInfo_dur.paSpkrPat == NULL) xfInfo_dur.paSpkrPat = xfInfo_dur.outSpkrPat; 
      if (uFlags_dur&UPXFORM) {
         if ((hsKind != PLAINHS) && (hsKind != SHAREDHS))
            HError(999,"Can only estimated transforms with PLAINHS and SHAREDHS!");
         xfInfo_dur.useOutXForm = TRUE;
      /* This initialises things - temporary hack - THINK!! */
         CreateAdaptXForm(dset, &xfInfo_dur, "tmp");
      }
      if ((uFlags_dur&UPXFORM) || (uFlags_dur&UPSEMIT))
         CheckAdaptSetUp(dset,&xfInfo_dur);
      
   } 

   if (trace&T_TOP) {
      printf("\n");
      fflush(stdout);
   }
   
   /* initialise and  pass information to the forward backward library */
   if (!up_durLoaded && up_durMMF[0]=='\0')
      InitialiseForBack(fbInfo, x, hset, uFlags_hmm, NULL, (UPDSet)0, 
                        pruneInit, pruneInc, pruneLim, minFrwdP, useAlign, FALSE);
   else
      InitialiseForBack(fbInfo, x, hset, uFlags_hmm, dset, uFlags_dur, 
                        pruneInit, pruneInc, pruneLim, minFrwdP, useAlign, ((up_durLoaded)?FALSE:TRUE));

   if (parMode != 0) {
      ConvLogWt(hset);
   }
   /* 2-model reestimation */
   if (al_hmmUsed){
       if (trace&T_TOP)
           printf("2-model re-estimation enabled (HMM) \n");
       /* load alignment HMM set */
       CreateHMMSet(&al_hset,&hmmStack,TRUE);
       xfInfo_hmm.al_hset = &al_hset;
       if (xfInfo_hmm.al_inXFormExt == NULL) xfInfo_hmm.al_inXFormExt = xfInfo_hmm.inXFormExt;
       /* load multiple MMFs */
       if (strlen(al_hmmMMF) > 0 ) {
           char *p,*q;
           Boolean eos;
           p=q=al_hmmMMF;
           for(;;) {
              eos = (*p=='\0') ? TRUE:FALSE;
               if ( ( isspace((int) *p) || *p == '\0' ) && (q!=p) ) {
                   *p='\0';
                   if (trace&T_TOP) { 
                       printf("Loading alignment HMM set %s\n",q);
                   }
                   AddMMF(&al_hset,q);
                   if (eos)
                       break;
                   q=p+1;
               }
               p++;
           }
       }
       if (strlen(al_hmmLst) > 0 ) 
           MakeHMMSet(&al_hset, al_hmmLst );
       else /* use same hmmList */
           MakeHMMSet(&al_hset, hmmListFn );
       if (strlen(al_hmmDir) > 0 )
           LoadHMMSet(&al_hset,al_hmmDir,al_hmmExt);
       else
           LoadHMMSet(&al_hset,NULL,NULL);

       if (parMode != 0) {
	  ConvDiagC(&al_hset,TRUE);
	  ConvLogWt(&al_hset);
       }

       /* and echo status */
       if (trace&T_TOP) { 
           if (strlen(al_hmmDir) > 0 )
               printf(" HMM Dir %s",al_hmmDir);
           if (strlen(al_hmmExt) > 0 )
               printf(" Ext %s",al_hmmExt);
           /* printf("\n"); */
           if (strlen(al_hmmLst) > 0 )
               printf("HMM List %s\n",al_hmmLst);
           printf(" %d Logical/%d Physical Models Loaded, VecSize=%d\n",
                  al_hset.numLogHMM,al_hset.numPhyHMM,al_hset.vecSize);
       }
   }
   
   if (al_durUsed){
      if (trace&T_TOP)
         printf("\n2-model re-estimation enabled (duration model)\n");
      /* load alignment duration model set */
      CreateHMMSet(&al_dset,&durStack,TRUE);
      xfInfo_dur.al_hset = &al_dset;
      if (xfInfo_dur.al_inXFormExt == NULL) xfInfo_dur.al_inXFormExt = xfInfo_dur.inXFormExt;
      /* load multiple MMFs */
      if (strlen(al_durMMF) > 0 ) {
         char *p,*q;
          Boolean eos;
          p=q=al_durMMF;
          for (;;) {
             eos = (*p=='\0') ? TRUE:FALSE;
             if ( ( isspace((int) *p) || *p == '\0' ) && (q!=p) ) {
                *p='\0';
                if (trace&T_TOP) { 
                   printf("Loading alignment duration model set %s\n",q);
                }
                AddMMF(&al_dset,q);
                if (eos) break;
                q=p+1;
             }
             p++;
          }
      }
      if (strlen(al_durLst) > 0 ) 
         MakeHMMSet(&al_dset, al_durLst );
      else /* use same durList */
         MakeHMMSet(&al_dset, durListFn );
      if (strlen(al_durDir) > 0 )
         LoadHMMSet(&al_dset,al_durDir,al_durExt);
      else
         LoadHMMSet(&al_dset,NULL,NULL);

      if (parMode != 0) {
         ConvDiagC(&al_dset,TRUE);
         ConvLogWt(&al_dset);
      }
       
      /* and echo status */
      if (trace&T_TOP) { 
         if (strlen(al_durDir) > 0 )
            printf(" Duration Model Dir %s",al_durDir);
         if (strlen(al_durExt) > 0 )
            printf(" Ext %s",al_durExt);
         /* printf("\n"); */
         if (strlen(al_durLst) > 0 )
            printf("Duration Model List %s\n",al_durLst);
         printf(" %d Logical/%d Physical Models Loaded, VecSize=%d\n",
                al_dset.numLogHMM,al_dset.numPhyHMM,al_dset.vecSize);
      }
   }
   
   if (trace&T_TOP) {
      printf("\n");
      fflush(stdout);
   }
   
   /* switch model set */
   if (al_durUsed || al_hmmUsed)
      UseAlignHMMSet(fbInfo,x,((al_hmmUsed)?&al_hset:NULL),((al_durUsed)?&al_dset:NULL));
}

/* ------------------- Statistics Reporting  -------------------- */

/* PrintStats: for given hmm */
void PrintStats(HMMSet *hset,FILE *f, int n, HLink hmm, long numEgs)
{
   char buf[MAXSTRLEN];
   StateInfo *si;
   int i,N;
   float occ;
    
   N = hmm->numStates;
   ReWriteString(HMMPhysName(hset,hmm),buf,DBL_QUOTE);
   fprintf(f,"%4d %14s %4ld ",n,buf,numEgs);
   for (i=2;i<N;i++) {
      si = hmm->svec[i].info;
      if (hset->numSharedStreams>0)  /* if streams are shared, using wa->occ is not appropriate */
         memcpy(&occ,&(si->hook),sizeof(float));
      else
         occ = ((WtAcc *)((si->pdf+1)->info->hook))->occ;
      fprintf(f," %10f",occ);
   }
   fprintf(f,"\n");
}

/* StatReport: print statistics report */
void StatReport(HMMSet *hset)
{
   HMMScanState hss;
   HLink hmm;
   FILE *f;
   int px;

   if ((f = fopen(statFN,"w")) == NULL){
      HError(2311,"StatReport: Unable to open stats file %s",statFN);
      return;
   }
   NewHMMScan(hset,&hss);
   px=1;
   do {
      hmm = hss.hmm;
      PrintStats(hset,f,px,hmm,(long)hmm->hook);
      px++;
   } while (GoNextHMM(&hss));
   EndHMMScan(&hss);
   fclose(f);
}

/* ------------------- Duration model related operations  -------------------- */
void GenDurMMF (HMMSet *hset, char *durfn)
{
   FILE *fp;
   char buf[MAXSTRLEN];
   int h,i,N,MaxN;
   Boolean isPipe;
   MLink mac;
   HLink hmm;
   
   /* vSize of dur MMF */
   MaxN  = MaxStatesInSet(hset)-2;
   
   /* ------- Output duration model ------- */
   if (trace & T_TOP)
      printf("Generatnig dur MMF from HMM MMF...\n");
      
   if ((fp = FOpen(durfn,NoOFilter,&isPipe)) == NULL)
      HError(7360,"GenDurMMF: Cannot create dur MMF file %s", durfn);
   
   /* output model definition and dummy state-transition matrix */   
   fprintf(fp,"~o\n");
   fprintf(fp,"<STREAMINFO> %d", MaxN);
   for (i=1; i<=MaxN; i++)
      fprintf(fp," 1");
   fprintf(fp,"\n<MSDINFO> %d", MaxN);
   for (i=1; i<=MaxN; i++)
      fprintf(fp," 0");
   fprintf(fp,"\n<VECSIZE> %d <NULLD><DIAGC><%s>\n",MaxN,ParmKind2Str(hset->pkind,buf));
   fprintf(fp,"~t \"trP_1\"\n<TRANSP> 3\n");
   fprintf(fp,"0 1 0\n0 0 1\n0 0 0\n");

   /* output vfloor macro */
   for (i=1; i<=MaxN; i++) {
      fprintf(fp,"~v \"varFloor%d\" \n",i);
      fprintf(fp,"<VARIANCE> 1\n");
      fprintf(fp,"%e\n",minVar);
   }

   /* output dummy state duration PDFs */
   for (h=0; h<MACHASHSIZE; h++) {
      for (mac=hset->mtab[h]; mac!=NULL; mac=mac->next) {
         if (mac->type=='h') {
            hmm = mac->structure;
            N = hmm->numStates-2;
            if (N!=MaxN)
               HError(7399,"GenDurMMF: Number of states is inconsistent in %s", mac->id->name);

            fprintf(fp,"~h \"%s\"\n",mac->id->name);
            fprintf(fp,"<BEGINHMM>\n<NUMSTATES> 3\n<STATE> 2\n");
            
            /* output dummy mean & variance */
            for (i=1; i<=N; i++) {
               fprintf(fp,"<STREAM> %d\n", i);
               fprintf(fp,"<MEAN> 1\n");
               fprintf(fp," 0.0\n");
               fprintf(fp,"<VARIANCE> 1\n");
               fprintf(fp," 1.0\n");
            }

            /* output link to the dummy transition matrix macro */
            fprintf(fp,"~t \"trP_1\"\n<ENDHMM>\n");
         }
      }
   }
   
   FClose(fp,isPipe);
   
   if (trace&T_TOP) {
      printf("done");
      fflush(stdout);
   }
   
   return;
}

/* UpdateVFloors: use acc values to calc new estimate of variance floors */
void UpdateVFloors (HMMSet *hset, const double minVar, const double percent)
{
   int s;
   float mean;
   double occ,sum,sqr;
   HMMScanState hss;
   MuAcc *ma=NULL;
   VaAcc *va=NULL;
   Vector varFloor[SMAX];
   
   /* get current variance floor */
   SetVFloor(hset,varFloor,minVar);
   
   for (s=1; s<=hset->swidth[0]; s++) {
      /* initialization */
      occ = sum = sqr = 0.0;
      
      /* acc statistics for variance flooring */
      NewHMMScan(hset,&hss);
      while (GoNextMix(&hss,FALSE)) {
         if ((hss.s==s) && (VectorSize(hss.mp->mean)==hset->swidth[s])) {  /* MSD check */
            /* get mean & variance accumulators */
            va = (VaAcc *) GetHook(hss.mp->cov.var);
            ma = (MuAcc *) GetHook(hss.mp->mean);
            mean = hss.mp->mean[1];
            
            /* accumulate */
            occ += va->occ;
            sum += ma->mu[1] + ma->occ*mean;
            sqr += va->cov.var[1] + 2*mean*(ma->mu[1]+ma->occ*mean) - va->occ*mean*mean; 
         }
      }
      EndHMMScan(&hss);

      /* update vfloor */
      varFloor[s][1] = (sqr/occ - sum/occ*sum/occ) * percent/100.0;
   }
                    
   /* Reset vfloor */
   ResetVFloor(hset,varFloor);
   
   return;
}


/* -------------------- Top Level of F-B Updating ---------------- */


/* Load data and call FBUtt: apply forward-backward to given utterance */
void DoForwardBackward(FBInfo *fbInfo, UttInfo *utt, char * datafn, char * datafn2)
{
   char datafn_lab[MAXFNAMELEN];

   utt->twoDataFiles = twoDataFiles ;
   utt->S = fbInfo->al_hset->swidth[0];

   /* Load the labels - support for label masks */
   if (labFileMask) {
      if (!MaskMatch (labFileMask, datafn_lab, datafn))
         HError(2319,"HERest: LABFILEMASK %s has no match with segemnt %s", labFileMask, datafn);
   }
   else
      strcpy (datafn_lab, datafn);
   LoadLabs(utt, lff, datafn_lab, labDir, labExt);
   /* Load the data */
   LoadData(fbInfo->al_hset, utt, dff, datafn, datafn2);

      InitUttObservations(utt, fbInfo->al_hset, datafn, fbInfo->maxMixInS);
  
   /* fill the alpha beta and otprobs (held in fbInfo) */
   if (FBUtt(fbInfo, utt)) {
      /* update totals */
      totalT += utt->T ;
      totalPr += utt->pr ;
      /* Handle the input xform Jacobian if necssary */
      if (fbInfo->al_hset->xf != NULL) {
         totalPr += utt->T*0.5*fbInfo->al_hset->xf->xform->det;
      }
   }
   ResetHeap(&fbInfo->ab->abMem);
   ResetUttObservations(utt, fbInfo->al_hset);
}

/* --------------------------- Model Update --------------------- */

static int nFloorVar = 0;     /* # of floored variance comps */
static int nFloorVarMix = 0;  /* # of mix comps with floored vars */

/* UpdateTrans: use acc values to calc new estimate for transP */
void UpdateTrans(HMMSet *hset, int px, HLink hmm)
{
   int i,j,N;
   float x,occi;
   TrAcc *ta;
   
   ta = (TrAcc *) GetHook(hmm->transP);
   if (ta==NULL) return;   /* already done */
   N = hmm->numStates;
   for (i=1;i<N;i++) {
      occi = ta->occ[i];
      if (occi > 0.0) 
         for (j=2;j<=N;j++) {
            x = ta->tran[i][j]/occi;
            hmm->transP[i][j] = (x>MINLARG)?log(x):LZERO;
         }
      else
         HError(-2326,"UpdateTrans: Model %d[%s]: no transitions out of state %d",
                px,HMMPhysName(hset,hmm),i);
   }
   SetHook(hmm->transP,NULL);
}

/* FloorMixes: apply floor to given mix set */
void FloorMixes(HMMSet *hset, MixtureElem *mixes, int M, float floor)
{
   float sum,fsum,scale;
   MixtureElem *me;
   int m;
   
   if (hset->logWt == TRUE) HError(999,"FloorMixes requires linear weights");
   sum = fsum = 0.0;
   for (m=1,me=mixes; m<=M; m++,me++) {
      if (me->weight>floor)
         sum += me->weight;
      else {
         fsum += floor; me->weight = floor;
      }
   }
   if (fsum>1.0) HError(2327,"FloorMixes: Floor sum too large");
   if (fsum == 0.0) return;
   if (sum == 0.0) HError(2328,"FloorMixes: No mixture weights above floor");
   scale = (1.0-fsum)/sum;
   for (m=1,me=mixes; m<=M; m++,me++)
      if (MixWeight(hset,me->weight)>floor) me->weight *= scale;
}

/* FloorTMMixes: apply floor to given tied mix set */
void FloorTMMixes(Vector mixes, int M, float floor)
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
   if (fsum>1.0) HError(2327,"FloorTMMixes: Floor sum too large");
   if (fsum == 0.0) return;
   if (sum == 0.0) HError(2328,"FloorTMMixes: No mixture weights above floor");
   scale = (1.0-fsum)/sum;
   for (m=1; m<=M; m++){
      fltWt = mixes[m];
      if (fltWt>floor)
         mixes[m] = fltWt*scale;
   }
}

/* FloorDProbs: apply floor to given discrete prob set */
void FloorDProbs(ShortVec mixes, int M, float floor)
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

/* UpdateWeights: use acc values to calc new estimate of mix weights */
void UpdateWeights(HMMSet *hset, int px, HLink hmm)
{
   int i,s,m,M=0,N,S;
   float x,occi;
   WtAcc *wa;
   StateElem *se;
   StreamElem *ste;
   StreamInfo *sti;
   MixtureElem *me;
   HSetKind hsKind;

   N = hmm->numStates;
   se = hmm->svec+2;
   hsKind = hset->hsKind;
   S = hset->swidth[0];
   for (i=2; i<N; i++,se++){
      ste = se->info->pdf+1;
      for (s=1;s<=S; s++,ste++){
         sti = ste->info;
         wa = (WtAcc *)sti->hook;
         switch (hsKind){
         case TIEDHS:
            M=hset->tmRecs[s].nMix;
            break;
         case DISCRETEHS:
         case PLAINHS:
         case SHAREDHS:
            M=sti->nMix;
            break;
         }
         if (wa != NULL) {
            occi = wa->occ;
            if (occi>0) {
               for (m=1; m<=M; m++){
                  x = wa->c[m]/occi;
                  if (x>1.0){
                     if (x>1.001)
                        HError(2393,"UpdateWeights: Model %d[%s]: mix too big in %d.%d.%d %5.5f",
                               px,HMMPhysName(hset,hmm),i,s,m,x);
                     x = 1.0;
                  }
                  switch (hsKind){
                  case TIEDHS:
                     sti->spdf.tpdf[m] = (x>MINMIX) ? x : 0.0;
                     break;
                  case DISCRETEHS:
                     sti->spdf.dpdf[m]=(x>MINMIX) ? DProb2Short(x) : DLOGZERO;
                     break;
                  case PLAINHS:
                  case SHAREDHS:
                     me=sti->spdf.cpdf+m;
                     me->weight = (x>MINMIX) ? x : 0.0;
                     break;
                  }
               }
               if (mixWeightFloor>0.0){
                  switch (hsKind){
                  case DISCRETEHS:
                     FloorDProbs(sti->spdf.dpdf,M,mixWeightFloor);
                     break;
                  case TIEDHS:
                     FloorTMMixes(sti->spdf.tpdf,M,mixWeightFloor);
                     break;
                  case PLAINHS:
                  case SHAREDHS:
                     FloorMixes(hset,sti->spdf.cpdf+1,M,mixWeightFloor);
                     break;
                  }
               }
            }else
               HError(-2330,"UpdateWeights: Model %d[%s]: no use of mixtures in %d.%d",
                      px,HMMPhysName(hset,hmm),i,s);
            sti->hook = NULL;
         }
      }
   }
}
      
/* UpdateMeans: use acc values to calc new estimate of means */
void UpdateMeans(HMMSet *hset, int px, HLink hmm)
{
   int i,s,m,k,M,N,S,vSize;
   float occim;
   MuAcc *ma;
   StateElem *se;
   StreamElem *ste;
   StreamInfo *sti;
   MixtureElem *me;
   Vector mean, mu;
   
   N = hmm->numStates;
   se = hmm->svec+2;
   S = hset->swidth[0];
   for (i=2; i<N; i++,se++){
      ste = se->info->pdf+1;
      for (s=1;s<=S;s++,ste++){
         sti = ste->info; 
         me = sti->spdf.cpdf + 1; M = sti->nMix;
         for (m=1;m<=M;m++,me++)
            if (me->weight > MINMIX){
               mean = me->mpdf->mean;
               /* nuisance dimensions not updated */   
               vSize = VectorSize(mean) - hset->projSize;
               ma = (MuAcc *) GetHook(mean);
               if (ma != NULL){
                  occim = ma->occ;
                  mu    = ma->mu;
                  if (occim > 0.0)
                     /* if you use Intel C compiler, following loop will be vectorized */
                     for (k=1; k<=vSize; k++) 
                        mean[k] += mu[k]/occim;
                  else if (trace&T_UPD)
                     HError(-2330,"UpdateMeans: Model %d[%s]: no use of mean %d.%d.%d",
                            px,HMMPhysName(hset,hmm),i,s,m);
                  SetHook(mean,NULL);
               }
            }
      }
   }
}

/* UpdateTMMeans: use acc values to calc new estimate of means for TIEDHS */
void UpdateTMMeans(HMMSet *hset)
{
   int s,m,k,M,S,vSize;
   float occim;
   MuAcc *ma;
   MixPDF *mpdf;
   Vector mean;
   
   S = hset->swidth[0];
   for (s=1;s<=S;s++){
      vSize = hset->swidth[s];
      M = hset->tmRecs[s].nMix;
      for (m=1;m<=M;m++){
         mpdf = hset->tmRecs[s].mixes[m];
         mean = mpdf->mean;
         ma = (MuAcc *) GetHook(mean);
         if (ma != NULL){
            occim = ma->occ;
            if (occim > 0.0)
               for (k=1; k<=vSize; k++)
                  mean[k] += ma->mu[k]/occim;
            else
               HError(-2330,"UpdateMeans: No use of mean %d in stream %d",m,s);
            SetHook(mean,NULL);
         }
      }
   }
}

/* UpdateVars: use acc values to calc new estimate of variances */
void UpdateVars(HMMSet *hset, int px, HLink hmm)
{
   int i,s,m,k,l,M,N,S,vSize;
   float occim,x,muDiffk,muDiffl;
   Vector minV,mpV;
   VaAcc *va;
   MuAcc *ma;
   StateElem *se;
   StreamElem *ste;
   StreamInfo *sti;
   MixtureElem *me;
   Vector mean;
   Covariance cov;
   Boolean mixFloored,shared;
   
   N = hmm->numStates;
   se = hmm->svec+2;
   S = hset->swidth[0];
   for (i=2; i<N; i++,se++){
      ste = se->info->pdf+1;
      for (s=1;s<=S;s++,ste++){
         sti = ste->info;
         minV = vFloor[s];
         me = sti->spdf.cpdf + 1; M = sti->nMix;
         for (m=1;m<=M;m++,me++)
            if (me->weight > MINMIX){
               if (me->mpdf->vFloor == NULL) mpV=minV;
               else mpV=me->mpdf->vFloor;
               cov = me->mpdf->cov;
               va = (VaAcc *) GetHook(cov.var);
               mean = me->mpdf->mean;
               vSize = VectorSize(mean) - hset->projSize;
               ma = (MuAcc *) GetHook(mean);
               if (va != NULL){
                  occim = va->occ;
                  mixFloored = FALSE;
                  if (occim > 0.0){
                     shared=((GetUse(cov.var)>1 || ma==NULL || ma->occ<=0.0)) ? TRUE:FALSE;
                     if (me->mpdf->ckind==DIAGC) {
                        for (k=1; k<=vSize; k++){
                           muDiffk=(shared)?0.0:ma->mu[k]/ma->occ;
                           x = va->cov.var[k]/occim - muDiffk*muDiffk;
                           if (applyVFloor && x<mpV[k]) {
                              x = mpV[k];
                              nFloorVar++;
                              mixFloored = TRUE;
                           }
                           cov.var[k] = x;
                        }
                     }
                     else { /* FULLC */
                        for (k=1; k<=vSize; k++){
                           muDiffk=(shared)?0.0:ma->mu[k]/ma->occ;
                           for (l=1; l<=k; l++){
                              muDiffl=(shared)?0.0:ma->mu[l]/ma->occ;
                              x = va->cov.inv[k][l]/occim - muDiffk*muDiffl; 
                              if (applyVFloor && k==l && x<mpV[k]) {
                                 x = mpV[k];
                                 nFloorVar++;
                                 mixFloored = TRUE;
                              }
                              cov.inv[k][l] = x;
                           }
                        }
                        CovInvert(cov.inv,cov.inv);
                     }
                  }
                  else if (trace&T_UPD)    /* PLAINHS & MSD, this warning often happen */
                    HError(-2330,"UpdateVars: Model %d[%s]: no use of variance %d.%d.%d",
                           px,HMMPhysName(hset,hmm),i,s,m);
                  if (mixFloored == TRUE) nFloorVarMix++;
                  SetHook(cov.var,NULL);
               }
            }
      }
   }
   
   return;
}

/* UpdateTMVars: use acc values to calc new estimate of vars for TIEDHS */
void UpdateTMVars(HMMSet *hset)
{
   int s,m,k,l,M,S,vSize;
   float occim,x,muDiffk,muDiffl;
   Vector minV;
   VaAcc *va;
   MuAcc *ma;
   MixPDF *mpdf;
   Vector mean;
   Covariance cov;
   Boolean mixFloored,shared;
   
   S = hset->swidth[0];
   for (s=1;s<=S;s++){
      vSize = hset->swidth[s];
      minV = vFloor[s];
      M = hset->tmRecs[s].nMix;
      for (m=1;m<=M;m++){
         mpdf = hset->tmRecs[s].mixes[m];
         cov = mpdf->cov;
         va = (VaAcc *) GetHook(cov.var);
         mean = mpdf->mean;
         ma = (MuAcc *) GetHook(mean);
         if (va != NULL){
            occim = va->occ;
            mixFloored = FALSE;
            if (occim > 0.0){
               shared=((GetUse(cov.var)>1 || ma==NULL || ma->occ<=0.0)) ? TRUE:FALSE;
               if (mpdf->ckind==DIAGC) {
                  for (k=1; k<=vSize; k++){
                     muDiffk=(shared)?0.0:ma->mu[k]/ma->occ;
                     x = va->cov.var[k]/occim - muDiffk*muDiffk;
                     if (applyVFloor && x<minV[k]) {
                        x = minV[k];
                        nFloorVar++;
                        mixFloored = TRUE;
                     }
                     cov.var[k] = x;
                  }
               }
               else { /* FULLC */
                  for (k=1; k<=vSize; k++){
                     muDiffk=(shared)?0.0:ma->mu[k]/ma->occ;
                     for (l=1; l<=k; l++){
                        muDiffl=(shared)?0.0:ma->mu[l]/ma->occ;
                        x = va->cov.inv[k][l]/occim - muDiffk*muDiffl;
                        if (applyVFloor && k==l && x<minV[k]) {
                           x = minV[k];
                           nFloorVar++;
                           mixFloored = TRUE;
                        }
                        cov.inv[k][l] = x;
                     }
                  }
                  CovInvert(cov.inv,cov.inv);
               }
            }
            else
               HError(-2330,"UpdateTMVars: No use of var %d in stream %d",m,s);
            if (mixFloored == TRUE) nFloorVarMix++;
            SetHook(cov.var,NULL);
         }
      }
   }
     
   return;
}

static  int fltcompare(const void *_i, const void *_j)
{
  const float *i = (const float*)_i;
  const float *j = (const float*)_j;
  if (*i > *j)
    return (1);
  if (*i < *j)
    return (-1);
  return (0);
}

void FloorVars(HMMSet *hset1, int s)
{
  HMMScanState hss1;
  int vsize;
  int i;
  if(!(hset1->hsKind==PLAINHS || hset1->hsKind==SHAREDHS)){
     HError(1, "Percentile var flooring not supported for this kind of hmm set. (e.g. tied.) should be easy.");
  } else { 
     float **varray;
     int M=0,m=0,floored=0;
     vsize = hset1->swidth[s];
     
     NewHMMScan(hset1,&hss1); 
     while(GoNextMix(&hss1,FALSE)){
         if ((hss1.s==s) && (VectorSize(hss1.mp->mean)==vsize))  /* MSD */
            M++;
     }
     EndHMMScan(&hss1); 

     varray = New(&gstack, sizeof(float*) * (vsize+1));
      for (i=1; i<=vsize; i++) 
         varray[i] = New(&gstack, sizeof(float) * M);

     NewHMMScan(hset1,&hss1); 
     while(GoNextMix(&hss1,FALSE)){
         if ((hss1.s==s) && (VectorSize(hss1.mp->mean)==vsize)) {  /* MSD */
           int k;
           if(hss1.mp->ckind != DIAGC ) HError(1, "FloorVars expects DIAGC covariances. ");
           
           for(k=1;k<=vsize;k++){
              varray[k][m] = hss1.mp->cov.var[k];
           }
           m++;
        }
     }
     EndHMMScan(&hss1); 
     
     for(i=1;i<=vsize;i++){
        qsort((char *) varray[i], M, sizeof(float), fltcompare);
     }

     if(varFloorPercent <=0 || varFloorPercent >= 100) HError(1, "varFloorPercent should be <100 and >0..");
     

     NewHMMScan(hset1,&hss1); 
     while(GoNextMix(&hss1,FALSE)){
        if ((hss1.s==s) && (VectorSize(hss1.mp->mean)==vsize)) {  /* MSD */
           int k, Pos = (int)(varFloorPercent*0.01*M);
           for(k=1;k<=vsize;k++){
              if(hss1.mp->cov.var[k] < varray[k][Pos]){
                 hss1.mp->cov.var[k] =  varray[k][Pos];
                 floored++;
              }
           }
        }
     }
     EndHMMScan(&hss1); 
     printf("Floored %d (expected to floor %d)\n", floored, (int)( varFloorPercent * 0.01 * M * vsize));

      for (i=vsize; i>=1; i--)
         Dispose(&gstack, varray[i]);
      Dispose(&gstack, varray);
  }

  FixAllGConsts(hset1);
 
   return;
}

void MLUpdateModels(HMMSet *hset, UPDSet uFlags)
{
   HSetKind hsKind;
   HMMScanState hss;
   HLink hmm;
   int px,maxM;
   long n;

   SetVFloor(hset,vFloor,minVar);
   hsKind = hset->hsKind;
   maxM = MaxMixInSet(hset);

   if (hsKind == TIEDHS){ /* TIEDHS - update mu & var once per HMMSet */
      if (uFlags & UPVARS)
         UpdateTMVars(hset);
      if (uFlags & UPMEANS)
         UpdateTMMeans(hset);
      if (uFlags & (UPMEANS|UPVARS))
         FixAllGConsts(hset);
   }

   NewHMMScan(hset,&hss);
   px=1;
   do {   
      hmm = hss.hmm;
      n = (long)hmm->hook;
      if (n<minEgs && !(trace&T_UPD))
         HError(-2331,"UpdateModels: %s[%d] copied: only %ld egs\n",
                HMMPhysName(hset,hmm),px,n);
      if (trace&T_UPD) {
         if (n<minEgs)
            printf("Model %s[%d] copied: only %ld examples\n",
                   HMMPhysName(hset,hmm),px,n);
         else
            printf("Model %s[%d] to be updated with %ld examples\n",
                   HMMPhysName(hset,hmm),px,n);
         fflush(stdout);
      }
      if (n>=minEgs && n>0) {
         if (uFlags & UPTRANS)
            UpdateTrans(hset,px,hmm);
         if (maxM>1 && uFlags & UPMIXES)
            UpdateWeights(hset,px,hmm);
         if (hsKind != TIEDHS){
            if (uFlags & UPVARS)
               UpdateVars(hset,px,hmm);
            if (uFlags & UPMEANS)
               UpdateMeans(hset,px,hmm);
            if (uFlags & (UPMEANS|UPVARS)) 
               FixGConsts(hmm);
         }  
      }
      px++;
   } while (GoNextHMM(&hss));
   EndHMMScan(&hss);
   if (trace&T_TOP) {
      if (nFloorVar > 0)
         printf("Total %d floored variance elements in %d different mixes\n",
                nFloorVar,nFloorVarMix);
      fflush(stdout);
   }
   ResetVFloor(hset,vFloor);
}

/* UpdateModels: update all models and save them in newDir if set,
   new files have newhmmExt if set */
void UpdateModels (HMMSet *hset, XFInfo *xfinfo, ParmBuf pbuf2, UPDSet uFlags)
{
   int maxM;
   static char str[MAXSTRLEN];
   BufferInfo info2;
   char macroname[MAXSTRLEN];

   nFloorVar = 0;     /* # of floored variance comps */
   nFloorVarMix = 0;  /* # of mix comps with floored vars */

   if (trace&T_UPD){
      printf("Starting Model Update\n"); fflush(stdout);
   }
   if (parMode == -1) {
      ForceDiagC(hset); /* invert the variances again */
      ConvExpWt(hset);
   }
   maxM = MaxMixInSet(hset);

   /* 
      This routine tidies up the semi-tied transform and 
      tidies the model up. The transition and priors are 
      not updated 
   */
   if (uFlags & UPSEMIT) {
      UpdateSemiTiedModels(hset, xfinfo);
      uFlags = (UPDSet)(uFlags & ~(UPMEANS|UPVARS));
   }

   if (uFlags & UPMAP)
     MAPUpdateModels(hset, uFlags);
   else
     MLUpdateModels(hset, uFlags);     
   
   if(varFloorPercent){
      int s;
      printf("Flooring all vars to the %f'th percentile of distribution... ", varFloorPercent);
      for(s=1;s<=hset->swidth[0];s++)
         FloorVars(hset,s);
   }

   ClearSeenFlags(hset,CLR_ALL);
   if (twoDataFiles){
      if (parMode == 0){
         SetChannel("HPARM2");
         nParm = GetConfig("HPARM2", TRUE, cParm, MAXGLOBS);
         if (GetConfStr(cParm,nParm,"TARGETKIND",str))
            hset->pkind = Str2ParmKind(str);
	 if (GetConfStr(cParm,nParm,"MATTRANFN",str)) {
            /* The transform needs to be set-up */
            hset->xf = LoadInputXForm(hset,NameOf(str,macroname),str);
         }
      } else {
         GetBufferInfo(pbuf2,&info2);
         hset->pkind = info2.tgtPK;
	 hset->xf = info2.xform;
      }
   }

   if (xfinfo->outFullC && (uFlags&UPSEMIT)) {
      SetXForm(hset,xfinfo,xfinfo->outXForm);
      ApplyHMMSetXForm(hset, xfinfo->outXForm, TRUE);
      hset->semiTied = NULL;
   }
}

/* ----------------------------------------------------------- */
/*                      END:  HERest.c                         */
/* ----------------------------------------------------------- */





