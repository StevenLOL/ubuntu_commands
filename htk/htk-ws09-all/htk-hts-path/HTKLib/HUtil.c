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
/*              2002  Cambridge University                     */
/*                    Engineering Department                   */
/*                                                             */
/*   Use of this software is governed by a License Agreement   */
/*    ** See the file License for the Conditions of Use  **    */
/*    **     This banner notice must not be removed      **    */
/*                                                             */
/* ----------------------------------------------------------- */
/*         File: HUtil.c      HMM utility routines             */
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

char *hutil_version = "!HVER!HUtil:   3.4.1 [CUED 12/03/09]";
char *hutil_vc_id = "$Id: HUtil.c,v 1.31 2011/06/16 04:18:29 uratec Exp $";

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
#include "HUtil.h"

/* --------------------------- Trace Flags ------------------------- */

#define T_TOP  0001       /* Top Level tracing */
#define T_ITM  0002       /* Item List tracing */
#define T_OCC  0004       /* Occupancy statistics tracing */

static int trace = 0;
static Boolean parsePhysicalHMM = FALSE;

static MemHeap setHeap;
static MemHeap itemHeap;

/* --------------------------- Initialisation ---------------------- */

static ConfParam *cParm[MAXGLOBS];      /* config parameters */
static int nParm = 0;

/* EXPORT->InitUtil: initialise configuration parameters */
void InitUtil(void)
{
   int i;

   Register(hutil_version,hutil_vc_id);
   nParm = GetConfig("HUTIL", TRUE, cParm, MAXGLOBS);
   if (nParm>0){
      if (GetConfInt(cParm,nParm,"TRACE",&i)) trace = i;
   }
   CreateHeap(&itemHeap,"HUtil: ItemList Heap",MHEAP,sizeof(ItemRec),
              1.0,200,8000);
   CreateHeap(&setHeap,"HUtil: IntSet Heap",MSTAK,1,1.0,2000,16000);
}

/* EXPORT->ResetUtil: reset created heaps in InitUtil */
void ResetUtil(void)
{
   ResetHeap(&setHeap);
   ResetHeap(&itemHeap);
   
   return;
}

/* EXPORT->ResetUtilItemList: frees all the memory from the ItemList heap */
void ResetUtilItemList(void)
{
   DeleteHeap(&itemHeap);
   CreateHeap(&itemHeap,"HUtil: ItemList Heap",MHEAP,sizeof(ItemRec),
             1.0,200,8000);
}

/* EXPORT->SetParsePhysicalHMM: only parse physical HMMs */
void SetParsePhysicalHMM(Boolean parse)
{
   parsePhysicalHMM = parse;
}

/* -------------------- Clone Routines ------------------------ */

/* CloneSVector: return a clone of given matrix */
SVector CloneSVector(MemHeap *hmem, SVector s, Boolean sharing)
{
   SVector t;  /* the target */
   
   if (s==NULL) return NULL;
   if (GetUse(s)>0 && sharing) {
      IncUse(s);
      return s;
   }
   t = CreateSVector(hmem,VectorSize(s));
   CopyVector(s,t);
   return t;
}

/* CloneSMatrix: return a clone of given Matrix */
SMatrix CloneSMatrix(MemHeap *hmem, SMatrix s, Boolean sharing)
{
   SMatrix t;  /* the target */
   
   if (s==NULL) return NULL;
   if (GetUse(s)>0 && sharing) {
      IncUse(s);
      return s;
   }
   t = CreateSMatrix(hmem,NumRows(s),NumCols(s));
   CopyMatrix(s,t);
   return t;
}

/* CloneSTriMat: return a clone of given TriMat */
STriMat CloneSTriMat(MemHeap *hmem, STriMat s, Boolean sharing)
{
   STriMat t;  /* the target */
   
   if (s==NULL) return NULL;
   if (GetUse(s)>0 && sharing) {
      IncUse(s);
      return s;
   }
   t = CreateSTriMat(hmem,TriMatSize(s));
   CopyTriMat(s,t);
   return t;
}

/* CloneMixPDF: return a clone of given MixPDF */
MixPDF *CloneMixPDF(HMMSet *hset, MixPDF *s, Boolean sharing)
{
   MixPDF *t;
   
   if (s->nUse>0 && sharing) { 
      ++s->nUse;
      return s;
   }
   t = (MixPDF*)New(hset->hmem,sizeof(MixPDF));
   t->nUse = 0; t->hook = NULL; t->gConst = s->gConst;
   t->info = s->info; /* handles semi-tied case */
   t->mean = CloneSVector(hset->hmem,s->mean,sharing);
   t->ckind = s->ckind;
   switch(s->ckind) {
   case DIAGC: 
   case INVDIAGC: 
      t->cov.var = CloneSVector(hset->hmem,s->cov.var,sharing);
      break;
   case FULLC:
   case LLTC:
      t->cov.inv = CloneSTriMat(hset->hmem,s->cov.inv,sharing);
      break;
   case XFORMC:
      t->cov.xform = CloneSMatrix(hset->hmem,s->cov.xform,sharing);
      break;
   }
   return t;
}

/* ClonePDF: return a clone of given stream (PDF) */
StreamInfo *ClonePDF(HMMSet *hset, int s, 
                     StreamInfo *ssti, Boolean sharing)
{
   int m,M;
   StreamInfo *tsti;
   MixtureElem *sme,*tme;

   if (ssti->nUse>0 && sharing) {
      ++ssti->nUse;
      return ssti;
   }
   M = ssti->nMix;
   tsti = (StreamInfo *)New(hset->hmem,sizeof(StreamInfo));
   tsti->nUse = 0;    tsti->nMix = ssti->nMix; 
   tsti->hook = NULL; tsti->stream = s;
      tme = (MixtureElem *)New(hset->hmem,M*sizeof(MixtureElem));
   tsti->spdf.cpdf = tme-1; sme = ssti->spdf.cpdf + 1;
   
      for (m=1; m<=M; m++,sme++,tme++){
         tme->weight = sme->weight;
      if ((MixWeight(hset,tme->weight)>MINMIX) || hset->msdflag[s])
         tme->mpdf = CloneMixPDF(hset,sme->mpdf,sharing);
      else 
         tme->mpdf = NULL;
   }

   return tsti;
}

