/* ----------------------------------------------------------------- */
/*           The HMM-Based Speech Synthesis System (HTS)             */
/*           developed by HTS Working Group                          */
/*           http://hts.sp.nitech.ac.jp/                             */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2008-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
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

char *hsmmalign_version = "!HVER!HSMMAlign: 2.2 [NIT 07/07/11]";
char *hsmmalign_vc_id = "$Id: HSMMAlign.c,v 1.7 2011/06/16 04:27:17 uratec Exp $";

/*
  This program is used for forced alignment of HSMMs.
*/

#include "HShell.h"
#include "HMem.h"
#include "HMath.h"
#include "HAudio.h"
#include "HWave.h"
#include "HParm.h"
#include "HLabel.h"
#include "HModel.h"
#include "HTrain.h"
#include "HUtil.h"
#include "HAdapt.h"
#include "HFB.h"

/* Trace Flags */
#define T_TOP 0001              /* Top level tracing */

/* Models */
static char *hmmDir = NULL;     /* directory to look for hmm def files */
static char *durDir = NULL;     /* directory to look for duration model def files */
static char *hmmExt = NULL;     /* hmm def file extension */
static char *durExt = NULL;     /* duration model def file extension */
static HMMSet hset;             /* HMM set */
static HMMSet dset;             /* Duration model set */

/* Utterance */
static UttInfo *utt;            /* utterance information storage */

/* Transforms */
static XFInfo xfInfo_hmm;
static XFInfo xfInfo_dur;

/* Label */
static char *outLabDir = NULL;  /* directory to save label files */
static char *outLabExt = "lab"; /* output label file extension */
static char *inLabDir = NULL;   /* label (transcription) file directory */
static char *inLabExt = "lab";  /* input label file extension */

/* Global settings */
static int trace = 0;           /* Trace level */
static FileFormat dff = UNDEFF; /* data file format */
static FileFormat lff = UNDEFF; /* label file format */
static ConfParam *cParm[MAXGLOBS];      /* configuration parameters */
static int nParm = 0;           /* total num params */
Boolean keepOccm = FALSE;

/* Search settings */
static LogDouble pruneInit = NOPRUNE;   /* pruning threshold initially */
static LogDouble pruneInc = 0.0;        /* pruning threshold increment */
static LogDouble pruneLim = NOPRUNE;    /* pruning threshold limit */
static double hsmmDurWeight = 1.0;      /* duration weight of state duration model */
static Boolean stateAlign = FALSE;      /* align flag */
static Boolean pruneFrame = FALSE;      /* prune hypo by using time information of label */

/* Statistics */
static Boolean stats = FALSE;   /* enable statistics reports */
static char *statFN = NULL;     /* stats file, if any */

/* Stack */
static MemHeap hmmStack;
static MemHeap durStack;
static MemHeap uttStack;
static MemHeap tmpStack;

/* Hypo */
typedef struct _HSMMAlignHypo {
   int frame_index;             /* index of frame (1 ... T) */
   double prob;                 /* total probability (feature + duration) */
   int duration;                /* state duration */
   struct _HSMMAlignHypo *up;
   struct _HSMMAlignHypo *down;
   struct _HSMMAlignHypo *right;
   struct _HSMMAlignHypo *left;
   struct _HSMMAlignHypo *next;
   struct _HSMMAlignHypo *prev;
   struct _HSMMAlignState *state;
} HSMMAlignHypo;

/* HSMMAlignHypoInitialize: initialize hypo */
void HSMMAlignHypoInitialize(HSMMAlignHypo * h)
{
   h->frame_index = -1;
   h->prob = LZERO;
   h->up = NULL;
   h->down = NULL;
   h->right = NULL;
   h->left = NULL;
   h->next = NULL;
   h->prev = NULL;
   h->state = NULL;
}

/* Hypo sequence */
typedef struct _HSMMAlignHypoSequence {
   HSMMAlignHypo *head;
   HSMMAlignHypo *tail;
} HSMMAlignHypoSequence;

/* HSMMAlignHypoSequenceInitialize: initialize hypo sequence */
void HSMMAlignHypoSequenceInitialize(HSMMAlignHypoSequence * hs)
{
   hs->head = NULL;
   hs->tail = NULL;
}

/* HSMMAlignHypoSequenceInitialize: create and add hypo */
void HSMMAlignHypoSequencePush(HSMMAlignHypoSequence * hs)
{
   HSMMAlignHypo *tmp;

   tmp = New(&tmpStack, sizeof(HSMMAlignHypo));
   HSMMAlignHypoInitialize(tmp);

   if (hs->head == NULL && hs->tail == NULL) {
      hs->head = hs->tail = tmp;
   } else {
      hs->tail->next = tmp;
      tmp->prev = hs->tail;
      hs->tail = tmp;
   }
}

