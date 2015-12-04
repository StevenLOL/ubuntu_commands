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
/*         File: HMTrain.c: Minimize generation error                */
/* ----------------------------------------------------------------- */

char *hmtrain_version = "!HVER!HMTrain: 2.2 [NIT 07/07/11]";
char *hmtrain_vc_id = "$Id: HMTrain.c,v 1.2 2011/06/16 04:27:17 uratec Exp $";

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
#include "math.h"

/* -------------------- Trace Information -------------------- */

#define T_TOP   0001            /* Top level tracing */
#define T_STT   0002            /* trace for outputing statistical info */

/* --------------------- Global Settings --------------------- */

#define SMALL_VALUE            1e-10
#define TRIM_SIL_STATE_NUM     0
#define MAX_MEAN_UP_STEP       0.04f    /* PD maximum step for updating mean para compared to variance */
#define MAX_VAR_UP_STEP        0.02f    /* PD maximum step for updating var para */
#define MAX_UP_RATE_MEAN       2.0f     /* maximum range for updating mean para */
#define MAX_UP_RATE_VAR        10.0f    /* maximum range for updating var para */
#define MAX_MEAN_UP_STEP_XFORM 0.002f   /* PD maximum step for updating transform of mean para */
#define MAX_VAR_UP_STEP_XFORM  0.02f    /* PD maximum step for updating transform of var para */
#define MAX_UP_RATE_MEAN_XFORM 0.5f     /* maximum range for updating transform of mean para */
#define MAX_UP_RATE_VAR_XFORM  5.0f     /* maximum range for updating transform of var para */
#define STAT_UP_STEP_LIMIT     TRUE

#define MAX_MSD_WGHT 0.99f
#define MIN_MSD_WGHT 0.01f

static double g_pfMeanDynScale[3] = { 1.0, 0.3, 3.0 };
static double g_pfVarDynScale[3] = { 1.0, 2.0, 4.0 };
static double g_pfMeanDynScale_xform[3] = { 1.0, 0.3, 3.0 };
static double g_pfVarDynScale_xform[3] = { 1.0, 1.0, 3.0 };
static double g_fMeanBiasScale_xform = 0.2f;

#define WEIGHT_MAX 1.2f
#define WEIGHT_MIN 0.5f

#define INVALID_SHIFT -100000

#define MAX_BOUND_ADJ_LEN  100

/* CalGainWght: Calculate gain weight for generation error */
static float CalGainWght(float fGain, float fComp)
{
   float fGainWght;

   fGainWght = fGain / fComp;

   if (fGainWght > WEIGHT_MAX)
      return WEIGHT_MAX;
   else if (fGainWght < WEIGHT_MIN)
      return WEIGHT_MIN;
   else
      return fGainWght;
}

/* CalDynFeat: Calculate dynamic feature for generated feature sequence */
static void CalDynFeat(PdfStream * pst, int m)
{
   int t, i, j;

   for (t = 1; t <= pst->T; t++) {
      for (i = 1; i < pst->win.num; i++) {
         pst->C[t][i * pst->order + m] = 0.0f;
         /* deal with the v/uv boundary */
         if (pst->vseq[t].var[i * pst->order + m] == 0.0)
            continue;
         for (j = pst->win.width[i][0]; j <= pst->win.width[i][1]; j++) {
            if (t + j > 0 && t + j <= pst->T)
               pst->C[t][i * pst->order + m] += pst->C[t + j][m] * pst->win.coef[i][j];
         }
      }
   }
}

/* EXPORT->InitMTrain: Initialise module */
void InitMTrain()
{
   Register(hmtrain_version, hmtrain_vc_id);
}

/* MatchHmmStream: Get the matched HMM stream index for current p and l */
static int MatchHmmStream(GenInfo * genInfo, int cur_p, int win_idx)
{
   int p, stream;

   stream = 1;
   for (p = 1; p < cur_p; p++)
      stream += genInfo->nPdfStream[p];
   if (genInfo->nPdfStream[p] > 1) {
      stream += win_idx;
   }

   return stream;
}

/* CheckData: Check data file consistent with HMM definition */
static void CheckData(char *fn, BufferInfo * bufInfo, GenInfo * genInfo)
{
   char tpk[MAXSTRLEN];
   char mpk[MAXSTRLEN];

   if (bufInfo->nObs != genInfo->tframe) {
      HError(6611, "CheckData: Frame number in %s[%d] is incompatible with hmm [%d]", fn, bufInfo->nObs, genInfo->tframe);
   }

   if (bufInfo->tgtPK != genInfo->hset->pkind) {
      HError(6611, "CheckData: Parameterisation in %s[%s] is incompatible with hmm [%s]", fn, ParmKind2Str(bufInfo->tgtPK, tpk), ParmKind2Str(genInfo->hset->pkind, mpk));
   }

   if (bufInfo->tgtVecSize != genInfo->hset->vecSize) {
      HError(6611, "CheckData: Vector size in %s[%d] is incompatible with hmm [%d]", fn, bufInfo->tgtVecSize, genInfo->hset->vecSize);
   }
}

/* LoadOrigData: */
static void LoadOrigData(MgeTrnInfo * mtInfo, char *datafn)
{
   GenInfo *genInfo;
   MgeStream *mst;
   PdfStream *pst;
   BufferInfo info;
   ParmBuf pbuf;
   Observation obs;
   int p, t, pt, i, j, s, stream;
   HMMSet *hset;
   Boolean eSep;

   genInfo = mtInfo->genInfo;
   /* open buffer to read data */
   pbuf = OpenBuffer(mtInfo->mgeMem, datafn, 10, UNDEFF, FALSE_dup, FALSE_dup);
   if (pbuf == NULL)
      HError(6611, "LoadOrigData: Can not open data file");
   GetBufferInfo(pbuf, &info);
   CheckData(datafn, &info, genInfo);

   /* create an object to hold the input parameters */
   hset = mtInfo->genInfo->hset;
   SetStreamWidths(hset->pkind, hset->vecSize, hset->swidth, &eSep);
   obs = MakeObservation(mtInfo->mgeMem, hset->swidth, hset->pkind, FALSE, eSep);

   for (p = stream = 1; p <= genInfo->nPdfStream[0]; p++) {
      if (!mtInfo->pbMTrn[p] && !mtInfo->pbAccErr[p]) {
         stream += genInfo->nPdfStream[p];
         continue;
      }

      mst = &(mtInfo->mst[p]);
      pst = &(genInfo->pst[p]);
      /* load each frames */
      for (pt = t = 1; t <= info.nObs; t++) {
         ReadAsTable(pbuf, t - 1, &obs);
         /* if the value of vector is less than LSMALL, regarded as zero-space */
         if (obs.fv[stream][1] > LSMALL) {
            i = 1;
            for (s = stream; s < stream + genInfo->nPdfStream[p]; s++) {
               for (j = 1; j <= VectorSize(obs.fv[s]); j++, i++) {
                  mst->origObs[pt][i] = obs.fv[s][j];
               }
            }
            pt++;
            /* set space index from orig data */
            genInfo->pst[p].ContSpace[t] = TRUE;
         } else {
            genInfo->pst[p].ContSpace[t] = FALSE;
         }
      }
      genInfo->pst[p].T = pt - 1;
      stream += genInfo->nPdfStream[p];
   }

   CloseBuffer(pbuf);
}

