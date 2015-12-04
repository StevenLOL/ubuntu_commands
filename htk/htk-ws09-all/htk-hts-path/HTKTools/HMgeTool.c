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

char *hmgetool_version = "!HVER!HMgeTool: 2.2 [NIT 07/07/11]";
char *hmgetool_vc_id = "$Id: HMgeTool.c,v 1.3 2011/06/19 11:53:07 uratec Exp $";

/*
  This program is used for MGE training
*/

/* ------------------- HMM ToolKit Modules ------------------- */
#include "HShell.h"
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
#include "HGen.h"
#include "HMTrain.h"

/* ----------------------- Trace Flags ----------------------- */
#define T_TOP   0001            /* Top level tracing */
#define T_PROC  0001            /* Process information */

/* --------------------- Global Settings --------------------- */
static char *labDir = NULL;     /* label (transcription) file directory */
static char *labExt = "lab";    /* label file extension */
static char *hmmDir = NULL;     /* directory to look for hmm def files */
static char *hmmExt = NULL;     /* hmm def file extension */
static char *outDir = NULL;     /* directory to store new hmm def files */
static char *outExt = NULL;     /* extension of new reestimated hmm files */
static char *winDir = NULL;     /* directory to look for window files */
static char *winExt = NULL;     /* extension for window file */
static char *mmfFn = NULL;      /* output MMF file, if any */
static char *scalefn = NULL;    /* scaling rate file */
static char *outLabDir = NULL;  /* output label file directory */
static char *hmmListFn = NULL;  /* hmm list file */
static char *xformfn = NULL;    /* transform file */

static UPDSet uFlags = (UPDSet) 0;      /* update flags */

static float MSDthresh = 0.5;   /* threshold for swithing space index for MSD */
static HTime frameRate = 50000; /* frame rate (default: 5ms) */

static MFType funcType = MGE_TRAIN;     /* MGE function type */
static Boolean bMgeUpdate = FALSE;      /* flag for mge training */
static int dftInvQuaSize = 100; /* default value for InvQuaSize */
static int startIter = 0;       /* start iteration index of MGE training */
static int endIter = 0;         /* end iteration index of MGE training */
static float A_STEP = 1000.0f;  /* parameter for step size = 1/(A + B*nSent) */
static float B_STEP = 1.0f;     /* parameter for step size = 1/(A + B*nSent) */

static int dftVarWinSize = -1;  /* default value for win size of variance calculation */
static float dftGVDistWght = 1.0f;      /* distance weight for gv component */
static float fGVDistWght = 1.0f;        /* distance weight for gv component */

static Boolean bBoundAdj = FALSE;       /* flag for boundary adjustment */
static int nBoundAdjWin = 5;    /* window size for boundary adjustment */
static int nBAEndIter = -1;     /* end iteration for boundary adjustment */
static int nMaxBAIter = 1;      /* maximum length for each bound adjustment (i.e. maximum times to shift the boundary */
static int nMaxBALen = 1;       /* maximum length for each boundary shifting, i.e. shift nMax frames */

static int trace = 0;           /* trace level */
static Boolean inBinary = FALSE;        /* save output in binary  */
Boolean keepOccm = FALSE;       /* keep mixture-level occ prob */
Boolean outProcData = FALSE;    /* output the process data (HMM) */

static MemHeap hmmStack;        /* for storage of HMM set */
static MemHeap orighmmStack;    /* for storage of HMM set */
static MemHeap accStack;        /* for storage of acc info */
static MemHeap genStack;        /* for storage of generation data */
static MemHeap mgeStack;        /* for storage of MGE training data */

static HMMSet hset;             /* Set of HMMs */
static HMMSet orighset;

static MTStatInfo *statInfo;    /* statistical info of MGE training */
static GenInfo *genInfo;        /* structure for generation */
static MgeTrnInfo *mtInfo;      /* structure for MGE training */

static ConfParam *cParm[MAXGLOBS];      /* configuration parameters */
static IntVec nPdfStr = NULL;   /* # of PdfStream */
static IntVec pdfStrOrder = NULL;       /* order of each PdfStream */
static char **winFn[SMAX];      /* fn of window */
static IntVec pbMTrn = NULL;    /* whether perform MGE train for the stream */
static IntVec pbGVTrn = NULL;   /* whether incorporate GV component into MGE training */
static IntVec pbAccErr = NULL;  /* accumulate generation error */
static IntVec pnInvQuaSize = NULL;      /* bandwith of quasi-diagonal inversion matrix */
static IntVec pnVarWin = NULL;  /* window for local variance calculation */
static Vector pfGVWght = NULL;  /* GV weights */

static IntVec pbGainWght = NULL;        /* gain weight for generation error */
static Boolean dftGWFlag = FALSE;       /* default flag for gain weight */

static int total_T[SMAX];
static int total_unstab_T[SMAX];
static int nSamples = 0;

static XFInfo xfInfo;

#define MAX_SENT_NUM    50000

/* Data File list struct */
typedef struct _TDataFile {
   char datafn[255];
   Boolean bValid;
} TDataFile, *PTDataFile;

static PTDataFile g_pDataFileList[MAX_SENT_NUM];
static PTDataFile g_pShufDFList[MAX_SENT_NUM];
static int g_nDataFileNum, g_nValidDfNum;

/* ------------------ Process Command Line -------------------------- */
static void Initialise();
static void PerformMgeTrain();
static void PerformMgeEval();
static void PerformMgeAdapt();

