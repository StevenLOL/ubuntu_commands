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

char *hmgens_version = "!HVER!HMGenS: 2.2 [NIT 07/07/11]";
char *hmgens_vc_id = "$Id: HMGenS.c,v 1.52 2011/06/16 04:27:17 uratec Exp $";

/*  
    This program is used to generate feature vector sequences 
    which maximize its output probability 
    from concatenated HMMs
    under the constraints between observed and            
    augmented vector sequences                            
*/

#include "HShell.h"
#include "HMem.h"
#include "HMath.h"
#include "HSigP.h"
#include "HAudio.h"
#include "HWave.h"
#include "HParm.h"
#include "HLabel.h"
#include "HModel.h"
#include "HTrain.h"
#include "HUtil.h"
#include "HAdapt.h"
#include "HMap.h"
#include "HFB.h"
#include "HGen.h"

/* ----------------------- Trace Flags ----------------------- */

#define T_TOP 0001              /* Top level tracing              */
#define T_DUR 0002              /* show duration of each state    */

/* ---------------- Global Data Structures --------------------- */


/* Global Settings */

static char *labDir = NULL;     /* label (transcription) file directory */
static char *labExt = "lab";    /* label file extension */
static char *hmmDir = NULL;     /* directory to look for hmm def files */
static char *hmmExt = NULL;     /* hmm def file extension */
static char *dmDir = NULL;      /* directory to look for dm def files */
static char *dmExt = NULL;      /* dm def file extension */
static char *genDir = NULL;     /* directory to store new hmm def files */
static char *genExt = "gen";    /* extension for generated sequence */
static char *durExt = "dur";    /* extension for state duration */
static char *pdfExt = "pdf";    /* extension for pdf sequence */
static char *winDir = NULL;     /* directory to look for window files */
static char *winExt = NULL;     /* extension for window file */

static MemHeap hmmStack;        /* Stack holds all hmm related info */
static MemHeap dmStack;         /* Stack holds all duration model related info */
static MemHeap genStack;        /* Heap holds all calcuration info */
static MemHeap uttStack;        /* Stack for utterance info */
static MemHeap fbInfoStack;     /* Stack for Forward-Backward info */

static HMMSet hmset;            /* HMM set */
static HMMSet dmset;            /* Duration model set */

static FileFormat lff = UNDEFF; /* label file format */

/* variables for forward-backward */
static ConfParam *cParm[MAXGLOBS];      /* configuration parameters */
static int nParm = 0;           /* total num params */
static UttInfo *utt;            /* utterance information storage */
static GenInfo *genInfo;        /* generation information */
static FBInfo *fbInfo;          /* forward-backward information storage */
static Boolean useHMMFB = FALSE;        /* do not use duration models in the forward-backward algorithm */
Boolean keepOccm = TRUE;        /* keep mixture-level occupancy prob for EM-based parameter generation */

/* Global settings */
static int trace = 0;           /* trace level */
static int totalT = 0;          /* total number of frames in training data */
static LogDouble totalPr = 0;   /* total log prob upto current utterance */

static LogDouble pruneInit = NOPRUNE;   /* pruning threshold initially */
static LogDouble pruneInc = 0.0;        /* pruning threshold increment */
static LogDouble pruneLim = NOPRUNE;    /* pruning threshold limit */
static float minFrwdP = NOPRUNE;        /* mix prune threshold */

/* information about transforms */
static XFInfo xfInfo_hmm;
static XFInfo xfInfo_dur;

/* configuration variables */
static Boolean inBinary = FALSE;        /* set to generated parameters in binary */
static Boolean outPdf = FALSE;  /* output pdf */
static ParmGenType type = CHOLESKY;     /* type of parameter generation algorithm */
static Boolean modelAlign = FALSE;      /* use model alignment from label */
static Boolean stateAlign = FALSE;      /* use state alignment from label */
static Boolean useAlign = FALSE;        /* use model alignment from label for EM-based parameter generation */
static float MSDthresh = 0.5;   /* threshold for swithing space index for MSD */
static HTime frameRate = 50000; /* frame rate (default: 5ms) */
static float speakRate = 1.0;   /* speaking rate (1.0 => standard speaking rate) */

static IntVec nPdfStr = NULL;   /* # of PdfStream */
static IntVec pdfStrOrder = NULL;       /* order of each PdfStream */
static char **pdfStrExt = NULL; /* fn ext of each PdfStream */
static char **winFn[SMAX];      /* fn of window */

