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
/* ----------------------------------------------------------- */
/*         Copyright:                                          */
/*                                                             */
/*              2003  M.J.F. Gales and                         */
/*                    Cambridge University                     */
/*                    Engineering Department                   */
/*                                                             */
/*   Use of this software is governed by a License Agreement   */
/*    ** See the file License for the Conditions of Use  **    */
/*    **     This banner notice must not be removed      **    */
/*                                                             */
/* ----------------------------------------------------------- */
/*         File: HAdapt.h      Adaptation Library module       */
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
/*                2008-2009  University of Edinburgh                 */
/*                           Centre for Speech Technology Research   */
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

#ifndef _HADAPT_H_
#define _HADAPT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* observation cache to save rotated observations */
typedef struct _ObsCache{
   int time;
   Vector obs;
   float det;
   struct _ObsCache *next;
} ObsCache;

/* acc cache to save accumulators related to parent XForm */
typedef struct _AccCache{
   int     baseclass;
   DVector bVector;
   DTriMat *bTriMat;
   struct _AccCache *next;
} AccCache;  

typedef struct {
  char *outSpkrPat;
  char *inSpkrPat;
  char *paSpkrPat;
  char *outXFormExt;
  char *inXFormExt;
  char *paXFormExt;
   char *al_inXFormExt;
   char *al_paXFormExt;
  char *outXFormDir;
  char *paXFormDir;
   char *al_inXFormDir;
   char *al_paXFormDir;
  Boolean useOutXForm;
  Boolean useInXForm;
  Boolean usePaXForm;
   Boolean use_alInXForm;
   Boolean use_alPaXForm;
   Boolean outFullC;
   Boolean inFullC;
  char *xformTMF;
  Boolean saveBinary;
  AdaptXForm *inXForm;
  AdaptXForm *outXForm;
  AdaptXForm *paXForm;
   AdaptXForm *diagCovXForm;
  HMMSet *al_hset;
  AdaptXForm *al_inXForm;
   AdaptXForm *al_paXForm;
   
   /* Transform Kind to be created */
   XFormKind outXKind;  
   
   /* whether a bias is to be estimated for the xform */
   Boolean useBias;
   
   /* split threshold definitions for each stream for each xform kind */
   Vector xformSplitThresh;
   Vector mllrMeanSplitThresh;
   Vector mllrCovSplitThresh;
   Vector cmllrSplitThresh;

   /* adaptation kind  definitions for each xform kind */
   AdaptKind xformAdaptKind;
   AdaptKind mllrMeanAdaptKind;
   AdaptKind mllrCovAdaptKind;
   AdaptKind cmllrAdaptKind;

   /* regression tree definitions for each xform kind */
   char *xformRegTree;
   char *mllrMeanRegTree;
   char *mllrCovRegTree;
   char *cmllrRegTree;

   /* baseclass definitions for each xform kind */
   char *xformBaseClass;
   char *mllrMeanBaseClass;
   char *mllrCovBaseClass;
   char *cmllrBaseClass;

   /* block size definitions for each xform kind for each stream */
   IntVec xformBlockSize[SMAX];
   IntVec mllrMeanBlockSize[SMAX];
   IntVec mllrCovBlockSize[SMAX];
   IntVec cmllrBlockSize[SMAX];

   /* band width definitions for each xform kind for each stream */
   IntVec xformBandWidth[SMAX];
   IntVec mllrMeanBandWidth[SMAX];
   IntVec mllrCovBandWidth[SMAX];
   IntVec cmllrBandWidth[SMAX];

   /* number of nuisance dimensions for HLDA transform estimation */
   int numNuisanceDim;
   
   char coutspkr[MAXSTRLEN];
   char cinspkr[MAXSTRLEN];
   char cpaspkr[MAXSTRLEN];
   int nspkr;
   
   /* current time when this changes accumulate complete stats */
   /* -1 indicates that this is the first frame of a new file */
   int baseTriMatTime;

   ObsCache *headboc;
   ObsCache *headpoc;
   AccCache *headac;

   MemHeap bobcaStack; /* base obscache */
   MemHeap pobcaStack; /* parent obscache */
   MemHeap acccaStack;			

   /* specifies whether the transforms change the model variances */
   Boolean covarChanged;
   Boolean covarPChanged;
} XFInfo;

/* -------------------- Initialisation Functions -------------------------- */

void InitAdapt(XFInfo *xfinfo_hmm, XFInfo *xfinfo_dur);
/*
   Initialise configuration parameters
*/

void ResetAdapt (XFInfo *xfinfo_hmm, XFInfo *xfinfo_dur);
/*
   Reset adaptation module 
*/

