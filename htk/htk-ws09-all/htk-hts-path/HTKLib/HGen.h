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
/*         File: HGen.c: Generate parameter sequence from HMM        */
/* ----------------------------------------------------------------- */

#ifndef _HGEN_H_
#define _HGEN_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAXWINNUM 30            /* maximum number of static + deltas */

typedef enum { WLEFT = 0, WRIGHT = 1 } WINWIDTH;
typedef enum { CHOLESKY = 0, MIX = 1, FB = 2 } ParmGenType;

typedef struct {
   char fn[MAXWINNUM][MAXFNAMELEN];     /* window coefficient file(s) */
   int num;                     /* number of windows */
   int maxw[2];                 /* max width [0(left) 1(right)] */
   int **width;                 /* width [0..num-1][0(left) 1(right)] */
   float **coef;                /* window coefficient [0..num][length[1]..length[2]] */
   int max_L;
} Window;

typedef struct {
   char ext[MAXSTRLEN];         /* filename extension for this PdfStream */
   IntVec ContSpace;            /* space indexes */
   Boolean fullCov;             /* full covariance flag */
   int vSize;                   /* vector size of observation vector */
   int order;                   /* vector size of static feature vector */
   int t;                       /* time counter */
   int origStart;               /* start pos of current segment in original utterance */
   int T;                       /* number of frames */
   int width;                   /* band width */
   Window win;                  /* window coefficients */
   Matrix mseq;                 /* sequence of mean vector */
   Covariance *vseq;            /* sequence of covariance matrices */
   Matrix C;                    /* generated parameter c */
   DVector g;
   DVector c;
   DMatrix WUW;
   DVector WUM;
   Vector gvmean;
   Covariance gvcov;
   Boolean *gvFlag;
   int gvT;
} PdfStream;

typedef struct {
   MemHeap *genMem;             /* generation memory heap */

   float speakRate;             /* speaking rate */
   float MSDthresh;             /* MSD threshold */
   Boolean modelAlign;          /* use model-level alignment from label */
   Boolean stateAlign;          /* use state-level alignment from label */
   HTime frameRate;             /* frame rate in 100ns */

   HMMSet *hset;                /* set of HMMs */
   HMMSet *dset;                /* set of duration models */
   int maxStates;               /* max # of states in hset */
   int maxMixes;                /* max # of mixes in hset */

   PdfStream pst[SMAX];         /* PdfStream for generation */
   int nPdfStream[SMAX];        /* # of PdfStreams and its size */

   Transcription *labseq;       /* input label sequence */
   int labseqlen;               /* # of labels */
   Label **label;               /* labels sequence */

   HLink *hmm;                  /* a sentence HMM for given label */
   HLink *dm;                   /* a sentence duration models for given label */

   IMatrix sindex;              /* state sequence indexes */
   IMatrix durations;           /* state durations */
   int tframe;                  /* total # of frames */
} GenInfo;

/* EXPORTED functions ------------------ */

void SetrFlags(char *s);
/* 
   Set random generation flags 
*/

void PrintrFlags(void);
/* 
   Print random generation flags 
*/

void InitGen(void);
/*
   Initialise module
*/

void ResetGen(void);
/*
   Reset module
*/

void SetTraceGen(void);
/* 
   Set trace level 
*/

void InitialiseGenInfo(GenInfo *, Transcription *, Boolean);
/*
 * Initialise GenInfo 
 */

void ResetGenInfo(GenInfo *);
/*
 * Reset GenInfo 
 */

void JointProb(GenInfo * genInfo, UttInfo * utt);
/* 
   joint probability of given observations and state sequence
*/

void ParamGen(GenInfo *, UttInfo *, FBInfo *, ParmGenType);
/*
   Generate parameter sequence 
 */

void SetupPdfStreams(GenInfo * genInfo, int start, int end);
/*
  Setup PdfStreams for parameter generation
*/

void Calc_WUM_and_WUW(PdfStream * pst, const int bias);
/*
  Calcurate W'*U^{-1}*M and W'*U^{-1}*W
*/

void Cholesky_Factorization(PdfStream * pst);
/*
  Compute Cholesky factor of matrix W'*U^{-1}*W
*/

void Forward_Substitution(PdfStream * pst);
/*
  Forward substitution to solve set of linear equations
*/

void Backward_Substitution(PdfStream * pst, const int bias);
/*
  Backward substitution to solve set of linear equations
*/

#ifdef __cplusplus
}
#endif

#endif                          /* _HGEN_H_ */

/* ------------------------ End of HGen.h -------------------------- */