/* --------------------- Process Command Line ---------------------- */

void SetConfParms(void)
{
   int s, i;
   char *c, buf[MAXSTRLEN], tmp[MAXSTRLEN];
   Boolean b;

   for (s = 1; s < SMAX; s++)
      winFn[s] = NULL;

   nParm = GetConfig("HMGENS", TRUE, cParm, MAXGLOBS);
   if (nParm > 0) {
      if (GetConfInt(cParm, nParm, "TRACE", &i))
         trace = i;
      if (GetConfBool(cParm, nParm, "SAVEBINARY", &b))
         inBinary = b;
      if (GetConfBool(cParm, nParm, "OUTPDF", &b))
         outPdf = b;
      if (GetConfInt(cParm, nParm, "PARMGENTYPE", &i))
         type = (ParmGenType) i;
      if (GetConfBool(cParm, nParm, "MODELALIGN", &b))
         modelAlign = b;
      if (GetConfBool(cParm, nParm, "STATEALIGN", &b))
         stateAlign = b;
      if (GetConfBool(cParm, nParm, "USEALIGN", &b))
         useAlign = b;
      if (GetConfBool(cParm, nParm, "USEHMMFB", &b))
         useHMMFB = b;
      if (GetConfStr(cParm, nParm, "INXFORMMASK", buf))
         xfInfo_hmm.inSpkrPat = xfInfo_dur.inSpkrPat = CopyString(&genStack, buf);
      if (GetConfStr(cParm, nParm, "PAXFORMMASK", buf))
         xfInfo_hmm.paSpkrPat = xfInfo_dur.paSpkrPat = CopyString(&genStack, buf);

      if (GetConfStr(cParm, nParm, "PDFSTRSIZE", buf))
         nPdfStr = ParseConfIntVec(&gstack, buf, TRUE);
      if (GetConfStr(cParm, nParm, "PDFSTRORDER", buf))
         pdfStrOrder = ParseConfIntVec(&gstack, buf, TRUE);
      if (GetConfStr(cParm, nParm, "PDFSTREXT", buf))
         pdfStrExt = ParseConfStrVec(&gstack, buf, TRUE);
      if (GetConfStr(cParm, nParm, "WINEXT", buf))
         winExt = CopyString(&gstack, buf);
      if (GetConfStr(cParm, nParm, "WINDIR", buf))
         winDir = CopyString(&gstack, buf);
      if (GetConfStr(cParm, nParm, "WINFN", buf)) {
         for (s = 1, c = buf; s < SMAX && c != NULL; s++) {
            winFn[s] = ParseConfStrVec(&gstack, c, FALSE);
            for (i = 1; i <= winFn[s][0][0] + 2; i++)   /* "StrVec" + "n" + # of file names */
               c = ParseString(c, tmp);
            if (ParseString(c, tmp) == NULL)
               break;
         }
      }
   }
}

void ReportUsage(void)
{
   printf("\nUSAGE: HMGenS [options] hmmList dmList labFiles...\n\n");
   printf(" Option                                                    Default\n\n");
   printf(" -a      Use an input linear transform for HMMs            off\n");
   printf(" -b      Use an input linear transform for dur models      off\n");
   printf(" -c n    type of parameter generation algorithm            0\n");
   printf("          0: both mix and state sequences are given        \n");
   printf("          1: state sequence is given,                      \n");
   printf("             but mix sequence is hidden                    \n");
   printf("          2: both state and mix sequences are hidden       \n");
   printf(" -d s    dir to find hmm definitions                       current\n");
   printf(" -e      use model alignment from label for pruning        off\n");
   printf(" -f f    frame shift in 100 ns                             50000\n");
   printf(" -g f    Mixture pruning threshold                         10.0\n");
   printf(" -h s [s] set speaker name pattern to s,                   *.%%%%%%\n");
   printf("         optionally set parent patterns                    \n");
   printf(" -m      use model alignment for duration                  off\n");
   printf(" -n s    dir to find duration model definitions            current\n");
   printf(" -p      output pdf sequences                              off\n");
   printf(" -r f    speaking rate factor (f<1: fast  f>1: slow)       1.0\n");
   printf(" -s      use state alignment for duration                  off\n");
   printf(" -t f [i l] set pruning to f [inc limit]                   inf\n");
   printf(" -v f    threshold for switching spaces for MSD            0.5\n");
   printf(" -x s    extension for hmm files                           none\n");
   printf(" -y s    extension for duration model files                none\n");
   PrintStdOpts("BEGHIJLMNSTWXY");
   printf("\n");
   Exit(0);
}