/* CloneState: return a clone of given State */
StateInfo *CloneState(HMMSet *hset, StateInfo *ssi, Boolean sharing)
{
   StateInfo *tsi;  /* the target */
   StreamElem *tste,*sste;
   int s,S;
   
   if (ssi->nUse>0 && sharing) { 
      ++ssi->nUse;
      return ssi;
   }
   S = hset->swidth[0];
   tsi = (StateInfo *)New(hset->hmem,sizeof(StateInfo));
   tsi->nUse = 0; tsi->hook = NULL;
   tste = (StreamElem *)New(hset->hmem,S*sizeof(StreamElem));
   tsi->pdf = tste-1; sste = ssi->pdf + 1;
   for (s=1; s<=S; s++,tste++,sste++){
      tste->info = ClonePDF(hset,s,sste->info,sharing);
   }
   tsi->dur     = CloneSVector(hset->hmem,ssi->dur,sharing);
   tsi->weights = CloneSVector(hset->hmem,ssi->weights,sharing);
   return tsi;
}

/* EXPORT->CloneHMM: copy src HMM into tgt.  If sharing, then macros are shared */
void CloneHMM(HLink src, HLink tgt, Boolean sharing)
{
   StateElem *s,*t;
   int i;
   HMMSet *hset = src->owner;
   
   tgt->owner = src->owner;
   tgt->numStates = src->numStates;
   tgt->dur = CloneSVector(hset->hmem,src->dur,sharing);
   tgt->transP = CloneSMatrix(hset->hmem,src->transP,sharing);
   t = (StateElem *)New(hset->hmem,(tgt->numStates-2)*sizeof(StateElem));
   tgt->svec = t-2; s = src->svec+2;
   for (i=2; i<tgt->numStates; i++,s++,t++)
      t->info = CloneState(hset,s->info,sharing);
   tgt->hook = NULL; tgt->nUse = 0;
}

/* -------------------- Mapping Routines ------------------------ */

/* EXPORT->NewHMMScan:  create new HMM scan record */
void NewHMMScan(HMMSet *hset, HMMScanState *hss)
{
   hss->hset = hset;
   hss->S = hset->swidth[0];
   hss->isCont = ((hset->hsKind == PLAINHS) || (hset->hsKind == SHAREDHS)) ? TRUE : FALSE;
   hss->h = -1;
   hss->mac=NULL;
   if (!GoNextHMM(hss))
      HError(7220,"NewHMMScan: cannot find any physical HMMs to scan");
}

/* EXPORT->EndHMMScan: terminate scan and restore nUse flags */
void EndHMMScan(HMMScanState *hss)
{
   ClearSeenFlags(hss->hset,CLR_ALL);
   hss->hmm = NULL; hss->se = NULL; hss->ste = NULL; hss->me = NULL;
   hss->si = NULL; hss->sti = NULL; hss->mp = NULL;
}

/* EXPORT->GoNextHMM: Move to next unseen HMM in HMM set */
Boolean GoNextHMM(HMMScanState *hss)
{
   int M;
   MLink mac;

   if (hss->mac!=NULL)
      mac = hss->mac->next;
   else
      mac = NULL;

   if (mac==NULL) hss->h++;
   for (;hss->h<MACHASHSIZE;hss->h++)
      for (mac=((mac==NULL)?hss->hset->mtab[hss->h]:mac);
           mac!=NULL;mac=mac->next) {
         if (mac->type == 'h' && ((HLink)mac->structure)->numStates>1) {
            hss->mac = mac;
            hss->hmm = (HLink)mac->structure;
            hss->N = hss->hmm->numStates;
            hss->se = hss->hmm->svec+2; hss->i=2;
            hss->si = hss->se->info;
            hss->ste = hss->si->pdf+1; hss->s=1;
            hss->sti = hss->ste->info;
            M = hss->sti->nMix;
            hss->M = (M<0)?-M:M; hss->m=1;
            if (hss->isCont){
               hss->me = hss->sti->spdf.cpdf+1;
               hss->mp = hss->me->mpdf;
            } else if  (hss->hset->hsKind == TIEDHS) {
               hss->mp = hss->hset->tmRecs[hss->s].mixes[hss->m];
               hss->me = NULL;
            }
            return TRUE;
         }
      }
   hss->hmm = NULL;
   return FALSE;
}

/* EXPORT->GoNextState: move to next unseen state */
Boolean GoNextState(HMMScanState *hss, Boolean noSkip)
{
   Boolean stepping = FALSE, ok = TRUE;
   int M;
   
   while (IsSeen(hss->si->nUse) && ok){
      if (hss->i < hss->N-1) {
         ++hss->i; ++hss->se; stepping = TRUE;
         hss->si = hss->se->info;
      } else if (noSkip)
         return FALSE;
      else{
         stepping = FALSE;
         ok = GoNextHMM(hss); 
      }
   }
   if (ok) {
      Touch(&hss->si->nUse);
      if (stepping){
         hss->ste = hss->si->pdf+1; hss->s=1;
         hss->sti=hss->ste->info;
         M = hss->sti->nMix;
         hss->M = (M<0)?-M:M; hss->m=1;
         if (hss->isCont){
            hss->me = hss->sti->spdf.cpdf+1;
            hss->mp = hss->me->mpdf;
         } else if  (hss->hset->hsKind == TIEDHS) {
            hss->mp = hss->hset->tmRecs[hss->s].mixes[1];
            hss->me = NULL;
         }
      }
      return TRUE;
   }
   hss->se = NULL;
   return FALSE;
}

/* EXPORT->GoNextStream: move to next unseen stream info */
Boolean GoNextStream(HMMScanState *hss, Boolean noSkip)
{
   Boolean stepping = FALSE, ok = TRUE;
   int M;
   
   while (IsSeen(hss->sti->nUse) && ok) {
      if (hss->s < hss->S) {
         ++hss->s; ++hss->ste; stepping = TRUE;
         hss->sti = hss->ste->info;
      } else if (noSkip)
         return FALSE;
      else{
         stepping = FALSE;    
         ok = GoNextState(hss,FALSE);
      }
   }
   if (ok) {
      Touch(&hss->sti->nUse);
      if (stepping) {
         M = hss->sti->nMix;
         hss->M = (M<0)?-M:M; hss->m=1;
         if (hss->isCont){
            hss->me = hss->sti->spdf.cpdf+1;
            hss->mp = hss->me->mpdf;
         } else if  (hss->hset->hsKind == TIEDHS) {
            hss->mp = hss->hset->tmRecs[hss->s].mixes[1];
            hss->me = NULL;
         }
      }
      return TRUE;
   }
   hss->ste = NULL;
   return FALSE;
}