/* SetupMgeTrnInfo: Setup all info struct for MGE training */
static void SetupMgeTrnInfo(MgeTrnInfo * mtInfo, char *labfn, char *datafn)
{
   GenInfo *genInfo;
   Transcription *tr;
   PdfStream *pst;
   MgeStream *mst;
   int p, k, TBak;

   genInfo = mtInfo->genInfo;

   mtInfo->mrat = CreateDVector(mtInfo->mgeMem, WMAX);
   mtInfo->vrat = CreateDVector(mtInfo->mgeMem, WMAX);

   /* load a given input label file */
   tr = LOpen(mtInfo->mgeMem, labfn, UNDEFF);
   /* initialize the GenInfo structure */
   InitialiseGenInfo(genInfo, tr, TRUE);

   /* initialize MgeStream */
   for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
      /* p-th PdfStream */
      pst = &(genInfo->pst[p]);
      mst = &(mtInfo->mst[p]);

      if (pst->fullCov && mtInfo->pbMTrn[p])
         HError(6610, "SetupMgeTrnInfo: Do not support full covariance");

      /* before load the original data, we dont know how long is the voice region for F0 */
      TBak = pst->T;
      pst->T = genInfo->tframe;
      if (mtInfo->pbMTrn[p] || mtInfo->pbAccErr[p]) {
         mst->origObs = CreateMatrix(mtInfo->mgeMem, pst->T, pst->vSize);
      }

      if (mtInfo->pbMTrn[p]) {
         mst->nInvQuaSize = mtInfo->pnInvQuaSize[p];
         mst->quasi_invRef = CreateDMatrix(mtInfo->mgeMem, pst->T, mst->nInvQuaSize * 2);       /* L~-1, L*L'=WUW */
         mst->quasi_invWUW = CreateDMatrix(mtInfo->mgeMem, pst->T, mst->nInvQuaSize * 2);       /* (WUW)~-1 */
         mst->quasi_P = CreateDMatrix(mtInfo->mgeMem, pst->T, mst->nInvQuaSize * pst->win.num * 2);     /* P = (W'UW)~-1 * W' ~ */
      }

      pst->T = TBak;

      for (k = 0; k < pst->win.num; k++)
         mst->stmIdx[k + 1] = MatchHmmStream(genInfo, p, k);
   }

   LoadOrigData(mtInfo, datafn);
}

/* ResetMgeTrnInfo: */
static void ResetMgeTrnInfo(MgeTrnInfo * mtInfo)
{
   Dispose(mtInfo->mgeMem, mtInfo->mrat);
   ResetGenInfo(mtInfo->genInfo);
}

/* InverseWUW: */
static void InverseWUW(MgeStream * mst, PdfStream * pst)
{
   int i, j, k, ii, jj, kk;
   int nInvQuaSize;

   DMatrix WUW;
   DMatrix quasi_invRef;
   DMatrix quasi_invWUW;

   WUW = pst->WUW;
   quasi_invRef = mst->quasi_invRef;
   quasi_invWUW = mst->quasi_invWUW;
   nInvQuaSize = mst->nInvQuaSize;

   ZeroDMatrix(mst->quasi_invRef);
   for (i = 1; i <= pst->T; i++) {
      quasi_invRef[i][nInvQuaSize] = pst->WUW[i][1];
      for (k = 1; k < pst->width; k++) {
         if (i + k <= pst->T) {
            quasi_invRef[i][nInvQuaSize + k] = pst->WUW[i][k + 1];
            quasi_invRef[i + k][nInvQuaSize - k] = pst->WUW[i][k + 1];
         }
      }
   }

   /* inverse WUW */
   ZeroDMatrix(quasi_invWUW);
   /* zero left-down */
   for (i = 1; i <= pst->T; i++) {
      /* normalize one row */
      quasi_invWUW[i][nInvQuaSize] = 1.0 / quasi_invRef[i][nInvQuaSize];
      /* norm orig matrix */
      for (k = 1; k < pst->width; k++) {
         if (i + k <= pst->T)
            quasi_invRef[i][nInvQuaSize + k] /= quasi_invRef[i][nInvQuaSize];
      }
      /* norm inv matrix */
      j = i - nInvQuaSize + 1 > 0 ? i - nInvQuaSize + 1 : 1;
      for (; j < i; j++)
         quasi_invWUW[i][(j - i) + nInvQuaSize] /= quasi_invRef[i][nInvQuaSize];
      quasi_invRef[i][nInvQuaSize] = 1.0;       /* useless code */

      /* zero one colum */
      for (k = 1; k < pst->width; k++) {
         ii = i + k;
         kk = nInvQuaSize - k;
         if (ii <= pst->T) {
            /* process inv matrix */
            j = i - nInvQuaSize + k + 1 > 0 ? i - nInvQuaSize + k + 1 : 1;
            jj = j - i + nInvQuaSize;
            for (; j <= i; j++, jj++) {
               quasi_invWUW[ii][j - i + kk] -= quasi_invWUW[i][jj] * quasi_invRef[ii][kk];
            }
            /* process orig matrix */
            for (j = 1; j < pst->width; j++) {
               if (i + j <= pst->T)
                  quasi_invRef[ii][j + kk] -= quasi_invRef[i][j + nInvQuaSize] * quasi_invRef[ii][kk];
            }
         }
      }
   }

   /* zero right-upper */
   for (i = pst->T; i >= 1; i--) {
      /* zero one colum */
      for (k = 1; k < pst->width; k++) {
         ii = i - k;
         kk = k + nInvQuaSize;
         if (ii > 0) {
            /* process inv matrix */
            j = i - nInvQuaSize + 1 > 0 ? i - nInvQuaSize + 1 : 1;
            jj = j - i + nInvQuaSize;
            for (; j < i + nInvQuaSize - k; j++, jj++) {
               if (j > pst->T)
                  break;
               quasi_invWUW[ii][j - i + kk] -= quasi_invWUW[i][jj] * quasi_invRef[ii][kk];
            }
         }
      }
   }

   /* need to make it symmetrical */
   for (i = 1; i <= pst->T; i++) {
      for (j = nInvQuaSize - 1; j >= 1; j--) {
         jj = nInvQuaSize - j;
         if (i - jj > 0) {
            quasi_invWUW[i][j] = (quasi_invWUW[i][j] + quasi_invWUW[i - jj][nInvQuaSize + jj]) / 2;
            quasi_invWUW[i - jj][nInvQuaSize + jj] = quasi_invWUW[i][j];
         }
      }
   }
}

/* CalculateMatrixP: P = R~-1 * W' = (W'UW)~-1 * W' */
static void CalMatrixP(MgeStream * mst, PdfStream * pst, int m)
{
   int t, i, j, k, start, end, jj, tj;
   int nInvQuaSize;

   ZeroDMatrix(mst->quasi_P);

   nInvQuaSize = mst->nInvQuaSize;
   /* process each row */
   for (t = 1; t <= pst->T; t++) {
      start = t - nInvQuaSize >= 0 ? t - nInvQuaSize : 0;
      jj = pst->win.num * (start - t + nInvQuaSize);
      end = t + nInvQuaSize - 1 < pst->T ? t + nInvQuaSize - 1 : pst->T;
      start += (1 - t + nInvQuaSize);
      end += (1 - t + nInvQuaSize);
      for (j = start; j < end; j++, jj += pst->win.num) {
         mst->quasi_P[t][jj + 1] = mst->quasi_invWUW[t][j];
      }

      /* delta para & acc para */
      start = t - nInvQuaSize + pst->win.max_L > 0 ? t - nInvQuaSize + pst->win.max_L : 0;
      end = t + nInvQuaSize - 1 - pst->win.max_L < pst->T ? t + nInvQuaSize - 1 - pst->win.max_L : pst->T;
      start += (1 - t + nInvQuaSize);
      end += (1 - t + nInvQuaSize);
      for (i = 1; i < pst->win.num; i++) {
         jj = pst->win.num * (start - 1);
         tj = start + t - nInvQuaSize;
         for (j = start; j < end; j++, tj++, jj += pst->win.num) {
            /* check u/v boundary */
            if (pst->vseq[tj].var[i * pst->order + m] == 0.0)
               continue;
            for (k = pst->win.width[i][0]; k <= pst->win.width[i][1]; k++)
               mst->quasi_P[t][jj + i + 1] += mst->quasi_invWUW[t][j + k] * pst->win.coef[i][k];
         }
      }
   }
}

/* ParaGenOneDim: */
static void ParaGenOneDim(PdfStream * pst, int m, Boolean bCalDyn)
{
   Calc_WUM_and_WUW(pst, m - 1);
   Cholesky_Factorization(pst); /* Cholesky decomposition */
   Forward_Substitution(pst);   /* forward substitution   */
   Backward_Substitution(pst, m - 1);   /* backward substitution  */

   if (bCalDyn)
      CalDynFeat(pst, m);
}

/* CalcMgeTrnInfo: */
static void CalcMgeTrnInfo(MgeStream * mst, PdfStream * pst, int m)
{
   Calc_WUM_and_WUW(pst, m - 1);

   /* calculate inverse matrix (W'UW)~-1 */
   InverseWUW(mst, pst);
   /* calculate Matrix P = R~-1 * W' = (W'UW)~-1 * W' */
   CalMatrixP(mst, pst, m);

   Cholesky_Factorization(pst); /* Cholesky decomposition */
   Forward_Substitution(pst);   /* forward substitution   */
   Backward_Substitution(pst, m - 1);   /* backward substitution  */

   CalDynFeat(pst, m);
}