void CheckGenSetUp(void)
{
   int d, p, r, s, stream;
   PdfStream *pst = NULL;

   /* # of PdfStreams */
   genInfo->nPdfStream[0] = (nPdfStr == NULL) ? hmset.swidth[0] : IntVecSize(nPdfStr);
   if (genInfo->nPdfStream[0] > hmset.swidth[0])
      HError(999, "CheckGenSetUp: # of PdfStreams (%d) is too large (should be less than %d)", genInfo->nPdfStream[0], hmset.swidth[0]);

   /* size of each PdfStreams */
   r = hmset.swidth[0];
   for (p = stream = 1; p <= genInfo->nPdfStream[0]; stream += genInfo->nPdfStream[p++]) {
      pst = &(genInfo->pst[p]);

      /* # of streams in this PdfStream */
      genInfo->nPdfStream[p] = (nPdfStr == NULL) ? 1 : nPdfStr[p];
      r -= genInfo->nPdfStream[p];

      /* calculate vector size for this PdfStream */
      for (s = stream, pst->vSize = 0; s < stream + genInfo->nPdfStream[p]; s++)
         pst->vSize += hmset.swidth[s];

      /* order (vecSize of static feature vector) of this PdfStream */
      pst->order = (pdfStrOrder == NULL) ? 1 : pdfStrOrder[p];
      if (pst->order < 1 || pst->order > pst->vSize)
         HError(999, "CheckGenSetUp: Order of PdfStream %d should be within 1--%d", p, pst->vSize);

      /* window coefficients */
      if (winFn[p] == NULL)
         HError(9999, "CheckGenSetUp: window file names are not specified");
      pst->win.num = (int) winFn[p][0][0];
      if (pst->win.num > MAXWINNUM)
         HError(9999, "CheckGenSetUp: # of window out of range");
      if (pst->win.num * pst->order != pst->vSize)
         HError(9999, "CheckGenSetUp: # of window (%d) times order (%d) should be equal to vSize (%d)", pst->win.num, pst->order, pst->vSize);
      for (d = 0; d < pst->win.num; d++)
         MakeFN(winFn[p][d + 1], winDir, winExt, pst->win.fn[d]);

      /* generated filename ext */
      if (pdfStrExt == NULL)
         strcpy(pst->ext, genExt);
      else
         strcpy(pst->ext, pdfStrExt[p]);
   }
   if (r != 0)
      HError(9999, "CheckGenSetUp: # of streams in HMMSet (%d) and PdfStreams (%d) are inconsistent", hmset.swidth[0], genInfo->nPdfStream[0]);

   /* output trace information */
   if (trace & T_TOP) {
      for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
         printf("PdfStream [%d]:\n", p);
         printf("  #streams: %d (vSize=%d)\n", genInfo->nPdfStream[p], genInfo->pst[p].vSize);
         printf("  #order:   %d\n", genInfo->pst[p].order);
         printf("  file ext: %s\n", genInfo->pst[p].ext);
         for (d = 0; d < pst->win.num; d++)
            printf("  %d-th window: %s\n", d + 1, genInfo->pst[p].win.fn[d]);
      }
      printf("\n");
      fflush(stdout);
   }
}

