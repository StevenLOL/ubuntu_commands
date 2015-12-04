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
/*         File: HMTrain.h: Minimize generation error                */
/* ----------------------------------------------------------------- */

#ifndef HMTRAIN_H
#define HMTRAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#define WMAX               4    /* maximum # of window */
#define LOW_FLOOR          1
#define HIGH_FLOOR         2

typedef enum _ShiftType {
   ST_Left,
   ST_Right
} ShiftType;

typedef enum _MFType {
   MGE_EVAL = 0,
   MGE_TRAIN = 1,
   MGE_ADAPT = 2
} MFType;

/* Config information of MGE training */
typedef struct {
   int order;
   DVector initErrAcc[SMAX];    /* accumulation of generation error */
   DVector prevErrAcc[SMAX];    /* accumulation of generation error */
   DVector currErrAcc[SMAX];    /* accumulation of generation error */
   int pnTotalFrm[SMAX];        /* number of valid frames */
   IMatrix *pnMeanFloor[SMAX];  /* # of mean flooring[1..Order][1..WinNum][1(Lower) 2(Upper)] */
   IMatrix *pnVarFloor[SMAX];   /* # of var flooring [1..Order][1..WinNum][1(Lower) 2(Upper)] */
} MTStatInfo;

typedef struct {
   Matrix origObs;
   DMatrix quasi_invRef;
   DMatrix quasi_invWUW;
   DMatrix quasi_P;
   DMatrix origMean;            /* mean vector of original feature sequence */
   DMatrix origVar;             /* variance vector of original feature sequence */
   DMatrix genMean;             /* mean vector of generated feature sequence */
   DMatrix genVar;              /* variance vector of generated feature sequence */
   DMatrix varRatio;            /* ratio between original and generated variance */
   int stmIdx[WMAX];            /* model stream index [1..WinSize] */
   int nInvQuaSize;             /* bandwith of quasi-diagonal inversion matrix */
   int nVarWinSize;             /* size of the window for variance calculation */
} MgeStream;

typedef struct {
   MemHeap *mgeMem;
   GenInfo *genInfo;            /* Point to GenInfo */
   MTStatInfo *statInfo;        /* Point to MTStatInfo */
   MgeStream mst[SMAX];         /* Mge stream data */
   HMMSet *hset;
   HMMSet *orighset;            /* Original HMM set (before updating) */
   Boolean bOrigHmmRef;         /* Whether re-load the original HMM as refernece */
   MFType funcType;             /* Mge function type */
   Boolean pbMTrn[SMAX];        /* Whether perform MGE train for the stream */
   Boolean pbAccErr[SMAX];      /* Accumulate generation error */
   int pnInvQuaSize[SMAX];      /* Bandwith of quasi-diagonal inversion matrix */
   Boolean pbGVTrn[SMAX];       /* Whether incorporate GV component to MGE training */
   int pnVarWin[SMAX];          /* Size of the window for variance calculation */
   float fGVDistWght;           /* Distance weight for gv component */
   float pfGVWght[SMAX];        /* Distance weight for gv component */
   int nGainStreamIndex;        /* Stream index of gain in the vector */
   int nGainDimIndex;           /* Dimension index of gain in the vector */
   float fGainWghtComp;         /* Compensation for gain weighting */
   Boolean pbGainWght[SMAX];    /* Log gain for weighting generation error */
   float initStepSize;
   float currStepSize;
   Boolean bMVar;               /* Mutiply variance ratio for mean updating */
   Boolean bMScale;             /* Mutiply scale ratio for mean updating */
   Boolean bStepLimit;          /* Limit the updating rate for each step */
   UPDSet uFlags;               /* Update flags */
   Boolean bBoundAdj;           /* Adjust the segmentation boundary by MGE */
   int nBoundAdjWin;            /* Size of the window for boundary adjustment */
   int order;                   /* Total order of feature (order = vec_size / win_size) */
   Vector SRMean[SMAX];         /* Scaling rate for different model parameter updating (mean) */
   Vector SRVar[SMAX];          /* Scaling rate for different model parameter updating (var) */
   Vector DWght[SMAX];          /* Distance weight for different dimension of parameters */
   DVector mrat;                /* temporary updating rate for mean */
   DVector vrat;                /* temporary updating rate for variance */
} MgeTrnInfo;

void InitMTrain();
/*
  Initialise module
*/

void OneSentMgeTrain(MgeTrnInfo * mtInfo, char *labfn, char *datafn, float stepSize);
/*
  MGE-based HMM training for one utterance 
*/

void OneSentGenErrAcc(MgeTrnInfo * mtInfo, char *labfn, char *datafn);
/*
  Accumulate generation error for one utterance
*/

void UpdateAllMSDWeight(MgeTrnInfo * mtInfo);
/*
  Update MSD mixture weight for all state models 
*/

int OneSentBoundAdjust(MgeTrnInfo * mtInfo, char *labfn, char *datafn, char *outlabdir, int nBAIter, int nBALen, Boolean bAccErr);
/*
  Boundary adjustment for one utterance
*/

void OneSentTransform(MgeTrnInfo * mtInfo, char *labfn, char *datafn);
/*
  Apply transform to the models related to this sentences
*/

void OneSentMgeAdapt(MgeTrnInfo * mtInfo, char *labfn, char *datafn, float stepSize);
/*
  MGE-based adaptation for one utterance
*/

#ifdef __cplusplus
}
#endif

#endif                          /* _HMTRAIN_H_ */

/* ------------------------ End of HMTrain.h ----------------------- */