/* EXPORT->GoNextMix: move to next unseen mixture component */
Boolean GoNextMix(HMMScanState *hss, Boolean noSkip)
{
   Boolean ok = TRUE;
   
   if (hss->isCont || (hss->hset->hsKind == TIEDHS)) {
      while (IsSeen(hss->mp->nUse) && ok){
         if (hss->m < hss->M) {
            ++hss->m; 
            if (hss->isCont) {
               ++hss->me; 
               hss->mp = hss->me->mpdf;
            } else {
               hss->mp = hss->hset->tmRecs[hss->s].mixes[hss->m];
               hss->me = NULL;
            }
         } else if (noSkip)
            return FALSE;
         else
            ok = GoNextStream(hss,FALSE);
      }
      if (ok) {
         Touch(&hss->mp->nUse);
         return TRUE;
      }
   } else { /* There are no components in a DISCRETEHS system - use GoNextStream instead */
      HError(7231,"GoNextMix: Cannot specify mixture components unless continuous");
   }
   hss->me = NULL;
   return FALSE;
}

/* ----------------------- DiagC conversions ----------------------------- */

/* minimum and max values used in conversions */
#define MINVAR 1E-30
#define MAXVAR 1E+30

/* EXPORT->ConvDiagC Convert Diagonal Covariance Kind
  Converts all the HMMs in hset to INVDIAGC from DIAGC
  or vice versa. If convData is TRUE then each variance element is
  replaced by its reciprocal - otherwise only the CovKind in each HMM
  is changed and no data conversions are performed. */
void ConvDiagC(HMMSet *hset, Boolean convData)
{
   HMMScanState hss;
   SVector v;
   int k;

   if (hset->hsKind == DISCRETEHS || hset->hsKind == TIEDHS) 
      return;
   NewHMMScan(hset, &hss);
   while (GoNextMix(&hss,FALSE)) {
      if (hss.mp->ckind == DIAGC || hss.mp->ckind == INVDIAGC){
         hss.mp->ckind = (hss.mp->ckind == DIAGC)?INVDIAGC:DIAGC;
         if (convData){
            v = hss.mp->cov.var;
            if (! IsSeenV(v)) {
               for (k=1; k<=VectorSize(v); k++) {
                  if (v[k] > MAXVAR) v[k] = MAXVAR;
                  if (v[k] < MINVAR) v[k] = MINVAR;
                  v[k] = 1.0/v[k];
               }
               TouchV(v);
            }
         }
      }
   }
   EndHMMScan(&hss);
   ClearSeenFlags(hset,CLR_ALL);
}

/* EXPORT->ForceDiagC Convert Diagonal Covariance Kind
   Converts all the HMMs in hset to DIAGC from INVDIAGC */
void ForceDiagC(HMMSet *hset)
{
   HMMScanState hss;
   SVector v;
   int k;

   if (hset->hsKind == DISCRETEHS || hset->hsKind == TIEDHS) 
      return;
   NewHMMScan(hset, &hss);
   while (GoNextMix(&hss,FALSE)) {
      if (hss.mp->ckind == INVDIAGC){
         hss.mp->ckind = DIAGC;
         v = hss.mp->cov.var;
         if (! IsSeenV(v)) {
            for (k=1; k<=VectorSize(hss.mp->mean); k++) {
               if (v[k] > MAXVAR) v[k] = MAXVAR;
               if (v[k] < MINVAR) v[k] = MINVAR;
               v[k] = 1.0/v[k];
            }
            TouchV(v);
         }
      }
   }
   EndHMMScan(&hss);
   ClearSeenFlags(hset,CLR_ALL);
}

/* ----------------------- LogWt conversions ----------------------------- */

/* EXPORT->ConvLogWt Converts all mixture weights into log-weights. */
void ConvLogWt(HMMSet *hset)
{
   int m;
   HMMScanState hss;

   if (hset->hsKind == DISCRETEHS || hset->hsKind == TIEDHS || hset->logWt == TRUE) 
      return;
   NewHMMScan(hset, &hss);
   while (GoNextStream(&hss,FALSE)) {
      for (m=1;m<=hss.M;m++)
         hss.sti->spdf.cpdf[m].weight = MixLogWeight(hset,hss.sti->spdf.cpdf[m].weight);
   }
   EndHMMScan(&hss);
   hset->logWt = TRUE;
}

/* EXPORT->ConvExpWt Converts all mixture log-weights into weights. */
void ConvExpWt(HMMSet *hset)
{
   int m;
   HMMScanState hss;

   if (hset->hsKind == DISCRETEHS || hset->hsKind == TIEDHS  || hset->logWt == FALSE) 
      return;
   NewHMMScan(hset, &hss);
   while (GoNextStream(&hss,FALSE)) {
      for (m=1;m<=hss.M;m++)
         hss.sti->spdf.cpdf[m].weight = MixWeight(hset,hss.sti->spdf.cpdf[m].weight);
   }
   EndHMMScan(&hss);
   hset->logWt = FALSE;
}

/* ---------------------- HMM Identification ------------------- */

char *HMMPhysName(HMMSet *hset,HLink hmm)
{
   MLink ml;
   if ((ml=FindMacroStruct(hset,'h',hmm))==NULL)
      HError(7270,"HMMPhysName: Cannot find hmm definition");
   return(ml->id->name);
}

/* search hmm name from stream information */
char *HMMPhysNameFromStreamInfo(HMMSet *hset, StreamInfo *sti, int *state, int *stream)
{
   HMMScanState hss;
   char *name = NULL;

   NewHMMScan(hset, &hss);
   do {
      while (GoNextState(&hss, TRUE)) {
         while (GoNextStream(&hss, TRUE)) {
            if (hss.sti == sti) {
               name = HMMPhysName(hset, hss.hmm);
               if(state)
                  *state = hss.i;
               if(stream)
                  *stream = hss.s;
               break;
            }
         }
         if (name)
            break;
      }
      if (name)
         break;
   } while (GoNextHMM(&hss));
   EndHMMScan(&hss);

   return name;
}

/* --------------------- Item List Handling -------------------- */

/* 
   Lists of HMM structures represented by an 'itemlist' are 
   represented internally by ItemRec's
*/

/* AddItem: create an ItemRec holding x and prepend it to list */
void AddItem(HLink owner, Ptr item, ILink *list)
{
   ILink p;
   
   p=(ILink) New(&itemHeap,0);
   p->item=item; p->owner=owner; p->next=*list;
   *list=p;
}

/* NumItems: return number of items in given list */
int NumItems(ILink list)
{
   int n=0;
   
   while(list!=NULL) {
      n++;
      list=list->next;
   }
   return n;
}

/* FreeItems: return given list of items to free list */
void FreeItems(ILink *list)
{
   ILink p,q;
   
   for (p=*list;p!=NULL;p=q) {
      q=p->next;
      Dispose(&itemHeap,p);
   }
}

/* ----------------------- Integer Sets --------------------- */