/* CheckUpStepLimit: Check the updating para, and bound it if out of limit */
static Boolean CheckUpStepLimit(float *upRate, float limit, Boolean bRecord, IntVec pnFloorNum)
{
   if (*upRate > limit) {
      *upRate = limit;
      if (bRecord && pnFloorNum != NULL)
         pnFloorNum[HIGH_FLOOR]++;
      return TRUE;
   } else if (*upRate < -limit) {
      *upRate = -limit;
      if (bRecord && pnFloorNum != NULL)
         pnFloorNum[LOW_FLOOR]++;
      return TRUE;
   }
   return FALSE;
}

/* CheckLinUpRange: Check the updating para, and bound it if out of limit */
static Boolean CheckLinUpRange(float *data, float *ref_data, float limit, Boolean bRecord, IntVec pnFloorNum)
{
   if (*data - *ref_data > limit) {
      *data = *ref_data + limit;
      if (bRecord && pnFloorNum != NULL)
         pnFloorNum[HIGH_FLOOR]++;
      return TRUE;
   } else if (*data - *ref_data < -limit) {
      *data = *ref_data - limit;
      if (bRecord && pnFloorNum != NULL)
         pnFloorNum[LOW_FLOOR]++;
      return TRUE;
   }
   return FALSE;
}

/* CheckLogUpRange: Check the updating para, and bound it if out of limit */
static Boolean CheckLogUpRange(float *data, float *ref_data, float limit, Boolean bRecord, IntVec pnFloorNum)
{
   if (*data / *ref_data > limit) {
      *data = *ref_data * limit;
      if (bRecord && pnFloorNum != NULL)
         pnFloorNum[HIGH_FLOOR]++;
      return TRUE;
   } else if (*data / *ref_data < 1 / limit) {
      *data = *ref_data / limit;
      if (bRecord && pnFloorNum != NULL)
         pnFloorNum[LOW_FLOOR]++;
      return TRUE;
   }
   return FALSE;
}

/* CheckMeanUpdateRange: Check the updated mean HMM para, and bound it if out of range */
static Boolean CheckMeanUpdateRange(MixPDF * adj_mpdf, MixPDF * ref_mpdf, IntVec pnFloorNum, int k)
{
   float meanLimit;

   meanLimit = (float) (MAX_UP_RATE_MEAN / sqrt(ref_mpdf->cov.var[k]));
   if (adj_mpdf->mean[k] - ref_mpdf->mean[k] > meanLimit) {
      adj_mpdf->mean[k] = ref_mpdf->mean[k] + meanLimit;
      if (pnFloorNum != NULL)
         pnFloorNum[HIGH_FLOOR]++;
      return TRUE;
   } else if (adj_mpdf->mean[k] - ref_mpdf->mean[k] < -meanLimit) {
      adj_mpdf->mean[k] = ref_mpdf->mean[k] - meanLimit;
      if (pnFloorNum != NULL)
         pnFloorNum[LOW_FLOOR]++;
      return TRUE;
   }
   return FALSE;
}

/* CheckVarUpdateRange: Check the updated HMM var para, and bound it if out of range */
static Boolean CheckVarUpdateRange(MixPDF * adj_mpdf, MixPDF * ref_mpdf, IntVec pnFloorNum, int k)
{
   if (adj_mpdf->cov.var[k] > ref_mpdf->cov.var[k] * MAX_UP_RATE_VAR) {
      adj_mpdf->cov.var[k] = ref_mpdf->cov.var[k] * MAX_UP_RATE_VAR;
      if (pnFloorNum != NULL)
         pnFloorNum[HIGH_FLOOR]++;
      return TRUE;
   } else if (adj_mpdf->cov.var[k] < ref_mpdf->cov.var[k] / MAX_UP_RATE_VAR) {
      adj_mpdf->cov.var[k] = ref_mpdf->cov.var[k] / MAX_UP_RATE_VAR;
      if (pnFloorNum != NULL)
         pnFloorNum[LOW_FLOOR]++;
      return TRUE;
   }
   return FALSE;
}

/* MatchContSpace: */
static int MatchContSpace(IntVec ContSpace, int t)
{
   int ct;

   ct = 1;
   t--;
   while (t > 0) {
      if (ContSpace[t])
         ct++;
      t--;
   }
   return ct;
}

/* CalcUpdateRatio: Calculate updating ratio (mean & variance) for one state model */
static int CalcUpdateRatio(MgeTrnInfo * mtInfo, MgeStream * mst, PdfStream * pst, Label * label, int p, int m)
{
   int lowb, highb, nGainIdx;
   int i, k, l, t, tt, si, ei;
   int tstart, tend;
   double dfobs, tmrat, tvrat;
   int nCumWinSize = mst->nInvQuaSize - 2;
   MgeStream *gain_mst;

   /* for gain weight */
   nGainIdx = mtInfo->nGainDimIndex;
   gain_mst = &(mtInfo->mst[mtInfo->nGainStreamIndex]);

   ZeroDVector(mtInfo->mrat);
   ZeroDVector(mtInfo->vrat);
   /* range for accumulation of updating rate */
   si = (int) (label->start / mtInfo->genInfo->frameRate + 0.5) + 1;
   ei = (int) (label->end / mtInfo->genInfo->frameRate + 0.5) + 1;
   /* match the index of cont space */
   si = MatchContSpace(pst->ContSpace, si);
   ei = MatchContSpace(pst->ContSpace, ei);
   lowb = si - nCumWinSize > 0 ? si - nCumWinSize : 1;
   highb = ei + nCumWinSize <= pst->T ? ei + nCumWinSize : pst->T;
   for (t = lowb; t <= highb; t++) {
      /* distance between original and generated feature */
      dfobs = pst->C[t][m] - mst->origObs[t][m];
      /* gain weighting */
      if (mtInfo->pbGainWght[p]) {
         if (nGainIdx != m || p != mtInfo->nGainStreamIndex)
            dfobs *= CalGainWght(gain_mst->origObs[t][nGainIdx], mtInfo->fGainWghtComp);
      }

      /* accumulate updating ratio for mean and variance */
      tstart = (si >= t + 1 - nCumWinSize) ? si : t + 1 - nCumWinSize;
      tend = (ei <= t + 1 + nCumWinSize) ? ei : t + 1 + nCumWinSize;
      for (l = 0; l < pst->win.num; l++) {
         tt = pst->win.num * (tstart - t + mst->nInvQuaSize - 1) + l + 1;
         k = l * pst->order + m;
         tmrat = tvrat = 0.0;
         for (i = tstart; i < tend; i++, tt += pst->win.num) {
            if (mtInfo->uFlags & UPMEANS) {
               tmrat += mst->quasi_P[t][tt];
            }
            if (mtInfo->uFlags & UPVARS) {
               tvrat += mst->quasi_P[t][tt] * (pst->mseq[i][k] - pst->C[i][k]);
            }
         }
         if (mtInfo->uFlags & UPMEANS)
            mtInfo->mrat[l + 1] += dfobs * tmrat;
         if (mtInfo->uFlags & UPVARS)
            mtInfo->vrat[l + 1] += dfobs * tvrat;
      }
   }

   if (mtInfo->funcType == MGE_TRAIN) {
      for (l = 0; l < pst->win.num; l++) {
         mtInfo->mrat[l + 1] *= g_pfMeanDynScale[l];
         mtInfo->vrat[l + 1] *= g_pfVarDynScale[l];
      }
   } else if (mtInfo->funcType == MGE_ADAPT) {
      for (l = 0; l < pst->win.num; l++) {
         mtInfo->mrat[l + 1] *= g_pfMeanDynScale_xform[l];
         mtInfo->vrat[l + 1] *= g_pfVarDynScale_xform[l];
      }
   }

   return (ei - si);
}