static void SetConfParms(void)
{
   int s, i, nParm;
   char *c, buf[MAXSTRLEN], tmp[MAXSTRLEN];
   Boolean b;

   for (s = 1; s < SMAX; s++)
      winFn[s] = NULL;

   nParm = GetConfig("HMGETOOL", TRUE, cParm, MAXGLOBS);
   if (nParm > 0) {
      if (GetConfInt(cParm, nParm, "TRACE", &i))
         trace = i;
      if (GetConfBool(cParm, nParm, "SAVEBINARY", &b))
         inBinary = b;

      if (GetConfStr(cParm, nParm, "PDFSTRSIZE", buf))
         nPdfStr = ParseConfIntVec(&gstack, buf, TRUE);
      if (GetConfStr(cParm, nParm, "PDFSTRORDER", buf))
         pdfStrOrder = ParseConfIntVec(&gstack, buf, TRUE);

      if (GetConfStr(cParm, nParm, "MGETRNFLAG", buf))
         pbMTrn = ParseConfIntVec(&gstack, buf, TRUE);
      if (GetConfStr(cParm, nParm, "GVTRNFLAG", buf))
         pbGVTrn = ParseConfIntVec(&gstack, buf, TRUE);
      if (GetConfStr(cParm, nParm, "ACCERRFLAG", buf))
         pbAccErr = ParseConfIntVec(&gstack, buf, TRUE);
      if (GetConfStr(cParm, nParm, "INVQUASIZE", buf))
         pnInvQuaSize = ParseConfIntVec(&gstack, buf, TRUE);
      if (GetConfStr(cParm, nParm, "VARWINSIZE", buf))
         pnVarWin = ParseConfIntVec(&gstack, buf, TRUE);
      if (GetConfStr(cParm, nParm, "GVDISTWGHT", buf))
         pfGVWght = ParseConfVector(&gstack, buf, TRUE);
      if (GetConfStr(cParm, nParm, "GAINWTFLAG", buf))
         pbGainWght = ParseConfIntVec(&gstack, buf, TRUE);

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

/* SetuFlags: Set the updating flags */
static void SetuFlags(void)
{
   char *s;

   s = GetStrArg();
   while (*s != '\0') {
      switch (*s++) {
      case 'm':
         uFlags = (UPDSet) (uFlags + UPMEANS);
         break;
      case 'v':
         uFlags = (UPDSet) (uFlags + UPVARS);
         break;
      case 'w':
         uFlags = (UPDSet) (uFlags + UPMIXES);
         break;
      case 'a':
         uFlags = (UPDSet) (uFlags + UPXFORM);
         break;
      default:
         HError(2320, "SetuFlags: Unknown update flag %c", *s);
         break;
      }
   }
   if (uFlags & UPMEANS || uFlags & UPVARS || uFlags & UPMIXES || uFlags & UPXFORM)
      bMgeUpdate = TRUE;
}

/* CheckGenSetUp: Check & setup GenInfo structure */
static void CheckGenSetUp(void)
{
   int d, p, r, s, stream;
   PdfStream *pst = NULL;

   /* # of PdfStreams */
   genInfo->nPdfStream[0] = (nPdfStr == NULL) ? hset.swidth[0] : IntVecSize(nPdfStr);
   if (genInfo->nPdfStream[0] > hset.swidth[0])
      HError(6604, "CheckGenSetUp: # of PdfStreams (%d) is too large (should be less than %d)", genInfo->nPdfStream[0], hset.swidth[0]);

   /* size of each PdfStreams */
   r = hset.swidth[0];
   for (p = stream = 1; p <= genInfo->nPdfStream[0]; stream += genInfo->nPdfStream[p++]) {
      pst = &(genInfo->pst[p]);

      /* # of streams in this PdfStream */
      genInfo->nPdfStream[p] = (nPdfStr == NULL) ? 1 : nPdfStr[p];
      r -= genInfo->nPdfStream[p];

      /* calculate vector size for this PdfStream */
      for (s = stream, pst->vSize = 0; s < stream + genInfo->nPdfStream[p]; s++)
         pst->vSize += hset.swidth[s];

      /* order (vecSize of static feature vector) of this PdfStream */
      pst->order = (pdfStrOrder == NULL) ? 1 : pdfStrOrder[p];
      if (pst->order < 1 || pst->order > pst->vSize)
         HError(6604, "CheckGenSetUp: Order of PdfStream %d should be within 1--%d", p, pst->vSize);

      /* window coefficients */
      if (winFn[p] == NULL)
         HError(6604, "CheckGenSetUp: window file names are not specified");
      pst->win.num = (int) winFn[p][0][0];
      if (pst->win.num > MAXWINNUM)
         HError(6604, "CheckGenSetUp: # of window out of range");
      if (pst->win.num * pst->order != pst->vSize)
         HError(6604, "CheckGenSetUp: # of window (%d) times order (%d) should be equal to vSize (%d)", pst->win.num, pst->order, pst->vSize);
      for (d = 0; d < pst->win.num; d++)
         MakeFN(winFn[p][d + 1], winDir, winExt, pst->win.fn[d]);
   }
   if (r != 0)
      HError(6604, "CheckGenSetUp: # of streams in HMMSet (%d) and PdfStreams (%d) are inconsistent", hset.swidth[0], genInfo->nPdfStream[0]);

   /* output trace information */
   if (trace & T_TOP) {
      for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
         printf("PdfStream [%d]:\n", p);
         printf("  #streams: %d (vSize=%d)\n", genInfo->nPdfStream[p], genInfo->pst[p].vSize);
         printf("  #order:   %d\n", genInfo->pst[p].order);
         printf("  file ext: %s\n", genInfo->pst[p].ext);
         for (d = 0; d < pst->win.num; d++)
            printf("  %d-th window: %s\n", d, pst->win.fn[d]);
      }
      printf("\n");
      fflush(stdout);
   }
}

/* CheckMTInfoSetUp: Check & setup MgeTrnInfo structure */
static void CheckMTInfoSetUp(void)
{
   int p;
   if (pbMTrn != NULL) {
      if (mtInfo->genInfo->nPdfStream[0] != IntVecSize(pbMTrn))
         HError(6604, "CheckMTInfoSetUp: MTrn Flag is incompatible with PdfStream");
      for (p = 1; p <= IntVecSize(pbMTrn); p++)
         mtInfo->pbMTrn[p] = (Boolean) pbMTrn[p];
   }

   if (pbGVTrn != NULL) {
      if (mtInfo->genInfo->nPdfStream[0] != IntVecSize(pbGVTrn))
         HError(6604, "CheckMTInfoSetUp: GVTrn Flag is incompatible with PdfStream");
      for (p = 1; p <= IntVecSize(pbGVTrn); p++)
         mtInfo->pbGVTrn[p] = (Boolean) pbGVTrn[p];
   }

   if (pbAccErr != NULL) {
      if (mtInfo->genInfo->nPdfStream[0] != IntVecSize(pbAccErr))
         HError(6604, "CheckMTInfoSetUp: AccErr Flag is incompatible with PdfStream");
      for (p = 1; p <= IntVecSize(pbAccErr); p++)
         mtInfo->pbAccErr[p] = (Boolean) pbAccErr[p];
   } else {
      for (p = 1; p <= genInfo->nPdfStream[0]; p++)
         mtInfo->pbAccErr[p] = TRUE;    /* by default to accumulate the generation error */
   }

   if (pnInvQuaSize != NULL) {
      if (mtInfo->genInfo->nPdfStream[0] != IntVecSize(pnInvQuaSize))
         HError(6604, "CheckMTInfoSetUp: InvQuaSize is incompatible with PdfStream");
      for (p = 1; p <= IntVecSize(pnInvQuaSize); p++)
         mtInfo->pnInvQuaSize[p] = pnInvQuaSize[p];
   } else {
      for (p = 1; p <= genInfo->nPdfStream[0]; p++)
         mtInfo->pnInvQuaSize[p] = dftInvQuaSize;
   }

   if (pnVarWin != NULL) {
      if (mtInfo->genInfo->nPdfStream[0] != IntVecSize(pnVarWin))
         HError(6604, "CheckMTInfoSetUp: pnVarWin is incompatible with PdfStream");
      for (p = 1; p <= IntVecSize(pnVarWin); p++)
         mtInfo->pnVarWin[p] = pnVarWin[p];
   } else {
      for (p = 1; p <= genInfo->nPdfStream[0]; p++)
         mtInfo->pnVarWin[p] = dftVarWinSize;
   }

   if (pfGVWght != NULL) {
      if (mtInfo->genInfo->nPdfStream[0] != IntVecSize(pnVarWin))
         HError(6604, "CheckMTInfoSetUp: pnVarWin is incompatible with PdfStream");
      for (p = 1; p <= VectorSize(pfGVWght); p++)
         mtInfo->pfGVWght[p] = pfGVWght[p];
   } else {
      for (p = 1; p <= genInfo->nPdfStream[0]; p++)
         mtInfo->pfGVWght[p] = dftGVDistWght;
   }

   if (pbGainWght != NULL) {
      if (mtInfo->genInfo->nPdfStream[0] != IntVecSize(pbGainWght))
         HError(6604, "CheckMTInfoSetUp: Gain weight is incompatible with PdfStream");
      for (p = 1; p <= IntVecSize(pbGainWght); p++)
         mtInfo->pbGainWght[p] = (Boolean) pbGainWght[p];
   } else {
      for (p = 1; p < SMAX; p++)
         mtInfo->pbGainWght[p] = dftGWFlag;
   }
}

/* GetTotalOrder: */
static int GetTotalOrder(GenInfo * pGenInfo)
{
   int p, order;

   order = 0;
   for (p = 1; p <= pGenInfo->nPdfStream[0]; p++) {
      order += pGenInfo->pst[p].order;
   }
   return order;
}

/* ReadOneLine: */
static Boolean ReadOneLine(FILE * fp, char *line)
{
   if (feof(fp))
      return FALSE;
   line[0] = 0;
   fgets(line, 256, fp);
   if (line[0] == '#' || line[0] == 0x0a || line[0] == 0x0d)
      return ReadOneLine(fp, line);
   return TRUE;
}

/* LoadScaleRate: Load scale rate file for parameter updating */
static void LoadScaleRate(char *scalefile)
{
   char *line;
   FILE *fp;
   int i, p;
   float fScale;

   line = (char *) malloc(256 * sizeof(char));
   fp = fopen(scalefile, "rt");

   /* read the scale */
   ReadOneLine(fp, line);
   fScale = (float) atof(line);
   for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
      for (i = 1; i <= genInfo->pst[p].order; i++) {
         if (!ReadOneLine(fp, line))
            break;
         mtInfo->SRMean[p][i] = (float) atof(line) * fScale;
      }
   }

   /* read the scale */
   ReadOneLine(fp, line);
   fScale = (float) atof(line);
   for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
      for (i = 1; i <= genInfo->pst[p].order; i++) {
         if (!ReadOneLine(fp, line))
            break;
         mtInfo->SRVar[p][i] = (float) atof(line) * fScale;
      }
   }

   /* read the scale */
   ReadOneLine(fp, line);
   fScale = (float) atof(line);
   for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
      for (i = 1; i <= genInfo->pst[p].order; i++) {
         if (!ReadOneLine(fp, line))
            break;
         mtInfo->DWght[p][i] = (float) atof(line) * fScale;
      }
   }

   free(line);
   fclose(fp);
}

