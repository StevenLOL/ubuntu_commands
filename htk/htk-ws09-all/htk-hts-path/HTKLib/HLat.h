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
/*         2001-2002  Cambridge University                     */
/*                    Engineering Department                   */
/*                                                             */
/*   Use of this software is governed by a License Agreement   */
/*    ** See the file License for the Conditions of Use  **    */
/*    **     This banner notice must not be removed      **    */
/*                                                             */
/* ----------------------------------------------------------- */
/*       File: HLat.h:  Lattice Manipulation                   */
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

/* !HVER!HLat:   3.4.1 [CUED 12/03/09] */


#ifndef _HLAT_H_
#define _HLAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------ Initialisation --------------------------- */

void InitLat (void);
/*
   register module & set configuration parameters
*/

void ResetLat (void);
/*
   reset the module
*/

/* ------------------------ Datatype ----------------------------- */

/* Forward-Backward info structre attached to LNodes
     use doubles for imporved accuracy
*/
typedef struct FBlnodeInfo {
   LogDouble fwlike;     /* forward likelihood */
   LogDouble bwlike;     /* backward likelihood */
} FBinfo;

#define LNodeFw(ln)  (((FBinfo *) (ln)->hook)->fwlike)
#define LNodeBw(ln)  (((FBinfo *) (ln)->hook)->bwlike)

typedef enum {LATFB_SUM, LATFB_MAX} LatFBType;


/* ------------------------ Prototypes --------------------------- */

Transcription *LatFindBest (MemHeap *heap, Lattice *lat, int N);

void LatSetScores (Lattice *lat);

Lattice *LatPrune (MemHeap *heap, Lattice *lat, LogDouble thresh, float arcsPerSec);

void CalcStats (Lattice *lat);

LNode *LatStartNode (Lattice *lat);

LNode *LatEndNode (Lattice *lat);

void LatSetBoundaryWords (char *start, char *end, char  *startLM, char *endLM);

void LatCheck (Lattice *lat);

void FixBadLat (Lattice *lat);

void FixPronProbs (Lattice *lat, Vocab *voc);

Boolean LatTopSort (Lattice *lat, LNode **topOrder);

void LatAttachInfo (MemHeap *heap, size_t size, Lattice *lat);
void LatDetachInfo (MemHeap *heap, Lattice *lat);

LogDouble LatForwBackw (Lattice *lat, LatFBType type);


#ifndef NO_LAT_LM
Lattice *LatExpand (MemHeap *heap, Lattice *lat, LModel *lm);

void ApplyNGram2LabLat(Lattice *lat, LModel *lm);
#endif


Lattice *GetLattice (char *fn, char *path, char *ext,
                     /* arguments of ReadLattice() below */
                     MemHeap *heap, Vocab *voc, 
                     Boolean shortArc, Boolean add2Dict);

Lattice *MergeLatNodesArcs(Lattice *lat, MemHeap *heap, Boolean mergeFwd);

void ApplyWPNet2LabLat(Lattice *lat, Lattice *wdNet);

#ifdef __cplusplus
}
#endif

#endif  /* _HLAT_H_ */

/* ------------------------ End of HLat.h -------------------------- */