/* InitSet: create an IntSet of given size */
IntSet CreateSet(int size)
{
   int i;
   Boolean *b;
   IntSet s;
   
   s.nMembers = size;
   b = (Boolean*) New(&setHeap, sizeof(Boolean)*size);
   s.set = b-1;
   for (i=1;i<=size;i++) *b++ = FALSE;
   return s;
}

/* FreeSet: free space occupied by given set */
void FreeSet(IntSet s)
{
   Dispose(&setHeap,s.set+1);
}

/* IsMember: return TRUE if x is a member of s */
Boolean IsMember(IntSet s, int x)
{
   if (x<1 || x>s.nMembers)
      HError(7271,"IsMember: Illegal set member test");
   return s.set[x];
}

/* AddMember: add the given member to set s */
void AddMember(IntSet s, int x)
{
   if (x<1 || x>s.nMembers)
      HError(7271,"AddMember: Illegal set member addition");
   s.set[x] = TRUE;
}

/* IsFullSet: return TRUE if given set is full */
Boolean IsFullSet(IntSet s)
{
   int i;
   
   for (i=1;i<=s.nMembers;i++) 
      if (!s.set[i]) return FALSE;
   return TRUE;
}

/* ClearSet: clear the given set */
void ClearSet(IntSet s)
{
   int i;
   
   for (i=1;i<=s.nMembers;i++) 
      s.set[i] = FALSE;
}

/* SetSet: set the given set */
void SetSet(IntSet s)
{
   int i;
   
   for (i=1;i<=s.nMembers;i++) 
      s.set[i] = TRUE;
}

/* DupSet: duplicate given set to newset*/
void DupSet(IntSet oldSet, IntSet *newSet)
{
   int i;

   *newSet = CreateSet(oldSet.nMembers);   
   for (i=1;i<=oldSet.nMembers;i++) 
      newSet->set[i] = oldSet.set[i];
}

/* CopySet: copy given set to newset*/
void CopySet(IntSet oldSet, IntSet newSet)
{
   int i;

   if (newSet.nMembers!=oldSet.nMembers)
      HError(9999,"CopySet: numbers of members in old and new sets are different");
   
   for (i=1;i<=oldSet.nMembers;i++) 
      newSet.set[i] = oldSet.set[i];
}

/* -------------------- Item List Parser -------------------- */

static Source *source;         /* Current source for item list */
static int ch;                 /* Current character from source */
static char pattern[PAT_LEN];  /* A copy of the last pattern parsed */
static char *position;         /*  and the current position in copy */
static int maxStates;          /*  and its max number of states */
static int maxMixes;           /*  and max number of mix comps */

/* EdError: report a syntax error in input command */
static void EdError(char *s)
{
   char *where,*p;
   int i;
   
   where=position;
   for (i=1;i<20;i++) if (ch=='}' || ch=='\n' || ch==EOF) break;
   fprintf(stderr,"%s\n",pattern);
   for (p=pattern; p<where; p++) fputc(' ',stderr);
   fprintf(stderr,"^\n");
   fprintf(stderr,"Error %s\n",s);
   HError(7230,"EdError: item list parse error");
}

/* ReadCh: get next character from f into ch */
static void ReadCh(void)
{
   ch = GetCh(source);
   if (position < (pattern+PAT_LEN-1))
      *position++ = ch, *position = 0;
}

/* SkipSpaces: skip white space */
static void SkipSpaces(void)
{
   while (isspace(ch)) ReadCh();
}

/* GetAlpha: get an alphanumeric string */
/*  This is a copy of ReadString but allows additional terminating */
/*  characters for unquoted strings */
static char *GetAlpha(char *s)
{
   static char term[]=".,)}";
   Boolean wasQuoted;
   int i,n,q=0;

   wasQuoted=FALSE;
   SkipSpaces();
   if (ch == DBL_QUOTE || ch == SING_QUOTE){
      wasQuoted = TRUE; q = ch;
      ReadCh();
   }
   for (i=0; i<MAXSTRLEN ; i++){
      if (wasQuoted){
         if (ch == q) {
            ReadCh();
            s[i] = '\0';
            return s;
         }
      } else {
         if (isspace(ch) || strchr(term,ch)){
            s[i] = '\0';
            return s;
         }
      }
      if (ch==ESCAPE_CHAR) {
         ReadCh();
         if (ch<'0' || ch>'7') {
            n = ch - '0'; ReadCh();
            if (ch<'0' || ch>'7') EdError("Octal digit expected");
            n = n*8 + ch - '0'; ReadCh();
            if (ch<'0' || ch>'7') EdError("Octal digit expected");
            ch += n*8 - '0';
         }
      }
      s[i] = ch; ReadCh();
   }     
   EdError("String too long");
   return NULL;         /* never reached -- make compiler happy */
}

/* GetInt: read integer coerced given range */
static int GetInt(int lo, int hi)
{
   char buf[20];
   int i = 0, num;
   
   SkipSpaces();
   if (!isdigit(ch))
      EdError("Int expected");
   while (isdigit(ch)) {
      if (i==19)
         EdError("Integer too long!");
      buf[i++] = ch; ReadCh();
   }
   buf[i] = '\0';
   num = atoi(buf);
   if (num<lo) num=lo;
   if (num>hi) num=hi;
   return num;
}

typedef enum {
   TRANSP_KEY, STATE_KEY, DUR_KEY, WEIGHTS_KEY,
   MIX_KEY, MEAN_KEY, STREAM_KEY, COV_KEY
} Keyword;

static char *keymap[] = {
   "TRANSP", "STATE", "DUR", "WEIGHTS",
   "MIX", "MEAN", "STREAM", "COV"
};

/* GetKey: get a keyword */
static Keyword GetKey(void)
{
   char buf[20];
   int i = 0;
   Keyword k;
   
   SkipSpaces();
   if (!isalpha(ch))
      EdError("Keyword expected");
   while (isalpha(ch)) {
      buf[i++] = toupper(ch); ReadCh();
      if (i==20)
         EdError("Keyword too long!");
   }
   buf[i] = '\0';
   for (k=TRANSP_KEY; k<=COV_KEY; k=(Keyword) (k+1))
      if (strcmp(keymap[k],buf) == 0 )
         return k;
   EdError("Unknown Keyword"); 
   return COV_KEY;  /* never reached -- make compiler happy */
}

static void ChkType(char newtype, char *type)
{
   if (*type == ' ' || (*type =='a' && 
                        (newtype=='v' || newtype=='i' || newtype=='c' || newtype=='x')))
      *type = newtype;
   else
      if (newtype != *type)
         HError(7231,"ChkType: Attempt to form list from different types %c and %c",
                newtype,*type);
}