/* SeqPDUpdate: Parameter updating by online PD */
static void SeqPDUpdate(MgeTrnInfo * mtInfo, MgeStream * mst, PdfStream * pst, int p, int m)
{
   int s, i, j, k, l, stm;
   int seqlen;
   LabList *lablist;
   Label *label;
   LabId labid;
   HMMDef *hmm, *ref_hmm;
   MixPDF *mpdf, *ref_mpdf;
   float upRate, var, meanLimit, varLimit, limitStep;
   GenInfo *genInfo;
   MTStatInfo *statInfo;
   MuAcc *ma;
   float occ, all_occ;

   genInfo = mtInfo->genInfo;
   statInfo = mtInfo->statInfo;
   ref_hmm = NULL;
   ref_mpdf = NULL;
   limitStep = (float) sqrt(mtInfo->currStepSize / mtInfo->initStepSize);
   varLimit = MAX_VAR_UP_STEP * limitStep;

   lablist = genInfo->labseq->head;
   seqlen = CountLabs(lablist);
   label = GetLabN(lablist, 1);
   /*i: hmm index; j: state indexo; */
   labid = label->auxLab[1];
   for (s = 1, i = 1, j = 2; s <= TRIM_SIL_STATE_NUM; s++, j++) {
      label = label->succ;
      if (label->auxLab[1] != NULL) {
         i++;
         j = 1;
         labid = label->auxLab[1];
      }
   }

   for (; s <= seqlen - TRIM_SIL_STATE_NUM;) {
      hmm = genInfo->hmm[i];
      /* original HMM for reference to constrain the updating range */
      if (mtInfo->bOrigHmmRef) {
         if (j == 2)
            ref_hmm = FindMacroName(mtInfo->orighset, 'l', label->auxLab[1])->structure;
         else
            ref_hmm = FindMacroName(mtInfo->orighset, 'l', labid)->structure;
         hmm->hook = ref_hmm;   /* link reference hmm */
      }

      for (; j < hmm->numStates; j++) {
         all_occ = (int) ((label->end - label->start) / mtInfo->genInfo->frameRate + 0.5);
         /* calculate updating ratio for eah state HMM */
         occ = CalcUpdateRatio(mtInfo, mst, pst, label, p, m);

         /* update model parameters */
         for (l = 0; l < pst->win.num; l++) {
            /* get corresponding stream & dimension index */
            stm = mst->stmIdx[l + 1];
            if (genInfo->nPdfStream[p] > 1)
               k = m;
            else
               k = l * pst->order + m;

            mpdf = hmm->svec[j].info->pdf[stm].info->spdf.cpdf[1].mpdf;
            if (mtInfo->bOrigHmmRef && ref_hmm != NULL)
               ref_mpdf = ref_hmm->svec[j].info->pdf[stm].info->spdf.cpdf[1].mpdf;

            /* mean updating */
            if (mtInfo->uFlags & UPMEANS) {
               /* accumulate occ (only for mean) */
               /* for MSD mixture weight updating */
               ma = (MuAcc *) GetHook(mpdf->mean);
               if (ma != NULL) {
                  if (m == 1 && (genInfo->nPdfStream[p] > 1 || (genInfo->nPdfStream[p] == 1 && l == 0)))
                     ma->occ += occ;
                  if (genInfo->nPdfStream[p] > 1) {
                     /* second mixture is unvoiced mixture */
                     ma = (MuAcc *) GetHook(hmm->svec[j].info->pdf[stm].info->spdf.cpdf[2].mpdf->mean);
                     if (m == 1)
                        ma->occ += (all_occ - occ);
                  }
               }
               /* calculate updating step */
               upRate = mtInfo->currStepSize * (float) mtInfo->mrat[l + 1];
               if (mtInfo->bMScale)
                  upRate *= mtInfo->SRMean[p][m];
               if (mtInfo->bMVar)
                  upRate *= mpdf->cov.var[k];

               if (mtInfo->bStepLimit) {
                  if (mtInfo->bOrigHmmRef && ref_mpdf != NULL)
                     var = (float) sqrt(ref_mpdf->cov.var[k]);
                  else
                     var = (float) sqrt(mpdf->cov.var[k]);
                  meanLimit = MAX_MEAN_UP_STEP / var * limitStep;
                  if (statInfo != NULL)
                     CheckUpStepLimit(&upRate, meanLimit, !(mtInfo->bOrigHmmRef), statInfo->pnMeanFloor[p][m][l + 1]);
                  else
                     CheckUpStepLimit(&upRate, meanLimit, !(mtInfo->bOrigHmmRef), NULL);
               }
               /* update mean parameter */
               mpdf->mean[k] -= (float) upRate;
               if (mtInfo->bOrigHmmRef && ref_mpdf != NULL) {
                  if (statInfo != NULL)
                     CheckMeanUpdateRange(mpdf, ref_mpdf, statInfo->pnMeanFloor[p][m][l + 1], k);
                  else
                     CheckMeanUpdateRange(mpdf, ref_mpdf, NULL, k);
               }
            }

            /* variance updating */
            if (mtInfo->uFlags & UPVARS) {
               upRate = mtInfo->currStepSize * (float) mtInfo->vrat[l + 1];
               if (mtInfo->bMScale)
                  upRate *= mtInfo->SRVar[p][m];
               upRate *= mpdf->cov.var[k];

               if (mtInfo->bStepLimit) {
                  if (statInfo != NULL)
                     CheckUpStepLimit(&upRate, varLimit, !(mtInfo->bOrigHmmRef), statInfo->pnVarFloor[p][m][l + 1]);
                  else
                     CheckUpStepLimit(&upRate, varLimit, !(mtInfo->bOrigHmmRef), NULL);
               }
               /* update var parameter */
               mpdf->cov.var[k] /= (float) pow(10, upRate);
               if (mtInfo->bOrigHmmRef && ref_mpdf != NULL) {
                  if (statInfo != NULL)
                     CheckVarUpdateRange(mpdf, ref_mpdf, statInfo->pnVarFloor[p][m][l + 1], k);
                  else
                     CheckVarUpdateRange(mpdf, ref_mpdf, NULL, k);
               }
            }
         }
         s++;
         if (s > seqlen - TRIM_SIL_STATE_NUM)
            break;
         label = label->succ;
      }
      i++;
      j = 2;
   }
}

/* CalOneDimGenErr: Calcuate the generation error for certain dimension */
static float CalOneDimGenErr(MgeTrnInfo * mtInfo, MgeStream * mst, PdfStream * pst, int p, int m)
{
   int t, origStart, nGainIdx;
   float diff, err = 0.0f, wt;
   MgeStream *gain_mst;

   /* for gain weight */
   nGainIdx = mtInfo->nGainDimIndex;
   gain_mst = &(mtInfo->mst[mtInfo->nGainStreamIndex]);

   diff = 0.0f;
   origStart = pst->origStart - 1;
   for (t = 1; t <= pst->T; t++) {
      diff = (pst->C[t][m] - mst->origObs[t + origStart][m]);
      wt = 1.0f;
      /* gain weighting */
      if (mtInfo->pbGainWght[p]) {
         if (nGainIdx != m || p != mtInfo->nGainStreamIndex)
            wt *= CalGainWght(gain_mst->origObs[t][nGainIdx], mtInfo->fGainWghtComp);
      }
      err += diff * diff * wt;
   }
   return err;
}

/* AccOneDimGenErr: Accumulate the generation error for certain dimension */
static void AccOneDimGenErr(MgeTrnInfo * mtInfo, MgeStream * mst, PdfStream * pst, int p, int m)
{
   float fErr;

   /* calculate generation error for one dimension */
   fErr = CalOneDimGenErr(mtInfo, mst, pst, p, m);
   if (mtInfo->statInfo != NULL)
      mtInfo->statInfo->currErrAcc[p][m] += fErr;
}

/* CalGenErrorForBA: Calcuate the total generation error for current generated segment/utterance */
static float CalGenErrorForBA(MgeTrnInfo * mtInfo, GenInfo * genInfo)
{
   MgeStream *mst;
   PdfStream *pst;
   int p, m;
   float fTotalErr, fErr;

   fTotalErr = 0.0;
   for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
      if (!mtInfo->pbAccErr[p])
         continue;

      mst = &(mtInfo->mst[p]);
      pst = &(genInfo->pst[p]);
      for (m = 1; m <= pst->order; m += 1) {
         ParaGenOneDim(pst, m, FALSE);
         fErr = CalOneDimGenErr(mtInfo, mst, pst, p, m);
         fTotalErr += fErr * mtInfo->DWght[p][m];
      }
   }

   return fTotalErr;
}