/* HSMMAlignHypoSequenceClear: free hypo sequence */
void HSMMAlignHypoSequenceClear(HSMMAlignHypoSequence * hs)
{
   HSMMAlignHypo *tmp1;
   HSMMAlignHypo *tmp2;

   if (hs->head != NULL && hs->tail != NULL) {
      tmp1 = hs->tail;
      while (tmp1 != NULL) {
         tmp2 = tmp1->prev;
         Dispose(&tmpStack, tmp1);
         tmp1 = tmp2;
      }
   }
   HSMMAlignHypoSequenceInitialize(hs);
}

/* State */
typedef struct _HSMMAlignState {
   char *name;                  /* name of model */
   StateInfo *info;             /* state information */
   int model_index;             /* index of model (0 ... M) */
   int state_index;             /* index of state (2 ...) */
   MixPDF *dur_pdf;             /* PDF of state duration */
   long start;                  /* start frame for prune */
   long end;                    /* end frame for prune */
   struct _HSMMAlignState *next;
   struct _HSMMAlignState *prev;
   struct _HSMMAlignHypo *hypo_from_left;
} HSMMAlignState;

/* HSMMAlignStateInitialize: initialize state */
void HSMMAlignStateInitialize(HSMMAlignState * s, char *name, StateInfo * info, int model_index, int state_index, MixPDF * dur_pdf, long start, long end)
{
   if (s == NULL || name == NULL || info == NULL || model_index < 0 || state_index < 2)
      HError(2300, "HSMMAlign: faild");
   s->name = name;
   s->info = info;
   s->model_index = model_index;
   s->state_index = state_index;
   s->dur_pdf = dur_pdf;
   s->start = start;
   s->end = end;
   s->next = NULL;
   s->prev = NULL;
   s->hypo_from_left = NULL;
}

/* State sequence */
typedef struct _HSMMAlignStateSequecne {
   HSMMAlignState *head;
   HSMMAlignState *tail;
} HSMMAlignStateSequence;

/* HSMMAlignStateSequenceInitialize: initialize state sequence */
void HSMMAlignStateSequenceInitialize(HSMMAlignStateSequence * ss)
{
   ss->head = NULL;
   ss->tail = NULL;
}

/* HSMMAlignStateSequencePush: create and add state */
void HSMMAlignStateSequencePush(HSMMAlignStateSequence * ss, char *name, StateInfo * info, int model_index, int state_index, MixPDF * dur_pdf, long start, long end)
{
   HSMMAlignState *new_state;

   new_state = New(&tmpStack, sizeof(HSMMAlignState));
   HSMMAlignStateInitialize(new_state, name, info, model_index, state_index, dur_pdf, start, end);

   if (ss->head == NULL && ss->tail == NULL) {
      ss->head = ss->tail = new_state;
   } else {
      ss->tail->next = new_state;
      new_state->prev = ss->tail;
      ss->tail = new_state;
   }
}

/* HSMMAlignStateSequenceClear: free state sequence */
void HSMMAlignStateSequenceClear(HSMMAlignStateSequence * ss)
{
   HSMMAlignState *tmp1;
   HSMMAlignState *tmp2;

   if (ss->head != NULL && ss->tail != NULL) {
      tmp1 = ss->tail;
      while (tmp1 != NULL) {
         tmp2 = tmp1->prev;
         Dispose(&tmpStack, tmp1);
         tmp1 = tmp2;
      }
   }
   HSMMAlignStateSequenceInitialize(ss);
}

/* QuickSort: subfunction for sorting DVector */
void QuickSort(DVector v, int s, int e)
{
   int i = s;
   int j = e;
   double tmp;
   double x = v[(int) ((s + e) / 2)];

   while (1) {
      while (v[i] < x)
         i++;
      while (v[j] > x)
         j--;
      if (i >= j)
         break;
      tmp = v[i];
      v[i] = v[j];
      v[j] = tmp;
      i++;
      j--;
   }
   if (s + 1 < i)
      QuickSort(v, s, i - 1);
   if (j + 1 < e)
      QuickSort(v, j + 1, e);
}

/* SortDVector: sort DVector */
void SortDVector(DVector v)
{
   QuickSort(v, 1, DVectorSize(v));
}

/* ------------------ Process Command Line ------------------------- */