/* AddTransP: add all transP's of models to ilist */
static void AddTransP(ILink models, ILink *ilist, char *type)
{
   ILink h;
   HMMDef *hmm;
   
   ChkType('t',type);
   for (h=models; h!=NULL; h=h->next) {
      hmm = h->owner;
      AddItem(hmm,hmm,ilist);   /* tie ->transP */
   }
}

/* PIntRange: parse an integer range and add its members to s */
static void PIntRange(IntSet s)
{
   int i,i1,i2;
   
   i1 = GetInt(1,s.nMembers);
   SkipSpaces();
   if (ch == '-') {
      ReadCh();
      i2 = GetInt(1,s.nMembers);
   }
   else
      i2=i1;
   for (i=i1; i<=i2; i++)
      AddMember(s,i);
}

/* PIndex: parse and index and return in s */
static void PIndex(IntSet s)
{
   SkipSpaces();
   if (ch != '[')
      EdError("[ expected");
   ReadCh();
   PIntRange(s);
   SkipSpaces();
   while (ch==',') {
      ReadCh();
      PIntRange(s);
      SkipSpaces();
   }
   if (ch != ']')
      EdError("] expected");
   ReadCh();
}

/* PMix: parse a mixture spec */
static void PMix(ILink models, ILink *ilist, char *type,
                 IntSet states, IntSet streams,HMMSet *hset)
{
   IntSet mixes;
   HMMDef *hmm;
   ILink h;
   int s,j,m;
   MixtureElem *me;
   StreamElem *ste;
   StreamInfo *sti;
   enum {TMIX, TMEAN, TCOV} what;
   
   mixes = CreateSet(maxMixes);
   PIndex(mixes);
   SkipSpaces();
   what = TMIX;
   if (ch == '.') {
      ReadCh();
      switch(GetKey()) {
      case MEAN_KEY:
         what = TMEAN; ChkType('u',type); break;
      case COV_KEY:
         what = TCOV;
         ChkType('a',type);
         break;
      default:
         EdError("Mean or Cov expected");
      }
   } else
      ChkType('m',type);
   for (h=models; h!=NULL; h=h->next) {
      hmm = h->owner;
      for (j=2; j<hmm->numStates; j++) 
         if (IsMember(states,j)) {
            ste = hmm->svec[j].info->pdf+1;
            for (s=1; s<=hset->swidth[0]; s++,ste++) {
               sti = ste->info;
               if (IsMember(streams,s)) {
                  me = sti->spdf.cpdf+1;
                  for (m=1; m<=sti->nMix; m++,me++) 
                     if ((hset->msdflag[s] || MixWeight(hset,me->weight)>MINMIX) && IsMember(mixes,m)) {
                        switch (what) {
                        case TMIX: /* tie ->mpdf */
                           if (trace & T_ITM)
                              printf(" %12s.state[%d].stream[%d].mix[%d]\n",
                                     HMMPhysName(hset,hmm),j,s,m);
                           AddItem(hmm,me,ilist); 
                           break;
                        case TMEAN: /* tie ->mean */
                           ChkType('u',type); 
                           if (trace & T_ITM)
                              printf(" %12s.state[%d].stream[%d].mix[%d].mean\n",
                                     HMMPhysName(hset,hmm),j,s,m);
                           AddItem(hmm,me->mpdf,ilist); break;
                        case TCOV: /* tie ->cov  */
                           switch (me->mpdf->ckind) {
                           case INVDIAGC:
                           case DIAGC:
                              ChkType('v',type); break;
                           case FULLC:
                              ChkType('i',type); break;
                           case LLTC:
                              ChkType('c',type); break;
                           case XFORMC:
                              ChkType('x',type); break;
                           }
                           if (trace & T_ITM)
                              printf(" %12s.state[%d].stream[%d].mix[%d].%c\n",
                                     HMMPhysName(hset,hmm),j,s,m,*type);
                           AddItem(hmm,me->mpdf,ilist); 
                           break;
                        }
                     }
               }
         }
   }
   }
   FreeSet(mixes);
}

/* PStatecomp: parse a statecomp */
static void PStatecomp(ILink models, ILink *ilist, char *type, 
                       IntSet states, IntSet *streams, HMMSet *hset)
{
   HMMDef *hmm;
   ILink h;
   int s,j;
   Keyword kw;
   IntSet str;
   
   switch(kw=GetKey()) {
   case MIX_KEY:
      if (hset->hsKind==TIEDHS || hset->hsKind==DISCRETEHS)
         HError(7231,"PStatecomp: Cannot specify streams or mixes unless continuous");
      str = CreateSet(SMAX);
      SetSet(str);
      SkipSpaces();
      if (ch == '[')
         PMix(models,ilist,type,states,str,hset);
      else {
         ChkType('p',type);
         for (h=models; h!=NULL; h=h->next) {
            hmm = h->owner;
            for (j=2; j<hmm->numStates; j++)
            if (IsMember(states,j))
               for (s=1; s<=hset->swidth[0];s++)
                  if (IsMember(str,s)) { /* tie -> spdf */
                     if (trace & T_ITM)
                        printf(" %12s.state[%d].stream[%d]\n",
                               HMMPhysName(hset,hmm),j,s);
                        AddItem(hmm,hmm->svec[j].info->pdf+s,ilist);
                  }
         }
      }
      FreeSet(str);
      break;
   case STREAM_KEY:
      if (hset->hsKind==TIEDHS || hset->hsKind==DISCRETEHS)
         HError(7231,"PStatecomp: Cannot specify streams or mixes unless continuous");
         str = CreateSet(SMAX);
         PIndex(str);
         SkipSpaces();
         if (ch == '.') {
         ReadCh();
         if (GetKey() != MIX_KEY)
            EdError("Mix expected");
      SkipSpaces();
      if (ch=='[')
         PMix(models,ilist,type,states,str,hset);
      else {
         ChkType('p',type);
         for (h=models; h!=NULL; h=h->next) {
            hmm = h->owner;
            for (j=2; j<hmm->numStates; j++)
               if (IsMember(states,j))    
                  for (s=1; s<=hset->swidth[0];s++)   
                     if (IsMember(str,s)) { /* tie -> spdf */
                        if (trace & T_ITM)
                           printf(" %12s.state[%d].stream[%d]\n",
                                  HMMPhysName(hset,hmm),j,s);
                        AddItem(hmm,hmm->svec[j].info->pdf+s,ilist);
                     }
         }
      }
	 }
	 else {
         ChkType('p',type);
         for (h=models; h!=NULL; h=h->next) {
            hmm = h->owner;
            for (j=2; j<hmm->numStates; j++)
               if (IsMember(states,j)) {
                  if (streams!=NULL) {        /* for Tree-based Clustering */
                     AddItem(hmm,hmm->svec+j,ilist);
                  }
                  else
                     for (s=1; s<=hset->swidth[0];s++)   
                        if (IsMember(str,s)) { /* tie -> spdf */
                           if (trace & T_ITM)
                              printf(" %12s.state[%d].stream[%d]\n",
                                     HMMPhysName(hset,hmm),j,s);
                           AddItem(hmm,hmm->svec[j].info->pdf+s,ilist);
                        }
               }
         }
	 }
      if (streams != NULL)
         for (s=1;s<=SMAX;s++)
            streams->set[s] = str.set[s];
      FreeSet(str);
      break;
   case DUR_KEY:
      ChkType('d',type);
      for (h=models; h!=NULL; h=h->next) {
         hmm = h->owner;
         for (j=2; j<hmm->numStates; j++)
            if (IsMember(states,j)) {  /* tie ->dur */
               if (trace & T_ITM) 
                  printf(" %12s.state[%d].dur\n",
                         HMMPhysName(hset,hmm),j);
               AddItem(hmm,hmm->svec[j].info,ilist);
            }
      }
      break;
   case WEIGHTS_KEY:
      ChkType('w',type);
      for (h=models; h!=NULL; h=h->next) {
         hmm = h->owner;
         for (j=2; j<hmm->numStates; j++)
            if (IsMember(states,j)) { /* tie ->stream weights */
               if (trace & T_ITM)
                  printf(" %12s.state[%d].weights\n",
                         HMMPhysName(hset,hmm),j);
               AddItem(hmm,hmm->svec[j].info,ilist);
            }
      }
      break;
   default:
      EdError("dur, weight, stream or mix expected");
   }
}