/* HMMTrainByMge: */
static void HmmTrainByMge(MgeTrnInfo * mtInfo, float stepSize, Boolean bOnlyAccErr)
{
   GenInfo *genInfo;
   MgeStream *mst;
   PdfStream *pst;
   int p, m;

   mtInfo->currStepSize = stepSize;
   genInfo = mtInfo->genInfo;

   for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
      mst = &(mtInfo->mst[p]);
      pst = &(genInfo->pst[p]);
      for (m = 1; m <= pst->order; m += 1) {
         if (mtInfo->pbMTrn[p] && !bOnlyAccErr)
            CalcMgeTrnInfo(mst, pst, m);        /* calculate the info for MGE updating */
         else if (mtInfo->pbAccErr[p])
            ParaGenOneDim(pst, m, FALSE);

         /* calculation of MGE updating ratio */
         if (mtInfo->pbMTrn[p] && !bOnlyAccErr) {
            SeqPDUpdate(mtInfo, mst, pst, p, m);
         }
         if (mtInfo->pbAccErr[p])
            AccOneDimGenErr(mtInfo, mst, pst, p, m);
      }
   }
}

/* EXPORT->OneSentMgeTrain: MGE-based HMM training for one utterance */
void OneSentMgeTrain(MgeTrnInfo * mtInfo, char *labfn, char *datafn, float stepSize)
{
   SetupMgeTrnInfo(mtInfo, labfn, datafn);
   SetupPdfStreams(mtInfo->genInfo, -1, -1);
   HmmTrainByMge(mtInfo, stepSize, FALSE);
   ResetMgeTrnInfo(mtInfo);
}

/* AccGenError: Accumulate the generation error for each dimension */
static void AccGenError(MgeTrnInfo * mtInfo)
{
   HmmTrainByMge(mtInfo, 0.0, TRUE);
}

/* EXPORT->OneSentGenErrAcc: Accumulate generation error for one utterance */
void OneSentGenErrAcc(MgeTrnInfo * mtInfo, char *labfn, char *datafn)
{
   SetupMgeTrnInfo(mtInfo, labfn, datafn);
   SetupPdfStreams(mtInfo->genInfo, -1, -1);
   AccGenError(mtInfo);
   ResetMgeTrnInfo(mtInfo);
}

/* UpdateOneMSDWeight: Update MSD mixture weight for one state models */
static void UpdateOneMSDWeight(MgeTrnInfo * mtInfo, StateElem * se, float all_occ, int p)
{
   int l, stm;
   GenInfo *genInfo;
   PdfStream *pst;
   MgeStream *mst;
   MuAcc *ma;
   MixtureElem *cpdf;

   genInfo = mtInfo->genInfo;
   if (genInfo->nPdfStream[p] <= 1)
      return;
   pst = genInfo->pst + p;
   mst = mtInfo->mst + p;

   /* update Mixture weight */
   for (l = 0; l < pst->win.num; l++) {
      /* get corresponding stream & dimension index */
      stm = mst->stmIdx[l + 1];
      cpdf = se->info->pdf[stm].info->spdf.cpdf;        /* voiced space */
      /* mean acc */
      ma = (MuAcc *) GetHook(cpdf[1].mpdf->mean);
      if (ma != NULL) {
         cpdf[1].weight = ma->occ / all_occ;    /* voiced space weight */
         if (cpdf[1].weight > MAX_MSD_WGHT)
            cpdf[1].weight = MAX_MSD_WGHT;
         else if (cpdf[1].weight < MIN_MSD_WGHT)
            cpdf[1].weight = MIN_MSD_WGHT;
         cpdf[2].weight = 1.0f - cpdf[1].weight;        /* unvoiced space weight */
      }
   }
}

/* GetAllUVOcc: */
static float GetAllUVOcc(MgeTrnInfo * mtInfo, StateElem * se, int p)
{
   GenInfo *genInfo;
   PdfStream *pst;
   MgeStream *mst;
   MuAcc *ma;
   MixtureElem *cpdf;
   int stm;
   float all_occ = 0.0f;

   genInfo = mtInfo->genInfo;
   pst = genInfo->pst + p;
   mst = mtInfo->mst + p;
   stm = mst->stmIdx[1];
   cpdf = se->info->pdf[stm].info->spdf.cpdf;
   if (genInfo->nPdfStream[p] <= 1) {
      return 0.0f;
   }

   /* voiced space */
   ma = (MuAcc *) GetHook(cpdf[1].mpdf->mean);
   if (ma != NULL)
      all_occ += ma->occ;
   /* unvoice space */
   ma = (MuAcc *) GetHook(cpdf[2].mpdf->mean);
   if (ma != NULL)
      all_occ += ma->occ;
   return all_occ;
}

/* EXPORT->UpdateAllMSDWeight: Update MSD mixture weight for all state models */
void UpdateAllMSDWeight(MgeTrnInfo * mtInfo)
{
   int p;
   UPDSet uFlags;
   HMMSet *hset;
   HSetKind hsKind;
   HMMScanState hss;
   StateElem *se;
   GenInfo *genInfo;
   float all_occ;

   genInfo = mtInfo->genInfo;
   hset = mtInfo->genInfo->hset;
   hsKind = hset->hsKind;
   uFlags = mtInfo->uFlags;

   NewHMMScan(hset, &hss);
   do {
      se = hss.se;
      for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
         all_occ = GetAllUVOcc(mtInfo, se, p);
         if (all_occ <= 0.0f)
            continue;
         UpdateOneMSDWeight(mtInfo, se, all_occ, p);
      }
   } while (GoNextState(&hss, FALSE));
   EndHMMScan(&hss);
}

/* GetPrevSegIdx: Get the index of previous segment */
static Boolean GetPrevSegIdx(GenInfo * genInfo, int curr_i, int curr_j, int *prev_i, int *prev_j)
{
   int j;

   if (curr_j > 1) {
      *prev_i = curr_i;
      *prev_j = curr_j - 1;
      return TRUE;
   }
   if (curr_i <= 1)
      return FALSE;

   *prev_i = curr_i - 1;
   j = 1;
   while (genInfo->durations[*prev_i][j] > 0) {
      j++;
   }
   if (j <= 1)
      return FALSE;
   *prev_j = j - 1;
   return TRUE;
}

/* GetNextSegIdx: Get the index of next segment */
static Boolean GetNextSegIdx(GenInfo * genInfo, int curr_i, int curr_j, int *next_i, int *next_j)
{
   if (genInfo->durations[curr_i][curr_j + 1] > 0) {
      *next_i = curr_i;
      *next_j = curr_j + 1;
      return TRUE;
   }
   if (curr_i >= genInfo->labseqlen)
      return FALSE;

   *next_i = curr_i + 1;
   *next_j = 1;
   if (genInfo->durations[*next_i][*next_j] <= 0)
      return FALSE;
   return TRUE;
}

/* ShiftOneFrame: Shift one frame of the boundary (before the segment of (labi, statej)) */
static int ShiftOneFrame(GenInfo * genInfo, int labi, int statej, ShiftType type)
{
   int nMod, pi, pj, ni, nj;
   Boolean bFlag;

   nMod = 1;
   bFlag = GetPrevSegIdx(genInfo, labi, statej, &pi, &pj);
   if (!bFlag)
      return INVALID_SHIFT;

   if (type == ST_Left) {
      if (genInfo->durations[pi][pj] > 1) {
         genInfo->durations[pi][pj] -= 1;
         genInfo->durations[labi][statej] += 1;
      } else {
         nMod += ShiftOneFrame(genInfo, pi, pj, type);
         if (nMod < 0)
            return INVALID_SHIFT;
         genInfo->durations[pi][pj] -= 1;
         genInfo->durations[labi][statej] += 1;
      }
   } else if (type == ST_Right) {
      if (genInfo->durations[labi][statej] > 1) {
         genInfo->durations[labi][statej] -= 1;
         genInfo->durations[pi][pj] += 1;
      } else {
         bFlag = GetNextSegIdx(genInfo, labi, statej, &ni, &nj);
         if (!bFlag)
            return INVALID_SHIFT;
         nMod += ShiftOneFrame(genInfo, ni, nj, type);
         if (nMod < 0)
            return INVALID_SHIFT;
         genInfo->durations[labi][statej] -= 1;
         genInfo->durations[pi][pj] += 1;
      }
   }
   return nMod;
}