/* InitMgeTrnInfo: Initialize all info struct for MGE training */
static void InitMgeTrnInfo()
{
   int i, p, nWin;

   /* setup GenInfo */
   genInfo->genMem = &genStack;
   genInfo->hset = &hset;
   genInfo->maxStates = MaxStatesInSet(&hset);
   genInfo->MSDthresh = MSDthresh;
   genInfo->frameRate = frameRate;
   genInfo->stateAlign = TRUE;
   CheckGenSetUp();

   /* setup MTStatInfo */
   statInfo->order = GetTotalOrder(mtInfo->genInfo);
   for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
      statInfo->initErrAcc[p] = CreateDVector(&gstack, genInfo->pst[p].order * 2);
      statInfo->prevErrAcc[p] = CreateDVector(&gstack, genInfo->pst[p].order * 2);
      statInfo->currErrAcc[p] = CreateDVector(&gstack, genInfo->pst[p].order * 2);
      ZeroDVector(statInfo->initErrAcc[p]);
      ZeroDVector(statInfo->prevErrAcc[p]);
      ZeroDVector(statInfo->currErrAcc[p]);
   }

   nWin = mtInfo->genInfo->pst[1].win.num;
   for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
      statInfo->pnMeanFloor[p] = (IMatrix *) New(&gstack, statInfo->order * sizeof(IMatrix *));
      statInfo->pnMeanFloor[p]--;
      statInfo->pnVarFloor[p] = (IMatrix *) New(&gstack, statInfo->order * sizeof(IMatrix *));
      statInfo->pnVarFloor[p]--;
      for (i = 1; i <= statInfo->order; i++) {
         statInfo->pnMeanFloor[p][i] = CreateIMatrix(&gstack, nWin, 2); /* 2: High & Low floor */
         ZeroIMatrix(statInfo->pnMeanFloor[p][i]);
         statInfo->pnVarFloor[p][i] = CreateIMatrix(&gstack, nWin, 2);  /* 2: High & Low floor */
         ZeroIMatrix(statInfo->pnVarFloor[p][i]);
      }
   }

   /* setup MgeTrnInfo */
   mtInfo->nBoundAdjWin = nBoundAdjWin;
   mtInfo->fGVDistWght = fGVDistWght;
   mtInfo->uFlags = uFlags;
   mtInfo->funcType = funcType;
   for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
      mtInfo->SRMean[p] = CreateVector(&gstack, genInfo->pst[p].order);
      mtInfo->SRVar[p] = CreateVector(&gstack, genInfo->pst[p].order);
      mtInfo->DWght[p] = CreateVector(&gstack, genInfo->pst[p].order);
      ZeroVector(mtInfo->SRMean[p]);
      ZeroVector(mtInfo->SRVar[p]);
      ZeroVector(mtInfo->DWght[p]);
   }

   if (scalefn != NULL) {
      LoadScaleRate(scalefn);
      mtInfo->bMScale = TRUE;
   } else
      mtInfo->bMScale = FALSE;
   CheckMTInfoSetUp();

   return;
}

/* ReportUsage: */
static void ReportUsage(void)
{
   printf("\nMinimum Generation Error (MGE) based HMM training\n\n");
   printf("USAGE: HMgeTool [options] ScriptFile\n\n");
   printf(" Option                                                    Default\n\n");
   printf(" -a i j  i: max times to shift the boundary                none\n");
   printf("         j: max length for each boundary shifting          1\n");
   printf(" -b i j  i: end iteration for boundary adjustment          none\n");
   printf("         j: window size for boundary adjustment            5\n");
   printf(" -c      output the process data                           off\n");
   printf(" -d dir  HMM definition directory                          none\n");
   printf(" -e      enable limit the updating rate for each step      off\n");
   printf(" -f r    frame rate                                        50000\n");
   printf(" -g      enable multiply variance ratio for mean updating  off\n");
   printf(" -i i j  start/end iteration index of MGE training         0 0\n");
   printf(" -j flg  0: eval 1: train 2: adapt                         1\n");
   printf(" -l dir  output label directory                            none\n");
   printf(" -o ext  HMM def file extension                            none\n");
   printf(" -p a b  parameter for step size: 1/(a + b*n)              1000.0 1.0\n");
   printf(" -r file load HMM for reference                            none\n");
   printf(" -s file updating scale file                               none\n");
   printf(" -u mvwa update t)rans m)eans v)ars w)ghts                 none\n");
   printf("                a)daptation xform\n");
   printf(" -v f    threshold for switching spaces for MSD            0.5\n");
   printf(" -w f    distance weight for gv component                  1.0\n");
   printf(" -x ext  label file extension                              lab\n");
   PrintStdOpts("ABCDGHIJKLMSTX");
   printf("\n");
   Exit(0);
}