/* SetConfParms: set conf parms relevant to HSMMAlign */
void SetConfParms(void)
{
   int i;
   char buf[MAXSTRLEN];

   nParm = GetConfig("HSMMALIGN", TRUE, cParm, MAXGLOBS);
   if (nParm > 0) {
      if (GetConfInt(cParm, nParm, "TRACE", &i))
         trace = i;
      if (GetConfStr(cParm, nParm, "INXFORMMASK", buf))
         xfInfo_hmm.inSpkrPat = CopyString(&tmpStack, buf);
      if (GetConfStr(cParm, nParm, "PAXFORMMASK", buf))
         xfInfo_hmm.paSpkrPat = CopyString(&tmpStack, buf);
      if (GetConfStr(cParm, nParm, "DURINXFORMMASK", buf))
         xfInfo_dur.inSpkrPat = CopyString(&durStack, buf);
      if (GetConfStr(cParm, nParm, "DURPAXFORMMASK", buf))
         xfInfo_dur.paSpkrPat = CopyString(&durStack, buf);
   }
}

void ReportUsage(void)
{
   printf("\nUSAGE: HSMMAlign [options] hmmList durList dataFiles...\n\n");
   printf(" Option                                                    Default\n\n");
   printf(" -a      Use an input linear transform for HMMs            off\n");
   printf(" -b      Use an input linear transform for dur models      off\n");
   printf(" -c      Prune by time information of label                off\n");
   printf(" -d s    Dir to find hmm definitions                       current\n");
   printf(" -f      Output full state alignment                       off\n");
   printf(" -h s [s] Set speaker name pattern to s,                   *.%%%%%%\n");
   printf("         optionally set parent patterns                    \n");
   printf(" -n s    Dir to find duration model definitions            current\n");
   printf(" -m dir  Set output label dir                              current\n");
   printf(" -r ext  Output label file extension                       lab\n");
   printf(" -s s    print statistics to file s                        off\n");
   printf(" -t i    Set pruning threshold                             off\n");
   printf(" -w f    Duration weight                                   1.0\n");
   printf(" -x s    Extension for hmm files                           none\n");
   printf(" -y s    Extension for duration model files                none\n");
   PrintStdOpts("AECDFGHIJLNSTWXY");
   printf("\n\n");
   Exit(0);
}