/* PState: parse state and add all matches in models to ilist */
static void PState(ILink models, ILink *ilist, char *type, IntSet *streams, HMMSet *hset)
{
   IntSet states;
   int j;
   HMMDef *hmm;
   ILink h;
   
   states = CreateSet(maxStates);
   PIndex(states);
   SkipSpaces();
   if (ch == '.') {
      ReadCh();
      PStatecomp(models,ilist,type,states,streams,hset);
   } else {
      ChkType('s',type);
      for (h=models; h!=NULL; h=h->next) {
         hmm = h->owner;
         for (j=2; j<hmm->numStates; j++)
            if (IsMember(states,j)) {  /* tie ->info */
               if (trace & T_ITM)
                  printf(" %12s.state[%d]\n",
                         HMMPhysName(hset,hmm),j);
               AddItem(hmm,hmm->svec+j,ilist);
            }
      }
   }  
   FreeSet(states);  
}

/* PHIdent: parse a hmm ident and do pattern match */
static void PHIdent(ILink *models, HMMSet *hset)
{
   char pattern[MAXSTRLEN];
   int h; 
   MLink q;
   LabId hmmId;
   Boolean fullName=TRUE; /* are there wildcards in the name */
   char *p;
   
   SkipSpaces();
   GetAlpha(pattern);
   p = pattern; h=0;
   while ((*p != '\0') && (h<MAXSTRLEN) && (fullName)) {
     if ((*p=='*')||(*p=='?')) fullName=FALSE;
     h++; 
     p = pattern+h;
   }
   if (fullName) { /* this is the name of the model */
      hmmId = GetLabId(pattern,FALSE);
      q = FindMacroName(hset,'l',hmmId);
      if (q != NULL) {
         if (trace & T_ITM)
            printf("%s ",hmmId->name);
         AddItem((HLink) q->structure, q->structure, models);
      }
   } else { /* need to search for all models that match */
     for (h=0; h<MACHASHSIZE; h++)
       for (q=hset->mtab[h]; q!=NULL; q=q->next)
         if (((q->type=='h') && (parsePhysicalHMM)) || ((q->type=='l') && (!parsePhysicalHMM))) {
	   if (DoMatch(q->id->name,pattern)) {
	     if (trace & T_ITM)
	       printf("%s ",q->id->name);
	     AddItem((HLink) q->structure, q->structure, models);
	   }
         }
   }
}

/* PHName: parse hname and return list of matching models */
static void PHName(ILink *models,HMMSet *hset)
{
   if (trace & T_ITM)
      printf(" Models \n  ");
   
   SkipSpaces();
   if (ch == '(') {
      ReadCh();
      PHIdent(models,hset);
      SkipSpaces();
      while(ch == ',') {
         ReadCh();
         PHIdent(models,hset);
         SkipSpaces();
      }
      if (ch != ')')
         EdError(") expected");
      ReadCh();
   } else
      PHIdent(models,hset);
   if (trace & T_ITM) {
      printf("\n  %d models in itemlist\n",NumItems(*models));
      fflush(stdout);
   }
}

/* PItemSet: parse and item set appending items to ilist */
static void PItemSet(ILink *ilist, char *type, IntSet *streams, HMMSet *hset)
{
   ILink models = NULL;         /* list of hmms in item set */
   ILink p;
   
   if (ch == '}')   /* no list is specified */
      return;
   
   PHName(&models,hset);        /* parse hname and get list of models */
   SkipSpaces();
   if (ch == '.') {             /* look for subcomponents */
      ReadCh();
      switch (GetKey()) {
      case TRANSP_KEY: 
         AddTransP(models,ilist,type); 
         break;
      case STATE_KEY:  
         PState(models,ilist,type,streams,hset);
         break;
      default: 
         EdError("State or TransP expected");
      }
      FreeItems(&models);
   } else {                     /* append just the list of models to ilist */
      ChkType('h',type);
      p = *ilist;
      if (p==NULL)
         *ilist = models;
      else {
         while (p->next != NULL) p = p->next;
         p->next = models;
      }
   }
}

/* EXPORT->PItemList: parse items in item list setting ilist and type */
char *PItemList (ILink *ilist, char *type, HMMSet *hset, Source *s, IntSet *streams, 
                 const int maxM, const int maxS, const Boolean itrace)
{
   int rtrace;

   /* Initialise static variables */
   rtrace=trace;
   if (itrace)
      trace|=T_ITM;
   source = s;
   maxMixes  = (maxM==0) ? MaxMixInSet(hset)    : maxM;
   maxStates = (maxS==0) ? MaxStatesInSet(hset) : maxS;
   position=pattern;
   ch=GetCh(source);
   while (isspace(ch)) ch=GetCh(source);
   *position++=ch;
   *position=0;

   /* Parse item set */
   if (ch != '{')
      EdError("{ expected");
   ReadCh();
   PItemSet(ilist,type,streams,hset);
   SkipSpaces();
   while (ch == ',') {
      ReadCh();
      PItemSet(ilist,type,streams,hset);
      SkipSpaces();
   }
   if (ch != '}')
      EdError("} expected");

   if (trace & T_ITM)
      fflush(stdout);
   trace=rtrace;
   
   return(pattern);
}

