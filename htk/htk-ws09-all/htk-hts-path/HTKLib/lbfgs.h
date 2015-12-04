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
/*         File: lbfgs.h: L-BFGS routine                             */
/* ----------------------------------------------------------------- */

/* 

 This L-BFGS FORTRAN source code is originally distributed in

   http://www.ece.northwestern.edu/~nocedal/lbfgs.html

 We thank Prof. Jorge Nocedal of Northwestern University for permission to 
 redistribute this LBFGS code in the HTS releases.


 L-BFGS: Software for Large-scale Unconstrained Optimization
   
   L-BFGS is a limited-memory quasi-Newton code for unconstrained optimization. 
   The code has been developed at the Optimization Technology Center, a joint 
   venture of Argonne National Laboratory and Northwestern University.

 Condition for Use: 

   This software is freely available for educational or commercial purposes. We 
   expect that all publications describing work using this software quote at 
   least one of the references given below.

 References

   * J. Nocedal, "Updating Quasi-Newton Matrices with Limited Storage," 
     Mathematics of Computation 35, pp. 773-782, 1980.

   * D.C. Liu and J. Nocedal, "On the Limited Memory BFGS Method for Large Scale 
     Optimization," Mathematical Programming B, 45, 3, pp. 503-528, 1989.

*/

#ifndef _LBFGS_H_
#define _LBFGS_H_

#ifdef __cplusplus
extern "C" {
#endif 

void lbfgs_(int* n, int* m, double* x, double* f, double* g,
            int* diagco, double* diag, int* iprint, double* eps,
            double* xtol, double* w, int* iflag);

#ifdef __cplusplus
}
#endif

#endif  /* _LBFGS_H_ */

/* ------------------------ End of lbfgs.h ------------------------- */