/* main: */
int main(int argc, char *argv[])
{
   char *s;
   char fname[MAXSTRLEN];

   InitShell(argc, argv, hmgetool_version, hmgetool_vc_id);
   InitMem();
   InitMath();
   InitSigP();
   InitWave();
   InitLabel();
   InitModel();
   InitTrain();
   InitParm();
   InitUtil();
   InitFB();
   InitGen();
   InitAdapt(&xfInfo, NULL);
   InitMTrain();

   /* process argument */
   if (NumArgs() == 0)
      ReportUsage();
   CreateHeap(&hmmStack, "Model Stack", MSTAK, 1, 1.0, 80000, 4000000);
   CreateHeap(&orighmmStack, "Model Stack", MSTAK, 1, 1.0, 80000, 4000000);
   CreateHeap(&accStack, "Acc Stack", MSTAK, 1, 1.0, 80000, 400000);
   CreateHeap(&genStack, "Gen Stack", MSTAK, 1, 1.0, 80000, 400000);
   CreateHeap(&mgeStack, "MGE Train Stack", MSTAK, 1, 1.0, 80000, 400000);

   SetConfParms();
   CreateHMMSet(&hset, &hmmStack, TRUE);
   CreateHMMSet(&orighset, &orighmmStack, TRUE);

   statInfo = (MTStatInfo *) New(&gstack, sizeof(MTStatInfo));
   memset(statInfo, 0, sizeof(MTStatInfo));

   genInfo = (GenInfo *) New(&genStack, sizeof(GenInfo));
   memset(genInfo, 0, sizeof(GenInfo));
   genInfo->hset = &hset;
   genInfo->genMem = &genStack;

   mtInfo = (MgeTrnInfo *) New(&mgeStack, sizeof(MgeTrnInfo));
   memset(mtInfo, 0, sizeof(MgeTrnInfo));
   mtInfo->genInfo = genInfo;
   mtInfo->statInfo = statInfo;
   mtInfo->hset = &hset;
   mtInfo->orighset = &orighset;
   mtInfo->mgeMem = &mgeStack;

   while (NextArg() == SWITCHARG) {
      s = GetSwtArg();
      if (strlen(s) != 1)
         HError(6601, "HMgeTool: Bad switch %s; must be single letter", s);
      switch (s[0]) {
      case 'a':
         nMaxBAIter = GetChkedInt(1, 1000, s);
         nMaxBALen = GetChkedInt(1, 1000, s);
         break;
      case 'b':
         mtInfo->bBoundAdj = TRUE;
         nBAEndIter = GetChkedInt(0, 1000, s);
         nBoundAdjWin = GetChkedInt(1, 1000, s);
         break;
      case 'c':
         outProcData = TRUE;
         break;
      case 'd':
         if (NextArg() != STRINGARG)
            HError(6601, "HMgeTool: HMM definition directory expected");
         hmmDir = GetStrArg();
         break;
      case 'e':
         mtInfo->bStepLimit = TRUE;
         break;
      case 'f':
         frameRate = (HTime) GetChkedFlt(0.0, 10000000.0, s);
         break;
      case 'g':
         mtInfo->bMVar = TRUE;
         break;
      case 'i':
         startIter = GetChkedInt(0, 1000, s);
         endIter = GetChkedInt(startIter, 1000, s);
         break;
      case 'j':
         funcType = GetChkedInt(0, 2, s);
         mtInfo->funcType = funcType;
         break;
      case 'l':
         if (NextArg() != STRINGARG)
            HError(6601, "HMgeTool: Label file output directory expected");
         outLabDir = GetStrArg();
         break;
      case 'o':
         if (NextArg() != STRINGARG)
            HError(6601, "HMgeTool: HMM file extension expected");
         outExt = GetStrArg();
         break;
      case 'p':
         A_STEP = GetChkedFlt(0.0, 10000000.0, s);
         B_STEP = GetChkedFlt(0.0, 10000000.0, s);
         break;
      case 'r':
         mtInfo->bOrigHmmRef = TRUE;
         if (NextArg() != STRINGARG)
            HError(6601, "HMgeTool: HMM macro file name expected");
         s = GetStrArg();
         AddMMF(&orighset, s);
         break;
      case 's':                /* updating scale file */
         scalefn = GetStrArg();
         break;
      case 'u':
         SetuFlags();
         break;
      case 'v':
         MSDthresh = GetChkedFlt(0.0, 1.0, s);
         break;
      case 'w':
         fGVDistWght = GetChkedFlt(0.0, 1000.0, s);
         break;
      case 'x':
         if (NextArg() != STRINGARG)
            HError(6601, "HMgeTool: HMM file extension expected");
         hmmExt = GetStrArg();
         break;
      case 'B':
         inBinary = TRUE;
         break;
      case 'G':
         mtInfo->nGainStreamIndex = GetChkedInt(1, SMAX, s);
         mtInfo->nGainDimIndex = GetChkedInt(1, 1000, s);
         if (NextArg() == FLOATARG || NextArg() == INTARG)
            mtInfo->fGainWghtComp = GetChkedFlt(-10000.0, 1000000.0, s);
         break;
      case 'H':
         if (NextArg() != STRINGARG)
            HError(6601, "HMgeTool: HMM macro file name expected");
         mmfFn = GetStrArg();
         AddMMF(&hset, mmfFn);
         break;
      case 'I':
         if (NextArg() != STRINGARG)
            HError(6601, "HMgeTool: MLF file name expected");
         LoadMasterFile(GetStrArg());
         break;
      case 'J':                /* regression class and tree */
         if (NextArg() != STRINGARG)
            HError(6601, "HMgeTool: HMM regression class/tree file name expected");
         s = GetStrArg();
         AddMMF(&hset, s);
         AddMMF(&orighset, s);
         break;
      case 'K':
         if (NextArg() != STRINGARG)
            HError(6601, "HMgeTool: HMM transform file name expected");
         xformfn = GetStrArg();
         break;
      case 'L':
         if (NextArg() != STRINGARG)
            HError(6601, "HMgeTool: Label file directory expected");
         labDir = GetStrArg();
         break;
      case 'M':
         if (NextArg() != STRINGARG)
            HError(6601, "HMgeTool: Output macro file directory expected");
         outDir = GetStrArg();
         break;
      case 'T':
         trace = GetChkedInt(0, 0100000, s);
         break;
      case 'X':
         if (NextArg() != STRINGARG)
            HError(2319, "HMGenS: Label file extension expected");
         labExt = GetStrArg();
         break;
      default:
         HError(6601, "HMgeTool: Unknown switch %s", s);
      }
   }

   if (NextArg() != STRINGARG)
      HError(6601, "HMgeTool: file name of model list expected");
   hmmListFn = GetStrArg();
   Initialise();

   if (funcType == MGE_EVAL) {
      PerformMgeEval();
   } else if (funcType == MGE_TRAIN) {
      PerformMgeTrain();
      if (endIter > 0 && bMgeUpdate) {
         /* output HMM files */
         ConvDiagC(&hset, TRUE);
         SaveHMMSet(&hset, outDir, outExt, NULL, inBinary);
      }
   } else if (funcType == MGE_ADAPT) {
      PerformMgeAdapt();
      if (endIter > 0 && bMgeUpdate) {
         MakeFN(xformfn, outDir, NULL, fname);
         SaveOneXForm(&hset, hset.curXForm, fname, FALSE);
      }
   }

   ResetHeap(&hmmStack);
   ResetHeap(&orighmmStack);
   ResetHeap(&accStack);
   ResetHeap(&genStack);
   ResetHeap(&mgeStack);

   return 0;
}