AdaptXForm *GetMLLRDiagCov(XFInfo *xfinfo, AdaptXForm *xform);
void CheckAdaptSetUp (HMMSet *hset, XFInfo *xfinfo);

/* ---------------- Accumulation Control Functions ------------------------ */

void SetBaseAccsTime(XFInfo *xfinfo, const int t);
void TidyBaseAccs(XFInfo *xfinfo);
/*
  Modifies the internal time of current frames. Is used to ensure that
  last frame is correctly added in using UpdateBaseTriMat
*/


void AccAdaptFrame(XFInfo *xfinfo, double Lr, Vector svec, MixPDF *mp, int t);
/* 
   Accumulate frame stats into specific mixture comp transformed using parent
*/

void ZeroAdaptAccs(HMMSet *hset, AdaptXForm *xform);
/*
   Zero all adaptation accumulates
*/

/* ---------------- Applying Transform Functions ------------------------ */

void SetXForm(HMMSet *hset, XFInfo *xfinfo, AdaptXForm *xform);
/*
  Set the current transform to xform. This must be executed
  prior to applying any adaptation transforms. Setting xform 
  to NULL turns off the input transformation.
*/

void SetParentXForm(HMMSet *hset, XFInfo *xfinfo, AdaptXForm *xform);
/*
  Set the parent transform to xform. If this is not set the
  default functionality is to build a transform on top of
  the input transform if any. Setting xform to NULL means
  build a transform on the original model set and feature
  space.
*/

void ApplyHMMSetXForm(HMMSet *hset, AdaptXForm* xform, Boolean full);
/*
  Apply current transform (and parents) to complete model set.
*/

void ApplyCompXForm(MixPDF *mp, AdaptXForm* xform, Boolean full);
/*
  Apply current transform (and parents) to a component.
*/

Vector ApplyCompFXForm(MixPDF *mp, Vector svec, AdaptXForm* xform, LogFloat *det, int t);
/*
  Apply linear transform  (and parents) to observation for a component 
  return a vector of the transformed parameters.
  IMPORTANT: Do not alter the values of the returned vector
*/

void ResetObsCache(XFInfo *xfinfo);

void ResetXFormHMMSet(HMMSet *hset);
/*
  Return the model set to it's original state 
  IMPORTANT: if HADAPT:STOREMINFO=FALSE is used this
  will have no affect
*/

/* ---------------  Transform Copying Functions ----------------------- */

LinXForm *CopyLinXForm(MemHeap *x, LinXForm *xf);
/*
  Create a linxform that is a copy of xf
*/

XFormSet *CopyXFormSet(MemHeap *x, XFormSet *xfset);
/*
  Create a XFormSet that is a copy of xf
*/

AdaptXForm *CopyAdaptXForm(MemHeap *x, AdaptXForm *xform);
/*
  Create an AdaptXForm that is a copy of xf
*/


/* ---------------  Transform Estimation Functions ----------------------- */

AdaptXForm *CreateAdaptXForm(HMMSet *hset, XFInfo *xfinfo, char *xformName);
/*
  Creates a new output transform. xformName will eventually
  be used as the macroname for the transform.
*/

Boolean GenAdaptXForm(HMMSet *hset, XFInfo *xfinfo);
/*
  Estimate the transform using the information and regression
  trees specified in the configuration files. Returns FALSE
  if there was insufficient data to generate a transform.
*/

Boolean UpdateSpkrStats(HMMSet *hset, XFInfo *xfinfo, char *datafn);
/* 
   UpdateSpkrStats: monitor speaker changes and generate transforms
   at each speaker boundary, returns TRUE when the output speaker
   has changed
*/

Boolean HardAssign(AdaptXForm *xform);
/* 
   Whether the transform uses hard assignment or not - required
   for HModel to determine how to read/write transform
*/

void UpdateSemiTiedModels(HMMSet *hset, XFInfo *xfinfo);
/*
   Tidies the model parameters and creates the semi-tied macro
   and stores it.
*/

void UpdateProjectModels(HMMSet *hset, char *dir);
/*
  Applies the projection to the HMMSet and stores transforms etc.
*/

/* for AX command in HHEd */
void AttachXFormInfo(HMMSet *hset);
void ApplyMapXForm(MixPDF *mp, MixPDF *mp_map, AdaptXForm *xform, Boolean full);

LinXForm* GetRelateXForm(HMMSet *hset, MixPDF *mp, XFormKind xkind);

#ifdef __cplusplus
}
#endif

#endif  /* _HADAPT_H_ */

/* ------------------------ End of HAdapt.h ------------------------ */
