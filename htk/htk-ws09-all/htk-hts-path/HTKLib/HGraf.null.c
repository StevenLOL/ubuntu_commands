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
/*         File: HGraf.null.c:  HGraf (null implementation)    */
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

char *hgraf_version = "!HVER!HGraf(null):   3.4.1 [CUED 12/03/09]";
char *hgraf_vc_id = "$Id: HGraf.null.c,v 1.10 2011/06/16 04:18:29 uratec Exp $";

#include "HShell.h"
#include "HMem.h"
#include "HMath.h"
#include "HGraf.h"

/* --------------------------- Initialisation ---------------------- */

static ConfParam *cParm[MAXGLOBS];      /* config parameters */
static int nParm = 0;
static int trace = 0;                   /* Just for consistency */

/* EXPORT->InitGraf: initialise memory and configuration parameters */
void InitGraf(void)
{
   int i;
   
   Register(hgraf_version,hgraf_vc_id);
   nParm = GetConfig("HGRAF", TRUE, cParm, MAXGLOBS);
   if (nParm>0){
      if (GetConfInt(cParm,nParm,"TRACE",&i)) trace = i;
   }
}

/* EXPORT->ResetGraf: reset the module */
void ResetGraf(void)
{
   return;  /* do nothing */
}

/*
   This is the null implementation of HGraf.
*/

/* EXPORT->HGetEvent: return next relevant event in event queue */
HEventRec HGetEvent(Boolean anyEvent, void (*action)(void))
{
   HEventRec r={0,0,0,0,0};
   return r;
}

/* EXPORT->HEventsPending: Return number of events pending */
int HEventsPending(void)
{
   return 0;
}

/* EXPORT->HMousePos: return mouse pos in x, y, returns TRUE if the pointer 
   is on the window */
Boolean HMousePos(int *x, int *y)
{
   return FALSE;
}

/* EXPORT: IsInRect: return TRUE iff (x,y) is in the rectangle (x0,y0,x1,y1) */ 
Boolean IsInRect(int x, int y, int x0, int y0, int x1, int y1)
{
   return FALSE;
}

/* EXPORT-> HSetColour: Set current colour to c */
void HSetColour(HColour c)
{
}

/* EXPORT-> HSetGrey: Set current colour to grey level g */
void HSetGrey(int g)
{
}

/* EXPORT-> HDrawLines: Draw multiple lines */
void HDrawLines(HPoint *points, int n)
{
}

/* EXPORT-> HDrawRectangle: draw a rectangle */
void HDrawRectangle(int x0, int y0, int x1, int y1)
{
}

/* EXPORT-> HFillRectangle: fill a rectangle */
void HFillRectangle(int x0, int y0, int x1, int y1)
{
}

/* EXPORT-> HDrawLines: Draw multiple lines */
void HDrawLine(int x0, int y0, int x1, int y1)
{
}

/* EXPORT-> HFillPolygon: fill a convex polygon */
void HFillPolygon(HPoint *points, int n)
{
}

/* EXPORT-> HDrawArc: Draw arc from stAngle thru arcAngle degrees */
void HDrawArc(int x0, int y0, int x1, int y1, int stAngle, int arcAngle)
{
}

/* EXPORT-> HFillArc: Draw filled arc from stAngle thru arcAngle degrees */
void HFillArc(int x0,int y0,int x1,int y1,int stAngle,int arcAngle)
{
}

/* EXPORT-> HPrintf: works as printf on the graphics window at (x,y) */
void HPrintf(int x, int y, char *format, ...)
{
}

/* EXPORT-> copy rectangular area of the drawable */
void HCopyArea(int srcx, int srcy, int width, int height, int destx, int desty)
{
}

/* EXPORT-> HPlotVector: plot vector v in given rectangle */
void HPlotVector(int x0, int y0, int x1, int y1, Vector v, int st, int en, float ymax, float ymin)
{
}

/* ----------------------------- Global Settings ------------------------------- */

/* EXPORT-> HSetFontSize: Set font size in points, 0 selects the default font */
void HSetFontSize(int size)
{  
}

/* EXPORT-> HSetLineWidth: set the line width */
void HSetLineWidth(int w)
{
}

/* EXPORT-> HSetXMode: Set current transfer mode */
void HSetXMode(XferMode m)
{ 
}

/* EXPORT-> CentreX: return position at which the the h-center of str will be at x */
int CentreX(int x, char *str)
{
   return 0;
}

/* EXPORT-> CentreY: return position at which the the v-center of str will be at y */
int CentreY(int y, char *str)
{
   return 0;
}

/* EXPORT HTextWidth: return the width of s in pixels */
int HTextWidth(char *str)
{
   return 0;
}

/* --------------------------- Misc/Button Routines -----------------------------*/

/* EXPORT->HDrawImage: draw grey scale image stored in p */
void HDrawImage(unsigned char *p, int x, int y, int width, int height)
{
}

/* EXPORT->HFlush: flush any pending draw operations */
void HFlush(void)
{
}

/* EXPORT-> HSpoolGraf: start saving an image of window in fname */
void HSpoolGraf(char *fname)
{
}

/* EXPORT->HDumpGraf: dump a pixel image of current display into fname */
void HDumpGraf(char *fname)
{
}
/* EXPORT->CreateHButton: create a button object with the specified parameters */
HButton *CreateHButton(HButton *btnlst, ButtonId btnid, int x, int y, int w, 
                       int h, char *str, HColour fg, HColour bg, void (*action)(void))
{
   return NULL;
}

/* EXPORT->RedrawHButton: readraw a single button object */
void RedrawHButton(HButton *btn)
{
}

/* EXPORT->RedrawHButtonList: redraw the whole list of buttons */
void RedrawHButtonList(HButton *btnlst)
{
}

/* EXPORT->FindButton: find button given name */
HButton *FindButton(HButton *btnlst, ButtonId key)
{
   return NULL;
}

/* EXPORT->SetActive: set active field in button list */
void SetActive(HButton *btnlst, Boolean active)
{
}

/* EXPORT->CheckButtonList: find within which button the point(x,y) is */
HButton *CheckButtonList(HButton *btnlst, int x, int y)
{
   return NULL;
}

/* EXPORT->SetButtonLit: show button press */
void SetButtonLit(HButton *btn, Boolean lit)
{
}

/* EXPORT->TrackButtons: tracks the buttons until the mouse button is released */
ButtonId TrackButtons(HButton *btnlist, HEventRec hev)
{
   return 0;
}

/* EXPORT-> MakeXGraf: Connect to the X-server and init globals */
void MakeXGraf(char *wname, int x, int y, int w, int h, int bw)
{
   HError(6870,"MakeXGraf: Not compiled with X11 support: use HGraf.X.c");
}

/* EXPORT->TermHGraf: Terminate Graphics (also called via at_exit) */
void TermHGraf()
{
}

/* EXPORT HTextHeight: return the height of s in pixels */
int HTextHeight(char *str)
{
   return 0;
}

/* ------------------------ End of HGraf.null.c -------------------- */
