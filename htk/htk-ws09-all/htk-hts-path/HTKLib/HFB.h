/* ----------------------------------------------------------- */
/*                                                             */
/*                          ___                                */
/*                       |_| | |_/   SPEECH                    */
/*                       | | | | \   RECOGNITION               */
/*                       =========   SOFTWARE                  */ 
/*                                                             */
/*                                                             */
/* ----------------------------------------------------------- */
/*         Copyright: Microsoft Corporation                    */
/*          1995-2000 Redmond, Washington USA                  */
/*                    http://www.microsoft.com                 */
/*                                                             */
/*   Use of this software is governed by a License Agreement   */
/*    ** See the file License for the Conditions of Use  **    */
/*    **     This banner notice must not be removed      **    */
/*                                                             */
/* ----------------------------------------------------------- */
/*         File: HFB.h: Forward Backward routines module       */
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

/* !HVER!HFB:   3.4.1 [CUED 12/03/09] */

#ifndef _HFB_H_
#define _HFB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define NOPRUNE 1.0E20

/* structure for the utterance information */
typedef struct {

  MemHeap transStack; /* utterance transcript information heap */
  MemHeap dataStack;  /* utterance data information heap */
  MemHeap dataStack2; /* utterance data2 information heap */

  int Q;              /* number of models in transcription */
  Transcription *tr;  /* current transcription */

  Boolean twoDataFiles; /* Using two data files */
  int S;              /* number of data streams */
  int T;              /* number of frames in utterance */
  ParmBuf pbuf;       /* parameter buffer */
  ParmBuf pbuf2;      /* a second parameter buffer (if required) */

  HTime tgtSampRate;  /* frame rate */
  HTime tgtSampRate2; /* second frame rate */
    
  Observation *o;      /* Observations */
  Observation *o2;     /* Cepstral Mean Normalised obervation, used in
                               single pass re-training */

  LogDouble pr;        /* log prob of current utterance */

} UttInfo;


/* structure for the forward-backward  pruning information */
typedef struct {
 
  short *qHi;               /* array[1..T] of top of pruning beam */
  short *qLo;               /* array[1..T] of bottom of pruning beam */
  int maxBeamWidth;         /* max width of beam in model units */
  LogDouble maxAlphaBeta;   /* max alpha/beta product along beam ridge */
  LogDouble minAlphaBeta;   /* min alpha/beta product along beam ridge */
  LogDouble pruneThresh;    /* pruning threshold currently */

} PruneInfo;

/* structure for the forward-backward alpha-beta structures */
typedef struct {
  
  MemHeap abMem;      /* alpha beta memory heap */
  PruneInfo *pInfo;   /* pruning information */
  HLink *up_qList;    /* array[1..Q] of active HMM defs */
  HLink *al_qList;    /* array[1..Q] of active align HMM defs */
  HLink *up_dList;    /* array[1..Q] of active dur model defs */
  HLink *al_dList;    /* array[1..Q] of active align dur model defs */
  MLink *qLink;       /* array[1..Q] of link to active HMM defs */
  LabId  *qIds;       /* array[1..Q] of logical HMM names (in qList) */
  short *qDms;        /* array[1..Q] of minimum model duration */
  DVector **alphat;   /* array[1..Q][1..Nq] of prob */
  DVector **alphat1;  /* alpha[t-1] */
  DVector ***beta;    /* array[1..T][1..Q][1..Nq] of prob */
  float *****otprob;  /* array[1..T][1..Q][2..Nq-1][0..S][0..M] of prob */
  SVector **durprob;  /* array[1..Q][2..Nq-1][1..maxDur] of state dur prob */
  int **maxDur;       /* array[1..Q][1..Nq] of max state duration */
  LogDouble pr;       /* log prob of current utterance */
  Vector occt;        /* occ probs for current time t */
  Vector *occa;       /* array[1..Q][1..Nq] of occ probs (trace only) */
  Vector ****occm;    /* array[1..T][1..Q][1..Nq][1..S][1..M] of occ probs (param gen only) */

} AlphaBeta;