int main(int argc, char *argv[])
{
   char *s;
   char *labfn;
   int numUtt;

   void Initialise(void);
   void DoGeneration(char *labfn);

   if (InitShell(argc, argv, hmgens_version, hmgens_vc_id) < SUCCESS)
      HError(2300, "HMGenS: InitShell failed");
   InitMem();
   InitMath();
   InitSigP();
   InitWave();
   InitLabel();
   InitModel();
   if (InitParm() < SUCCESS)
      HError(2300, "HMGenS: InitParm failed");
   InitUtil();
   InitFB();
   InitAdapt(&xfInfo_hmm, &xfInfo_dur);
   InitMap();
   InitGen();

   if (NumArgs() == 0)
      ReportUsage();

   CreateHeap(&genStack, "genStore", MSTAK, 1, 1.0, 80000, 400000);
   CreateHeap(&uttStack, "uttStore", MSTAK, 1, 0.5, 100, 1000);
   CreateHeap(&fbInfoStack, "FBInfoStore", MSTAK, 1, 0.5, 100, 1000);
   CreateHeap(&hmmStack, "HmmStore", MSTAK, 1, 1.0, 50000, 500000);
   CreateHeap(&dmStack, "dmStore", MSTAK, 1, 1.0, 50000, 500000);
   SetConfParms();
   CreateHMMSet(&hmset, &hmmStack, TRUE);
   CreateHMMSet(&dmset, &dmStack, TRUE);

   utt = (UttInfo *) New(&uttStack, sizeof(UttInfo));
   genInfo = (GenInfo *) New(&genStack, sizeof(GenInfo));
   fbInfo = (FBInfo *) New(&fbInfoStack, sizeof(FBInfo));

   while (NextArg() == SWITCHARG) {
      s = GetSwtArg();
      if (strlen(s) != 1)
         HError(9919, "HMGenS: Bad switch %s; must be single letter", s);
      switch (s[0]) {
      case 'a':
         xfInfo_hmm.useInXForm = TRUE;
         break;
      case 'b':
         xfInfo_dur.useInXForm = TRUE;
         break;
      case 'c':
         if (NextArg() != INTARG)
            HError(2119, "HMGenS: Parameter generation algorithm type value expected");
         type = (ParmGenType) GetChkedInt(CHOLESKY, FB, s);
         break;
      case 'd':
         if (NextArg() != STRINGARG)
            HError(2319, "HMGenS: HMM definition directory expected");
         hmmDir = GetStrArg();
         break;
      case 'e':
         useAlign = TRUE;
         break;
      case 'f':
         frameRate = (HTime) GetChkedFlt(0.0, 10000000.0, s);
         break;
      case 'g':
         minFrwdP = GetChkedFlt(0.0, 1000.0, s);
         break;
      case 'h':
         if (NextArg() != STRINGARG)
            HError(1, "Speaker name pattern expected");
         xfInfo_hmm.inSpkrPat = xfInfo_dur.inSpkrPat = GetStrArg();
         if (NextArg() == STRINGARG)
            xfInfo_hmm.paSpkrPat = xfInfo_dur.paSpkrPat = GetStrArg();
         if (NextArg() != SWITCHARG)
            HError(2319, "HMGenS: cannot have -h as the last option");
         break;
      case 'm':
         modelAlign = TRUE;
         break;
      case 'n':
         if (NextArg() != STRINGARG)
            HError(2319, "HMGenS: duration model definition directory expected");
         dmDir = GetStrArg();
         break;
      case 'p':
         outPdf = TRUE;
         break;
      case 'r':
         if (NextArg() != FLOATARG)
            HError(2119, "HMGenS: Speaking rate value (float) expected");
         speakRate = GetChkedFlt(0.0, 3.0, s);
         break;
      case 's':
         stateAlign = TRUE;
         break;
      case 't':
         pruneInit = GetChkedFlt(0.0, 1.0E20, s);
         if (NextArg() == FLOATARG || NextArg() == INTARG) {
            pruneInc = GetChkedFlt(0.0, 1.0E20, s);
            pruneLim = GetChkedFlt(0.0, 1.0E20, s);
         } else {
            pruneInc = 0.0;
            pruneLim = pruneInit;
         }
         break;
      case 'v':
         MSDthresh = GetChkedFlt(0.0, 1.0, s);
         break;
      case 'x':
         if (NextArg() != STRINGARG)
            HError(2319, "HMGenS: HMM file extension expected");
         hmmExt = GetStrArg();
         break;
      case 'y':
         if (NextArg() != STRINGARG)
            HError(2319, "HMGenS: duration model file extension expected");
         dmExt = GetStrArg();
         break;
      case 'B':
         inBinary = TRUE;
         break;
      case 'E':
         if (NextArg() != STRINGARG)
            HError(2319, "HMGenS: parent transform directory expected");
         xfInfo_hmm.usePaXForm = TRUE;
         xfInfo_hmm.paXFormDir = GetStrArg();
         if (NextArg() == STRINGARG)
            xfInfo_hmm.paXFormExt = GetStrArg();
         if (NextArg() != SWITCHARG)
            HError(2319, "HMGenS: cannot have -E as the last option");
         break;
      case 'G':
         if (NextArg() != STRINGARG)
            HError(2119, "HMGenS: Label File format expected");
         if ((lff = Str2Format(GetStrArg())) == ALIEN)
            HError(-2189, "HMGenS: Warning ALIEN Label file format set");
         break;
      case 'H':
         if (NextArg() != STRINGARG)
            HError(3219, "HMGenS: HMM MMF File name expected");
         AddMMF(&hmset, GetStrArg());
         break;
      case 'I':
         if (NextArg() != STRINGARG)
            HError(2319, "HMGenS: MLF file name expected");
         LoadMasterFile(GetStrArg());
         break;
      case 'J':
         if (NextArg() != STRINGARG)
            HError(2319, "HMGenS: input transform directory expected");
         AddInXFormDir(&hmset, GetStrArg());
         if (NextArg() == STRINGARG)
            xfInfo_hmm.inXFormExt = GetStrArg();
         if (NextArg() != SWITCHARG)
            HError(2319, "HMGenS: cannot have -J as the last option");
         break;
      case 'L':
         if (NextArg() != STRINGARG)
            HError(2319, "HMGenS: Label file directory expected");
         labDir = GetStrArg();
         break;
      case 'M':
         if (NextArg() != STRINGARG)
            HError(2319, "HMGenS: Output macro file directory expected");
         genDir = GetStrArg();
         break;
      case 'N':
         if (NextArg() != STRINGARG)
            HError(3219, "HMGenS: Duration MMF File name expected");
         AddMMF(&dmset, GetStrArg());
         break;
      case 'T':
         if (NextArg() != INTARG)
            HError(2119, "HMGenS: Trace value expected");
         trace = GetChkedInt(0, 0002, s);
         break;
      case 'W':
         if (NextArg() != STRINGARG)
            HError(2319, "HMGenS: parent duration transform directory expected");
         xfInfo_dur.usePaXForm = TRUE;
         xfInfo_dur.paXFormDir = GetStrArg();
         if (NextArg() == STRINGARG)
            xfInfo_dur.paXFormExt = GetStrArg();
         if (NextArg() != SWITCHARG)
            HError(2319, "HMGenS: cannot have -W as the last option");
         break;
      case 'X':
         if (NextArg() != STRINGARG)
            HError(2319, "HMGenS: Label file extension expected");
         labExt = GetStrArg();
         break;
      case 'Y':
         if (NextArg() != STRINGARG)
            HError(2319, "HMGenS: input duration transform directory expected");
         AddInXFormDir(&dmset, GetStrArg());
         if (NextArg() == STRINGARG) {
            if (xfInfo_dur.inXFormExt == NULL)
               xfInfo_dur.inXFormExt = GetStrArg();
            else
               HError(2319, "MGenS: only one input duration transform extension may be specified");
         }
         if (NextArg() != SWITCHARG)
            HError(2319, "HMGenS: cannot have -Y as the last option");
         break;
      default:
         HError(9919, "HMGenS: Unknown switch %s", s);
      }
   }
   if (NextArg() != STRINGARG)
      HError(2319, "HMGenS: file name of vocabulary list expected");

   Initialise();
   InitUttInfo(utt, FALSE);
   numUtt = 1;

   if (trace & T_TOP)
      SetTraceGen();

   /* generate parameter sequences */
   do {
      if (NextArg() != STRINGARG)
         HError(2319, "HMGenS: data file name expected");
      labfn = GetStrArg();

      /* track speakers */
      if (UpdateSpkrStats(&hmset, &xfInfo_hmm, labfn)) {
         if (!xfInfo_hmm.useInXForm)
            xfInfo_hmm.inXForm = NULL;
      }
      if (UpdateSpkrStats(&dmset, &xfInfo_dur, labfn)) {
         if (!xfInfo_dur.useInXForm)
            xfInfo_dur.inXForm = NULL;
         else
            ResetDMMPreComps(&dmset);
      }

      fbInfo->xfinfo_hmm = &xfInfo_hmm;
      fbInfo->xfinfo_dur = &xfInfo_dur;
      fbInfo->inXForm_hmm = xfInfo_hmm.inXForm;
      fbInfo->inXForm_dur = xfInfo_dur.inXForm;
      fbInfo->al_inXForm_hmm = xfInfo_hmm.al_inXForm;
      fbInfo->al_inXForm_dur = xfInfo_dur.al_inXForm;
      fbInfo->paXForm_hmm = xfInfo_hmm.paXForm;
      fbInfo->paXForm_dur = xfInfo_dur.paXForm;

      /* generate parameters */
      DoGeneration(labfn);
      numUtt++;
   } while (NumArgs() > 0);

   if (trace & T_TOP) {
      printf("Generation complete - average log prob per frame = %e (%d frames)\n", totalPr / totalT, totalT);
   }

   /* Reset stacks */
   Dispose(&fbInfoStack, fbInfo);
   Dispose(&genStack, genInfo);
   Dispose(&uttStack, utt);
   ResetHeap(&fbInfoStack);
   ResetHeap(&uttStack);
   ResetHeap(&genStack);
   ResetHeap(&dmStack);
   ResetHeap(&hmmStack);

   /* Reset modules */
   ResetGen();
   ResetAdapt(&xfInfo_hmm, &xfInfo_dur);
   ResetFB();
   ResetUtil();
   ResetParm();
   ResetModel();
   ResetLabel();
   ResetWave();
   ResetSigP();
   ResetMath();
   ResetMem();
   ResetShell();

   Exit(0);
   return (0);                  /* never reached -- make compiler happy */
}