/* ------------------- Generic macro handling ----------------- */

/* EXPORT->GetMacroHook: Return value of hook field for any macro */
Ptr GetMacroHook(MLink ml)
{
   Ptr hook;

   switch(ml->type) {
   case 'l': /* HLink */
   case 'h': /* HLink */
      hook=((HLink)(ml->structure))->hook; break;
   case 's': /* StateInfo * */
      hook=((StateInfo *)(ml->structure))->hook; break;
   case 'p': /* StreamInfo * */
      hook=((StreamInfo *)(ml->structure))->hook; break;
   case 'm': /* MixPDF * */
      hook=((MixPDF *)(ml->structure))->hook; break;
   case 'c': /* STriMat */
   case 'i': /* STriMat */
   case 'x': /* SMatrix */
   case 't': /* SMatrix */
   case 'u': /* SVector */
   case 'd': /* SVector */
   case 'w': /* SVector */
   case 'v': /* SVector */
      hook=GetHook(ml->structure); break;
   case '*': /* deleted */
      hook=NULL; break;
   case 'o': /* fake */
   default:
      hook=NULL;
      HError(7270,"GetMacroHook: Getting hook of non-existant macro");
   }
   return(hook);
}

/* EXPORT->SetMacroHook: Set value of hook field for any macro */
void SetMacroHook(MLink ml,Ptr hook)
{
   switch(ml->type) {
   case 'l': /* HLink */
   case 'h': /* HLink */
      ((HLink)(ml->structure))->hook=hook; break;
   case 's': /* StateInfo * */
      ((StateInfo *)(ml->structure))->hook=hook; break;
   case 'p': /* StreamInfo * */
      ((StreamInfo *)(ml->structure))->hook=hook; break;
   case 'm': /* MixPDF * */
      ((MixPDF *)(ml->structure))->hook=hook; break;
   case 'c': /* STriMat */
   case 'i': /* STriMat */
   case 'x': /* SMatrix */
   case 't': /* SMatrix */
   case 'u': /* SVector */
   case 'd': /* SVector */
   case 'w': /* SVector */
   case 'v': /* SVector */
      SetHook(ml->structure,hook); break;
   case 'r': /* fake */
   case 'b':
   case 'j':
   case 'a':
      break;
   case '*': /* deleted */
   case 'o': /* fake */
   default:
      HError(7270,"SetMacroHook: Setting hook of non-existant macro %s",
             &(ml->type));
   }
}

/* EXPORT->GetMacroUse: Return value of use field for any macro */
int GetMacroUse(MLink ml)
{
   int use;

   switch(ml->type) {
   case 'l': /* HLink */
   case 'h': /* HLink */
      use=((HLink)(ml->structure))->nUse; break;
   case 's': /* StateInfo * */
      use=((StateInfo *)(ml->structure))->nUse; break;
   case 'p': /* StreamInfo * */
      use=((StreamInfo *)(ml->structure))->nUse; break;
   case 'm': /* MixPDF * */
      use=((MixPDF *)(ml->structure))->nUse; break;
   case 'j': /* InputXForm * */
      use=((InputXForm *)(ml->structure))->nUse; break;
   case 'c': /* STriMat */
   case 'i': /* STriMat */
   case 'x': /* SMatrix */
   case 't': /* SMatrix */
   case 'u': /* SVector */
   case 'd': /* SVector */
   case 'w': /* SVector */
   case 'v': /* SVector */
      use=GetUse(ml->structure); break;
   case '*': /* deleted */
   case 'b':
   case 'r':    
      use=-1; break;
   case 'o': /* fake */
   default:
      use=-1;
      HError(7270,"GetMacroUse: Getting use of non-existant macro");
   }
   return(use);
}

/* EXPORT->SetMacroUse: Set value of use field for any macro */
void SetMacroUse(MLink ml,int use)
{
   switch(ml->type) {
   case 'l': /* HLink */
   case 'h': /* HLink */
      ((HLink)(ml->structure))->nUse=use; break;
   case 's': /* StateInfo * */
      ((StateInfo *)(ml->structure))->nUse=use; break;
   case 'p': /* StreamInfo * */
      ((StreamInfo *)(ml->structure))->nUse=use; break;
   case 'm': /* MixPDF * */
      ((MixPDF *)(ml->structure))->nUse=use; break;
   case 'j': /* InputXForm * */
      ((InputXForm *)(ml->structure))->nUse=use; break;
   case 'c': /* STriMat */
   case 'i': /* STriMat */
   case 'x': /* SMatrix */
   case 't': /* SMatrix */
   case 'u': /* SVector */
   case 'd': /* SVector */
   case 'w': /* SVector */
   case 'v': /* SVector */
      SetUse(ml->structure,use); break;
   case '*': /* deleted */
   case 'o': /* fake */
   case 'b':
   case 'r': 
      break;
   default:
      HError(7270,"SetMacroUse: Setting use of non-existant macro");
   }
}

/* EXPORT->ResetHooks: stores NULL in all hook fields of all current macros */
void ResetHooks(HMMSet *hset,char *what)
{
   int h;
   MLink q;

   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next) {
         if (q->type=='*')  
            continue;
         if (what==NULL || strchr(what,q->type))
            SetMacroHook(q,NULL);
      }
}

/* ------------------- Load Statistics File  --------------------- */