/* structure storing the model set and a pointer to it's alpha-beta pass structure */
typedef struct {
  Boolean twoModels;  /* Enable two model reestimation */
  Boolean useAlign;   /* Using model alignment */
  HMMSet *up_hset;    /* set of HMMs to be re-estimated */
  HMMSet *al_hset;    /* HMMs to use for alignment */
                      /* these are equal unless 2 model reest */
  HMMSet *up_dset;    /* set of duration models to be estimated */
  HMMSet *al_dset;    /* duration models to use for alignment */
                      /* these are equal unless 2 model reest */
  HSetKind hsKind;    /* kind of the alignment HMM system */
  UPDSet uFlags_hmm;  /* parameter update flags for HMMs */
  UPDSet uFlags_dur;  /* parameter update flags for duration models */
  int skipstart;      /* Skipover region - debugging only */
  int skipend;
  int maxM;           /* maximum number of mixtures in hmmset */
  int maxMixInS[SMAX];/* array[1..swidth[0]] of max mixes */
  AlphaBeta *ab;      /* Alpha-beta structure for this model */

  XFInfo *xfinfo_hmm;         /* xform info for hmmset */
  XFInfo *xfinfo_dur;         /* xform info for dmset */
  AdaptXForm *inXForm_hmm;    /* current input transform for HMMs (if any) */
  AdaptXForm *inXForm_dur;    /* current input transform for duration models (if any) */
  AdaptXForm *al_inXForm_hmm; /* current input transform for al_hset (if any) */
  AdaptXForm *al_inXForm_dur; /* current input transform for al_dset (if any) */
  AdaptXForm *paXForm_hmm;    /* current parent transform for HMMs (if any) */
  AdaptXForm *paXForm_dur;    /* current parent transform for duration models (if any) */
} FBInfo;


/* EXPORTED FUNCTIONS-------------------------------------------------*/

/* Initialise HFB module */
void InitFB(void) ;

/* Reset HFB module */
void ResetFB(void);

/* Allow tools to enable top-level tracing in HFB. Only here for historical reasons */
void SetTraceFB(void);

/* Initialise the forward backward memory stacks etc */
void InitialiseForBack(FBInfo *fbInfo, MemHeap *x, HMMSet *hset, UPDSet uFlags_hmm, HMMSet *dset, UPDSet uFlags_dur,
                       LogDouble pruneInit, LogDouble pruneInc, LogDouble pruneLim, 
                       float minFrwdP, Boolean useAlign, Boolean genDur);

/* Use a different model set for alignment */
void UseAlignHMMSet(FBInfo* fbInfo, MemHeap* x, HMMSet *al_hset, HMMSet *al_dset);

/* Initialise the utterance Information */
void InitUttInfo(UttInfo *utt, Boolean twoFiles );

/* GetInputObs: Get input Observations for t */
void GetInputObs( UttInfo *utt, int t, HSetKind hsKind );

/* load the labels into the UttInfo structure from file */
void LoadLabs(UttInfo *utt, FileFormat lff, char * datafn,
	      char *labDir, char *labExt);

/* load the data file(s) into the UttInfo structure */
void LoadData(HMMSet *hset, UttInfo *utt, FileFormat dff, 
	      char * datafn, char * datafn2);

/* Initialise the observation structures within UttInfo */
void InitUttObservations(UttInfo *utt, HMMSet *hset,
			 char * datafn, int * maxmixInS);

/* reset the observation structures within UttInfo */ 
void ResetUttObservations (UttInfo *utt, HMMSet *hset);

/* reset all pre-calculated duration prob matrices */
void ResetDMMPreComps (HMMSet *dset);

/* FBUtt: apply forward-backward to given utterance */
Boolean FBUtt(FBInfo *fbInfo, UttInfo *utt);

/* PrLog: print a log value */
void PrLog(LogDouble x);

/* SetAlign: set start and end frame according to given label */
void SetAlign (long *st, long *en, short *qDms, const int Q, const int T);

#ifdef __cplusplus
}
#endif

#endif  /* _HFB_H_ */

/* ------------------------ End of HHB.h --------------------------- */