/* ShiftBackOneFrame: Shift back the boundary */
static Boolean ShiftBackOneFrame(GenInfo * genInfo, int labi, int statej, ShiftType type, int nMod)
{
   int ni, nj;

   if (type == ST_Right) {
      for (; nMod > 1; nMod--) {
         GetNextSegIdx(genInfo, labi, statej, &ni, &nj);
         labi = ni;
         statej = nj;
      }
      nMod = ShiftOneFrame(genInfo, labi, statej, ST_Left);
   } else if (type == ST_Left) {
      for (; nMod > 1; nMod--) {
         GetPrevSegIdx(genInfo, labi, statej, &ni, &nj);
         labi = ni;
         statej = nj;
      }
      nMod = ShiftOneFrame(genInfo, labi, statej, ST_Right);
   }
   assert(nMod > 0);
   return TRUE;
}

/* ShiftBackSegBound: Shift back the boundary for several frames */
static Boolean ShiftBackSegBound(GenInfo * genInfo, int labi, int statej, ShiftType type, int *pnMod, int nShiftFrms)
{
   int i, nMod;

   for (i = nShiftFrms; i >= 1; i--) {
      nMod = ShiftBackOneFrame(genInfo, labi, statej, type, pnMod[i]);
      assert(nMod > 0);
   }
   return TRUE;
}

/* ShiftSegBound: Shift the boundary for several frames */
static Boolean ShiftSegBound(GenInfo * genInfo, int labi, int statej, ShiftType type, int *pnMod, int nShiftFrms)
{
   int i, nMod;

   for (i = 1; i <= nShiftFrms; i++) {
      nMod = ShiftOneFrame(genInfo, labi, statej, type);
      if (nMod < 0) {
         for (i--; i >= 1; i--)
            ShiftBackOneFrame(genInfo, labi, statej, type, pnMod[i]);
         return FALSE;
      }
      pnMod[i] = nMod;
      if (nMod > pnMod[0]) {
         for (; i >= 1; i--)
            ShiftBackOneFrame(genInfo, labi, statej, type, pnMod[i]);
         return FALSE;
      }
   }
   return TRUE;
}

/* AdjustOneStepLab: Adjust one label from the results of bound adjust */
static void AdjustOneStepLab(Label * label, int nMod, HTime frameRate, ShiftType type)
{
   int j;

   /* shift left */
   if (type == ST_Left) {
      label->start -= frameRate;
      for (j = 0; j < nMod - 1; j++) {
         label = label->pred;
         label->start -= frameRate;
         label->end -= frameRate;
      }
      label->pred->end -= frameRate;
      assert(label->pred->end > label->pred->start);
   }
   /* shift right */
   else if (type == ST_Right) {
      label->pred->end += frameRate;
      for (j = 0; j < nMod - 1; j++) {
         label->start += frameRate;
         label->end += frameRate;
         label = label->succ;
      }
      label->start += frameRate;
      assert(label->start < label->end);
   }
}

/* AdjustLabel: Adjust the label from the results of bound adjust */
static void AdjustLabel(Label * label, int *pnMod, int nShiftSteps, HTime frameRate, ShiftType type)
{
   int i;

   for (i = 1; i <= nShiftSteps; i++) {
      AdjustOneStepLab(label, pnMod[i], frameRate, type);
   }
}

/* AdjustOneBound: Try to shift the boundary to left/right, and choose the direction with minimum generation error */
static Boolean AdjustOneBound(MgeTrnInfo * mtInfo, GenInfo * genInfo, float *fErr, int labi, int statej, int nBAStep)
{
   int start, end, ti, tj, nPos, sdx;
   float origErr, slErr, srErr;
   Label *label;
   int pnLeftMod[MAX_BOUND_ADJ_LEN], pnRightMod[MAX_BOUND_ADJ_LEN];
   Boolean bFlag;

   nPos = 1;
   for (sdx = 1, ti = 1; ti <= genInfo->labseqlen; ti++) {
      for (tj = 1; genInfo->durations[ti][tj] > 0; tj++, sdx++) {
         if (ti == labi && tj == statej)
            break;
         nPos += genInfo->durations[ti][tj];
      }
      if (genInfo->durations[ti][tj] > 0)
         break;
   }
   start = nPos - mtInfo->nBoundAdjWin;
   end = nPos + mtInfo->nBoundAdjWin;
   if (start <= 0)
      start = 1;
   if (end >= genInfo->tframe)
      end = genInfo->tframe;

   /* maximum forced shift length */
   pnLeftMod[0] = pnRightMod[0] = MAX_BOUND_ADJ_LEN;
   /* calculate the gen error for original boundary */
   SetupPdfStreams(genInfo, start, end);
   origErr = CalGenErrorForBA(mtInfo, genInfo);
   slErr = srErr = origErr + 10.0f;

   /* shift the boundary to left, and re-calculate the gen error */
   bFlag = ShiftSegBound(genInfo, labi, statej, ST_Left, pnLeftMod, nBAStep);
   if (bFlag) {
      SetupPdfStreams(genInfo, start, end);
      slErr = CalGenErrorForBA(mtInfo, genInfo);
      ShiftBackSegBound(genInfo, labi, statej, ST_Left, pnLeftMod, nBAStep);
   }

   /* shift the boundary to right, and re-calculate the gen error */
   bFlag = ShiftSegBound(genInfo, labi, statej, ST_Right, pnRightMod, nBAStep);
   if (bFlag) {
      SetupPdfStreams(genInfo, start, end);
      srErr = CalGenErrorForBA(mtInfo, genInfo);
      ShiftBackSegBound(genInfo, labi, statej, ST_Right, pnRightMod, nBAStep);
   }

   /* adjust label */
   label = GetLabN(genInfo->labseq->head, sdx);
   if (slErr < origErr && slErr < srErr) {
      ShiftSegBound(genInfo, labi, statej, ST_Left, pnLeftMod, nBAStep);
      AdjustLabel(label, pnLeftMod, nBAStep, genInfo->frameRate, ST_Left);
      if (fErr != NULL)
         *fErr = slErr;
      return TRUE;
   } else if (srErr < origErr) {
      ShiftSegBound(genInfo, labi, statej, ST_Right, pnRightMod, nBAStep);
      AdjustLabel(label, pnRightMod, nBAStep, genInfo->frameRate, ST_Right);
      if (fErr != NULL)
         *fErr = srErr;
      return TRUE;
   }

   return FALSE;
}

/* CheckAdjFlagWin: */
static Boolean CheckAdjFlagWin(IntVec pbAdjFlag, int nPos, int nWinLen)
{
   int nVecSize, i, start, end;

   nVecSize = IntVecSize(pbAdjFlag);
   start = nPos - nWinLen;
   end = nPos + nWinLen;
   if (start < 1)
      start = 1;
   if (end > nVecSize)
      end = nVecSize;
   for (i = start; i <= end; i++) {
      if (pbAdjFlag[i])
         return TRUE;
   }
   return FALSE;
}

/* EXPORT->OneSentBoundAdjust: Boundary adjustment for one utterance */
int OneSentBoundAdjust(MgeTrnInfo * mtInfo, char *labfn, char *datafn, char *outlabdir, int nBAIter, int nBALen, Boolean bAccErr)
{
   GenInfo *genInfo;
   int i, j, sdx, nSegNum, nAdjNum, nTotalAdj;
   Boolean bFlag;
   char labExt[16];
   float fErr = 0.0f;
   IntVec pbPrevAdjFlag, pbCurrAdjFlag;

   genInfo = mtInfo->genInfo;
   if (nBAIter <= 0)
      nBAIter = 1;              /* temporary maximum value */

   /* initializatioin for mge train info */
   SetupMgeTrnInfo(mtInfo, labfn, datafn);
   /* create vecctor to store the boundary adjustment flags of last iteration */
   for (nSegNum = 0, i = 1; i <= genInfo->labseqlen; i++) {
      for (j = 1; genInfo->durations[i][j] > 0; j++)
         nSegNum++;
   }
   pbPrevAdjFlag = CreateIntVec(mtInfo->mgeMem, nSegNum);
   pbCurrAdjFlag = CreateIntVec(mtInfo->mgeMem, nSegNum);
   for (sdx = 1; sdx <= nSegNum; sdx++) {
      pbPrevAdjFlag[sdx] = TRUE;
      pbCurrAdjFlag[sdx] = FALSE;
   }

   nTotalAdj = 0;
   do {
      nAdjNum = 0;
      for (sdx = 1, i = 1; i <= genInfo->labseqlen; i++) {
         for (j = 1; genInfo->durations[i][j] > 0; j++, sdx++) {
            /* check whether the boundary adjustment is necessary */
            if (CheckAdjFlagWin(pbPrevAdjFlag, sdx, mtInfo->nBoundAdjWin) == FALSE && CheckAdjFlagWin(pbCurrAdjFlag, sdx, mtInfo->nBoundAdjWin) == FALSE)
               continue;
            bFlag = AdjustOneBound(mtInfo, genInfo, &fErr, i, j, nBALen);
            if (bFlag)
               nAdjNum++;
            pbCurrAdjFlag[sdx] = bFlag;
         }
      }
      if (nAdjNum <= 0) {
         nBALen--;
         continue;
      }
      nBAIter--;

      nTotalAdj += nAdjNum;
      for (sdx = 1; sdx <= nSegNum; sdx++) {
         pbPrevAdjFlag[sdx] = pbCurrAdjFlag[sdx];
         pbCurrAdjFlag[sdx] = FALSE;
      }
   } while (nBAIter > 0 && nBALen > 0);

   /* output label files */
   if (outlabdir != NULL) {
      ExtnOf(labfn, labExt);
      MakeFN(datafn, outlabdir, labExt, labfn);
   }
   LSave(labfn, genInfo->labseq, HTK);

   /* accumulate generation error */
   SetupPdfStreams(mtInfo->genInfo, -1, -1);
   if (bAccErr)
      AccGenError(mtInfo);

   /* reset the mge train info */
   ResetMgeTrnInfo(mtInfo);
   return nTotalAdj;
}

