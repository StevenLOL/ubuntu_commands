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
/*          2001-2002 Cambridge University                     */
/*                    Engineering Department                   */
/*                                                             */
/*   Use of this software is governed by a License Agreement   */
/*    ** See the file License for the Conditions of Use  **    */
/*    **     This banner notice must not be removed      **    */
/*                                                             */
/* ----------------------------------------------------------- */
/*         File: HLM.h language model handling                 */
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

/* !HVER!HLM:   3.4.1 [CUED 12/03/09] */

#ifndef _HLM_H_
#define _HLM_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { boNGram=1, matBigram, hlmModel } LMType;

#define MAX_LMID 65534          /* Max number of words */
typedef unsigned short lmId;    /* Type used by lm to id words  1..MAX_LMID */
typedef unsigned short lmCnt;   /* Type used by lm to count wds 0..MAX_LMID */

#define NSIZE 4                 /* Max length of ngram 2==bigram etc */



typedef struct sentry {         /* HLM NGram probability */
   lmId word;                   /* word id */
   float prob;                  /* probability */
} SEntry;

typedef struct nentry {         /* HLM NGram history */
   lmId word[NSIZE-1];          /* Word history representing this entry */
   lmCnt nse;                   /* Number of ngrams for this entry */
   float bowt;                  /* Back-off weight */
   SEntry *se;                  /* Array[0..nse-1] of ngram probabilities */
   struct nentry *link;         /* Next entry in hash table */
   void *user;                  /* Accumulator or cache storage */
} NEntry;

typedef struct ngramlm {
   int nsize;                   /* Unigram==1, Bigram==2, Trigram==3 */
   unsigned int hashsize;       /* Size of hashtab (adjusted by lm counts) */
   NEntry **hashtab;            /* Hash table for finding NEntries */
   int counts[NSIZE+1];         /* Number of [n]grams */
   int vocSize;                 /* Core LM size */
   Vector unigrams;             /* Unigram probabilities */
   LabId *wdlist;               /* Lookup table for words from lmId */
   MemHeap *heap;               /* Pointer to heap */
} NGramLM;

typedef struct matbilm {
   lmCnt numWords;              /* Number of words for language model */
   Matrix bigMat;               /* Actual probs */
   LabId *wdlist;               /* Lookup table for words from lmId */
   MemHeap *heap;               /* Pointer to heap */
} MatBiLM;

typedef struct lmodel {
   char *name;                  /* Name used for identifying lm */
   LMType type;                 /* LM type */
   LogFloat pen;                /* Word insertion penalty */
   float scale;                 /* Language model scale */
   union {
      MatBiLM *matbi;
      NGramLM *ngram;
      void *hlmModel;
   }
   data;
   MemHeap *heap;               /* Heap for allocating lm structs */
} LModel;


void InitLM(void);
/*
   Initialise the module
*/

void ResetLM(void);
/*
   Reset the module
*/

/* ---------------- Lower Level Routines ----------------- */

NGramLM *CreateBoNGram(LModel *lm,int vocSize,int counts[NSIZE+1]);
/*
   Create backoff NGram language models with size defined by counts.
    vocSize=number of words in vocabulary
    counts[1]=number of unigrams
    counts[2]=approximate number of bigrams
    counts[3]=approximate number of trigrams
               (approximate sizes are used to determine hash table size)
*/
MatBiLM *CreateMatBigram(LModel *lm,int nw);
/*
   Create matrix bigram language models of specified size.
*/

NEntry *GetNEntry(NGramLM *nglm,lmId ndx[NSIZE],Boolean create);
/*
   Find [create] ngram entry for word histories ...ndx[1] ndx[0].
*/

/* --------------- Higher Level Routines ----------------- */

float GetLMProb(LModel *lm, LabId prid[NSIZE], LabId wdid);
/*
   Calculate probability for wdid following prid
*/

LModel *ReadLModel(MemHeap *heap,char *fn);
void WriteLModel(LModel *lm,char *fn,int flags);
/*
   Read/write language model from/to specified file.
   Flags control format for writing.
*/

void ClearLModel(LModel *lm);
/* 
   Clear LModel before deletion
*/

#ifndef NO_LAT_LM
typedef Ptr LMState;

LogFloat LMTrans (LModel *lm, LMState src, LabId wdid, LMState *dest);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* _HLM_H_ */

/* ------------------------ End of HLM.h --------------------------- */