int main(int argc, char *argv[])
{
   char *datafn = NULL;
   char *s;
   int *maxMixInS;
   int i;
   LogDouble beam;

   void Initialise();
   Boolean HSMMAlign(UttInfo * utt, char *datafn, char *outLabDir, int beam, int *maxMixInS);
   void StatReport(HMMSet * hset);

   if (InitShell(argc, argv, hsmmalign_version, hsmmalign_vc_id) < SUCCESS)
      HError(2300, "HSMMAlign: InitShell failed");
   InitMem();
   InitMath();
   InitWave();
   InitLabel();
   InitModel();
   if (InitParm() < SUCCESS)
      HError(2300, "HSMMAlign: InitParm failed");
   InitUtil();
   InitFB();
   InitAdapt(&xfInfo_hmm, &xfInfo_dur);

   if (NumArgs() == 0)
      ReportUsage();

   CreateHeap(&hmmStack, "HmmStore", MSTAK, 1, 1.0, 50000, 500000);
   CreateHeap(&durStack, "DurStore", MSTAK, 1, 1.0, 50000, 500000);
   CreateHeap(&uttStack, "uttStore", MSTAK, 1, 0.5, 100, 1000);
   CreateHeap(&tmpStack, "tmpStore", MSTAK, 1, 1.0, 50000, 500000);
   SetConfParms();
   CreateHMMSet(&hset, &hmmStack, TRUE);
   CreateHMMSet(&dset, &durStack, TRUE);

   utt = (UttInfo *) New(&uttStack, sizeof(UttInfo));

   while (NextArg() == SWITCHARG) {
      s = GetSwtArg();
      if (strlen(s) != 1)
         HError(2319, "HSMMAlign: Bad switch %s; must be single letter", s);
      switch (s[0]) {
      case 'a':
         xfInfo_hmm.useInXForm = TRUE;
         break;
      case 'b':
         xfInfo_dur.useInXForm = TRUE;
         break;
      case 'c':
         pruneFrame = TRUE;
         break;
      case 'd':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: HMM definition directory expected");
         hmmDir = GetStrArg();
         break;
      case 'f':
         stateAlign = TRUE;
         break;
      case 'h':
         if (NextArg() != STRINGARG)
            HError(1, "HSMMAlign: Speaker name pattern expected");
         xfInfo_hmm.inSpkrPat = xfInfo_dur.inSpkrPat = GetStrArg();
         if (NextArg() == STRINGARG)
            xfInfo_hmm.paSpkrPat = xfInfo_dur.paSpkrPat = GetStrArg();
         if (NextArg() != SWITCHARG)
            HError(2319, "HSMMAlign: Cannot have -h as the last option");
         break;
      case 'm':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: Output label file directory expected");
         outLabDir = GetStrArg();
         break;
      case 'n':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: Duration model definition directory expected");
         durDir = GetStrArg();
         break;
      case 'r':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: Output label file extension expected");
         outLabExt = GetStrArg();
         break;
      case 's':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: Stats file name expected");
         stats = TRUE;
         statFN = GetStrArg();
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
      case 'w':
         hsmmDurWeight = GetChkedFlt(0.0, 100000.0, s);
         break;
      case 'x':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: HMM file extension expected");
         hmmExt = GetStrArg();
         break;
      case 'y':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: Duration model file extension expected");
         durExt = GetStrArg();
         break;
      case 'E':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: Parent transform directory expected");
         xfInfo_hmm.usePaXForm = TRUE;
         xfInfo_hmm.paXFormDir = GetStrArg();
         if (NextArg() == STRINGARG)
            xfInfo_hmm.paXFormExt = GetStrArg();
         if (NextArg() != SWITCHARG)
            HError(2319, "HSMMAlign: Cannot have -E as the last option");
         break;
      case 'F':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: Data File format expected");
         if ((dff = Str2Format(GetStrArg())) == ALIEN)
            HError(-2389, "HSMMAlign: Warning ALIEN Data file format set");
         break;
      case 'G':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: Label File format expected");
         if ((lff = Str2Format(GetStrArg())) == ALIEN)
            HError(-2389, "HSMMAlign: Warning ALIEN Label file format set");
         break;
      case 'H':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: HMM MMF file name expected");
         AddMMF(&hset, GetStrArg());
         break;
      case 'I':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: MLF file name expected");
         LoadMasterFile(GetStrArg());
         break;
      case 'J':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: Input transform directory expected");
         AddInXFormDir(&hset, GetStrArg());
         if (NextArg() == STRINGARG)
            xfInfo_hmm.inXFormExt = GetStrArg();
         if (NextArg() != SWITCHARG)
            HError(2319, "HSMMAlign: Cannot have -J as the last option");
         break;
      case 'L':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: Label file directory expected");
         inLabDir = GetStrArg();
         break;
      case 'N':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: Duration MMF file name expected");
         AddMMF(&dset, GetStrArg());
         break;
      case 'T':
         if (NextArg() != INTARG)
            HError(2119, "HSMMAlign: Trace value expected");
         trace = GetChkedInt(0, 0002, s);
         break;
      case 'W':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: Parent duration transform directory expected");
         xfInfo_dur.usePaXForm = TRUE;
         xfInfo_dur.paXFormDir = GetStrArg();
         if (NextArg() == STRINGARG)
            xfInfo_dur.paXFormExt = GetStrArg();
         if (NextArg() != SWITCHARG)
            HError(2319, "HSMMAlign: Cannot have -W as the last option");
         break;
      case 'X':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: Input label file extension expected");
         inLabExt = GetStrArg();
         break;
      case 'Y':
         if (NextArg() != STRINGARG)
            HError(2319, "HSMMAlign: Input duration transform directory expected");
         AddInXFormDir(&dset, GetStrArg());
         if (NextArg() == STRINGARG) {
            if (xfInfo_dur.inXFormExt == NULL)
               xfInfo_dur.inXFormExt = GetStrArg();
            else
               HError(2319, "HSMMAlign: Only one input duration transform extension may be specified");
         }
         if (NextArg() != SWITCHARG)
            HError(2319, "HSMMAlign: Cannot have -Y as the last option");
         break;
      default:
         HError(2319, "HSMMAlign: Unknown switch %s", s);
      }
   }
   if (NextArg() != STRINGARG)
      HError(2319, "HSMMAlign: File name of vocabulary list expected");

   Initialise();
   InitUttInfo(utt, FALSE);

   /* generate parameter sequences */
   maxMixInS = CreateIntVec(&tmpStack, hset.swidth[0]);
   for (i = 1; i <= hset.swidth[0]; i++)
      maxMixInS[i] = MaxMixInSetS(&hset, i);
   do {
      if (NextArg() != STRINGARG)
         HError(2319, "HSMMAlign: Data file name expected");
      datafn = GetStrArg();

      if (UpdateSpkrStats(&hset, &xfInfo_hmm, datafn)) {
         if (!xfInfo_hmm.useInXForm)
            xfInfo_hmm.inXForm = NULL;
      }
      if (UpdateSpkrStats(&dset, &xfInfo_dur, datafn)) {
         if (!xfInfo_dur.useInXForm)
            xfInfo_dur.inXForm = NULL;
         else
            ResetDMMPreComps(&dset);
      }

      for (beam = pruneInit; beam <= pruneLim; beam += pruneInc) {
         if (HSMMAlign(utt, datafn, outLabDir, (int) beam, maxMixInS) == TRUE)
            break;
         if (pruneInit == NOPRUNE || pruneInc <= 0.0)
            break;
      }
   } while (NumArgs() > 0);

   if (stats)
      StatReport(&hset);

   ResetHeap(&tmpStack);
   Dispose(&uttStack, utt);
   ResetHeap(&uttStack);
   ResetHeap(&durStack);
   ResetHeap(&hmmStack);

   /* Reset modules */
   ResetAdapt(&xfInfo_hmm, &xfInfo_dur);
   ResetFB();
   ResetUtil();
   ResetParm();
   ResetModel();
   ResetLabel();
   ResetWave();
   ResetMath();
   ResetMem();
   ResetShell();

   Exit(0);
   return (0);
}