/* CopyOneMixPDF: */
static void CopyOneMixPDF(HMMSet * hset, MixPDF * mp_src, MixPDF * mp_dest)
{
   int size;

   size = VectorSize(mp_src->mean);
   if (size <= 0)
      return;

   /* copy mean parameter */
   CopyVector(mp_src->mean, mp_dest->mean);
   /* copy variance parameter */
   /* only support diagonal covariance */
   if (mp_src->ckind == DIAGC || mp_src->ckind == INVDIAGC) {
      CopyVector(mp_src->cov.var, mp_dest->cov.var);
   }
}

/* CopyOneModelPara: */
static void CopyModelPara(HMMSet * hset, HLink hmm_src, HLink hmm_dest)
{
   int j, s, m;
   StreamInfo *sti_src, *sti_dest;

   for (j = 2; j < hmm_src->numStates; j++) {
      for (s = 1; s <= hset->swidth[0]; s++) {
         sti_src = hmm_src->svec[j].info->pdf[s].info;
         sti_dest = hmm_dest->svec[j].info->pdf[s].info;
         for (m = 1; m <= sti_src->nMix; m++) {
            CopyOneMixPDF(hset, sti_src->spdf.cpdf[m].mpdf, sti_dest->spdf.cpdf[m].mpdf);
         }
      }
   }
}

/* TransformOneModel: Transform model parameter */
static void TransformOneModel(HMMSet * hset, HLink hmm)
{
   int j, s, m;
   StreamInfo *sti;
   AdaptXForm *xform;
   Boolean full;

   xform = hset->curXForm;
   if (xform->xformSet->xkind == MLLRCOV || xform->xformSet->xkind == CMLLR || xform->xformSet->xkind == SEMIT)
      full = TRUE;
   else
      full = FALSE;

   for (j = 2; j < hmm->numStates; j++) {
      for (s = 1; s <= hset->swidth[0]; s++) {
         sti = hmm->svec[j].info->pdf[s].info;
         for (m = 1; m <= sti->nMix; m++) {
            ApplyCompXForm(sti->spdf.cpdf[m].mpdf, xform, full);
         }
      }
   }
}

/* EXPORT->OneSentTransform: Apply transform to the models related to this sentences */
void OneSentTransform(MgeTrnInfo * mtInfo, char *labfn, char *datafn)
{
   int i;
   Transcription *tr;
   MLink hmacro;
   HLink hmm_trans, hmm_orig;
   Label *label;
   LabId id;

   /* load a given input label file */
   tr = LOpen(mtInfo->mgeMem, labfn, UNDEFF);
   /* transform the models related to each labels */
   for (i = 1; i <= CountLabs(tr->head); i++) {
      label = GetLabN(tr->head, i);
      if (label->auxLab[1] == NULL)     /* usually model name is written in auxLab[1] */
         continue;

      /* find model */
      id = label->auxLab[1];
      if ((hmacro = FindMacroName(mtInfo->hset, 'l', id)) == NULL)
         HError(9935, "OneSentTransform: Cannot find hmm %s in current model list", id->name);
      hmm_trans = (HLink) hmacro->structure;
      if ((hmacro = FindMacroName(mtInfo->orighset, 'l', id)) == NULL)
         HError(9935, "OneSentTransform: Cannot find hmm %s in original model list", id->name);
      hmm_orig = (HLink) hmacro->structure;
      hmm_trans->hook = hmm_orig;       /* link reference hmm */

      /* first copy model parameter from original hmm */
      CopyModelPara(mtInfo->hset, hmm_orig, hmm_trans);

      /* underlying assumption: no tying between mixs inside one hmm */
      /* transform model parameter */
      TransformOneModel(mtInfo->hset, hmm_trans);
   }
}

/* UpdateMeanXForm: Update transform of mean */
static void UpdateMeanXForm(MgeTrnInfo * mtInfo, LinXForm * lxform, LinXForm * ref_lxform, Vector ref_mean, Vector ivar, int dim, float rate, float limitStep, IntVec pnFloorNum)
{
   int b, bsize;
   Matrix A, ref_A;
   float upRate, meanLimit;
   float fold, fmean, fsvar, fbias;
   int bi, bj, i, j;

   /* check dimensions */
   assert(lxform->vecSize > 0);
   limitStep = MAX_MEAN_UP_STEP_XFORM * limitStep;

   bsize = 0;
   /* locate the corresponding block */
   for (b = 1, i = 1; b <= IntVecSize(lxform->blockSize); b++) {
      bsize = lxform->blockSize[b];
      if (i + bsize > dim)
         break;
      i += bsize;
   }
   A = lxform->xform[b];

   /* reference for limit the update range */
   if (ref_lxform != NULL)
      ref_A = ref_lxform->xform[b];
   else
      ref_A = NULL;

   fsvar = 1.0f / (float) sqrt(ivar[dim]);
   fbias = 0.0f;
   bi = dim - i + 1;
   for (bj = 1, j = i; bj <= bsize; bj++, j++) {
      /* ignore if A[i][j] = 0 */
      if (A[bi][bj] == 0.0f)
         continue;
      /* update rate for each element */
      fmean = ref_mean[j];

      upRate = fmean * rate;
      if (fmean != 0.0f)
         meanLimit = limitStep * fsvar / (float) fabs(fmean);
      else
         meanLimit = limitStep * fsvar;
      /* limit the update step */
      if (mtInfo->bStepLimit) {
         CheckUpStepLimit(&upRate, meanLimit, STAT_UP_STEP_LIMIT, pnFloorNum);
      }
      fold = A[bi][bj];
      /* update parameter */
      A[bi][bj] -= upRate;
      /* limit the update range */
      if (ref_A != NULL) {
         CheckLinUpRange(A[bi] + bj, ref_A[bi] + bj, MAX_UP_RATE_MEAN_XFORM, !STAT_UP_STEP_LIMIT, pnFloorNum);
      }
      /* prevent A[bi][bj] equal to zero */
      if (A[bi][bj] == 0.0f)
         A[bi][bj] = (float) SMALL_VALUE;
   }

   /* apply bias if required (igore if bias[i] = 0 */
   if (lxform->bias != NULL && lxform->bias[dim] != 0.0f) {
      upRate = rate * g_fMeanBiasScale_xform;
      meanLimit = limitStep / (float) sqrt(ivar[dim]);
      /* limit the update step */
      if (mtInfo->bStepLimit) {
         CheckUpStepLimit(&upRate, meanLimit, STAT_UP_STEP_LIMIT, pnFloorNum);
      }
      /* update parameter */
      lxform->bias[dim] -= upRate;
      /* limit the update range */
      if (ref_lxform != NULL) {
         CheckLinUpRange(lxform->bias + dim, ref_lxform->bias + dim, MAX_UP_RATE_MEAN_XFORM, !STAT_UP_STEP_LIMIT, pnFloorNum);
      }
      /* prevent A[bi][bj] equal to zero */
      if (lxform->bias[dim] == 0.0f)
         lxform->bias[dim] = (float) SMALL_VALUE;
   }
}