/* AttachAccToModel: */
static void AttachAccToModel()
{
   AttachAccsParallel(&hset, &accStack, uFlags, 1);
   ZeroAccsParallel(&hset, uFlags, 1);
}

/* Initialise: */
static void Initialise()
{
   char macroname[MAXSTRLEN];
   AdaptXForm *xform;

   /* load HMMs and init HMMSet related global variables */
   if (trace & T_TOP)
      printf("Loading HMM for updating ... ... \n");
   if (MakeHMMSet(&hset, hmmListFn) < SUCCESS)
      HError(2321, "Initialise: MakeHMMSet failed");
   if (LoadHMMSet(&hset, hmmDir, hmmExt) < SUCCESS)
      HError(2321, "Initialise: LoadHMMSet failed");
   SetParmHMMSet(&hset);
   /* variance inversion, MGE training only support INVDIAGC case */
   if (hset.ckind == DIAGC)
      ConvDiagC(&hset, TRUE);
   /* load transform */
   if (xformfn != NULL) {
      xform = LoadOneXForm(&hset, NameOf(xformfn, macroname), xformfn);
      SetXForm(&hset, &xfInfo, xform);
   }

   if (funcType == MGE_TRAIN)
      AttachAccToModel();

   /* reload the HMM for reference in training */
   if (mtInfo->bOrigHmmRef) {
      if (trace & T_TOP)
         printf("Loading HMM for reference ... ... \n");
      if (MakeHMMSet(&orighset, hmmListFn) < SUCCESS)
         HError(2321, "Initialise: MakeHMMSet failed");
      if (LoadHMMSet(&orighset, hmmDir, hmmExt) < SUCCESS)
         HError(2321, "Initialise: LoadHMMSet failed");
      SetParmHMMSet(&orighset);
      /* variance inversion, MGE training only support INVDIAGC case */
      if (hset.ckind == DIAGC)
         ConvDiagC(&orighset, TRUE);
      /* load transform */
      if (xformfn != NULL) {
         xform = LoadOneXForm(&orighset, NameOf(xformfn, macroname), xformfn);
         SetXForm(&orighset, &xfInfo, xform);
      }
   }

   if (trace & T_TOP) {
      printf("System is ");
      switch (hset.hsKind) {
      case PLAINHS:
         printf("PLAIN\n");
         break;
      case SHAREDHS:
         printf("SHARED\n");
         break;
      case TIEDHS:
         printf("TIED\n");
         break;
      case DISCRETEHS:
         printf("DISCRETE\n");
         break;
      }

      printf("%d Logical/%d Physical Models Loaded, VecSize=%d\n", hset.numLogHMM, hset.numPhyHMM, hset.vecSize);
      if (hset.numFiles > 0)
         printf("%d MMF input files\n", hset.numFiles);
      if (mmfFn != NULL)
         printf("Output to MMF file:  %s\n", mmfFn);
      fflush(stdout);
   }

   /* initialize a MgeTrnInfo for MGE training of HMM */
   InitMgeTrnInfo();
}

/* CreateDataFileList: */
static void CreateDataFileList()
{
   char *datafn;
   g_nDataFileNum = 0;
   do {
      if (NextArg() != STRINGARG)
         HError(2319, "HMGenS: data file name expected");
      datafn = GetStrArg();

      g_pDataFileList[g_nDataFileNum] = (TDataFile *) New(&gstack, sizeof(TDataFile));
      strcpy(g_pDataFileList[g_nDataFileNum]->datafn, datafn);
      g_pDataFileList[g_nDataFileNum]->bValid = TRUE;
      g_nDataFileNum++;
   } while (NumArgs() > 0);
}

/* ExtraceDataFile: */
static TDataFile *ExtractDataFile(int nIndex)
{
   int i;
   nIndex = nIndex % g_nValidDfNum;
   for (i = 0; i < g_nDataFileNum; i++) {
      if (g_pDataFileList[i]->bValid) {
         if (nIndex == 0)
            return g_pDataFileList[i];
         nIndex--;
      }
   }
   return NULL;
}

/* UniformRandom: Random function */
static float UniformRandom()
{
   static long unsigned int x = 100;
   const long unsigned int a = 2147437301;
   const double m = 2147483648.0;
   const double c = 347813413.0;

   x = (long unsigned int) fmod(a * x + c, m);
   return ((float) x / (float) m);
}

/* ShuffleDataFileList: Shuffle data file list */
static void ShuffleDataFileList()
{
   int i, nIndex;

   g_nValidDfNum = g_nDataFileNum;
   /* shuffle data file list */
   for (i = 0; i < g_nDataFileNum; i++) {
      nIndex = (int) (UniformRandom() * g_nDataFileNum);
      g_pShufDFList[i] = ExtractDataFile(nIndex);
      g_pShufDFList[i]->bValid = FALSE;
      g_nValidDfNum--;
   }

   /* copy shuffle list to orig list */
   for (i = 0; i < g_nDataFileNum; i++) {
      g_pDataFileList[i] = g_pShufDFList[i];
      g_pDataFileList[i]->bValid = TRUE;
   }
}

/* ResetAllStateInfo: */
static void ResetAllStatInfo(int nIter)
{
   int p, i;
   for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
      total_T[p] = 0;
      total_unstab_T[p] = 0;
      statInfo->pnTotalFrm[p] = 0;
      CopyDVector(statInfo->currErrAcc[p], statInfo->prevErrAcc[p]);
      ZeroDVector(statInfo->currErrAcc[p]);
      for (i = 1; i <= statInfo->order; i++) {
         ZeroIMatrix(statInfo->pnMeanFloor[p][i]);
         ZeroIMatrix(statInfo->pnVarFloor[p][i]);
      }
   }
}