/* -------------------------- Initialization ----------------------- */

void Initialise(void)
{
   /* load HMM mmf */
   if (MakeHMMSet(&hset, GetStrArg()) < SUCCESS)
      HError(9928, "HSMMAlign: MakeHMMSet failed");
   if (LoadHMMSet(&hset, hmmDir, hmmExt) < SUCCESS)
      HError(9928, "HSMMAlign: LoadHMMSet failed");
   if (hset.hsKind == DISCRETEHS)
      HError(9999, "HSMMAlign: Only continuous model is surpported");
   ConvDiagC(&hset, TRUE);

   /* load duration mmf */
   if (MakeHMMSet(&dset, GetStrArg()) < SUCCESS)
      HError(9928, "HSMMAlign: MakeHMMSet failed");
   if (LoadHMMSet(&dset, durDir, durExt) < SUCCESS)
      HError(9928, "HSMMAlign: LoadHMMSet failed");
   if (hset.hsKind == DISCRETEHS)
      HError(9999, "HSMMAlign: Only continuous duration model mmf is surpported");
   ConvDiagC(&dset, TRUE);

   /* setup EM-based parameter generation */
   AttachAccs(&hset, &gstack, (UPDSet) 0);
   ZeroAccs(&hset, (UPDSet) 0);

   AttachAccs(&dset, &gstack, (UPDSet) 0);
   ZeroAccs(&dset, (UPDSet) 0);

   /* handle input xform */
   xfInfo_hmm.inFullC = xfInfo_dur.inFullC = TRUE;
}

/* ---------------------------- Viterbi ---------------------------- */