/* -------------------------- Initialisation ----------------------- */

void Initialise(void)
{
   /* load HMM mmf */
   if (MakeHMMSet(&hmset, GetStrArg()) < SUCCESS)
      HError(9928, "Initialise: MakeHMMSet failed");
   if (LoadHMMSet(&hmset, hmmDir, hmmExt) < SUCCESS)
      HError(9928, "Initialise: LoadHMMSet failed");
   if (hmset.hsKind == DISCRETEHS)
      HError(9999, "Initialise: Only continuous model is surpported");
   ConvDiagC(&hmset, TRUE);

   if (trace & T_TOP) {
      PrintrFlags();
      printf("HMMSet is ");
      switch (hmset.hsKind) {
      case PLAINHS:
         printf("PLAIN\n");
         break;
      case SHAREDHS:
         printf("SHARED\n");
         break;
      case TIEDHS:
         printf("TIED\n");
         break;
      default:
         HError(9999, "Initialise: Only continuous model is supported");
      }

      printf("%d Logical/%d Physical Models Loaded, VecSize=%d\n", hmset.numLogHMM, hmset.numPhyHMM, hmset.vecSize);
      if (hmset.numFiles > 0)
         printf("%d MMF input files\n", hmset.numFiles);
      fflush(stdout);
   }

   /* load duration mmf */
   if (!stateAlign) {
      if (MakeHMMSet(&dmset, GetStrArg()) < SUCCESS)
         HError(9928, "Initialise: MakeHMMSet failed");
      if (LoadHMMSet(&dmset, dmDir, dmExt) < SUCCESS)
         HError(9928, "Initialise: LoadHMMSet failed");
      if (hmset.hsKind == DISCRETEHS)
         HError(9999, "Initialise: Only continuous duration model mmf is surpported");
      ConvDiagC(&dmset, TRUE);

      if (trace & T_TOP) {
         printf("DurationModelSet is ");
         switch (dmset.hsKind) {
         case PLAINHS:
            printf("PLAIN\n");
            break;
         case SHAREDHS:
            printf("SHARED\n");
            break;
         case TIEDHS:
            printf("TIED\n");
            break;
         default:
            HError(9999, "Initialise: Only continuous model is supported");
         }

         printf("%d Logical/%d Physical Models Loaded, VecSize=%d\n", dmset.numLogHMM, dmset.numPhyHMM, dmset.vecSize);
         if (dmset.numFiles > 0)
            printf("%d MMF input files\n", dmset.numFiles);
         fflush(stdout);
      }
   }

   /* setup GenInfo */
   genInfo->genMem = &genStack;
   genInfo->hset = &hmset;
   genInfo->dset = &dmset;
   genInfo->maxStates = MaxStatesInSet(&hmset);
   genInfo->maxMixes = MaxMixInSet(&hmset);
   genInfo->speakRate = speakRate;
   genInfo->MSDthresh = MSDthresh;
   genInfo->modelAlign = modelAlign;
   genInfo->stateAlign = stateAlign;
   genInfo->frameRate = frameRate;

   CheckGenSetUp();

   /* setup EM-based parameter generation */
   AttachAccs(&hmset, &gstack, (UPDSet) 0);
   ZeroAccs(&hmset, (UPDSet) 0);

   if (!stateAlign) {
      AttachAccs(&dmset, &gstack, (UPDSet) 0);
      ZeroAccs(&dmset, (UPDSet) 0);
   }

   if (type != CHOLESKY)
      InitialiseForBack(fbInfo, &fbInfoStack, &hmset, (UPDSet) 0, ((useHMMFB || stateAlign) ? NULL : &dmset), (UPDSet) 0, pruneInit, pruneInc, pruneLim, minFrwdP, useAlign, FALSE);

   /* handle input xform */
   xfInfo_hmm.inFullC = xfInfo_dur.inFullC = TRUE;

   /* semi-tied case */
   if (hmset.semiTied != NULL) {
      SetXForm(&hmset, NULL, hmset.semiTied);
      ApplyHMMSetXForm(&hmset, hmset.semiTied, TRUE);
   }

   /* linear weight -> log weight */
   ConvLogWt(&hmset);
}