/* PrintFinalResult: */
static void PrintFinalResult(int nSent, int nIter)
{
   int p, i;
   double init_err, curr_err, ti_err, tc_err;

   if (total_T[1] <= 0)
      return;

   ti_err = 0.0;
   tc_err = 0.0;
   fprintf(stdout, "\n\n========= Final report of generation error ========\n");
   for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
      if (!mtInfo->pbMTrn[p] && !mtInfo->pbAccErr[p])
         continue;

      fprintf(stdout, "\nGen Error of PdfStream [%d]: %d / %d / %d (sentences / frames)\n", p, nSent, total_T[p], total_unstab_T[p]);
      for (i = 1; i <= genInfo->pst[p].order; i++) {
         init_err = statInfo->initErrAcc[p][i] / total_T[p];
         curr_err = statInfo->currErrAcc[p][i] / total_T[p];
         fprintf(stdout, "   Dim %d: %.6e -> %.6e (imp %.2f%%)\n", i, init_err, curr_err, 100.0 * (init_err - curr_err) / init_err);
         ti_err += statInfo->initErrAcc[p][i] * mtInfo->DWght[p][i];
         tc_err += statInfo->currErrAcc[p][i] * mtInfo->DWght[p][i];
      }

      if (mtInfo->pbGVTrn[p]) {
         /* output gen error for global variance */
         for (; i <= genInfo->pst[p].order * 2; i++) {
            init_err = statInfo->initErrAcc[p][i] / total_T[p];
            curr_err = statInfo->currErrAcc[p][i] / total_T[p];
            fprintf(stdout, "   Dim %d: %.6e -> %.6e (imp %.2f%%)\n", i, init_err, curr_err, 100.0 * (init_err - curr_err) / init_err);
         }
      }

   }
   fflush(stdout);

   ti_err /= total_T[1];
   tc_err /= total_T[1];
   fprintf(stdout, "\nTotal Gen Error: %.6e -> %.6e (imp %.2f%%)\n", ti_err, tc_err, 100.0 * (ti_err - tc_err) / ti_err);
   fflush(stdout);
}

/* PrintStateInfo: */
static void PrintStatInfo(int nSent, int nIter)
{
   int p, i, j;
   double prev_err, curr_err, tp_err, tc_err;

   if (total_T[1] <= 0)
      return;

   tp_err = tc_err = 0.0;
   for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
      if (!mtInfo->pbMTrn[p] && !mtInfo->pbAccErr[p])
         continue;

      fprintf(stdout, "\nGen Error of PdfStream [%d]: %d / %d / %d (sentences / frames)\n", p, nSent, total_T[p], total_unstab_T[p]);
      for (i = 1; i <= genInfo->pst[p].order; i++) {
         prev_err = statInfo->prevErrAcc[p][i] / total_T[p];
         curr_err = statInfo->currErrAcc[p][i] / total_T[p];
         if (nIter > 0)
            fprintf(stdout, "   Dim %d: %.6e -> %.6e (imp %.2f%%)\n", i, prev_err, curr_err, 100.0 * (prev_err - curr_err) / fabs(prev_err));
         else
            fprintf(stdout, "   Dim %d: %.6e\n", i, curr_err);
         tp_err += statInfo->prevErrAcc[p][i] * mtInfo->DWght[p][i];
         tc_err += statInfo->currErrAcc[p][i] * mtInfo->DWght[p][i];
      }

      if (mtInfo->pbGVTrn[p]) {
         /* output gen error for global variance */
         for (; i <= genInfo->pst[p].order * 2; i++) {
            prev_err = statInfo->prevErrAcc[p][i] / total_T[p];
            curr_err = statInfo->currErrAcc[p][i] / total_T[p];
            if (nIter > 0)
               fprintf(stdout, "   Dim %d: %.6e -> %.6e (imp %.2f%%)\n", i, prev_err, curr_err, 100.0 * (prev_err - curr_err) / fabs(prev_err));
            else
               fprintf(stdout, "   Dim %d: %.6e\n", i, curr_err);
         }
      }

      if (!mtInfo->pbMTrn[p])
         continue;
      if (nIter > 0 && (trace & T_PROC)) {
         fprintf(stdout, "\nUpdate Floor of PdfStream [%d]: %d / %d (sentences / frames)\n", p, nSent, total_T[p]);
         for (i = 1; i <= genInfo->pst[p].order; i++) {
            fprintf(stdout, "   Mean_D_%d = ", i);
            for (j = 1; j <= genInfo->pst[p].win.num; j++) {
               fprintf(stdout, "H:%5d L:%5d;  ", statInfo->pnMeanFloor[p][i][j][HIGH_FLOOR], statInfo->pnMeanFloor[p][i][j][LOW_FLOOR]);
            }
            fprintf(stdout, "\n");
            fprintf(stdout, "    Var_D_%d = ", i);
            for (j = 1; j <= genInfo->pst[p].win.num; j++) {
               fprintf(stdout, "H:%5d L:%5d;  ", statInfo->pnVarFloor[p][i][j][HIGH_FLOOR], statInfo->pnVarFloor[p][i][j][LOW_FLOOR]);
            }
            fprintf(stdout, "\n");
         }
      }
      fflush(stdout);
   }

   tp_err /= total_T[1];
   tc_err /= total_T[1];
   if (nIter > 0)
      fprintf(stdout, "\nTotal Generation Error: %.6e -> %.6e (imp %.2f%%)\n", tp_err, tc_err, 100.0 * (tp_err - tc_err) / fabs(tp_err));
   else
      fprintf(stdout, "\nTotal Generation Error: %.6e\n", tc_err);
   fflush(stdout);

   /* if first iteration, set up initial statistical info */
   if (nIter == startIter) {
      for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
         CopyDVector(statInfo->currErrAcc[p], statInfo->initErrAcc[p]);
      }
   }
}

/* LoadMgeTrnHmmFile: */
static void LoadMgeTrnHmmFile(int nIter)
{
   char hext[15], hmmFile[255];

   ResetHeap(&hmmStack);
   CreateHeap(&hmmStack, "Model Stack", MSTAK, 1, 1.0, 80000, 400000);

   sprintf(hext, "mmf%d", nIter);
   MakeFN(mmfFn, outDir, hext, hmmFile);

   CreateHMMSet(&hset, &hmmStack, TRUE);
   AddMMF(&hset, hmmFile);
   if (trace & T_TOP)
      printf("Loading MGE-trained HMM in iteration %d ... ... \n", nIter);
   if (MakeHMMSet(&hset, hmmListFn) < SUCCESS)
      HError(2321, "Initialise: MakeHMMSet failed");
   if (LoadHMMSet(&hset, hmmDir, hmmExt) < SUCCESS)
      HError(2321, "Initialise: LoadHMMSet failed");
   SetParmHMMSet(&hset);
   /* variance inversion, MGE training only support INVDIAGC case */
   ConvDiagC(&hset, TRUE);

   if (funcType == MGE_TRAIN)
      AttachAccToModel();
}

/* OutputIntmdHmmFile: Output intermedia HMM files */
static void OutputIntmdHmmFile(int nIter)
{
   char hext[16];

   if (trace & T_TOP) {
      fprintf(stdout, "\nSaving HMM to %s ... \n", outDir);
      fflush(stdout);
   }
   /* fix variance floor */
   ApplyVFloor(&hset);

   /* variance re-inversion for output HMM */
   ConvDiagC(&hset, TRUE);
   sprintf(hext, "mmf%d", nIter);
   SaveHMMSet(&hset, outDir, NULL, hext, TRUE);
   ConvDiagC(&hset, TRUE);
}