Boolean HSMMAlign(UttInfo * utt, char *datafn, char *outLabDir, int beam, int *maxMixInS)
{
   int i, j, l, t;
   char ilabfn[MAXFNAMELEN];
   char olabfn[MAXFNAMELEN];
   char basefn[MAXFNAMELEN];
   char namefn[MAXFNAMELEN];
   FILE *fp;
   char *name;
   LLink llink;
   MLink mlink_hset;
   HLink hlink_hset;
   MLink mlink_dset;
   HLink hlink_dset;
   MixPDF *pdf;
   LogFloat p = LZERO;
   LogFloat mixp, wt, x;
   Boolean isPipe;

   /* alignment */
   HSMMAlignStateSequence sseq;
   HSMMAlignState *prev_s = NULL;
   HSMMAlignState *next_s = NULL;
   HSMMAlignHypoSequence hseq;
   HSMMAlignHypo *prev_h = NULL;
   HSMMAlignHypo *next_h = NULL;
   HSMMAlignHypo *initial_hypo = NULL;
   HSMMAlignHypo *final_hypo = NULL;

   /* pruning */
   int q;
   long *st = NULL, *en = NULL;
   short *mindur = NULL;

   DVector problist;
   int size;
   double threshold = 0.0;
   Boolean threshold_flag = FALSE;

   /* calculation */
   StateInfo *si;
   StreamElem *ste;
   int s, S;
   StreamInfo *sti;
   Vector v;
   int m, M;
   MixtureElem *me;
   MixPDF *mp;
   LogFloat det;
   Vector dur;

   /* result */
   HSMMAlignHypo *result_hypo = NULL;
   HSMMAlignHypo *tmp1_hypo = NULL;
   HSMMAlignHypo *tmp2_hypo = NULL;
   IntVec framelist;
   int total_nstate;
   HSMMAlignState *tmp_state = NULL;
   unsigned long tmp_ulong = 0;
   Boolean ret = TRUE;

   /* stats */
   float tmp_f;
   long tmp_l;

   /* load utterance */
   utt->twoDataFiles = FALSE;
   utt->S = hset.swidth[0];
   strcpy(ilabfn, datafn);
   LoadLabs(utt, lff, ilabfn, inLabDir, inLabExt);
   LoadData(&hset, utt, dff, datafn, NULL);
   InitUttObservations(utt, &hset, datafn, maxMixInS);
   BaseOf(datafn, basefn);

   if (trace & T_TOP) {
      printf(" Processing Data: %s ;", NameOf(datafn, namefn));
      printf(" Label %s.%s\n", basefn, inLabExt);
      fflush(stdout);
   }

   /* create state sequence */
   HSMMAlignStateSequenceInitialize(&sseq);
   total_nstate = 0;
   for (llink = utt->tr->head->head, l = 0; llink != NULL; llink = llink->succ) {
      if (llink->labid != NULL) {
         name = llink->labid->name;
         mlink_hset = FindMacroName(&hset, 'l', llink->labid);
         hlink_hset = (HLink) mlink_hset->structure;
         mlink_dset = FindMacroName(&dset, 'l', llink->labid);
         hlink_dset = (HLink) mlink_dset->structure;
         for (i = 2; i < hlink_hset->numStates; i++) {
            pdf = hlink_dset->svec[2].info->pdf[i - 1].info->spdf.cpdf[1].mpdf;
            HSMMAlignStateSequencePush(&sseq, name, hlink_hset->svec[i].info, l, i, pdf, 1, utt->T);
            total_nstate++;
         }
         l++;
      }
   }

   /* set alignment */
   if (pruneFrame == TRUE) {
      st = (long *) New(&tmpStack, utt->Q * sizeof(long));
      en = (long *) New(&tmpStack, utt->Q * sizeof(long));
      mindur = (short *) New(&tmpStack, utt->Q * sizeof(short));
      st--;
      en--;
      mindur--;
      for (llink = utt->tr->head->head, q = 1; llink != NULL; llink = llink->succ) {
         if (llink->labid != NULL) {
            st[q] = (long) llink->start * (1.0 / utt->tgtSampRate) + 1;
            en[q] = (long) llink->end * (1.0 / utt->tgtSampRate);
            if (en[q] > utt->T)
               en[q] = utt->T;
            mindur[q] = (short) total_nstate / utt->Q;
            q++;
         }
      }
      /* get time */
      SetAlign(st, en, mindur, utt->Q, utt->T);
      /* set time */
      for (llink = utt->tr->head->head, tmp_state = sseq.head, q = 1; llink != NULL; llink = llink->succ) {
         if (llink->labid != NULL) {
            for (i = 0; i < total_nstate / utt->Q; i++) {
               tmp_state->start = st[q];
               tmp_state->end = en[q];
               tmp_state = tmp_state->next;
            }
            q++;
         }
      }
      mindur++;
      en++;
      st++;
      Dispose(&tmpStack, mindur);
      Dispose(&tmpStack, en);
      Dispose(&tmpStack, st);
   }

   /* viterbi */
   dur = CreateVector(&tmpStack, 1);
   HSMMAlignHypoSequenceInitialize(&hseq);
   for (t = 1; t <= utt->T; t++) {
      if (t == 1) {
         /* create */
         HSMMAlignHypoSequencePush(&hseq);
         next_h = hseq.tail;
         next_s = sseq.head;
         /* set */
         next_h->frame_index = t;
         next_h->state = next_s;
         si = next_s->info;
         ste = si->pdf + 1;
         S = hset.swidth[0];
         p = 0;
         for (s = 1; s <= S; s++, ste++) {
            sti = ste->info;
            v = utt->o[t].fv[s];
            M = sti->nMix;
            me = sti->spdf.cpdf + 1;
            x = LZERO;
            for (m = 1; m <= M; m++, me++) {
               mp = me->mpdf;
               mixp = MOutP(ApplyCompFXForm(mp, v, xfInfo_hmm.inXForm, &det, t), mp);
               mixp += det;
               wt = MixLogWeight(&hset, me->weight);
               x = LAdd(x, wt + mixp);
            }
            if (si->weights)
               p += si->weights[s] * x;
            else
               p += x;
         }
         next_h->prob = p;
         next_h->duration = 1;
         next_s->hypo_from_left = next_h;
         initial_hypo = next_h;
         final_hypo = next_h;
      } else {
         /* transition */
         prev_h = initial_hypo;
         initial_hypo = NULL;
         while (1) {
            if (threshold_flag == FALSE || prev_h->prob >= threshold) {
               /* go to next state */
               prev_s = prev_h->state;
               next_s = prev_s->next;
               if (next_s != NULL) {
                  dur[1] = (float) prev_h->duration;
                  p = MOutP(ApplyCompFXForm(prev_s->dur_pdf, dur, xfInfo_dur.inXForm, &det, prev_h->duration), prev_s->dur_pdf);
                  p += det;
                  p = prev_h->prob + hsmmDurWeight * p;
                  if (next_s->hypo_from_left != NULL && next_s->hypo_from_left->frame_index == t) {
                     if (next_s->hypo_from_left->prob < p) {
                        /* replace */
                        next_h = next_s->hypo_from_left;
                        next_h->prob = p;
                        next_h->duration = 1;
                        next_h->left = prev_h;
                        prev_h->right = next_h;
                     }
                  } else if (next_s->start <= t && t <= next_s->end) {
                     /* create */
                     HSMMAlignHypoSequencePush(&hseq);
                     next_h = hseq.tail;
                     next_h->frame_index = t;
                     next_h->state = next_s;
                     next_h->prob = p;
                     next_h->duration = 1;
                     next_h->left = prev_h;
                     prev_h->right = next_h;
                     next_s->hypo_from_left = next_h;
                     if (initial_hypo == NULL)
                        initial_hypo = next_h;
                  }
               }
               if (prev_s->start <= t && t <= prev_s->end) {
                  /* stay current state */
                  HSMMAlignHypoSequencePush(&hseq);
                  next_h = hseq.tail;
                  next_h->frame_index = t;
                  next_h->state = prev_s;
                  next_h->prob = prev_h->prob;
                  next_h->duration = prev_h->duration + 1;
                  next_h->up = prev_h;
                  prev_h->down = next_h;
                  if (initial_hypo == NULL)
                     initial_hypo = next_h;
               }
            }
            /* next hypo */
            if (prev_h == final_hypo)
               break;
            prev_h = prev_h->next;
         }
         final_hypo = hseq.tail;
         /* calc */
         next_h = initial_hypo;
         size = 0;
         if (t == utt->T) {
            p = LZERO;
            result_hypo = NULL;
         }
         while (1) {
            next_s = next_h->state;
            si = next_s->info;
            ste = si->pdf + 1;
            S = hset.swidth[0];
            p = 0;
            for (s = 1; s <= S; s++, ste++) {
               sti = ste->info;
               v = utt->o[t].fv[s];
               M = sti->nMix;
               me = sti->spdf.cpdf + 1;
               x = LZERO;
               for (m = 1; m <= M; m++, me++) {
                  mp = me->mpdf;
                  mixp = MOutP(ApplyCompFXForm(mp, v, xfInfo_hmm.inXForm, &det, t), mp);
                  mixp += det;
                  wt = MixLogWeight(&hset, me->weight);
                  x = LAdd(x, wt + mixp);
               }
               if (si->weights)
                  p += si->weights[s] * x;
               else
                  p += x;
            }
            next_h->prob += p;
            size++;
            if (t == utt->T) {
               if (next_s->next == NULL && (p < next_h->prob || result_hypo == NULL)) {
                  p = next_h->prob;
                  result_hypo = next_h;
               }
            }
            /* next hypo */
            if (next_h == final_hypo)
               break;
            next_h = next_h->next;
         }
         /* sort */
         if (t != utt->T) {
            if (beam > 0 && size > beam) {
               problist = CreateDVector(&tmpStack, size);
               next_h = initial_hypo;
               for (i = 1;; i++) {
                  problist[i] = next_h->prob;
                  if (next_h == final_hypo)
                     break;
                  next_h = next_h->next;
               }
               SortDVector(problist);
               threshold = problist[size - beam + 1];
               FreeDVector(&tmpStack, problist);
               threshold_flag = TRUE;
            } else {
               threshold_flag = FALSE;
            }
         }
      }
   }

   /* save label file */
   if (result_hypo == NULL) {
      HError(-9999, "HSMMAlign: No tokens survived to final node of network at beam %d\n", beam);
      ret = FALSE;
   } else {
      if (outLabDir != NULL)
         sprintf(olabfn, "%s%c%s.%s", outLabDir, PATHCHAR, basefn, outLabExt);
      else
         sprintf(olabfn, "%s.%s", basefn, outLabExt);
      if ((fp = FOpen(olabfn, NoOFilter, &isPipe)) == NULL)
         HError(2611, "HSMMAlign: Cannot open label file %s", olabfn);

      if (trace & T_TOP) {
         printf(" Utterance prob per frame = %e\n", result_hypo->prob / utt->T);
         fflush(stdout);
      }

      /* get state duration */
      framelist = CreateIntVec(&tmpStack, total_nstate);
      tmp1_hypo = result_hypo;
      i = tmp1_hypo->frame_index;
      j = total_nstate;
      while (tmp1_hypo != NULL) {
         if (tmp1_hypo->up != NULL)
            tmp2_hypo = tmp1_hypo->up;
         else
            tmp2_hypo = tmp1_hypo->left;
         if (tmp2_hypo == NULL || tmp1_hypo->state != tmp2_hypo->state) {
            framelist[j--] = i - tmp1_hypo->frame_index + 1;
            if (tmp2_hypo != NULL)
               i = tmp2_hypo->frame_index;
         }
         tmp1_hypo = tmp2_hypo;
      }

      /* output */
      if (stateAlign == TRUE) {
         tmp_ulong = 0;
         for (tmp_state = sseq.head, i = 1; tmp_state != NULL; tmp_state = tmp_state->next, i++) {
            if (tmp_state->state_index == 2)
               fprintf(fp, "%lu %lu %s[%d] %s\n", tmp_ulong, tmp_ulong + (unsigned long) (framelist[i] * utt->tgtSampRate), tmp_state->name, tmp_state->state_index, tmp_state->name);
            else
               fprintf(fp, "%lu %lu %s[%d]\n", tmp_ulong, tmp_ulong + (unsigned long) (framelist[i] * utt->tgtSampRate), tmp_state->name, tmp_state->state_index);
            tmp_ulong += (unsigned long) (framelist[i] * utt->tgtSampRate);
         }
      } else {
         j = 0;
         tmp_ulong = 0;
         for (tmp_state = sseq.head, i = 1; tmp_state != NULL; tmp_state = tmp_state->next, i++) {
            j += framelist[i];
            if (tmp_state->next == NULL || tmp_state->next->state_index <= tmp_state->state_index) {
               fprintf(fp, "%lu %lu %s\n", tmp_ulong, tmp_ulong + (unsigned long) (j * utt->tgtSampRate), tmp_state->name);
               tmp_ulong += (unsigned long) (j * utt->tgtSampRate);
               j = 0;
            }
         }
      }

      /* store stats */
      if (stats) {
         j = 0;
         for (llink = utt->tr->head->head; llink != NULL; llink = llink->succ) {
            if (llink->labid != NULL) {
               name = llink->labid->name;
               mlink_hset = FindMacroName(&hset, 'l', llink->labid);
               hlink_hset = (HLink) mlink_hset->structure;
               tmp_l = (long) hlink_hset->hook;
               hlink_hset->hook = (void *) (tmp_l + 1);
               for (i = 2; i < hlink_hset->numStates; i++) {
                  si = hlink_hset->svec[i].info;
                  if (hset.numSharedStreams > 0) {
                     memcpy(&tmp_f, &(si->hook), sizeof(float));
                     tmp_f += (float) framelist[j];
                     memcpy(&(si->hook), &tmp_f, sizeof(float));
                  } else {
                     tmp_f = ((WtAcc *) ((si->pdf + 1)->info->hook))->occ;
                     ((WtAcc *) ((si->pdf + 1)->info->hook))->occ = tmp_f + (float) framelist[j];
                  }
                  j++;
               }
            }
         }
      }

      FreeIntVec(&tmpStack, framelist);
      FClose(fp, isPipe);
   }

   /* free hypo sequence */
   HSMMAlignHypoSequenceClear(&hseq);

   FreeVector(&tmpStack, dur);

   /* free state sequence */
   HSMMAlignStateSequenceClear(&sseq);

   /* reset utterance */
   ResetUttObservations(utt, &hset);

   return ret;
}