/* EXPORT->LoadStatsFile: load the statistics file output by HERest */
void LoadStatsFile(char *statfile,HMMSet *hset,Boolean otrace)
{
   Source src;
   char hname[MAXSTRLEN];
   int i,s,idx,count,N,lnum = 0;
   float x,tmp;
   HMMDef *hmm;
   MLink ml;
   LabId hmmId;
   double occSum = 0.0;
   long occN = 0;
   StateInfo *si;
   Boolean bin=FALSE;

   if(InitSource(statfile,&src,NoFilter)<SUCCESS)
      HError(7210,"LoadStatsFile: Can't open file %s", statfile);
   while(ReadInt(&src,&idx,1,bin)) {
      ++lnum;
      if (!ReadString(&src,hname) || !ReadInt(&src,&count,1,bin))
         HError(7250,"LoadStatsFile: Format error in file %s line %d",
                statfile,lnum);

      /* look up hname and find num states N */
      if ((hmmId = GetLabId(hname,FALSE))==NULL)
         HError(7251,"LoadStatsFile: unknown name %s at line %d",
                hname,lnum);
      if ((ml = FindMacroName(hset,'l',hmmId))==NULL)
         HError(7251,"LoadStatsFile: unknown model %s at line %d",
                hname,lnum);
      hmm = (HMMDef *) ml->structure;
      N = hmm->numStates;
      for (i=2; i<N; i++) {
         if (!ReadFloat(&src,&x,1,bin))
            HError(7250,"LoadStatsFile: Float format error file %s line %d\n",
                   statfile,lnum);
         si = hmm->svec[i].info;
         si->stateCounter = count;/* load the # of times the state occurred */
         memcpy(&(si->hook),&x,sizeof(float)); /* !! */
         
         for (s=1; s<=hset->swidth[0]; s++) {
            if (si->pdf[s].info->hook==NULL)
               tmp = 0.0;
            else
               memcpy(&tmp,&(si->pdf[s].info->hook),sizeof(float));
            tmp += x;
            memcpy(&(si->pdf[s].info->hook),&tmp,sizeof(float));
         }
         
         occSum += x; ++occN;
      }
   }
   CloseSource(&src);
   if (otrace || (trace & T_OCC)) {
      printf("  Stats loaded for %d models\n",lnum);
      printf("  Mean Occupation Count = %f\n",occSum/occN);
      fflush(stdout);
   }
}

/* ------------------- Configuration File Parsing  --------------------- */

/* EXPORT->ParseConfIntVec: interpret config string as integer array */
IntVec ParseConfIntVec (MemHeap *x, char *inbuf, Boolean residual)
{
   IntVec ivec = NULL;
   int size,cnt;
   char buf[MAXSTRLEN],tbuf[MAXSTRLEN];

   if ((inbuf=ParseString(inbuf,buf))>0) {
      if (strcmp(buf,"IntVec") != 0)
         HError(999,"ParseConfIntVec: format is 'IntVec n i1 i2 ... in'");
      sscanf(inbuf,"%d",&size);
      inbuf=ParseString(inbuf,tbuf);
      ivec = CreateIntVec(x,size);
      cnt = 1;
      while ((strlen(inbuf)>0) && (cnt<=size) &&
             (sscanf(inbuf,"%d",&(ivec[cnt])))) {
         inbuf=ParseString(inbuf,tbuf);
         cnt++;
      }
      if (residual && strlen(inbuf)>0)
         HError(999,"ParseConfIntVec: residual elements - format is  n i1 ... in");
   } else
      HError(999,"ParseConfIntVec: format is  n i1 ... in");
   return ivec;
}

/* EXPORT->ParseConfVector: interpret config string as float array */
Vector ParseConfVector (MemHeap *x, char *inbuf, Boolean residual)
{
   Vector vec = NULL;
   int size,cnt;
   char buf[MAXSTRLEN],tbuf[MAXSTRLEN];

   if ((inbuf=ParseString(inbuf,buf))>0) {
      if (strcmp(buf,"Vector") != 0)
         HError(999,"ParseConfVector: format is 'Vector n f1 f2 ... fn'");
      sscanf(inbuf,"%d",&size);
      inbuf=ParseString(inbuf,tbuf);
      vec = CreateVector(x,size);
      cnt = 1;
      while ((strlen(inbuf)>0) && (cnt<=size) &&
             (sscanf(inbuf,"%f",&(vec[cnt])))) {
         inbuf=ParseString(inbuf,tbuf);
         cnt++;
      }
      if (residual && strlen(inbuf)>0)
         HError(999,"ParseConfVector: residual elements - format is  n f1 ... fn");
   } else
      HError(999,"ParseConfVector: format is  n f1 ... fn");
   return vec;
}

/* EXPORT->ParseConfStrVec: interpret config string as string array */
char **ParseConfStrVec (MemHeap *x, char *inbuf, Boolean residual)
{
   char **str=NULL;
   int size,cnt;
   char buf[MAXSTRLEN],tbuf[MAXSTRLEN];

   if ((inbuf=ParseString(inbuf,buf))>0) {
      if (strcmp(buf,"StrVec") != 0)
         HError(999,"ParseConfStrVec: format is 'StrVec n s1 s2 ... sn'");
      sscanf(inbuf,"%d",&size);
      inbuf=ParseString(inbuf,tbuf);

      str = (char **) New(x, (size+1)*sizeof(char *));
      str[0] = (char *) New(x, sizeof(char));
      str[0][0] = (char) size;  /* size should be within char range */
      for (cnt=1; cnt<=size; cnt++)
         str[cnt] = (char *) New(x, MAXSTRLEN*sizeof(char));

      cnt = 1;
      while ((strlen(inbuf)>0) && (cnt<=size) &&
             (inbuf=ParseString(inbuf,str[cnt]))) {
         cnt++;
      }
      if (residual && strlen(inbuf)>0)
         HError(999,"ParseConfStrVec: residual elements - format is  n s1 ... sn");
   } else
      HError(999,"ParseConfStrVec: format is  n s1 ... sn");

   return str;
}

/* EXPORT->ParseConfBoolVec: interpret config string as Boolean array */
Boolean *ParseConfBoolVec (MemHeap *x, char *inbuf, Boolean residual)
{
   Boolean *vec=NULL;
   int size,cnt;
   char buf[MAXSTRLEN],tbuf[MAXSTRLEN];

   if ((inbuf=ParseString(inbuf,buf))>0) {
      if (strcmp(buf,"BoolVec") != 0)
         HError(999,"ParseConfBoolVec: format is 'BoolVec n f1 f2 ... fn'");
      sscanf(inbuf,"%d",&size);
      inbuf=ParseString(inbuf,tbuf);
      vec = (Boolean *) New(x,size*sizeof(Boolean));
      cnt = 1;
      while ((strlen(inbuf)>0) && (cnt<=size) && (sscanf(inbuf,"%s",buf))) {
         /* parse input as Boolean value */
         if (strcmp(buf,"T")==0 || strcmp(buf,"TRUE")==0)
            vec[cnt] = TRUE;
         else if (strcmp(buf,"F")==0 || strcmp(buf,"FALSE")==0)
            vec[cnt] = FALSE;
         else
            HError(9999,"ParseConfBoolVec: %s is not a Boolean value", buf);

         inbuf=ParseString(inbuf,tbuf);
         cnt++;
      }
      if (residual && strlen(inbuf)>0)
         HError(999,"ParseConfBoolVec: residual elements - format is  n f1 ... fn");
   } else
      HError(999,"ParseConfBoolVec: format is  n f1 ... fn");

   return vec;
}

/* ------------------------ End of HUtil.c ------------------------- */