/* OutputIntmdXForm: Output intermedia HMM files */
static void OutputIntmdXForm(int nIter)
{
   char hext[16], fname[MAXSTRLEN];

   if (trace & T_TOP) {
      fprintf(stdout, "\nSaving XForm to %s ... \n", outDir);
      fflush(stdout);
   }
   sprintf(hext, ".%d", nIter);
   MakeFN(xformfn, outDir, NULL, fname);
   strcat(fname, hext);
   SaveOneXForm(&hset, hset.curXForm, fname, FALSE);
}

/* OneIterMgeEval: */
static int OneIterMgeEval(int nIter)
{
   char *datafn, labfn[256], basefn[255];
   int nSent, nAdjNum, p, nTotalAdj;

   if (nIter > 1)
      nMaxBALen = 1;
   /* set all stat info to zero */
   ResetAllStatInfo(nIter);
   nTotalAdj = 0;
   /* process all data */
   for (nSent = 0; nSent < g_nDataFileNum; nSent++) {
      datafn = g_pDataFileList[nSent]->datafn;
      /* after first boundary adjustment, use the adjusted label files */
      if (nIter >= 1 && bBoundAdj && outLabDir != NULL)
         MakeFN(datafn, outLabDir, labExt, labfn);
      else
         MakeFN(datafn, labDir, labExt, labfn);
      NameOf(datafn, basefn);

      if (bBoundAdj) {
         nAdjNum = OneSentBoundAdjust(mtInfo, labfn, datafn, outLabDir, nMaxBAIter, nMaxBALen, FALSE);
         if (trace & T_PROC) {
            fprintf(stdout, "Total BA Number: %d\n", nAdjNum);
            fflush(stdout);
         }
         nTotalAdj += nAdjNum;
      }
      /* accumulation of generation error only */
      OneSentGenErrAcc(mtInfo, labfn, datafn);
      for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
         total_T[p] += genInfo->pst[p].T;
      }
   }

   return nSent;
}

/* MGE evaluation main procedure */
static void PerformMgeEval()
{
   int nIter, nSent;

   /* create data file list */
   CreateDataFileList();
   nSent = g_nDataFileNum;
   nSamples = 0;

   /* synchronize parameters with previous iterations */
   if (nBAEndIter < 0)
      nBAEndIter = endIter;
   for (nIter = 1; nIter < startIter; nIter++) {
      nSamples += nSent;
      if (nMaxBAIter > 1 && nIter > 0)
         nMaxBAIter -= 1;
      if (nMaxBALen > 1 && nIter > 0)
         nMaxBALen -= 1;
   }

   /* apply transform to related models */
   if (hset.curXForm != NULL)
      ApplyHMMSetXForm(&hset, hset.curXForm, TRUE);

   /* iteration from startIter to endIter */
   for (nIter = startIter; nIter <= endIter; nIter++) {
      if (trace & T_TOP) {
         fprintf(stdout, "\n\n\n--------------------- Iteration %d -------------------\n", nIter);
         fflush(stdout);
      }
      /* load MGE-trained HMM files */
      if (nIter > 0)
         LoadMgeTrnHmmFile(nIter);
      /* set the flag for boundary adjustment */
      bBoundAdj = mtInfo->bBoundAdj;

      /* one iteration of MGE training */
      nSent = OneIterMgeEval(nIter);
      /* print the statistical information of training */
      PrintStatInfo(nSent, nIter);
      /* reduce the shift iterations and length after each iteration until it reach to 1 */
      /* heuristic code to improve the efficiency */
      if (nMaxBAIter > 1 && nIter > 0)
         nMaxBAIter -= 1;
      if (nMaxBALen > 1 && nIter > 0)
         nMaxBALen -= 1;
   }

   PrintFinalResult(nSent, nIter);
}

/* OneIterMgeTrain: */
static int OneIterMgeTrain(int nIter)
{
   char *datafn, labfn[256], basefn[255];
   int nSent, nAdjNum, p, nTotalAdj;
   float stepSize;

   /* set all stat info to zero */
   ResetAllStatInfo(nIter);
   /* zero all acc info for batch mode updating */
   ZeroAccsParallel(&hset, uFlags, 1);

   stepSize = 1 / (A_STEP + B_STEP * nSamples);
   nAdjNum = 0;
   nTotalAdj = 0;
   /* first refine the boundary for all data */
   if (bBoundAdj && (nIter != 0 && nIter != startIter - 1)) {
      for (nSent = 0; nSent < g_nDataFileNum; nSent++) {
         datafn = g_pDataFileList[nSent]->datafn;
         /* after first boundary adjustment, use the adjusted label files */
         if (nIter > 1 && bBoundAdj && outLabDir != NULL)
            MakeFN(datafn, outLabDir, labExt, labfn);
         else
            MakeFN(datafn, labDir, labExt, labfn);
         NameOf(datafn, basefn);

         if (trace & T_TOP) {
            fprintf(stdout, "Boundary refining %4d %s ... \n", nSent, basefn);
            fflush(stdout);
         }
         nAdjNum = OneSentBoundAdjust(mtInfo, labfn, datafn, outLabDir, nMaxBAIter, nMaxBALen, !bMgeUpdate);
         if (trace & T_TOP) {
            fprintf(stdout, "BA Number: %d\n", nAdjNum);
            fflush(stdout);
         }
         nTotalAdj += nAdjNum;
      }
   }
   /* process all data */
   for (nSent = 0; nSent < g_nDataFileNum; nSent++) {
      datafn = g_pDataFileList[nSent]->datafn;
      /* after first boundary adjustment, use the adjusted label files */
      if (nIter >= 1 && bBoundAdj && outLabDir != NULL)
         MakeFN(datafn, outLabDir, labExt, labfn);
      else
         MakeFN(datafn, labDir, labExt, labfn);
      NameOf(datafn, basefn);

      if (trace & T_TOP) {
         fprintf(stdout, "Prcessing %4d %s ... \n", nSent, basefn);
         fflush(stdout);
      }
      /* accumulation of generation error only */
      if (nIter == 0 || nIter == startIter - 1) {
         OneSentGenErrAcc(mtInfo, labfn, datafn);
      } else if (bMgeUpdate) {
         OneSentMgeTrain(mtInfo, labfn, datafn, stepSize);
         nSamples++;
      }

      for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
         total_T[p] += genInfo->pst[p].T;
      }
      /* update step size */
      stepSize = 1 / (A_STEP + B_STEP * nSamples);
      /* fix variance floor */
      if ((uFlags & UPVARS) && nSent % 100 == 0)
         ApplyVFloor(&hset);
   }

   if (nIter > 0 && bBoundAdj) {
      if (trace & T_TOP) {
         fprintf(stdout, "Bound Adjust Number: %d\n", nTotalAdj);
         fflush(stdout);
      }
   }

   if (nIter > 0 && bBoundAdj && (uFlags & UPMIXES))
      UpdateAllMSDWeight(mtInfo);

   return nSent;
}