/* PrintStats: for given hmm */
void PrintStats(HMMSet * hset, FILE * f, int n, HLink hmm, long numEgs)
{
   char buf[MAXSTRLEN];
   StateInfo *si;
   int i, N;
   float occ;

   N = hmm->numStates;
   ReWriteString(HMMPhysName(hset, hmm), buf, DBL_QUOTE);
   fprintf(f, "%4d %14s %4ld ", n, buf, numEgs);
   for (i = 2; i < N; i++) {
      si = hmm->svec[i].info;
      if (hset->numSharedStreams > 0)   /* if streams are shared, using wa->occ is not appropriate */
         memcpy(&occ, &(si->hook), sizeof(float));
      else
         occ = ((WtAcc *) ((si->pdf + 1)->info->hook))->occ;
      fprintf(f, " %10f", occ);
   }
   fprintf(f, "\n");
}

/* StatReport: print statistics report */
void StatReport(HMMSet * hset)
{
   HMMScanState hss;
   HLink hmm;
   FILE *f;
   int px;

   if ((f = fopen(statFN, "w")) == NULL) {
      HError(2311, "StatReport: Unable to open stats file %s", "hsmmalign.stats");
      return;
   }
   NewHMMScan(hset, &hss);
   px = 1;
   do {
      hmm = hss.hmm;
      PrintStats(hset, f, px, hmm, (long) hmm->hook);
      px++;
   } while (GoNextHMM(&hss));
   EndHMMScan(&hss);
   fclose(f);
}

/* ----------------------------------------------------------- */
/*                      END:  HSMMAlign.c                      */
/* ----------------------------------------------------------- */