/* -------------------- output routines ------------------------- */

/* WriteStateDurations: output state duration to file */
void WriteStateDurations(char *labfn, GenInfo * genInfo)
{
   char fn[MAXFNAMELEN];
   int i, j, k, s, cnt, nState, modeldur;
   float modelMean;
   Label *label;
   FILE *durfp;
   Vector mean = NULL;
   Boolean isPipe;

   /* open file pointer for saving state durations */
   MakeFN(labfn, genDir, durExt, fn);
   if ((durfp = FOpen(fn, NoOFilter, &isPipe)) == NULL)
      HError(9911, "WriteStateDurations: Cannot create output file %s", fn);

   /* prepare mean vector */
   mean = CreateVector(genInfo->genMem, genInfo->maxStates);

   for (i = 1; i <= genInfo->labseqlen; i++) {
      label = GetLabN(genInfo->labseq->head, i);
      nState = genInfo->hmm[i]->numStates - 2;

      /* compose mean vector of the i-th state duration model */
      for (s = cnt = 1; s <= genInfo->dset->swidth[0]; s++) {
         for (k = 1; k <= genInfo->dset->swidth[s]; k++, cnt++)
            mean[cnt] = genInfo->dm[i]->svec[2].info->pdf[s].info->spdf.cpdf[1].mpdf->mean[k];
      }

      modeldur = 0;
      modelMean = 0.0;
      for (j = 1; genInfo->sindex[i][j] != 0; j++) {
         /* output state duration */
         fprintf(durfp, "%s.state[%d]: duration=%d (frame), mean=%e\n", label->labid->name, genInfo->sindex[i][j], genInfo->durations[i][j], mean[genInfo->sindex[i][j] - 1]);
         fflush(durfp);

         if (trace & T_DUR) {
            printf("%s.state[%d]: duration=%d (frame), mean=%e\n", label->labid->name, genInfo->sindex[i][j], genInfo->durations[i][j], mean[genInfo->sindex[i][j] - 1]);
            fflush(stdout);
         }

         modeldur += genInfo->durations[i][j];
         modelMean += mean[genInfo->sindex[i][j] - 1];
      }

      fprintf(durfp, "%s: duration=%d (frame), mean=%e\n", label->labid->name, modeldur, modelMean);
      fflush(durfp);

      if (trace & T_DUR) {
         printf("%s: duration=%d (frame), mean=%e\n", label->labid->name, modeldur, modelMean);
         fflush(stdout);
      }
   }

   /* dispose mean vector */
   FreeVector(genInfo->genMem, mean);

   /* close file pointer for saving state durations */
   FClose(durfp, isPipe);

   return;
}