/* PerformMgeTrain: MGE training main procedur */
static void PerformMgeTrain()
{
   int nIter, nSent;

   /* create data file list */
   CreateDataFileList();
   nSent = g_nDataFileNum;
   nSamples = 0;

   /* synchronize parameters with previous iterations */
   if (nBAEndIter < 0)
      nBAEndIter = endIter;
   for (nIter = 1; nIter < startIter; nIter++) {
      nSamples += nSent;
      ShuffleDataFileList();
      if (nMaxBAIter > 1 && nIter > 0)
         nMaxBAIter -= 1;
      if (nMaxBALen > 1 && nIter > 0)
         nMaxBALen -= 1;
   }
   /* initialize step size */
   mtInfo->initStepSize = 1 / A_STEP;

   /* apply transform to related models */
   if (hset.curXForm != NULL) {
      ApplyHMMSetXForm(&hset, hset.curXForm, TRUE);
      if (mtInfo->bOrigHmmRef)
         ApplyHMMSetXForm(&orighset, orighset.curXForm, TRUE);
   }

   /* iteration from startIter ~ endIter */
   for (nIter = startIter; nIter <= endIter; nIter++) {
      /* shuffle data file list */
      if (nIter != 0)
         ShuffleDataFileList();

      if (trace & T_TOP) {
         fprintf(stdout, "\n\n\n--------------------- Iteration %d -------------------\n", nIter);
         fflush(stdout);
      }
      /* set the flag for boundary adjustment */
      if (nMaxBAIter > 1 || nIter <= nBAEndIter)
         bBoundAdj = mtInfo->bBoundAdj;
      else
         bBoundAdj = FALSE;

      /* one iteration of MGE training */
      nSent = OneIterMgeTrain(nIter);

      /* print the statistical information of training */
      PrintStatInfo(nSent, nIter);

      /* output intermedia HMM files */
      if (nIter > 0 && outProcData && bMgeUpdate)
         OutputIntmdHmmFile(nIter);
      /* reduce the shift length after each iteration until it reach to 2 */
      /* heuristic code to improve the efficiency */
      if (nMaxBAIter > 1 && nIter > 0)
         nMaxBAIter -= 1;
      if (nMaxBALen > 1 && nIter > 0)
         nMaxBALen -= 1;
   }

   PrintFinalResult(nSent, nIter);
}

/* OneIterMgeAdapt: */
static int OneIterMgeAdapt(int nIter)
{
   char *datafn, labfn[256], basefn[255];
   int nSent, nAdjNum, p, nTotalAdj;
   float stepSize;

   if (nIter > 1)
      nMaxBALen = 1;
   /* set all stat info to zero */
   ResetAllStatInfo(nIter);

   stepSize = 1 / (A_STEP + B_STEP * nSamples);
   nAdjNum = 0;
   nTotalAdj = 0;
   /* process all data */
   for (nSent = 0; nSent < g_nDataFileNum; nSent++) {
      datafn = g_pDataFileList[nSent]->datafn;
      /* after first boundary adjustment, use the adjusted label files */
      if (nIter > 1 && bBoundAdj && outLabDir != NULL)
         MakeFN(datafn, outLabDir, labExt, labfn);
      else
         MakeFN(datafn, labDir, labExt, labfn);
      NameOf(datafn, basefn);

      if (trace & T_TOP) {
         fprintf(stdout, "Prcessing %4d %s ... \n", nSent, basefn);
         fflush(stdout);
      }

      /* apply transform to related models */
      if (hset.curXForm != NULL)
         OneSentTransform(mtInfo, labfn, datafn);

      /* accumulation of generation error only */
      if (nIter == 0) {
         OneSentGenErrAcc(mtInfo, labfn, datafn);
      } else {
         if (bBoundAdj) {
            nAdjNum = OneSentBoundAdjust(mtInfo, labfn, datafn, outLabDir, nMaxBAIter, nMaxBALen, !bMgeUpdate);
            if (trace & T_TOP) {
               fprintf(stdout, "BA Number: %d\n", nAdjNum);
               fflush(stdout);
            }
            nTotalAdj += nAdjNum;
         }
         if (bMgeUpdate) {
            OneSentMgeAdapt(mtInfo, labfn, datafn, stepSize);
            nSamples++;
         }
      }

      for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
         total_T[p] += genInfo->pst[p].T;
      }

      /* update step size */
      stepSize = 1 / (A_STEP + B_STEP * nSamples);
      /* fix variance floor */
      if ((uFlags & UPVARS) && nSent % 100 == 0)
         ApplyVFloor(&hset);
   }
   if (nIter > 0 && bBoundAdj) {
      if (trace & T_TOP) {
         fprintf(stdout, "Bound Adjust Number: %d\n", nTotalAdj);
         fflush(stdout);
      }
   }

   return nSent;
}

/* PerformMgeAdapt: MGE adaptation main procedure */
static void PerformMgeAdapt()
{
   int nIter, nSent;

   /* create data file list */
   CreateDataFileList();
   nSent = g_nDataFileNum;
   nSamples = 0;

   /* synchronize parameters with previous iterations */
   if (nBAEndIter < 0)
      nBAEndIter = endIter;
   for (nIter = 1; nIter < startIter; nIter++) {
      nSamples += nSent;
      ShuffleDataFileList();
      if (nMaxBAIter > 1 && nIter > 0)
         nMaxBAIter -= 1;
      if (nMaxBALen > 1 && nIter > 0)
         nMaxBALen -= 1;
   }
   /* initialize step size */
   mtInfo->initStepSize = 1 / A_STEP;

   /* iteration from startIter to endIter */
   for (nIter = startIter; nIter <= endIter; nIter++) {
      /* shuffle data file list */
      if (nIter != 0)
         ShuffleDataFileList();

      if (trace & T_TOP) {
         fprintf(stdout, "\n\n\n--------------------- Iteration %d -------------------\n", nIter);
         fflush(stdout);
      }
      /* set the flag for boundary adjustment */
      if (nMaxBAIter > 1 || nIter <= nBAEndIter)
         bBoundAdj = mtInfo->bBoundAdj;
      else
         bBoundAdj = FALSE;

      /* one iteration of MGE training */
      nSent = OneIterMgeAdapt(nIter);

      /* print the statistical information of training */
      PrintStatInfo(nSent, nIter);

      /* output intermedia HMM files */
      if (nIter > 0 && outProcData && bMgeUpdate)
         OutputIntmdXForm(nIter);
      /* reduce the shift length after each iteration until it reach to 2 */
      /* heuristic code to improve the efficiency */
      if (nMaxBAIter > 1 && nIter > 0)
         nMaxBAIter -= 1;
      if (nMaxBALen > 1 && nIter > 0)
         nMaxBALen -= 1;
   }

   PrintFinalResult(nSent, nIter);
}

/* ----------------------------------------------------------- */
/*                      END:  HMgeTool.c                       */
/* ----------------------------------------------------------- */