/* UpdateVarXForm: Update transform of variance. currently, only support diagonal variance matrix */
static void UpdateVarXForm(MgeTrnInfo * mtInfo, LinXForm * lxform, LinXForm * ref_lxform, Vector ref_ivar, int dim, float rate, float limitStep, IntVec pnFloorNum)
{
   int b, bsize;
   Matrix A, ref_A;
   float upRate, varLimit;
   int bi, bj, i, j;

   /* check dimensions */
   assert(lxform->vecSize > 0);
   varLimit = MAX_VAR_UP_STEP_XFORM * limitStep;

   bsize = 0;
   /* locate the corresponding block */
   for (b = 1, i = 1; b <= IntVecSize(lxform->blockSize); b++) {
      bsize = lxform->blockSize[b];
      if (i + bsize > dim)
         break;
      i += bsize;
   }
   A = lxform->xform[b];

   /* reference for limit the update range */
   if (ref_lxform != NULL)
      ref_A = ref_lxform->xform[b];
   else
      ref_A = NULL;

   bi = dim - i + 1;
   for (bj = 1, j = i; bj <= bsize; bj++, j++) {
      /* ignore if A[i][j] = 0 */
      if (A[bi][bj] == 0.0f)
         continue;
      /* update rate for each element */
      /* to be optimized, log vs. linear */
      upRate = rate * ref_ivar[j] / A[bi][bj];
      /* limit the update step */
      if (mtInfo->bStepLimit) {
         CheckUpStepLimit(&upRate, varLimit, STAT_UP_STEP_LIMIT, pnFloorNum);
      }
      /* update parameter */
      A[bi][bj] *= (float) pow(10, upRate);
      /* limit the update range */
      if (ref_A != NULL) {
         CheckLogUpRange(A[bi] + bj, ref_A[bi] + bj, MAX_UP_RATE_VAR_XFORM, !STAT_UP_STEP_LIMIT, pnFloorNum);
      }
      /* prevent A[bi][bj] less than zero */
      if (A[bi][bj] <= 0.0f)
         A[bi][bj] = (float) SMALL_VALUE;
   }
}

/* SeqPDAdapt: Parameter updating by online PD */
static void SeqPDAdapt(MgeTrnInfo * mtInfo, MgeStream * mst, PdfStream * pst, int p, int m)
{
   int s, i, j, k, l, stm, seqlen;
   LabList *lablist;
   Label *label;
   LabId labid;
   HMMDef *hmm, *ref_hmm;
   MixPDF *mpdf, *ref_mpdf;
   float upRate, limitStep;
   GenInfo *genInfo;
   MTStatInfo *statInfo;
   float occ, all_occ;
   LinXForm *xform, *ref_xform;
   IntVec pnFloorNum = NULL;

   genInfo = mtInfo->genInfo;
   statInfo = mtInfo->statInfo;
   ref_hmm = NULL;
   ref_mpdf = NULL;
   limitStep = (float) sqrt(mtInfo->currStepSize / mtInfo->initStepSize);

   lablist = genInfo->labseq->head;
   seqlen = CountLabs(lablist);
   label = GetLabN(lablist, 1);
   /*i: hmm index; j: state indexo; */
   labid = label->auxLab[1];
   for (s = 1, i = 1, j = 2; s <= TRIM_SIL_STATE_NUM; s++, j++) {
      label = label->succ;
      if (label->auxLab[1] != NULL) {
         i++;
         j = 1;
         labid = label->auxLab[1];
      }
   }

   ref_xform = NULL;
   for (; s <= seqlen - TRIM_SIL_STATE_NUM;) {
      hmm = genInfo->hmm[i];
      ref_hmm = hmm->hook;      /* ref_hmm has been hooked in ::OneSentTransform */

      for (; j < hmm->numStates; j++) {
         all_occ = (int) ((label->end - label->start) / mtInfo->genInfo->frameRate + 0.5);
         /* calculate updating ratio for eah state HMM */
         occ = CalcUpdateRatio(mtInfo, mst, pst, label, p, m);

         /* update transform parameters */
         for (l = 0; l < pst->win.num; l++) {
            /* get corresponding stream & dimension index */
            stm = mst->stmIdx[l + 1];
            if (genInfo->nPdfStream[p] > 1)
               k = m;
            else
               k = l * pst->order + m;

            mpdf = hmm->svec[j].info->pdf[stm].info->spdf.cpdf[1].mpdf;
            if (ref_hmm != NULL)
               ref_mpdf = ref_hmm->svec[j].info->pdf[stm].info->spdf.cpdf[1].mpdf;

            /* updating transform of mean */
            if (mtInfo->uFlags & UPMEANS) {
               /* get related transform */
               xform = GetRelateXForm(mtInfo->hset, mpdf, MLLRMEAN);
               if (ref_hmm != NULL)
                  ref_xform = GetRelateXForm(mtInfo->orighset, mpdf, MLLRMEAN);

               /* calculate updating step */
               upRate = mtInfo->currStepSize * (float) mtInfo->mrat[l + 1];
               if (mtInfo->bMScale)
                  upRate *= mtInfo->SRMean[p][m];
               if (mtInfo->bMVar)
                  upRate *= mpdf->cov.var[k];

               if (statInfo != NULL)
                  pnFloorNum = statInfo->pnMeanFloor[p][m][l + 1];
               /* update transform of mean */
               UpdateMeanXForm(mtInfo, xform, ref_xform, ref_mpdf->mean, mpdf->cov.var, k, upRate, limitStep, pnFloorNum);
            }

            /* updating transform of variance */
            /* currently, only support diagonal covariance */
            if (mtInfo->uFlags & UPVARS) {
               /* get related transform */
               xform = GetRelateXForm(mtInfo->hset, mpdf, MLLRVAR);
               if (ref_hmm != NULL)
                  ref_xform = GetRelateXForm(mtInfo->orighset, mpdf, MLLRVAR);

               /* calculate updating step */
               upRate = mtInfo->currStepSize * (float) mtInfo->vrat[l + 1];
               if (mtInfo->bMScale)
                  upRate *= mtInfo->SRVar[p][m];

               if (statInfo != NULL)
                  pnFloorNum = statInfo->pnVarFloor[p][m][l + 1];
               /* update transform of variance */
               UpdateVarXForm(mtInfo, xform, ref_xform, ref_mpdf->cov.var, k, upRate, limitStep, pnFloorNum);
            }
         }
         s++;
         if (s > seqlen - TRIM_SIL_STATE_NUM)
            break;
         label = label->succ;
      }
      i++;
      j = 2;
   }
}

/* AdaptHMMByMge: MGE adaptation training, currenly only support SEQ_PD updating algorithm */
static void AdaptHMMByMge(MgeTrnInfo * mtInfo, float stepSize, Boolean bOnlyAccErr)
{
   GenInfo *genInfo;
   MgeStream *mst;
   PdfStream *pst;
   int p, m;

   mtInfo->currStepSize = stepSize;
   genInfo = mtInfo->genInfo;

   for (p = 1; p <= genInfo->nPdfStream[0]; p++) {
      mst = &(mtInfo->mst[p]);
      pst = &(genInfo->pst[p]);
      for (m = 1; m <= pst->order; m += 1) {
         if (mtInfo->pbMTrn[p] && !bOnlyAccErr)
            CalcMgeTrnInfo(mst, pst, m);        /* calculate the info for MGE updating */
         else if (mtInfo->pbAccErr[p])
            ParaGenOneDim(pst, m, FALSE);

         /* calculation of MGE updating ratio */
         if (mtInfo->pbMTrn[p] && (mtInfo->uFlags & UPXFORM) && !bOnlyAccErr) {
            SeqPDAdapt(mtInfo, mst, pst, p, m);
         }
         if (mtInfo->pbAccErr[p])
            AccOneDimGenErr(mtInfo, mst, pst, p, m);
      }
   }
}

/* EXPORT->OneSentMgeAdapt: MGE-based adaptation for one utterance */
void OneSentMgeAdapt(MgeTrnInfo * mtInfo, char *labfn, char *datafn, float stepSize)
{
   SetupMgeTrnInfo(mtInfo, labfn, datafn);
   SetupPdfStreams(mtInfo->genInfo, -1, -1);
   AdaptHMMByMge(mtInfo, stepSize, FALSE);
   ResetMgeTrnInfo(mtInfo);
}

/* ------------------------ End of HMTrain.c ----------------------- */