/* WriteParms: write generated parameter vector sequences */
void WriteParms(char *labfn, GenInfo * genInfo)
{
   int p, t, v, k;
   char ext[MAXSTRLEN], fn[MAXFNAMELEN];
   float ig;
   Vector igvec;
   TriMat igtm;
   FILE *parmfp = NULL, *pdffp = NULL;
   Boolean isPipe1, isPipe2;
   PdfStream *pst;

   /* get ignore value for MSD */
   ig = ReturnIgnoreValue();

   /* save generated parameters */
   for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
      /* p-th PdfStream */
      pst = &(genInfo->pst[p]);

      /* create ignore value vector/triangular matrix */
      igvec = CreateVector(&genStack, pst->vSize);
      igtm = CreateTriMat(&genStack, pst->vSize);
      for (v = 1; v <= pst->vSize; v++) {
         igvec[v] = ig;
         for (k = 1; k <= v; k++)
            igtm[v][k] = ig;
      }

      /* open file pointer for saving generated parameters */
      MakeFN(labfn, genDir, pst->ext, fn);
      if ((parmfp = FOpen(fn, NoOFilter, &isPipe1)) == NULL)
         HError(9911, "WriteParms: Cannot create ouput file %s", fn);

      /* open file pointer for saving pdf parameters */
      if (outPdf) {
         sprintf(ext, "%s_%s", pst->ext, pdfExt);
         MakeFN(labfn, genDir, ext, fn);
         if ((pdffp = FOpen(fn, NoOFilter, &isPipe2)) == NULL)
            HError(9911, "WriteParms: Cannot create output file %s", fn);
      }

      /* output generated parameter sequence */
      for (t = pst->t = 1; t <= genInfo->tframe; t++) {
         if (pst->ContSpace[t]) {
            /* output generated parameters */
            WriteVector(parmfp, pst->C[pst->t], inBinary);

            /* output pdfs */
            if (outPdf) {
               WriteVector(pdffp, pst->mseq[pst->t], inBinary);
               if (pst->fullCov)
                  WriteTriMat(pdffp, pst->vseq[pst->t].inv, inBinary);
               else
                  WriteVector(pdffp, pst->vseq[pst->t].var, inBinary);
            }

            pst->t++;
         } else {
            /* output ignoreValue symbol for generated parameters */
            WriteFloat(parmfp, &igvec[1], pst->order, inBinary);

            /* output ignoreValue symbol for pdfs */
            if (outPdf) {
               WriteVector(pdffp, igvec, inBinary);
               if (pst->fullCov)
                  WriteTriMat(pdffp, igtm, inBinary);
               else
                  WriteVector(pdffp, igvec, inBinary);
            }
         }
      }

      /* close file pointer */
      if (outPdf)
         FClose(pdffp, isPipe2);
      FClose(parmfp, isPipe1);

      /* free igvec */
      FreeVector(&genStack, igvec);
   }

   return;
}

