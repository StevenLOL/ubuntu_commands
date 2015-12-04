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
/*              2002  Cambridge University                     */
/*                    Engineering Department                   */
/*                                                             */
/*   Use of this software is governed by a License Agreement   */
/*    ** See the file License for the Conditions of Use  **    */
/*    **     This banner notice must not be removed      **    */
/*                                                             */
/* ----------------------------------------------------------- */
/*         File: HFBLat.h   Lattice Forward Backward routines  */
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

/* !HVER!HNET:   3.4.1 [CUED 12/03/09] */


/*
   This module provides facilities to apply an HMM set to 
   forward-backward align a sequence of training files and
   collect statistics from each alignment.  The exact
   statistics to capture are defined by a set of flags and
   they are stored in the accumulators attached to the HMM
   components.    For MLLR adaptation, the HAdapt routine
   AddAdaptFrame is called for each input frame within
   each mixture component.
*/   

typedef struct {
  /* protected [readonly] : */
  int T;
  LogDouble pr;
  int MPEFileLength;
  float AvgCorr; /* averate correctness. */

  /* Private: */
  ArcInfo lattices; 
  Lattice *numLat; /* for MPE. */
  int Q;
  HMMSet *hset;

  Boolean MPE;  /* currently doing MPE/MWE, this may change from call to call
		   (since we will sometimes want to do the correct transcription) */
  float AccScale; /*normally 1.0*/
  int S;
  HSetKind hsKind;              /* kind of the alignment HMM system */
  UPDSet uFlags;
  Boolean firstTime ;     /* Flag used to enable creation of ot */


  ArcInfo  *aInfo;

  Boolean twoDataFiles;
  Observation al_ot;
  Observation up_ot;
  Observation adapt_ot;   /* for parent XForm */

  BufferInfo al_info;             /* info from buffer on second data file */
  BufferInfo up_info;             /* info from buffer on second data file */
  ParmBuf al_pbuf;
  ParmBuf up_pbuf;

  MemHeap arcStack;
  MemHeap tempStack;
  MemHeap miscStack;
  
  MemHeap al_dataStack;
  MemHeap up_dataStack;

  float num_index; /*make sure set. */
  float den_index; /*only for MPE. */ /*make sure set. */

  Boolean InUse; /* FALSE if stacks are cleared and lattices empty. */

  XFInfo *xfinfo;
  AdaptXForm *inXForm;/* current input transform (if any) */
  AdaptXForm *paXForm;/* current parent transform (if any) */
  /* ... */
} FBLatInfo;

void InitFBLat(void);
/*
   Initialise module and set configuration parameters. 
*/

void ResetFBLat(void);

void InitialiseFBInfo(FBLatInfo *fbInfo, HMMSet *hset, UPDSet uset, Boolean TwoDataFiles); 



void FBLatAddLattice (FBLatInfo *fbInfo, Lattice *lat); /* add this lattice, can do this repeatedly. */



void FBLatSetAccScale(FBLatInfo *fbInfo, float AccScale); /*prepare to scale accumulators by this amount. */



void FBLatFirstPass(FBLatInfo *fbInfo, 
		    FileFormat dff, char *datafn, char *datafn2 /*for single-pass retraining*/,
		    Lattice *MPECorrLat /* Only used in MPE, equals correct lattice.*/ );

void FBLatSecondPass(FBLatInfo *fbInfo, 
                     int index, /* in MMI case, this is the index to store the accs. */
                     int den_index /* den_index is used only for MPE, for negative accs.*/ );

#define SUPPORT_QUINPHONE 


/*For use in HExactLat.c: */
int GetNoContextPhone(LabId phone, int *nStates_quinphone, int *state_quinphone, HArc *a, int *frame_end); 
void GetTimes(LArc *larc, int i, int *start, int *end);   /*gets times as ints. */

/* EXPORT-> SetDoingFourthAcc: Indicate whether it is currently storing MMI statistics */
void SetDoingFourthAcc(Boolean DO, int indx);

/* ------------------------ End of HFBLat.h ------------------------ */