/* ----------------------- Generate parameter sequences from HMMs ----------------------- */

/* DoGeneration: Generate parameter sequences from HMMs */
void DoGeneration(char *labfn)
{
   char labFn[MAXFNAMELEN], buf[MAXSTRLEN];
   int t;
   Boolean eSep;
   Transcription *tr;

   if (trace & T_TOP) {
      printf(" Generating Label %s\n", NameOf(labfn, buf));
      fflush(stdout);
   }

   /* load a given input label file */
   ResetHeap(&utt->transStack);
   MakeFN(labfn, labDir, labExt, labFn);
   tr = LOpen(&genStack, labFn, lff);

   /* compose a sentence HMM corresponding to the input label */
   InitialiseGenInfo(genInfo, tr, FALSE);

   /* set utterance informations for forward-backward algorithm */
   SetStreamWidths(hmset.pkind, hmset.vecSize, hmset.swidth, &eSep);
   utt->tr = tr;
   utt->Q = genInfo->labseqlen;
   utt->T = genInfo->tframe;
   utt->twoDataFiles = FALSE;
   utt->o = (Observation *) New(&gstack, utt->T * sizeof(Observation));
   utt->o--;
   for (t = 1; t <= utt->T; t++)
      utt->o[t] = MakeObservation(&gstack, hmset.swidth, hmset.pkind, FALSE, eSep);

   /* parameter generation */
   ParamGen(genInfo, utt, fbInfo, type);

   /* output state durations and generated parameter sequences */
   if (!stateAlign)
      WriteStateDurations(labfn, genInfo);
   WriteParms(labfn, genInfo);

   /* free memory */
   Dispose(&gstack, ++utt->o);
   ResetGenInfo(genInfo);

   /* increment total number of generated frames */
   totalT += utt->T;
   totalPr += utt->pr;

   return;
}

/* ----------------------------------------------------------- */
/*                      END:  HMGenS.c                         */
/* ----------------------------------------------------------- */
