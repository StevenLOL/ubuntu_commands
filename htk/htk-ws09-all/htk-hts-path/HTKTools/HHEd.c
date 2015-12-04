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
/*          2002-2004 Cambridge University                     */
/*                    Engineering Department                   */
/*                                                             */
/*   Use of this software is governed by a License Agreement   */
/*    ** See the file License for the Conditions of Use  **    */
/*    **     This banner notice must not be removed      **    */
/*                                                             */
/* ----------------------------------------------------------- */
/*         File: HHEd:  HMM Source Definition Editor           */
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

char *hhed_version = "!HVER!HHEd:   3.4.1 [CUED 12/03/09]";
char *hhed_vc_id = "$Id: HHEd.c,v 1.105 2011/06/16 05:18:47 uratec Exp $";

/*
   This program is used to read in a set of HMM definitions
   and then edit them according to the contents of a 
   file of edit commands.  See the routine called Summary for
   a list of these or run HHEd with the command option
*/

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
#include "HTrain.h"
#include "HAdapt.h"

#define FLOAT_MAX 1E10      /* Limit for float arguments */
#define BIG_FLOAT 1E20      /* Limit for float arguments */
#define MAX_ITER  500        /* Maximum number of iterations */

                             /* Lowest level of tracing reset at end of each command block */
#define T_BAS 0x0001        /* Basic progess tracing */
#define T_INT 0x0002        /* Intermediate progress tracing */
#define T_DET 0x0004        /* Detailed progress tracing */
#define T_ITM 0x0008        /* Show item lists */
#define T_MEM 0x0010        /* Show state of memory (once only) */

/* General tracing flags from command line */
#define T_MAC 0x0040        /* Trace changes to macro definitions */
#define T_SIZ 0x0080        /* Trace changes to stream widths */

/* Detailed control over clustering output */
#define T_CLUSTERS   0x0100 /* Show contents of clusters */
#define T_QST        0x0200 /* Show items specified by questions */
#define T_TREE_ANS   0x0400 /* Trace best unseen triphone tree filtering */
#define T_TREE_BESTQ 0x0800 /* Trace best question for splitting each node */
#define T_TREE_BESTM 0x1000 /* Trace best merge of terminal nodes  */
#define T_TREE_OKQ   0x2000 /* Trace all questions exceeding threshold */
#define T_TREE_ALLQ  0x4000 /* Trace all questions */
#define T_TREE_ALLM  0x8000 /* Trace all possible merges */

/* Extra flags to allow easier multiple checks */
#define T_TREE       0xf400 /* Any specific tree tracing */
#define T_IND        0x0006 /* Intermediate or detailed tracing */
#define T_BID        0x0007 /* Basic, intermediate or detailed tracing */
#define T_MD         0x10000 /* Trace mix down detail merge */

static MemHeap questHeap;   /* Heap holds all questions */
static MemHeap hmmHeap;     /* Heap holds all hmm related info */
static MemHeap tmpHeap;     /* Temporary (duration of command or less) heap */
static Vector vf[SMAX];     /* variance flooring */

/* Global Settings */

static char * hmmDir = NULL;     /* directory to look for hmm def files */
static char * hmmExt = NULL;     /* hmm def file extension */
static char * newDir = NULL;     /* directory to store new hmm def files */
static char * newExt = NULL;     /* extension of new edited hmm files */
static char * hmmListFn = NULL;  /* HMM list file */
static Boolean noAlias = FALSE;  /* set to zap all aliases in hmmlist */
static Boolean inBinary = FALSE; /* set to save models in binary */
static char * mmfFn  = NULL;     /* output MMF file, if any */
static int  cmdTrace    = 0;     /* trace level from command line */
static int  trace    = 0;        /* current trace level */

/* Global Data Structures */

static HMMSet hSet;        /* current HMM set */
static HMMSet *hset;       /* current HMM set */
static int fidx;           /* current macro file id */
static int maxStates;      /* max number of states in current HMM set */
static int maxMixes;       /* max number of mixes in current HMM set */

static Source source;      /* the current input file */

static MixtureElem *joinSet;           /* current join Mix Set */
static int nJoins;                     /* current num mixs in joinSet */
static int joinSize=0;                 /* number of mixes in a joined pdf */
static float joinFloor;                /* join mix weight floor (* MINMIX) */
static Boolean badGC = FALSE;          /* set TRUE if gConst out of date */
static float meanGC,stdGC;             /* mean and stdev of GConst */
static Boolean occStatsLoaded = FALSE; /* set when RO/LS has loaded occ stats */
static float outlierThresh = -1.0;     /* outlier threshold set by RO cmd */
static IntSet streams;                 /* information for tree-based clustering */

static int thisCommand;                /* index of current command */
static int lastCommand=0;              /* index of previous command */
static Boolean equivState = TRUE;      /* TRUE if states can be equivalent */
                                       /*  but not identical */
static Boolean useModelName = TRUE;    /* Use base-phone name as tree name */
static Boolean usePattern   = FALSE;   /* Use pattern as tree name */
static int useRefTree = -1;            /* Use reference tree for tree-based clustering */
static Boolean saveHMMSet   = TRUE;    /* Save the HMMSet */

/* information about transforms */
static XFInfo xfInfo;                  /* for AX command */

/* ---------------- Configuration Parameters --------------------- */

static ConfParam *cParm[MAXGLOBS];
static int nParm = 0;            /* total num params */
static Boolean treeMerge = TRUE; /* After tree spltting merge leaves */
static char tiedMixName[MAXSTRLEN] = "TM"; /* Tied mixture base name */
static Boolean useLeafStats = TRUE; /* Use leaf stats to init macros */
static char mmfIdMask[MAXSTRLEN] = "*";       /* MMF Id Mask for baseclass */
static Boolean applyVFloor = TRUE; /* apply modfied varFloors to vars in model set */ 
static Boolean singleTree = FALSE;            /* construct a tree for each state position */
static Boolean applyMDL = FALSE;              /* apply MDL principle for clustering */
static float MDLfactor = 1.0;                 /* MDL control factor */
static Boolean ignoreStrW = FALSE;            /* ignore stream weight */
static float minVar = 1.0E-6;                 /* minimum variance for clusterd item */
static int reduceMem = 0;                     /* reduce memory requirement for decision-tree clustering (0: no reduction, 1: mid reduction but fast, 2: large reduction but slow */
static float minLeafOcc = 0.0;                /* minimum occ for each leaf node */
static float minMixOcc = 0.0;                 /* minimum occ for each mix */
static Vector shrinkOccThresh=NULL;           /* occupancy threshold for shrinking decision trees */

/* ------------------ Process Command Line -------------------------- */

void SetConfParms(void)
{
   char buf[MAXSTRLEN];
   Boolean b;
   double f;
   int i;

   nParm = GetConfig("HHED", TRUE, cParm, MAXGLOBS);
   if (nParm>0) {
      if (GetConfInt(cParm,nParm,"TRACE",&i)) trace = i;
      if (GetConfBool(cParm,nParm,"TREEMERGE",&b)) treeMerge = b;
      if (GetConfBool(cParm,nParm,"USELEAFSTATS",&b)) useLeafStats = b;
      if (GetConfBool(cParm,nParm,"APPLYVFLOOR",&b)) applyVFloor = b;
      if (GetConfBool(cParm,nParm,"USEMODELNAME",&b)) useModelName = b;
      if (GetConfBool(cParm,nParm,"USEPATTERN",&b)) usePattern = b;
      if (GetConfBool(cParm,nParm,"SINGLETREE",&b)) singleTree = b;
      if (GetConfBool(cParm,nParm,"APPLYMDL",&b)) applyMDL = b;
      if (GetConfBool(cParm,nParm,"IGNORESTRW",&b)) ignoreStrW = b;
      if (GetConfInt(cParm,nParm,"REDUCEMEM",&i)) reduceMem = i;
      if (GetConfFlt(cParm,nParm,"MINVAR",&f)) minVar = f;
      if (GetConfFlt(cParm,nParm,"MDLFACTOR",&f)) MDLfactor = f;
      if (GetConfFlt(cParm,nParm,"MINLEAFOCC",&f)) minLeafOcc = f;
      if (GetConfFlt(cParm,nParm,"MINMIXOCC",&f)) minMixOcc = f;
      if (GetConfStr (cParm,nParm,"SHRINKOCCTHRESH",buf))
         shrinkOccThresh = ParseConfVector(&gstack,buf,TRUE);
      GetConfStr(cParm,nParm,"TIEDMIXNAME",tiedMixName);
      GetConfStr(cParm,nParm,"MMFIDMASK",mmfIdMask);
   }
}

void Summary(void)
{
   printf("\nModified for HTS\n");
   printf("\nHHEd Command Summary\n\n");
   printf("AT i j prob itemlist - Add Transition from i to j in given mats\n");
   printf("AU hmmlist           - Add Unseen triphones in given hmmlist to\n");
   printf("                       currently loaded HMM list using previously\n");
   printf("                       built decision trees.\n");
   printf("AX filename          - Set the Adapt XForm to filename\n");
   printf("AX filename smtable L2.mmf L2.list out.mmf\n");
   printf("                     - Set filename xform to L2 model using state mapping table\n");
   printf("CL hmmList           - CLone hmms to give new hmmList\n");
   printf("CM directory         - Convert models to pdf for speech synthesizer\n");
   printf("CO newHmmList        - COmpact identical HMM's by sharing same phys model\n");
   printf("CT directory         - Convert trees/questions for speech synthesizer\n");
   printf("DM type macroname    - Delete macro from model-set\n");
   printf("DP s n id ...        - Duplicate the hmm set n times using id to differentiate\n");
   printf("                       the new hmms and macros.  Only macros the type of which\n");
   printf("                       appears in s will be duplicated, others will be shared.\n");
   printf("DR id                - Convert decision trees to a regression tree.\n");
   printf("DV                   - Convert full covariance to diagonal variances\n"); 
   printf("FA f                 - Set variance floor to average within state variance * f\n");
   printf("FV vFloorfile        - Load variance floor from file\n");
   printf("FC                   - Convert diagonal variances to full covariances\n");
   printf("HK hsetkind          - Change current set to hsetkind\n");
   printf("IT filename          - Clustering while imposing loaded tree structure\n");
   printf("                       If any empty leaf nodes exist, loaded trees are pruned \n");
   printf("                       and then saved to filename\n");
   printf("IX filename          - Set the Input Xform to filename\n");
   printf("JO size floor        - Set size and min mix weight for a JOin\n");
   printf("JM hmmFile itemlist  - Join Models on stream or state level\n");
   printf("LS statsfile         - Load named statsfile\n");
   printf("LT filename          - Load Questions and Trees from filename\n");
   printf("MD n itemlist        - MixDown command, change mixtures in itemlist to n\n");
   printf("MM s itemlist        - Make each item in list into a macro with usage==1\n");
   printf("MT triHmmList        - Make Triphones from loaded biphones\n");
   printf("MU n itemlist        - MixUp command, change mixtures in itemlist to n\n");
   printf("NC n macro itemlist  - N-Cluster specified components and tie\n");
   printf("PR                   - Convert model-set with PROJSIZE to compact form\n");
   printf("                       also supplying a regression tree identifier/label name\n");
   printf("                       Optional itemList to specify non-speech sounds\n");
   printf("PX filename          - Set the Parent Xform to filename\n");
   printf("QS name itemlist     - Define a question as a list of model names\n");
   printf("RC n id [itemList]   - Build n regression classes (for adaptation purposes)\n");
   printf("                       this disables the storing of the models\n");  
   printf("RM hmmfile           - Rem mean in state 2, mix 1 of hmmfile from all \n");
   printf("                       loaded models nb. whole mean is removed incl. dels\n");
   printf("RN hmmSetIdentifier  - Rename the hmm mmf with a new identifier name\n");
   printf("                       If omitted and MMF contains no identifier, then\n");
   printf("                       the MMF is given the identifier \"Standard\"\n");
   printf("RO f [statsfile]     - Remove outliers with counts < f as the\n");
   printf("                       final phase in the TC/NC commands.  If statsfile\n");
   printf("                       is omitted, it must be already loaded (see LS)\n");
   printf("RT i j itemlist      - Rem Transition from i to j in given mats\n");
   printf("SH                   - Show the current HMM set (for debugging)\n");
   printf("SK sk                - Set sample kind of all models to sk\n");
   printf("SM smtable mmf hlist - Output KLD-based state mapping table\n");
   printf("SS n                 - Split into n data Streams\n");
   printf("ST filename          - Save Questions and Trees to filename\n");
   printf("SU n w1 .. wn        - Split into user defined stream widths\n");
   printf("SW s n               - Set width of stream s to n\n");
   printf("TB f macro itemlist  - Tree build using QS questions and likelihood\n");
   printf("                       based clustering criterion.\n");
   printf("TC f macro itemlist  - Thresh Cluster specified comps to thresh f and tie\n");
   printf("TI macro itemlist    - TIe the specified components\n");
   printf("TR n                 - Set trace level to n (overrides -T option)\n");
   printf("UT itemlist          - UnTie the specified components\n");
   printf("// comment           - Comment line (ignored)\n");
   Exit(0);
}

void ReportUsage(void)
{
   printf("\nModified for HTS\n");
   printf("\nUSAGE: HHEd [options] editF hmmList\n\n");
   printf(" Option                                       Default\n\n");
   printf(" -a f    factor to control the second term in the MDL      1.0\n");
   printf(" -d s    dir to find hmm definitions          current\n");
   printf(" -i      ignore stream weight                              off\n");
   printf(" -m      apply MDL principle for clustering                off\n");
   printf(" -o s    extension for new hmm files          as source\n");
   printf(" -p      use pattern instead of base phone                 off\n");
   printf(" -q n    use reference tree for clustering                 off\n");
   printf("          0: clustering is stopped by threshold\n");
   printf("          1: clustering is stopped when leaf don't have occ\n");
   printf("          2: clustering is stopped by threshold\n");
   printf("             After that, standard clustering is not performed\n");
   printf("          3: clustering is stopped when leaf don't have occ\n");
   printf("             After that, standard clustering is not performed\n");
   printf(" -r n    reduce memory usage on clustering                 0\n");
   printf("          0: no memory reduction                           \n");
   printf("          1: mid reduction but fast                        \n");
   printf("          2: large reduction but slow                      \n");
   printf(" -s      construct single tree                             off\n");
   printf(" -v f    Set minimum variance to f                         1.0E-6\n");
   printf(" -w mmf  Save all HMMs to macro file mmf s    as source\n");
   printf(" -x s    extension for hmm files              none\n");
   printf(" -z      zap aliases in hmmList\n");
   PrintStdOpts("BHMQ");
}

int main(int argc, char *argv[])
{
   char *s, *editFn;
   void DoEdit(char * editFn);
   void ZapAliases(void);
   void Initialise(char *hmmListFn);
   
   if(InitShell(argc,argv,hhed_version,hhed_vc_id)<SUCCESS)
      HError(2600,"HHEd: InitShell failed");
   InitMem();   InitLabel();
   InitMath();  InitSigP();
   InitWave();  InitAudio();
   InitVQ();    InitModel();
   if(InitParm()<SUCCESS)  
      HError(2600,"HHEd: InitParm failed");
   InitUtil();
   InitAdapt(&xfInfo,NULL);

   if (!InfoPrinted() && NumArgs() == 0)
      ReportUsage();
   if (NumArgs() == 0) Exit(0);
   SetConfParms();
 
   CreateHeap(&hmmHeap,"Model Heap",MSTAK,1,1.0,40000,1600000);
   CreateHMMSet(&hSet,&hmmHeap,TRUE);hset=&hSet;fidx=0;

   while (NextArg() == SWITCHARG) {
      s = GetSwtArg();
      if (strlen(s)!=1) 
         HError(2619,"HHEd: Bad switch %s; must be single letter",s);
      switch(s[0]) {
      case 'a':
         MDLfactor = GetChkedFlt(0.0,100000000.0,s); break;
      case 'd':
         if (NextArg()!=STRINGARG)
            HError(2619,"HHEd: Input HMM definition directory expected");
         hmmDir = GetStrArg(); break;  
      case 'i':
         ignoreStrW = TRUE; break;
      case 'm':
         applyMDL = TRUE; break;
      case 'o':
         if (NextArg()!=STRINGARG)
            HError(2619,"HHEd: Output HMM file extension expected");
         newExt = GetStrArg(); break;
      case 'p':
         usePattern = TRUE; break;
      case 'q':
         useRefTree = GetChkedInt(0,3,s); break;
      case 'r':
         reduceMem = GetChkedInt(0,2,s); break;
      case 's':
         singleTree = TRUE; break;
      case 'v':
         minVar = GetChkedFlt(0.0,100.0,s); break;
      case 'w':
         if (NextArg()!=STRINGARG)
            HError(2619,"HHEd: Output MMF file name expected");
         mmfFn = GetStrArg();
         break;
      case 'x':
         if (NextArg()!=STRINGARG)
            HError(2619,"HHEd: Input HMM file extension expected");
         hmmExt = GetStrArg(); break;
      case 'z':
         noAlias = TRUE; break;
      case 'B':
         inBinary=TRUE; break;
      case 'J':
         if (NextArg()!=STRINGARG)
            HError(2319,"HERest: input transform directory expected");
         AddInXFormDir(hset,GetStrArg());
         break;              
      case 'H':
         if (NextArg()!=STRINGARG)
            HError(2619,"HHEd: Input MMF file name expected");
         AddMMF(hset,GetStrArg());
         break;
      case 'M':
         if (NextArg()!=STRINGARG)
            HError(2619,"HHEd: Output HMM definition directory expected");
         newDir = GetStrArg(); break;  
      case 'Q':
         Summary(); break;
      case 'T':
         trace = cmdTrace = GetChkedInt(0,0x3FFFF,s); break;
      default:
         HError(2619,"HHEd: Unknown switch %s",s);
      }
   } 
   if (NextArg()!=STRINGARG)
      HError(2619,"HHEd: Edit script file name expected");
   editFn = GetStrArg();
   if (NextArg() != STRINGARG)
      HError(2619,"HHEd: HMM list file name expected");
   if (NumArgs()>1)
      HError(2619,"HHEd: Unexpected extra args on command line");

   hmmListFn = GetStrArg();
   Initialise(hmmListFn);

   if (hset->logWt == TRUE) HError(999,"HHEd requires linear weights");

   DoEdit(editFn);

   /* reset heaps */
   ResetHeap(&hmmHeap);
   ResetHeap(&tmpHeap);
   ResetHeap(&questHeap);
   
   /* reset modules */
   ResetAdapt(&xfInfo,NULL);
   ResetUtil();   
   ResetParm();
   ResetModel();
   ResetVQ();
   ResetAudio();
   ResetWave();
   ResetSigP();
   ResetMath();
   ResetLabel();
   ResetMem();
   ResetShell();
   
   Exit(0);
   return (0);          /* never reached -- make compiler happy */
}

/* ----------------------- Lexical Routines --------------------- */

int ChkedInt(char *what,int min,int max)
{
   int ans;

   if (!ReadInt(&source,&ans,1,FALSE))
      HError(2650,"ChkedInt: Integer read error - %s",what);
   if (ans<min || ans>max)
      HError(2651,"ChkedInt: Integer out of range - %s",what);
   return(ans);
}

float ChkedFloat(char *what,float min,float max)
{
   float ans;

   if (!ReadFloat(&source,&ans,1,FALSE))
      HError(2650,"ChkedFloat: Float read error - %s",what);
   if (ans<min || ans>max)
      HError(2651,"ChkedFloat: Float out of range - %s",what);
   return(ans);
}

char *ChkedAlpha(char *what,char *buf)
{
   if (!ReadString(&source,buf))
      HError(2650,"ChkedAlpha: String read error - %s",what);
   return(buf);
}

/* ------------- Question Handling for Tree Building ----------- */

static int nQuestions=0;
static int nPatterns=0;
static char **QMTable=NULL;
static Boolean setQMTable=FALSE;

typedef struct {
   char *pat;
   int index;
}IPat;

typedef struct _Question{      /* each question stored as both pattern and  */
   LabId qName;                 /* an expanded list of model names */
   ILink patList;               
   ILink ilist;
   char pattern[PAT_LEN];
   int index;
   Boolean used;
} Question;

static ILink pList = NULL;      /* pattern list */
static ILink qList = NULL;      /* question list */

/* TraceQuestion: output given questions */
void TraceQuestion(char *cmd, Question *q)
{
   ILink ip;
   
   printf("   %s %s: defines %d % d models\n       ",
          cmd,q->qName->name,NumItems(q->ilist),hset->numLogHMM);
   for (ip=q->patList; ip!=NULL; ip=ip->next)
      printf("%s ",((IPat *)ip->item)->pat);
   printf("\n");
   fflush(stdout);
}

/* ParseAlpha: get next string from src and store it in s */
/* This is a copy of ParseString with extra terminators */
char *ParseAlpha(char *src, char *s)
{
   static char term[]=".,)";      /* Special string terminators */
   Boolean wasQuoted;
   int c,q=0;

   wasQuoted=FALSE;
   while (isspace((int) *src)) src++;
   if (*src == DBL_QUOTE || *src == SING_QUOTE){
      wasQuoted = TRUE; q = *src;
      src++;
   }
   while(*src) {
      if (wasQuoted) {
         if (*src == q) return (src+1);
      } else {
         if (isspace((int) *src) || strchr(term,*src)) return (src);
      }
      if (*src==ESCAPE_CHAR) {
         src++;
         if (src[0]>='0' && src[1]>='0' && src[2]>='0' &&
             src[0]<='7' && src[1]<='7' && src[2]<='7') {
            c = 64*(src[0] - '0') + 8*(src[1] - '0') + (src[2] - '0');
            src+=2;
         } else
            c = *src;
      }
      else c = *src;
      *s++ = c;
      *s=0;
      src++;
   }
   return NULL;
}

/* ParsePattern: parse given string and return pattern list */
void ParsePattern (ILink *patList, char *pattern)
{
   char *p,*r,buf[PAT_LEN];
   IPat *ip=NULL;
   ILink i;
   
   for (p=pattern;*p && isspace((int) *p);p++);
   
   /* parse head '{' and '(' */
   if (*p!='{')
      if (p==NULL) HError(2660,"ParsePattern: no { in itemlist");
   ++p;
   if (*p=='(')
      ++p;
   
   /* parse tail '}' and ')' */
   for (r=pattern+strlen(pattern)-1;r>=pattern && isspace((int) *r);r--);
   if (*r!='}') HError(2660,"ParsePattern: no } in itemlist");
   if (pattern<r && *(r-1)==')') 
      --r;
   *r = ',';
   
   /* parse model patterns from item list */
   do { 
      p=ParseAlpha(p,buf);
      while(isspace((int) *p)) p++;
      if (*p!=',')
         HError(2660,"ParsePattern: missing , in itemlist"); 
      p++;
      for (i=pList; i!=NULL; i=i->next)
	if (strcmp(((IPat *)i->item)->pat,buf)==0)
           break;
      if (i==NULL) {
      ip=(IPat*) New(&questHeap,sizeof(IPat));
      ip->pat = NewString(&questHeap,strlen(buf));
      ip->index = nPatterns++;
      strcpy(ip->pat,buf);
      AddItem(NULL,ip,&pList);
      }
      else {
         ip=(IPat *)i->item;
      }
      AddItem(NULL,ip,patList);
   } while (p<r);
   
   return;
}

/* LoadQuestion: store given question in question list */
void LoadQuestion(char *qName, ILink ilist, char *pattern)
{
   ILink i;
   Question *q,*c=NULL;
   LabId labid;
   
   labid=GetLabId(qName,TRUE);
   for (i=qList;i!=NULL;i=i->next) {
      c = (Question *)i->item;
      if (c->qName==labid) break;
   }
   
   if (i!=NULL) {
      if (strcmp(c->pattern,pattern)!=0 && useRefTree < 0)
         HError(2661,"LoadQuestion: Question name %s invalid",qName);
      else
         return;
   }
   
   q=(Question *) New(&questHeap,sizeof(Question));
   q->used=FALSE; q->qName=labid; q->patList=NULL; q->index=nQuestions++;
   q->ilist = ilist;
   labid->aux=q; 
   strcpy(q->pattern, pattern);
   AddItem(NULL,q,&qList);

   ParsePattern(&q->patList,pattern);
}

/* IPatMatch: return true if given name matches pattern list */
Boolean IPatMatch (char *name, ILink patList)
{
   ILink ip;
   
   for (ip=patList; ip!=NULL; ip=ip->next)
      if (DoMatch(name, ((IPat *)(ip->item))->pat)) return TRUE;
   return FALSE;
}

/* QMatch: return true if given name matches question */
Boolean QMatch(char *name, Question *q)
{
   return IPatMatch(name, q->patList);
}

/* ----------------------- HMM Management ---------------------- */

typedef enum { baseNorm=0, baseLeft, baseRight, baseMono } baseType;

/* FindBaseModel: return the idx in hList of base model corresponding 
   to id, type determines type of base model wanted and is 
   baseMono (monophone); baseRight (right biphone); baseLeft (left biphone) */
HLink FindBaseModel(HMMSet *hset,LabId id,baseType type)
{
   char baseName[MAXSTRLEN],buf[MAXSTRLEN],*p;
   LabId baseId;
   MLink ml;
   
   strcpy(baseName,id->name);
   if (type==baseMono || type==baseRight) { /* strip Left context */
      strcpy(buf,baseName);
      if ((p = strchr(buf,'-')) != NULL)
         strcpy(baseName,p+1);
   }
   if (type==baseMono || type==baseLeft) { /* strip Right context */
      strcpy(buf,baseName);
      if ((p = strchr(buf,'+')) != NULL) {
         *p = '\0';
         strcpy(baseName,buf);
      }
   }
   if ((baseId = GetLabId(baseName,FALSE))==NULL)
      HError(2635,"FindBaseModel: No Base Model %s for %s",baseName,id->name);
   ml = FindMacroName(hset,'l',baseId);
   if (ml==NULL)
      HError(2635,"FindBaseModel: Cannot Find HMM %s in Current List",baseName);
   return ((HLink) ml->structure);
}

/* OutMacro: output the name of macro associated with structure */
void OutMacro(char type,Ptr structure)
{
   MLink ml;
   
   if ((ml = FindMacroStruct(hset,type,structure))==NULL)
      HError(2635,"OutMacro: Cannot find macro to print");
   printf(" ~%c:%s",type,ml->id->name);
}

/* ShowWhere: print state, stream, mix if necessary */
void ShowWhere(int state, int stream, int mix)
{
   static int j = -1;
   static int s = -1;
   static int m = -1;
   
   if (state != j) {
      printf("\n  State %d: ",state);
      s = -1; m = -1; j = state;
   }
   if (stream != s && stream != -1) {
      printf("\n    Stream %d: ",stream);
      m = -1; s = stream;
   }
   if (mix != m && mix != -1) {
      printf("\n      Mix %d: ",mix);
      m = mix;
   }
}


/* ShowMacros: list macros used by given HMM */
void ShowMacros(HMMDef *hmm)
{
   StateElem *se;
   StateInfo *si;
   MixtureElem *me;
   StreamElem *ste;
   StreamInfo *sti;
   MixPDF *mp;
   Ptr strct;
   int i,j,s;
   char type;
   
   if (GetUse(hmm->transP) > 0) {
      printf("\n   TransP: ");
      OutMacro('t',hmm->transP);
   }
   se = hmm->svec+2;
   for (i=2;i<hmm->numStates;i++,se++) {
      si = se->info;
      if (si->nUse > 0) {
         ShowWhere(i,-1,-1);
         OutMacro('s',si);
      }     
      if (si->dur != NULL && GetUse(si->dur) > 0) {
         ShowWhere(i,-1,-1);
         OutMacro('d',si->dur);
      }
      if (si->weights != NULL && GetUse(si->weights) > 0) {
         ShowWhere(i,-1,-1);
         OutMacro('w',si->weights);
      }
      ste = si->pdf +1;

      if (hset->hsKind==PLAINHS || hset->hsKind==SHAREDHS)
         for (s=1; s<=hset->swidth[0]; s++,ste++) {
            sti = ste->info;
            me = sti->spdf.cpdf + 1;
            for (j=1; j<=sti->nMix;j++,me++) {
               if (me->weight > MINMIX || hset->msdflag[s]) {
                  mp = me->mpdf;
                  if (mp->nUse>0) {
                     ShowWhere(i,s,j); OutMacro('m',mp);
                  }
                  if (GetUse(mp->mean) > 0) {
                     ShowWhere(i,s,j); OutMacro('u',mp->mean);
                  }
                  switch(mp->ckind) {
                  case FULLC:
                     strct=mp->cov.inv; type='i'; break;
                  case LLTC:
                     strct=mp->cov.inv; type='c'; break;
                  case XFORMC:
                     strct=mp->cov.xform; type='x'; break;
                  case DIAGC:
                  case INVDIAGC:
                     strct=mp->cov.var; type='v'; break;
                  default:
                     strct=NULL; type=0; break;
                  }
                  if (type && GetUse(strct) > 0) {
                     ShowWhere(i,s,j);
                     OutMacro(type,strct);
                  }
               }
            }
         }
   }
}

/* ShowHMMSet: print a summary of currently loaded HMM Set */
void ShowHMMSet(void)
{
   HMMScanState hss;
   LabId id;
   int h=0;
   
   printf("Summary of Current HMM Set\n");
   NewHMMScan(hset,&hss);
   do {
      id = hss.mac->id;
      h++;
      printf("\n%d.Name: %s",h,id->name);
      ShowMacros(hss.hmm);
   }
   while(GoNextHMM(&hss));
   printf("\n------------\n"); fflush(stdout);
}

/* ZapAliases: reduce hList to just distinct physical HMM's */
void ZapAliases(void)
{
   int h;
   MLink q;
   HLink hmm;
   
   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next)
         if (q->type=='l')
            DeleteMacro(hset,q);

   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next)
         if (q->type=='h') {
            hmm=(HLink) q->structure;
            NewMacro(hset,fidx,'l',q->id,hmm);
            hmm->nUse=1;
         }
}

/* EquivMix: return TRUE if both mixtures are identical */
Boolean EquivMix(MixPDF *a, MixPDF *b)
{
   if (a->mean != b->mean ||
       a->cov.var != b->cov.var ) return FALSE;
   return TRUE;
}

/* EquivStream: return TRUE if both streams are identical */
Boolean EquivStream(StreamInfo *a, StreamInfo *b)
{
   int m,M;
   MixtureElem *mea,*meb;
   MixPDF *ma,*mb;
   
   mea = a->spdf.cpdf+1; meb = b->spdf.cpdf+1;
   M = a->nMix;
   for (m=1; m<=M; m++,mea++,meb++) {   
      /* This really should search b for matching mixture */
      if (M>1 && mea->weight != meb->weight)
         return FALSE;
      ma = mea->mpdf; mb = meb->mpdf;
      if (ma != mb && !EquivMix(ma,mb))
         return FALSE;
   }
   return TRUE;
}

/* EquivState: return TRUE if both states are identical (only CONT) */
Boolean EquivState(StateInfo *a, StateInfo *b, int S)
{
   int s;
   StreamElem *stea,*steb;
   
   stea = a->pdf+1; steb = b->pdf+1;
   for (s=1; s<=S; s++,stea++,steb++) {
      if ((stea->info->nMix!=steb->info->nMix) || 
          !EquivStream(stea->info,steb->info))
         return FALSE;
   }
   return TRUE;
}

/* EquivHMM: return TRUE if both given hmms are identical */
Boolean EquivHMM(HMMDef *a, HMMDef *b)
{
   int i;
   StateInfo *ai,*bi;
   
   if (a->numStates!=b->numStates) return FALSE;

   for (i=2;i<a->numStates;i++) {
      ai=a->svec[i].info; bi=b->svec[i].info;
      if (ai!=bi && (!equivState ||
                     hset->hsKind==TIEDHS || hset->hsKind==DISCRETEHS ||
                     !EquivState(ai, bi, hset->swidth[0])))
         return FALSE;
   }
   return TRUE;
}

/* PurgeMacros: purge all unused macros */
void PurgeMacros(HMMSet *hset)
{
   int h,n;
   MLink q;
   
   /* First mark all macros with usage > 0 */
   ResetHooks(hset,NULL);
   SetVFloor(hset,NULL,0.0);
   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next) {
         if ((q->type=='*') || (q->type=='a')|| (q->type=='a'))  continue;
         n=GetMacroUse(q);
         if (n>0 && !IsSeen(n)) {
            Touch(&n);
            SetMacroUse(q,n);
         }
      }
   /* Delete unused macros in next pass */
   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next) {
         if ((q->type=='*') || (q->type=='a')|| (q->type=='a')) continue;
         n=GetMacroUse(q);
         if (!IsSeen(n)) {
            SetMacroUse(q,0);
            DeleteMacro(hset,q);
         }
      }
   /* Finally unmark all macros */
   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next) {
         if ((q->type=='*') || (q->type=='a')|| (q->type=='a')) continue;
         n=GetMacroUse(q);
         if (IsSeen(n)) {
            Untouch(&n);
            SetMacroUse(q,n);
         }
      }
}

/* -------------------- Vector/Matrix Resizing Operations -------------------- */

/* SwapMacro: swap macro from old to new, and delete old */
void SwapMacro(HMMSet *hset,char type,Ptr o, Ptr n)
{
   MLink ml;
   
   ml = FindMacroStruct(hset,type,o);
   if (trace & (T_MAC | T_DET)) {
      printf("   Swapping Macro %s to resized vector",ml->id->name);
      fflush(stdout);
   }
   DeleteMacro(hset,ml);
   NewMacro(hset, fidx, type, ml->id, n);
}

void SetVSize(SVector v,int n)
{
   *((int*) v)=n;
}

void SetMSize(STriMat m,int n)
{
   *((int*) m)=n;
}

/* ResizeSVector: resize the given vector to n components, if
   extension is needed, new elements have pad value */
SVector ResizeSVector(HMMSet *hset,SVector v, int n, char type, float pad)
{
   int i,u,z;
   SVector w;

   u = GetUse(v);
   if (u==0 || !IsSeen(u)) {
      if (u!=0)
         TouchV(v);
      z = VectorSize(v);
      if (z >= n) {
         w = v;
         SetVSize(v,n);
      }
      else {
         w = CreateSVector(&hmmHeap,n);
         for (i=1;i<=z;i++) w[i] = v[i];
         for (i=z+1;i<=n;i++) w[i] = pad;
         if (u!=0)
            SwapMacro(hset,type,v,w);
      }
   }
   else {
      w=(SVector) GetHook(v);
      IncUse(w);
   }
   return(w);
}

/* ResizeSTriMat: resize the given matrix to be n x n, if
   extension is needed, new diag elements have pad value
   and the rest are zero */
STriMat ResizeSTriMat(HMMSet *hset,STriMat m, int n, char type, float pad)
{
   int i,j,u,z;
   STriMat w;

   u = GetUse(m);
   if (u==0 || !IsSeen(u)) {
      if (u!=0)
         TouchV(m);
      z = NumRows(m);
      if (z >= n) {
         w = m;
         SetMSize(m,n);
      }
      else {
         w = CreateSTriMat(&hmmHeap,n);
         CovInvert(m,m);
         ZeroTriMat(w);
         for (i=1; i<=z; i++)
            for (j=1;j<=i; j++)
               w[i][j] = m[i][j];
         for (i=z+1; i<=n; i++) 
            w[i][i] = pad;
         CovInvert(w,w);
         if (u!=0)
            SwapMacro(hset,type,m,w);
      }
   }
   else {
      w=(STriMat) GetHook(m);
      IncUse(w);
   }
   return(w);
}

/* ----------------- Stream Splitting Operations ------------------- */

/* SliceVector: return vec[i..j] as a vector */
Vector SliceVector(Vector vec, int i, int j)
{
   Vector w;
   int k,l;
   
   w = CreateSVector(&hmmHeap,j-i+1);
   for (l=1,k=i; k<=j; k++,l++)
      w[l] = vec[k];
   return w;
}

/* SliceTriMat: return submatrix mat[i..j,i..j] */
Matrix SliceTriMat(Matrix mat, int i, int j)
{
   TriMat w;
   int k,l,m,n;
   
   w = CreateSTriMat(&hmmHeap,j-i+1);
   for (l=1,k=i; k<=j; k++,l++)
      for (n=1,m=i; m<=k; m++,n++)
         w[l][n] = mat[k][m];
   return w;
}

/* ChopVector: return i,j,k elements of vec as a vector,
   element k is optional */
Vector ChopVector(Vector vec, int i, int j, int k)
{
   Vector w;
   int size;
   
   size =  (k>0) ? 3 : 2;
   w = CreateSVector(&hmmHeap,size);
   w[1] = vec[i]; w[2] = vec[j];
   if (k>0) w[3] = vec[k];
   return w;
}

/* ChopTriMat: return submatrix formed by extracting the intersections
   of i, j, and k from trimat mat, k'th is optional */
TriMat ChopTriMat(TriMat mat, int i, int j, int k)
{
   TriMat w;
   int size;
   
   size =  (k>0) ? 3 : 2;
   w = CreateSTriMat(&hmmHeap,size);
   w[1][1] = mat[i][i];
   w[2][1] = mat[j][i]; w[2][2] = mat[j][j];
   if (k>0) {
      w[1][3] = mat[i][k];
      w[2][3] = mat[j][k];
      w[3][3] = mat[k][k];
   }
   return w;
}

/* SplitStreams: split streams of given HMM as per swidth info */
void SplitStreams(HMMSet *hset,StateInfo *si,Boolean simple,Boolean first)
{
   int j,s,S,m,M,width,V,next;
   StreamElem *ste,*oldste;
   StreamInfo *sti,*oldsti;
   MixtureElem *me,*oldme;
   MixPDF *mp, *oldmp;
   Boolean hasN;
   int epos[4];
   int eposIdx;
   
   S = hset->swidth[0]; V = hset->vecSize;
   hasN = HasNulle(hset->pkind);
   oldste = si->pdf+1;
   ste = (StreamElem *)New(hset->hmem,S*sizeof(StreamElem));
   si->pdf = ste - 1;
   next = 1;      /* start of next slice to take from src vector */
   eposIdx = 0;
   for (j=0; j<3; j++) epos[j] = 0;
   for (s=1;s<=S;s++,ste++) {
      ste->info = (StreamInfo *)New(hset->hmem,sizeof(StreamInfo));
      sti = ste->info;
      oldsti = oldste->info;
      sti->nUse = oldsti->nUse;  sti->hook = NULL; sti->stream = s;
      width = hset->swidth[s];
      M = sti->nMix = oldsti->nMix;
      sti->hook = NULL;
      me = (MixtureElem *) New(hset->hmem,M*sizeof(MixtureElem));
      sti->spdf.cpdf = me-1;
      oldme=oldsti->spdf.cpdf+1;
      for (m=1; m<=M; m++, me++,oldme++) {
         oldmp = oldme->mpdf;
         if (oldmp->nUse>0)
            HError(2640,"SplitStreams: MixPDF is shared");
         if (GetUse(oldmp->mean) > 0)
            HError(2640,"SplitStreams: Mean is shared");
         switch(oldmp->ckind) {
         case DIAGC:
            if (GetUse(oldmp->cov.var) > 0)
               HError(2640,"SplitStreams: variance is shared");
            break;
         case LLTC:
         case FULLC:
            if (GetUse(oldmp->cov.inv) > 0)
               HError(2640,"SplitStreams: covariance is shared");
            break;
         default:
            HError(2640,"SplitStreams: not implemented for CovKind==%d",
                   oldmp->ckind);
            break;
         }
         me->weight = oldme->weight;
         mp = (MixPDF *) New(hset->hmem,sizeof(MixPDF));
         mp->nUse = 0; mp->hook = NULL; mp->gConst = LZERO;
         mp->ckind = oldmp->ckind;
         me->mpdf = mp;
         if (simple || s<S) {    
            /* straight segmentation of original stream */
            if ((trace & (T_SIZ | T_IND)) && first)
               printf("  Slicing  at stream %d, [%d->%d]\n",
                      s,next,next+width-1);
            mp->mean = SliceVector(oldmp->mean,next,next+width-1);
            if (mp->ckind == DIAGC)
               mp->cov.var = SliceVector(oldmp->cov.var,next,next+width-1);
            else
               mp->cov.inv = SliceTriMat(oldmp->cov.inv,next,next+width-1);
         } else {
            if ((trace & (T_SIZ | T_DET)) && first)
               printf("  Chopping at stream %d, [%d,%d,%d]\n",
                      s,epos[0],epos[1],epos[2]);
            mp->mean = ChopVector(oldmp->mean,epos[0],epos[1],epos[2]);
            if (oldmp->ckind == DIAGC)
               mp->cov.var = ChopVector(oldmp->cov.var,
                                        epos[0],epos[1],epos[2]);
            else
               mp->cov.inv = ChopTriMat(oldmp->cov.inv,
                                        epos[0],epos[1],epos[2]);
         }  
      }
      next += width;
      if (!simple && !(hasN && s==1)) {   /* remember positions of E coefs */
         epos[eposIdx++] = next; ++next;
      }
   }
   si->weights = CreateSVector(hset->hmem,S);
   for (s=1; s<=S; s++)
      si->weights[s] = 1.0;
}


/* -------------------- Tying Operations --------------------- */

/*
  TypicalState: extract a typical state from given list ie the
  state with largest total gConst in stream 1 (indicating broad 
  variances) and as few as possible zero mixture weights
*/
ILink TypicalState(ILink ilist, LabId macId)
{
   ILink i,imax;
   LogFloat gsum,gmax;
   int m,M;
   StateInfo *si;
   StreamInfo *sti;
   
   gmax = LZERO; imax = NULL;
   for (i=ilist; i!=NULL; i=i->next) {
      si = ((StateElem *)(i->item))->info;
      sti = si->pdf[1].info;
      M = sti->nMix;
      gsum = 0.0;
      for (m=1; m<=M; m++)
         if (sti->spdf.cpdf[m].weight>MINMIX)
            gsum += sti->spdf.cpdf[m].mpdf->gConst;
         else
            gsum -= 200.0;      /* arbitrary penaltly */
      if (gsum>gmax) {
         gmax = gsum; imax = i;
      }
   }
   if (imax==NULL) {
      HError(-2638,"TypicalState: No typical state for %s",macId->name);
      imax=ilist;
   }
   return imax;
}

/* TieState: tie all state info fields in ilist to typical state */
void TieState(ILink ilist, LabId macId)
{
   ILink i,ti;
   StateInfo *si,*tsi;
   StateElem *se;
   
   if (badGC) {
      FixAllGConsts(hset);         /* in case any bad gConsts around */
      badGC=FALSE;
   }
   if (hset->hsKind==TIEDHS || hset->hsKind==DISCRETEHS)
      ti=ilist;
   else
      ti = TypicalState(ilist,macId);
   se = (StateElem *) ti->item;
   tsi = se->info;
   if (tsi->nUse>0)
      DeleteMacroStruct(hset,'s',tsi);
   else
   tsi->nUse = 1;
   NewMacro(hset,fidx,'s',macId,tsi);
   for (i=ilist; i!=NULL; i=i->next) {
      se = (StateElem *)i->item; si = se->info;
      if (si != tsi) {
         if (si->nUse>0) si->nUse--;
         se->info = tsi;
         ++tsi->nUse;
      }
   }
}

void TieLeafStream(ILink ilist, LabId macId, int stream)
{
   ILink i;
   StreamInfo *sti;
   
   if (badGC) {
      FixAllGConsts(hset);         /* in case any bad gConsts around */
      badGC=FALSE;
   }
   sti = ((StateElem *)ilist->item)->info->pdf[stream].info;
   sti->nUse = 1;
   NewMacro(hset,fidx,'p',macId,sti);
   for (i=ilist->next; i!=NULL; i=i->next) {
      ((StateElem *)i->item)->info->pdf[stream].info = sti;
      ++sti->nUse;
   }
}

/* TieTrans: tie all transition matrices in ilist to 1st */
void TieTrans(ILink ilist, LabId macId)
{
   ILink i;
   SMatrix tran;
   HMMDef *hmm;
   
   hmm = ilist->owner;
   tran = hmm->transP;
   if (GetUse(tran)>0)
      DeleteMacroStruct(hset,'t',tran);
   else
   SetUse(tran,1); 
   NewMacro(hset,fidx,'t',macId,tran);
   for (i=ilist->next; i!=NULL; i=i->next) {
      hmm = i->owner;
      if (hmm->transP != tran) {
         if (GetUse(hmm->transP)>0) DecUse(hmm->transP);
         hmm->transP = tran;
         IncUse(tran);
      }
   }
}

/* TieDur: tie all dur vectors in ilist to 1st */
void TieDur(ILink ilist, LabId macId)
{
   ILink i;
   StateInfo *si;
   SVector v;
   
   si = (StateInfo *)ilist->item;
   if ((v=si->dur)==NULL)
      HError(2630,"TieDur: Attempt to tie null duration as macro %s",
             macId->name);
   if (GetUse(v)>0)
      DeleteMacroStruct(hset,'d',v);
   else
   SetUse(v,1);
   NewMacro(hset,fidx,'d',macId,v);
   for (i=ilist->next; i!=NULL; i=i->next) {
      si = (StateInfo *)i->item;
      if (si->dur==NULL)
         HError(-2630,"TieDur: Attempt to tie null duration to macro %s",
                macId->name);
      if (si->dur != v) {
         if (GetUse(si->dur)>0) DecUse(si->dur);
         si->dur = v;
         IncUse(v);
      }
   }
}

/* TieWeights: tie all stream weight vectors in ilist to 1st */
void TieWeights(ILink ilist, LabId macId)
{
   ILink i;
   StateInfo *si;
   SVector v;
   
   si = (StateInfo *)ilist->item;
   v = si->weights;
   if (v==NULL)
      HError(2630,"TieWeights: Attempt to tie null stream weights as macro %s",
             macId->name);
   if (GetUse(v)>0)
      DeleteMacroStruct(hset,'w',v);
   else
   SetUse(v,1);
   NewMacro(hset,fidx,'w',macId,v);
   for (i=ilist->next; i!=NULL; i=i->next) {
      si = (StateInfo *)i->item;
      if (si->weights==NULL)
         HError(-2630,"TieWeights: Attempt to tie null stream weights to macro %s",macId->name);
      if (si->weights != v) {
         if (GetUse(si->weights)>0) DecUse(si->weights); 
         si->weights = v;
         IncUse(v);
      }
   }
}

/* VAdd: add vector b to vector a */
void VAdd(Vector a, Vector b)
{
   int k,V;
   
   V = VectorSize(a);
   for (k=1; k<=V; k++) a[k] += b[k];
}

/* VMult: multiply vector a by n */
void VMult(Vector a, int n)
{
   int k,V;
   
   V = VectorSize(a);
   for (k=1; k<=V; k++) a[k] *= n;
}

/* VMax: max vector b to vector a */
void VMax(Vector a, Vector b)
{
   int k,V;
   
   V = VectorSize(a);
   for (k=1; k<=V; k++) 
      if (b[k]>a[k]) a[k] = b[k];
}

/* VNorm: divide vector a by n */
void VNorm(Vector a, int n)
{
   int k,V;
   
   V = VectorSize(a);
   for (k=1; k<=V; k++) a[k] /= n;
}

/* TieMean: tie all mean vectors to average */
void TieMean(ILink ilist, LabId macId)
{
   ILink i;
   MixPDF *mp;
   Vector tmean;
   int tSize,vSize;
   
   mp = (MixPDF *)ilist->item; tmean = mp->mean;
   tSize = VectorSize(tmean);
   if (GetUse(tmean)>0) {
      DeleteMacroStruct(hset,'u',tmean);
      VMult(tmean,GetUse(tmean));
   }
   else
   SetUse(tmean,1);
   NewMacro(hset,fidx,'u',macId,tmean);
   for (i=ilist->next; i!=NULL; i=i->next) {
      mp = (MixPDF *)i->item;
      if (mp->mean != tmean) {
         vSize = VectorSize(mp->mean);
         if (tSize != vSize)
            HError(2630,"TieMean: Vector size mismatch %d vs %d",
                   tSize, vSize);
         VAdd(tmean,mp->mean);
         if (GetUse(mp->mean)>0) DecUse(mp->mean);
         mp->mean = tmean;
         IncUse(tmean);
      }
   }
   VNorm(tmean,GetUse(tmean));
}

/* TieVar: tie all variance vectors in ilist to max */
void TieVar(ILink ilist, LabId macId)
{
   ILink i;
   MixPDF *mp;
   Vector tvar;
   int tSize,vSize;
   
   mp = (MixPDF *)ilist->item; tvar = mp->cov.var;
   tSize = VectorSize(tvar);
   if (GetUse(tvar)>0)
      DeleteMacroStruct(hset,'v',tvar);
   else
   SetUse(tvar,1);
   NewMacro(hset,fidx,'v',macId,tvar);
   for (i=ilist->next; i!=NULL; i=i->next) {
      mp = (MixPDF *)i->item;
      if (mp->cov.var!=tvar) {
         vSize = VectorSize(mp->cov.var);
         if (tSize != vSize)
            HError(2630,"TieVar: Vector size mismatch %d vs %d",
                   tSize, vSize);
         VMax(tvar,mp->cov.var);
         if (GetUse(mp->cov.var)>0) DecUse(mp->cov.var);
         mp->cov.var = tvar; IncUse(tvar);
      }
   }
   badGC=TRUE;                  /* gConsts no longer valid */
}

/* TieInv: tie all invcov mats in ilist to first */
void TieInv(ILink ilist, LabId macId)
{
   ILink i;
   MixPDF *mp;
   STriMat tinv;
   int tSize,mSize;
   
   mp = (MixPDF *)ilist->item; tinv = mp->cov.inv;
   tSize = NumRows(mp->cov.inv);
   if (GetUse(tinv)>0)
      DeleteMacroStruct(hset,'i',tinv);
   else
   SetUse(tinv,1);
   NewMacro(hset,fidx,'i',macId,tinv);
   for (i=ilist->next; i!=NULL; i=i->next) {
      mp = (MixPDF *)i->item;
      if (mp->cov.inv!=tinv) {
         mSize = NumRows(mp->cov.inv);
         if (tSize != mSize)
            HError(2630,"TieInv: Matrix size mismatch %d vs %d",
                   tSize, mSize);
         if (GetUse(mp->cov.inv)>0) DecUse(mp->cov.inv);
         mp->cov.inv = tinv; IncUse(tinv);
      }
   }
   badGC=TRUE;                  /* gConsts no longer valid */
}

/* TieXform: tie all xform mats in ilist to first */
void TieXform(ILink ilist, LabId macId)
{
   ILink i;
   MixPDF *mp;
   SMatrix txform;
   int tr,tc,mr,mc;
   
   mp = (MixPDF *)ilist->item; txform = mp->cov.xform;
   tr = NumRows(mp->cov.xform); tc = NumCols(mp->cov.xform);
   if (GetUse(txform)>0)
      DeleteMacroStruct(hset,'x',txform);
   else
   SetUse(txform,1);
   NewMacro(hset,fidx,'x',macId,txform);
   for (i=ilist->next; i!=NULL; i=i->next) {
      mp = (MixPDF *)i->item;
      if (mp->cov.xform!=txform) {
         mr = NumRows(mp->cov.xform); mc = NumCols(mp->cov.xform);
         if (tc != mc || tr != mr)
            HError(2630,"TieXform: Matrix size mismatch");
         if (GetUse(mp->cov.xform)>0) DecUse(mp->cov.xform);
         mp->cov.xform = txform; IncUse(txform);
      }
   }
   badGC=TRUE;                  /* gConsts no longer valid */
}

/* TieMix: tie all mix pdf's in ilist to 1st */
void TieMix(ILink ilist, LabId macId)
{
   ILink i;
   MixtureElem *me;
   MixPDF *tmpdf;
   
   me = (MixtureElem *)ilist->item; /* should choose max(Sigma) for */
   tmpdf = me ->mpdf;           /* consistency with tie states */
   if (tmpdf->nUse>0)
      DeleteMacroStruct(hset,'m',tmpdf);
   else
   tmpdf->nUse = 1;
   NewMacro(hset,fidx,'m',macId,tmpdf);
   for (i=ilist->next; i!=NULL; i=i->next) {
      me = (MixtureElem *)i->item;
      if (me->mpdf!=tmpdf) {
         if (me->mpdf->nUse>0) me->mpdf->nUse--;
         me->mpdf = tmpdf;
         ++tmpdf->nUse;
      }
   }
}

/* CreateJMixSet: create an array of 'joinSize' mixture elems */
MixtureElem * CreateJMixSet(void)
{
   MixtureElem *me;
   int m;
   
   me = (MixtureElem*) New(&hmmHeap,sizeof(MixtureElem)*joinSize);
   --me;
   for (m=1;m<=joinSize;m++) {
      me[m].weight = 0.0;
      me[m].mpdf = NULL;
   }
   return me;
}

/* AddJMix: add given mixture elem to joinSet inserting it in
   order of mixture weight */
void AddJMix(MixtureElem *me)
{
   int i,j;
   
   if (nJoins<joinSize || me->weight > joinSet[nJoins].weight) {
      if (nJoins == joinSize) /* if full discard last mix */
         --nJoins;
      i = 1;                    /* find insertion point */
      while (me->weight < joinSet[i].weight) i++;
      for (j=nJoins; j>=i; j--) /* make space */
         joinSet[j+1] = joinSet[j];
      joinSet[i] = *me;         /* insert new mix */
      ++nJoins;
   }
}

/* RemTop: remove and return top mix in join set */
MixtureElem RemTop(void)
{
   int j;
   MixtureElem me;
   
   if (nJoins==0)
      HError(2691,"RemTop: Attempt to rem top of empty join set");
   me = joinSet[1];
   for (j=1;j<nJoins;j++)
      joinSet[j] = joinSet[j+1];
   joinSet[nJoins].weight = 0.0;
   --nJoins;
   return me;
}

/* SplitMix: split given mix mi into two */
void SplitMix(MixtureElem *mi,MixtureElem *m01,MixtureElem *m02,int vSize)
{
   const float pertDepth = 0.2;
   int k;
   long splitcount;
   float x;
   TriMat mat=NULL;
   
   splitcount = (long) mi->mpdf->hook + 1;
   m01->mpdf = CloneMixPDF(hset,mi->mpdf,FALSE); 
   m02->mpdf = CloneMixPDF(hset,mi->mpdf,FALSE);
   m01->weight = m02->weight = mi->weight/2.0;
   m01->mpdf->hook = m02->mpdf->hook = (void *)splitcount;
   if (mi->mpdf->ckind==FULLC || mi->mpdf->ckind==LLTC) {
      mat = CreateTriMat(&tmpHeap,vSize);
      CovInvert(mi->mpdf->cov.inv,mat);
   }
   for (k=1;k<=vSize;k++) {     /* perturb them */
      if (mi->mpdf->ckind == FULLC || mi->mpdf->ckind==LLTC)
         x = sqrt(mat[k][k])*pertDepth;
      else
         x = sqrt(mi->mpdf->cov.var[k])*pertDepth;
      m01->mpdf->mean[k] += x;
      m02->mpdf->mean[k] -= x;
   }
   if (mat!=NULL) FreeTriMat(&tmpHeap,mat);
}

/* FixWeights: Fix the weights of me using the old stream info */
void FixWeights(MixtureElem *me, HMMDef *owner, StreamInfo *sti)
{
   MixtureElem *sme;
   double w,p,wSum,fSum,maxW,floor,lFloor;
   int i=0,m,sm;
   
   if (trace & T_DET)
      printf("   For  %-12s  ",HMMPhysName(hset,owner));
   maxW = LZERO;
   for (m=1;m<=joinSize;m++) {  /* set weights to SOutP(owner) */
      w=LZERO;
      for (sm=1,sme=sti->spdf.cpdf+1;sm<=sti->nMix;sm++,sme++) {
         Untouch(&me[m].mpdf->nUse);
         p = MOutP(me[m].mpdf->mean,sme->mpdf);
         w = LAdd(w,log(sme->weight)+p);
      }
      me[m].weight = w;
      if (w > maxW) maxW = w; 
   }
   wSum = 0.0; fSum = 0.0; 
   floor = joinFloor*MINMIX; lFloor = log(floor*joinSize);
   for (m=1;m<=joinSize;m++) {  /* make all weights > floor */
      w = me[m].weight-maxW;
      if (w < lFloor) {
         me[m].weight = floor; fSum += floor;
      } else {
         w = exp(w); me[m].weight = w;
         wSum += w;
      }
   }
   if (fSum>=0.9)
      HError(2634,"FixWeights: Join Floor too high");
   if (wSum==0.0)
      HError(2691,"FixWeights: No positive weights");
   wSum /= (1-fSum);            /* finally normalise */
   for (m=1;m<=joinSize;m++) {   
      w = me[m].weight;
      if (w>floor) {
         w /= wSum; me[m].weight = w;
         if (trace & T_DET) {
            if ((++i%4)==0) printf("\n    ");
            printf(" %3d == %7.4e",m,w);
         }
      }
   }
   if (trace & T_DET) {
      printf("\n");
      fflush(stdout);
   }
}

/* Create macros for each mix in joinSet & reset nUse */
void CreateJMacros(LabId rootMacId)
{
   int m;
   char buf[MAXSTRLEN];
   MixPDF *mp;
   
   for (m=1;m<=joinSize;m++) {
      sprintf(buf,"%s%d",rootMacId->name,m);
      mp = joinSet[m].mpdf;
      mp->nUse = 0;
      NewMacro(hset,fidx,'m',GetLabId(buf,TRUE),mp);
      if (trace & (T_DET | T_MAC)) {
         printf("   Join Macro: ~m:%s created\n",buf);
         fflush(stdout);
      }
   }
}

/* TiePDF: tie stream if joinSize==0. otherwise join mixtures for set of 'joinSize' tied pdfs in ilist
   This is standard 'tied mixture' case. Min mix weight is joinFloor * MINMIX   */
void TiePDF(ILink ilist, LabId macId)
{
   ILink i;
   int m,nSplit,vs,vSize=0;
   StreamElem *ste;
   StreamInfo *sti,*tsti;
   MixtureElem *me, mix,mix1,mix2;
   
   if (badGC) {
      FixAllGConsts(hset);         /* in case any bad gConsts around */
      badGC=FALSE;
   }

   if (joinSize==0) {  /* Tie StreamInfo */
      ste = (StreamElem *) ilist->item;
      tsti = ste->info;
      if (tsti->nUse>0)
         DeleteMacroStruct(hset,'p',tsti);
      else
         tsti->nUse = 1;
      NewMacro(hset,fidx,'p',macId,tsti);
      for (i=ilist->next; i!=NULL; i=i->next) {
         ste = (StreamElem *)i->item; sti = ste->info;
         if (sti != tsti) {
            if (sti->nUse>0) sti->nUse--;
            ste->info = tsti;
            ++tsti->nUse;
         }
      }
   }
   else {  /* Create Tied Mixture */
   nJoins = 0; joinSet = CreateJMixSet();
   for (i=ilist; i!=NULL; i=i->next) {
      ste = (StreamElem *) i->item;
         sti = ste->info;
         vs = VectorSize(sti->spdf.cpdf[1].mpdf->mean);
      if (vSize==0)
         vSize = vs;
      else if (vs != vSize)
         HError(2630,"TiePDF: incompatible vector sizes %d vs %d",vs,vSize);
         for (m=1;m<=sti->nMix;m++) {
            me = sti->spdf.cpdf+m;
         if (IsSeen(me->mpdf->nUse)) continue;
         Touch(&me->mpdf->nUse); /* Make sure we don't add twice */
         AddJMix(me);
      }
   }
   if (trace & T_BID) {
      printf("  Joined %d mixtures.",nJoins);
      if (nJoins==joinSize) 
         printf("  [Full set]\n");
      else
         printf("  [%d more needed]\n",joinSize-nJoins);
      fflush(stdout);
   }
   nSplit=0;
   while (nJoins<joinSize) {    /* split best mixes if needed */
      mix = RemTop();
      SplitMix(&mix,&mix1,&mix2,vSize);
      AddJMix(&mix1); AddJMix(&mix2);
      ++nSplit;
   }

   CreateJMacros(macId);
   for (i=ilist; i!=NULL; i=i->next) { /* replace pdfs */
      ste = (StreamElem *) i->item;
         sti = ste->info;
         if (IsSeen(sti->nUse)) continue;
      me = CreateJMixSet();
      for (m=1;m<=joinSize;m++) {
         me[m].mpdf = joinSet[m].mpdf;
         ++(joinSet[m].mpdf->nUse);
      }
         FixWeights(me,i->owner,sti);
         sti->spdf.cpdf = me; sti->nMix = joinSize;
         Touch(&sti->nUse);
   }
   for (i=ilist; i!=NULL; i=i->next) { /* reset nMix flags */
      ste = (StreamElem *) i->item;
         sti = ste->info;
         Untouch(&sti->nUse);
   }
   if (joinSize>maxMixes) maxMixes=joinSize;
}
}

/* TieHMMs: tie all hmms in ilist to 1st */
void TieHMMs(ILink ilist,LabId macId)
{
   ILink i;
   HLink hmm,cur;
   MLink q;
   int h,c,j;
   char type;

   hmm=ilist->owner;    /* get first in list */
   q=FindMacroStruct(hset,'h',hmm);
   DeleteMacro(hset,q);
   NewMacro(hset,fidx,'h',macId,hmm);
   for (i=ilist->next,c=1; i!=NULL; i=i->next,c++)
      i->owner->owner=NULL;
   hmm->owner=hset;    /* In case of duplicates in ilist */
   for (h=0,j=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next) {
         if (q->type!='h' && q->type!='l') continue;
         cur = (HLink) q->structure; 
         if (cur->owner==NULL) {
            type = q->type;
            DeleteMacro(hset,q);
            if (type=='l')
               NewMacro(hset,fidx,'l',q->id,hmm);
            hmm->nUse++;
            j++;
         }
      }
}

/* ApplyTie: tie all structures in list of give type */
void ApplyTie(ILink ilist, char *macName, char type)
{
   LabId macId;
   int use;
   
   if (ilist == NULL) {
      HError(-2631,"ApplyTie: Macro %s has nothing to tie of type %c",
             macName,type);
      return;
   }
   macId = GetLabId(macName,TRUE);
   switch (type) {
   case 's':   TieState(ilist,macId); break;
   case 't':   TieTrans(ilist,macId); break;
   case 'm':   TieMix(ilist,macId); break;
   case 'u':   TieMean(ilist,macId); break;
   case 'v':   TieVar(ilist,macId); break;
   case 'i':   TieInv(ilist,macId); break;
   case 'x':   TieXform(ilist,macId); break;
   case 'p':   TiePDF(ilist,macId); break;
   case 'w':   TieWeights(ilist,macId); break;
   case 'd':   TieDur(ilist,macId); break;
   case 'h':   TieHMMs(ilist,macId); break;
   }
   if ((trace & (T_DET | T_MAC)) && type!='p') {
      use=GetMacroUse(FindMacroName(hset,type,macId));
      printf("   Macro: ~%c:%s created for %d/%d items\n",
             type,macName,use,NumItems(ilist));
      fflush(stdout);
   }
}

/* ------------------ Untying Operations --------------------- */

/* UntieState: untie (ie clone) all items in ilist */
void UntieState(ILink ilist)
{
   ILink i;
   StateElem *se;
   StateInfo *si;
   int nu;
   
   for (i=ilist; i!=NULL; i=i->next) {
      se = (StateElem *)i->item;
      si = se->info;
      nu = si->nUse; 
      si->nUse = 0;
      if (nu==1)
         DeleteMacroStruct(hset,'s',si);
      else if (nu>1) {
         se->info = CloneState(hset,si,FALSE);
         si->nUse = nu-1;
      }
   }
}

/* UntiePDF: untie (ie clone) all items in ilist */
void UntiePDF(ILink ilist)
{
   ILink i;
   StreamElem *ste;
   StreamInfo *sti;
   int nu,s;
   
   for (i=ilist; i!=NULL; i=i->next) {
      ste = (StreamElem *)i->item;
      sti = ste->info;
      s = ste->info->stream;
      nu = sti->nUse; 
      sti->nUse = 0;
      if (nu==1)
         DeleteMacroStruct(hset,'p',sti);
      else if (nu>1) {
         ste->info = ClonePDF(hset,s,sti,FALSE);
         sti->nUse = nu-1;
      }
   }
}

/* UntieMix: untie (ie clone) all items in ilist */
void UntieMix(ILink ilist)
{
   ILink i;
   MixtureElem *me;
   MixPDF *mp;
   int nu,vSize;
   
   for (i=ilist; i!=NULL; i=i->next) {
      me = (MixtureElem *)i->item;
      if (me->weight > MINMIX) {
         mp = me->mpdf; vSize = VectorSize(mp->mean);
         nu = mp->nUse; 
         mp->nUse = 0;
         if (nu==1)
            DeleteMacroStruct(hset,'m',mp);
         else if (nu>1) {
            me->mpdf = CloneMixPDF(hset,mp,FALSE);
            mp->nUse = nu-1;
         }
      }
   }
}

/* UntieMean: untie (ie clone) all items in ilist */
void UntieMean(ILink ilist)
{
   ILink i;
   MixPDF *mp;
   Vector v;
   int nu;
   
   for (i=ilist; i!=NULL; i=i->next) {
      mp = (MixPDF *)i->item;
      v = mp->mean;
      nu = GetUse(v); 
      SetUse(v,0);
      if (nu==1)
         DeleteMacroStruct(hset,'u',v);
      else if (nu>1) {
         mp->mean = CloneSVector(hset->hmem,v,FALSE);
         SetUse(v,nu-1);
      }
   }
}

/* UntieVar: untie (ie clone) all items in ilist */
void UntieVar(ILink ilist)
{
   ILink i;
   MixPDF *mp;
   Vector v;
   int nu;
   
   for (i=ilist; i!=NULL; i=i->next) {
      mp = (MixPDF *)i->item;
      v = mp->cov.var;
      nu = GetUse(v); 
      SetUse(v,0);
      if (nu==1)
         DeleteMacroStruct(hset,'v',v);
      else if (nu>1) {
         mp->cov.var = CloneSVector(hset->hmem,v,FALSE);
         SetUse(v,nu-1);
      }
   }
}

/* UntieInv: untie (ie clone) all items in ilist */
void UntieInv(ILink ilist)
{
   ILink i;
   MixPDF *mp;
   SMatrix m;
   int nu;
   
   for (i=ilist; i!=NULL; i=i->next) {
      mp = (MixPDF *)i->item;
      m = mp->cov.inv;
      nu = GetUse(m);
      SetUse(m,0);
      if (nu==1)
         DeleteMacroStruct(hset,'i',m);
      else if (nu>1) {
         mp->cov.inv = CloneSTriMat(hset->hmem,m,FALSE);
         SetUse(m,nu-1);
      }
   }
}

/* UntieXform: untie (ie clone) all items in ilist */
void UntieXform(ILink ilist)
{
   ILink i;
   MixPDF *mp;
   SMatrix m;
   int nu;
   
   for (i=ilist; i!=NULL; i=i->next) {
      mp = (MixPDF *)i->item;
      m = mp->cov.xform;
      nu = GetUse(m);
      SetUse(m,0);
      if (nu==1)
         DeleteMacroStruct(hset,'x',m);
      else if (nu>0) {
         mp->cov.xform = CloneSMatrix(hset->hmem,m,FALSE);
         SetUse(m,nu-1);
      }
   }
}

/* UntieTrans: untie (ie clone) all items in ilist */
void UntieTrans(ILink ilist)
{
   ILink i;
   HMMDef *hmm;
   SMatrix m;
   int nu;
   
   for (i=ilist; i!=NULL; i=i->next) {
      hmm = (HMMDef *)i->item;
      m = hmm->transP;
      nu = GetUse(m); 
      SetUse(m,0);
      if (nu==1)
         DeleteMacroStruct(hset,'t',m);
      else if (nu>0) {
         hmm->transP = CloneSMatrix(hset->hmem,m,FALSE);
         SetUse(m,nu-1);
      }
   }
}

/* -------------------- Cluster Operations --------------------- */

typedef struct _CRec *CLink;
typedef struct _CRec{
   ILink item;                  /* a single item in this cluster (group) */
   int idx;                     /* index of this item */
   Boolean ans;                 /* answer to current question */
   short state;                 /* state of clink */
   Ptr owner;                   /* owner of clink */
   CLink next;                  /* next item in group */
}CRec;

/* TDistance: return mean sq diff between Tied Mix weights */
float TDistance(StreamInfo *sti1, StreamInfo *sti2)
{
   int m,M;
   float *mw1,*mw2;
   float x,sum=0.0;
   
   mw1 = sti1->spdf.tpdf+1; mw2 = sti2->spdf.tpdf+1; 
   M = sti1->nMix;
   for (m=1; m<=M; m++, mw1++, mw2++) {
      x = (*mw1 - *mw2);
      sum += x*x;
   }
   return sqrt(sum/M);
}

/* DDistance: return mean sq log diff between Discrete Probabilities */
float DDistance(StreamInfo *sti1, StreamInfo *sti2)
{
   int m,M;
   short *mw1,*mw2;
   float x,sum=0.0;
   
   mw1 = sti1->spdf.dpdf+1; mw2 = sti2->spdf.dpdf+1; 
   M = sti1->nMix;
   for (m=1; m<=M; m++, mw1++, mw2++) {
      x = (*mw1 - *mw2);
      sum += x*x;
   }
   return sqrt(sum/M);
}

/* Divergence: return divergence between two Gaussians */
float Divergence(StreamInfo *sti1, StreamInfo *sti2)
{
   int k,V;
   MixPDF *m1,*m2;
   float x,v1,v2,sum=0.0;
   
   m1 = (sti1->spdf.cpdf+1)->mpdf; m2 = (sti2->spdf.cpdf+1)->mpdf;
   V = VectorSize(m1->mean);
   for (k=1; k<=V; k++) {
      x = m1->mean[k] - m2->mean[k];
      v1 = m1->cov.var[k]; v2 = m2->cov.var[k];
      sum += x*x / sqrt(v1*v2);
      /* The following is closer to the true divergence but the */
      /* increased sensitivity to undertrained variances is undesirable */
      /*    sum += v1/v2 + v2/v1 - 2.0 + (1.0/v1 + 1.0/v2)*x*x; */
   }
   return sqrt(sum/V);
}

/* GDistance: return general distance between two arbitrary pdfs
   by summing the log probabilities of each mixture mean with
   respect to the other pdf  */
float GDistance(int s, StreamInfo *sti1, StreamInfo *sti2)
{
   int m,M;
   MixtureElem *me;
   Observation dummy;
   float sum=0.0;
   
   M = sti1->nMix;
   for (m=1,me = sti1->spdf.cpdf+1; m<=M; m++, me++) {
      dummy.fv[s]=me->mpdf->mean;
      sum += SOutP(hset,s,&dummy,sti2);
   }
   M = sti2->nMix;
   for (m=1,me = sti2->spdf.cpdf+1; m<=M; m++, me++) {
      dummy.fv[s]=me->mpdf->mean;
      sum += SOutP(hset,s,&dummy,sti1);
   }
   return -(sum/M);
}

/* StateDistance: return distance between given states */
float StateDistance(ILink i1, ILink i2)
{
   StateElem *se1, *se2;
   StateInfo *si1, *si2;
   StreamElem *ste1,*ste2;
   StreamInfo *sti1,*sti2;
   float x = 0.0;
   int s,S;
   
   se1 = (StateElem *)i1->item; si1 = se1->info;
   se2 = (StateElem *)i2->item; si2 = se2->info;
   S = hset->swidth[0];
   ste1 = si1->pdf+1; ste2 = si2->pdf+1;
   for (s=1;s<=S;s++,ste1++,ste2++) {
      sti1 = ste1->info;
      sti2 = ste2->info;
      if (hset->hsKind == TIEDHS)
         x += TDistance(sti1,sti2);
      else if (hset->hsKind==DISCRETEHS)
         x += DDistance(sti1,sti2);
      else if (maxMixes == 1 && sti1->spdf.cpdf[1].mpdf->ckind==DIAGC && 
               sti2->spdf.cpdf[1].mpdf->ckind==DIAGC) 
         x += Divergence(sti1,sti2);
      else
         x += GDistance(s,sti1,sti2);
   }
   return x/S;
}

/* SetGDist: compute inter group distances */
void SetGDist(CLink *cvec, Matrix id, Matrix gd, int N)
{
   int i,j;
   CLink p,q,pp;
   float maxd;
   
   for (i=1; i<=N; i++) gd[i][i]=0.0;
   for (i=1; i<N; i++) {
      p = cvec[i];
      for (j=i+1; j<=N; j++) {
         q = cvec[j];
         maxd = 0.0;
         while (q != NULL) {
            pp = p;
            while (pp != NULL) {
               if (id[pp->idx][q->idx] > maxd)
                  maxd = id[pp->idx][q->idx];
               pp = pp->next;
            }
            q = q->next;
         }
         gd[i][j] = gd[j][i] = maxd;
      }
   }
}

/* SetIDist: compute inter item distances */
void SetIDist(CLink *cvec, Matrix id, int N, char type)
{
   ILink ii,jj;
   int i,j;
   float dist=0.0;
   
   for (i=1; i<=N; i++) id[i][i]=0.0;
   for (i=1; i<N; i++) {
      ii=cvec[i]->item;
      for (j=i+1; j<=N; j++) {
         jj = cvec[j]->item;
         switch(type) {
         case 's': 
            dist = StateDistance(ii,jj);
            break;
         default:
            HError(2640,"SetIDist: Cant compute distances for %c types",type);
         }
         id[i][j] = id[j][i] = dist;
      }
   }
}

/* MinGDist: find min inter group distance */
float MinGDist(Matrix g, int *ix, int *jx, int N)
{
   int mini,minj;
   float min;
   int i,j;
   
   min = g[1][2]; mini=1; minj=2;
   for (i=1; i<N; i++)
      for (j=i+1; j<=N; j++)
         if (g[i][j]<min) {
            min = g[i][j]; mini = i; minj = j;
         }
   *ix = mini; *jx = minj; 
   return min;
}


/* MergeGroups:  merge the two specified groups */
void MergeGroups(int i, int j, CLink *cvec, int N)
{
   int k;
   CLink p;
   
   p = cvec[i];
   while(p->next != NULL) 
      p = p->next;
   p->next = cvec[j];
   for (k=j; k<N; k++) 
      cvec[k] = cvec[k+1];
}

/* BuildCVec: allocate space for cvec and create item sized groups */
CLink *BuildCVec(int numClust, ILink ilist)
{
   CLink *cvec,p;
   int i;
   
   /* Allocate extra space to allow for call to Dispose(&tmpHeap,cvec) */
   cvec=(CLink*) New(&tmpHeap,(numClust+1)*sizeof(CLink));
   for (i=1;i<=numClust; i++) {
      p=(CLink) New(&tmpHeap,sizeof(CRec));
      if ((p->item = ilist) == NULL)
         HError(2690,"BuildCVec: numClust<NumItems(ilist) %d",i);
      ilist = ilist->next;
      p->idx = i; p->next = NULL;
      cvec[i] = p;
   }
   if (ilist!=NULL)
      HError(2690,"BuildCVec: numClust>NumItems(ilist)");
   return cvec;
}

/* SetOccSums: return a vector of cluster occupation sums.  The occ count
   for each state was stored in the hook of the StateInfo rec by
   the RO command */
Vector SetOccSums(CLink *cvec, int N)
{
   int i;
   float sum,x;
   CLink p;
   Vector v;
   StateElem *se;
   StateInfo *si;
   ILink ip;
   
   v = CreateVector(&tmpHeap,N);
   for (i=1; i<=N; i++) {
      sum = 0.0;
      for (p=cvec[i]; p != NULL; p = p->next) {
         ip = p->item;
         se = (StateElem *)ip->item; 
         si = se->info;
         memcpy(&x,&(si->hook),sizeof(float));
         sum += x;
      }
      v[i] = sum;
   }
   return v;
}

/* UpdateOccSums: update the occSum array following MergeGroups */
void UpdateOccSums(int i, int j, Vector occSum, int N)
{
   int k;
   
   occSum[i] += occSum[j];
   for (k=j; k<N; k++)
      occSum[k] = occSum[k+1];
}

/* MinOccSum: return index of min occ sum */
int MinOccSum(Vector occSum, int N)
{
   float min;
   int mini,i;
   
   mini = 1; min = occSum[mini];
   for (i=2; i<=N; i++)
      if (occSum[i]<min) {
         mini = i; min = occSum[mini];
      }
   return mini;
}

/* RemOutliers: remove any cluster for which the total state occupation
   count is below the 'outlierThresh' set by the RO command */
void RemOutliers(CLink *cvec, Matrix idist, Matrix gdist, int *numClust, 
                 Vector occSum)
{
   int N;                       /* current num clusters */
   int sparsest,i,mini;
   Vector gd;
   float min;
   
   N = *numClust;
   sparsest = MinOccSum(occSum,N);
   while (N>1 && occSum[sparsest] < outlierThresh) {
      gd = gdist[sparsest];     /* find best merge */
      mini = (sparsest==1)?2:1;
      min = gd[mini];
      for (i=2; i<=N; i++)
         if (i != sparsest && gd[i]<min) {
            mini = i; min = gd[mini];
         }
      MergeGroups(sparsest,mini,cvec,N);
      UpdateOccSums(sparsest,mini,occSum,N);
      --N;
      SetGDist(cvec,idist,gdist,N);
      sparsest = MinOccSum(occSum,N);
   }
   *numClust = N;
}

/* Clustering: split ilist into sublists where each sublist contains one
   cluster of items. Return list of sublists in cList.  It uses a 
   simple 'Furthest neighbour hierarchical cluster algorithm' */
void Clustering(ILink ilist, int *numReq, float threshold,
                char type, char *macName)
{
   Vector occSum=NULL;          /* array[1..N] of cluster occupation sum */
   int numClust;                /* current num clusters */
   CLink *cvec;                 /* array[1..numClust] of ->CRec */
   Matrix idist;                /* item distance matrix */
   Matrix gdist;                /* group cluster matrix */
   CLink p;
   ILink l;
   float ming,min;
   int i,j,k,n,numItems;
   char buf[40];

   if (badGC) {
      FixAllGConsts(hset);         /* in case any bad gConsts around */
      badGC=FALSE;
   }
   numItems = NumItems(ilist);
   numClust = numItems;         /* each item is separate cluster initially */
   if (trace & T_IND) {
      printf(" Start %d items\n",numClust);
      fflush(stdout);
   }
   
   cvec = BuildCVec(numClust,ilist);
   idist = CreateMatrix(&tmpHeap,numClust,numClust);
   gdist = CreateMatrix(&tmpHeap,numClust,numClust);
   SetIDist(cvec,idist,numClust,type); /* compute inter-item distances */
   CopyMatrix(idist,gdist);     /* 1 item per group so dmats same */
   ming = MinGDist(gdist,&i,&j,numClust);
   while (numClust>*numReq && ming<threshold) { /* merge closest two groups */
      MergeGroups(i,j,cvec,numClust);
      --numClust;
      SetGDist(cvec,idist,gdist,numClust); /* recompute gdist */
      ming = MinGDist(gdist,&i,&j,numClust);
   }
   n = numClust;
   if (occStatsLoaded) {
      if (trace & T_IND) {
         printf(" Via %d items before removing outliers\n",numClust);
         fflush(stdout);
      }
      occSum = SetOccSums(cvec,numClust);
      RemOutliers(cvec,idist,gdist,&numClust,occSum);
   }
   *numReq = numClust;          /* in case this is thresh limited case */
   if (trace & T_IND) {
      printf(" End %d items\n",numClust);
      fflush(stdout);
   }
   for (i=1; i<=numClust; i++) {
      if (trace & T_CLUSTERS) {
         for (j=1,min=99.999,k=0;j<=numClust;j++)
            if (i!=j && gdist[i][j]<min) min=gdist[i][j],k=j;
         printf("  C.%-2d MinG %6.3f[%d]",i,min,k); 
         if (occSum != NULL) printf(" (%.1f) ==",occSum[i]);
         for (p=cvec[i]; p!=NULL; p=p->next)
            printf(" %s",HMMPhysName(hset,p->item->owner));
         printf("\n");
         fflush(stdout);
      }
      sprintf(buf,"%s%d",macName,i);    /* construct macro name */
      for (p=cvec[i],l=NULL;p!=NULL;p=p->next)
         p->item->next=l,l=p->item;     /* and item list */
      ApplyTie(l,buf,type);             /* and tie it */
      FreeItems(&l);                    /* free items in sub list */
   }

   FreeMatrix(&tmpHeap, gdist);
   FreeMatrix(&tmpHeap, idist);
   Dispose(&tmpHeap,cvec);
}

/* ---------- Up Num Mixtures Operations --------------------- */

/* SetGCStats: scan loaded models and compute average gConst */
void SetGCStats(void)
{
   HMMScanState hss;
   LogDouble sum = 0.0, sumsq = 0.0;
   float x;
   int count = 0;
   
   FixAllGConsts(hset);         /* in case any bad gConsts around */
   badGC=FALSE;
   
   NewHMMScan(hset,&hss);
   while(GoNextMix(&hss,FALSE)) {
      if (VectorSize(hss.me->mpdf->mean)>0) {  /* check MSD */
      x = hss.me->mpdf->gConst;
      sum += x; sumsq += x*x;
      count++;
   }
   }
   EndHMMScan(&hss);
   meanGC = sum / count;
   stdGC = sqrt(sumsq / count - meanGC*meanGC);
   if (trace & T_IND) {
      printf("  Mean GC = %f, Std Dev GC = %f\n",meanGC,stdGC);
      fflush(stdout);
   }
}

/* HeaviestMix: find the heaviest mixture in me, the hook field of each mix
   holds a count of the number of times that the mixture has been
   split.  Each mixture is then scored as:            
   mixture weight - num splits
   but if the mix gConst is less than 4 x stddev below average
   then the score is reduced by 5000 making it very unlikely to 
   be selected */
int HeaviestMix(char *hname, MixtureElem *me, const int M, Vector mixOcc)
{
   float max,w,gThresh;
   int m,maxm;
   MixPDF *mp;
   
   /* find first continuous space mixture component */
   for (m=1; m<=M; m++)
      if (VectorSize(me[m].mpdf->mean)>0) /* MSD check */
         break;
   if (m>M)  /* no continuous space in this MixtureElem */
      HError(9999, "HeaviestMix: there is no continuous mixtures");

   /* find first mixture component whose occ count is larger than the threshold */
   for (m=m; m<=M; m++)
      if (VectorSize(me[m].mpdf->mean)>0 && mixOcc[m]>minMixOcc) 
         break;
   if (m>M)  /* no mixture */
      return 0;

   gThresh = meanGC - 4.0*stdGC;
   maxm = m;  mp = me[m].mpdf;
   max = me[m].weight - (long)mp->hook;
   if ((long)mp->hook < 5000 && mp->gConst < gThresh) {
      max -= 5000.0; mp->hook = (void *)5000;
      HError(-2637,"HeaviestMix: mix %d in %s has v.small gConst [%f]",
             m,hname,mp->gConst);
   }
   for (m=maxm+1; m<=M; m++) {
      mp = me[m].mpdf;
      w = me[m].weight - (long)mp->hook;
      if (VectorSize(mp->mean) > 0) {  /* MSD check */
         if ((long)mp->hook < 5000 && mp->gConst < gThresh) {
         w -= 5000.0; mp->hook = (void *)5000;
         HError(-2637,"HeaviestMix: mix %d in %s has v.small gConst [%f]",
                m,hname,mp->gConst);
      }
         if (w>max && mixOcc[m]>minMixOcc) {
         max = w; maxm = m;
      }
   }
   }
   if (me[maxm].weight<=MINMIX)
      HError(2697,"HeaviestMix:  heaviest mix is defunct!");
   if (trace & T_DET) {
      printf("               : Split %d (weight=%.3f, count=%ld, score=%.3f)\n",
             maxm, me[maxm].weight, (long)me[maxm].mpdf->hook, max);
      fflush(stdout);
   }
   return maxm;
}

/* SetMixOcc: set mixture-level occupancy counts */
void SetMixOcc(StreamInfo *sti, Vector mixOcc)
{
   int m;
   float occ;

   ZeroVector(mixOcc);

   for (m=1; m<=sti->nMix; m++) {
      if (occStatsLoaded) {
         memcpy(&occ,&(sti->hook),sizeof(float));
      }
      else {
         if (minMixOcc>0.0)
            HError(9999,"SetMixOcc: No stats loaded - use LS command");
         occ = 1.0;
      }
      mixOcc[m] = MixWeight(hset,sti->spdf.cpdf[m].weight) * occ;
   }
   
   return;
}

/* UpMix: increase number of mixes in stream from oldM to newM */
void UpMix(char *hname, StreamInfo *sti, int oldM, int newM)
{
   MixtureElem *me,m1,m2;
   int m,count,vSize;
   Vector mixOcc;
   
   mixOcc = CreateVector(&gstack, newM);
   SetMixOcc(sti,mixOcc);
   
   me = (MixtureElem*) New(&hmmHeap,sizeof(MixtureElem)*newM);
   --me;
   vSize = VectorSize(sti->spdf.cpdf[1].mpdf->mean);
   for (m=1;m<=oldM;m++)
      me[m] = sti->spdf.cpdf[m];
   count=oldM;
   while (count<newM) {
      m = HeaviestMix(hname,me,count,mixOcc);
      if (m==0) break;
      ++count;
      SplitMix(me+m,&m1,&m2,vSize);
      me[m] = m1; me[count] = m2;
      mixOcc[m] = mixOcc[count] = mixOcc[m] / 2;
   }
   sti->spdf.cpdf = me; sti->nMix = count;
   
   FreeVector(&gstack,mixOcc);
}

/* CountDefunctMix: return number of defunct mixtures in given stream */
int CountDefunctMix(StreamInfo *sti)
{
   int m,defunct;
   
   for (m=1,defunct=0; m<=sti->nMix; m++)
      if (sti->spdf.cpdf[m].weight <= MINMIX)
         ++defunct;
   return defunct;
}

/* FixDefunctMix: restore n defunct mixtures */
void FixDefunctMix(char *hname,StreamInfo *sti, int n)
{
   MixtureElem *me,m1,m2;
   int m,M,l,count,vSize;
   Vector mixOcc;

   mixOcc = CreateVector(&gstack, sti->nMix);
   SetMixOcc(sti,mixOcc);

   me = sti->spdf.cpdf; M = sti->nMix;
   for (count = 0; count<n; ++count) {
      for (l=1; l<=M; l++)
         if (me[l].weight <= MINMIX)
            break;
      m = HeaviestMix(hname,me,M,mixOcc);
      if(m < 1) HError(1,"FixDefunctMix: Wrong mixture weight.\n");
      vSize = VectorSize(me[m].mpdf->mean);
      SplitMix(me+m,&m1,&m2,vSize);
      me[m] = m1; me[l] = m2;
      mixOcc[m] = mixOcc[l] = mixOcc[m] / 2;
   }

   FreeVector(&gstack,mixOcc);
}

/* ------------------- MixDown Operations --------------------- */

/* MixMergeCost: compute cost of merging me1 and me2 */
float MixMergeCost(MixtureElem *me1,MixtureElem *me2)
{
   float w1,w2,v,d=0.0,v1k,v2k;
   int vs,k;
   Vector m1,m2,v1,v2;
   
   /* normalise weight */
   w1=me1->weight/(me1->weight+me2->weight);
   w2=me2->weight/(me1->weight+me2->weight);
   m1=me1->mpdf->mean;
   m2=me2->mpdf->mean;
   v1=me1->mpdf->cov.var;
   v2=me2->mpdf->cov.var;
   vs=VectorSize(m1);

   for (k=1; k<=vs; k++) {
      v1k = v1[k]; v2k = v2[k];
      v = w1*v1k+w2*v2k+m1[k]*m1[k]*(w1-w1*w1)+m2[k]*m2[k]*(w2-w2*w2)-
         2*m1[k]*m2[k]*w1*w2;
      d += -0.5*(me1->weight*(log(TPI*v)-log(TPI*v1k))+
                 me2->weight*(log(TPI*v)-log(TPI*v2k)));
   }
   return(d);
}
   
/* MergeMix: merge components p and q from given stream elem.  If inPlace
   overwrite existing components, otherwise create new mix component */
void MergeMix(StreamInfo *sti,int p,int q, Boolean inPlace)
{
   float w1,w2,m,v,p0,p1,p2,v1k,v2k;
   int vs,k;
   Vector m1,m2,v1,v2,mt,vt;
   MixtureElem me, *meq, *mep;

   if (q!=sti->nMix) {
      if (p==sti->nMix) {
         p=q; q=sti->nMix;
      }
      else {
         me=sti->spdf.cpdf[q];
         sti->spdf.cpdf[q]=sti->spdf.cpdf[sti->nMix];
         sti->spdf.cpdf[sti->nMix]=me;
         q=sti->nMix;
      }
   }
   /* the mixuture components */ 
   meq = sti->spdf.cpdf + q;                   mep = sti->spdf.cpdf + p;
   w1=mep->weight/(mep->weight+meq->weight);   w2=meq->weight/(mep->weight+meq->weight);
   m1=mep->mpdf->mean;                         m2=meq->mpdf->mean;
   v1=mep->mpdf->cov.var;                      v2=meq->mpdf->cov.var;
   vs=VectorSize(m1);

   if (inPlace) {
      mt = m1; vt = v1;
   } else {
      mep->mpdf->mean = mt = CreateSVector(&hmmHeap,vs);
      vt = CreateSVector(&hmmHeap,vs);
      mep->mpdf->cov.var = vt;
   }

   p0=p1=p2=0.0;
   for (k=1;k<=vs;k++) {
      v1k = v1[k]; v2k = v2[k];
      m=w1*m1[k]+w2*m2[k];
      v=w1*v1k+w2*v2k+m1[k]*m1[k]*(w1-w1*w1)+m2[k]*m2[k]*(w2-w2*w2)-
         2*m1[k]*m2[k]*w1*w2;
      if (trace & T_DET) {
         p0+=-0.5*(1.0+log(TPI*v));
         p1+=-0.5*(1.0+log(TPI*v1k));
         p2+=-0.5*(1.0+log(TPI*v2k));
      }
      mt[k]=m;
      vt[k]=v;
   }
   
   mep->weight += meq->weight;
   if (trace & T_DET)
      printf("     Likelihood from %.2f,%.2f to %.2f\n",p1,p2,p0);
}

/* DownMixSingle
   replace the mixture with a single Gaussian
 */
void DownMixSingle(StreamInfo *sti,Boolean inPlace)
{
    int k,i,vs;
    float w;
    SVector mt,vt;
    MixPDF *m;
    MixtureElem *me;
    
    me = sti->spdf.cpdf; m=(me+1)->mpdf;
    vs=VectorSize(m->mean);
    mt=CreateSVector(&hmmHeap,vs);
    vt=CreateSVector(&hmmHeap,vs);
    for (k=1;k<=vs;k++) {
       /* mean */
       mt[k] = 0.0; 
       for (i=1; i<=sti->nMix; i++) {
          m=(me+i)->mpdf;w=(me+i)->weight;
          if ( m->ckind != DIAGC ) /* only support DIAGC */
             HError(2640,"DownMix: covariance type not supported");
          mt[k] += w*m->mean[k]; 
       }
       /* variance */
       vt[k] = 0.0;
       for (i=1; i<=sti->nMix; i++) { 
          m=(me+i)->mpdf; w=(me+i)->weight;
          vt[k] +=w*m->cov.var[k]-w*m->mean[k]*(mt[k]-m->mean[k]);
       }
    }
    /* decrement use of mean and var */
    for(i=2;i<=sti->nMix;i++) {
       m=(me+i)->mpdf;
       DecUse(m->mean); DecUse(m->cov.var);
    }
    m=(me+1)->mpdf;
    if (inPlace) {
       /* copy vectors */
       for (k=1;k<=vs;k++) {
          m->mean[k] = mt[k]; m->cov.var[k] = vt[k];
       }
       FreeSVector(&hmmHeap,vt);
       FreeSVector(&hmmHeap,mt);
    }
    else {
       DecUse(m->mean); DecUse(m->cov.var);
       m->mean = mt; m->cov.var = vt; 
    }
    (me+1)->weight=1.0;
    sti->nMix=1;
}

/* DownMix: merge mixtures in stream to maxMix. If inPlace then existing
   mean and variance vectors will be overwritten.  Otherwise, new vectors
   are created for changed components */
void DownMix(char *hname, StreamInfo *sti, int maxMix, Boolean inPlace)
{
   int i,j,m,p,q,oldM,k,V;
   float bc,mc;
   MixPDF *m1,*m2;
   float x,v1,v2,sum=0.0;
    
   oldM=m=sti->nMix;
   if (trace & T_IND)
      printf("   Mix Down for %s\n",hname);

   /* special case mixdown to single Gaussian */
   if (maxMix==1)
      DownMixSingle(sti,inPlace);

   while(sti->nMix>maxMix) { /* merge pairs until maxMix */
      p=q=0;
      bc = LZERO;
      /* find paur with lowest cost */ 
      for (i=1; i<=sti->nMix; i++) {
         /* only support for CovKind  DIAGC */
         if ( (MixtureElem*)(sti->spdf.cpdf+i)->mpdf->ckind != DIAGC )
            HError(2640,"DownMix: covariance type not supported");
         for (j=i+1; j<=sti->nMix; j++) {
            mc=MixMergeCost(sti->spdf.cpdf+i,sti->spdf.cpdf+j);
            if (trace & T_MD) {
               printf("Could merge %d (%.2f) with %d (%.2f) for %f\n",
                      i,sti->spdf.cpdf[i].weight,j,sti->spdf.cpdf[j].weight,mc);
               m1 = (sti->spdf.cpdf+i)->mpdf; m2 = (sti->spdf.cpdf+j)->mpdf;
               V = VectorSize(m1->mean);
               for (k=1; k<=V; k++){
                  x = m1->mean[k] - m2->mean[k];
                  v1 = m1->cov.var[k]; v2 = m2->cov.var[k];
                  sum += x*x / sqrt(v1*v2);
               }
               printf("Divergence %.2f\n",sqrt(sum/V));
            }
            if (mc>bc) {
               p=i; q=j; bc=mc;
            }
         }
      }
      if (trace & T_DET)
         printf("    Merging %d with %d for %f\n",p,q,bc);
      MergeMix(sti,p,q,inPlace);
      sti->nMix--;
   }
   if ((trace & T_DET) && (sti->nMix!=oldM))
      printf("    %s: mixdown %d -> %d\n",hname,oldM,sti->nMix);
}

/* ------------------- Tree Building Routines ------------------- */

typedef struct _AccSumMixE {
   DVector sum;
   DVector sqr;
   double occ;
   int vSize;
} AccSumMixE;

typedef struct _AccSumStrE {
   int nMix;
   double weight;
   AccSumMixE *accme;
} AccSumStrE;

typedef struct _AccSum {        /* Accumulator Record for storing */
   DVector sum;                 /* the sum, sqr and occupation  */
   DVector sqr;                 /* count statistics             */
   double  occ;
   int nStream;
   AccSumStrE *accse;
} AccSum;

typedef struct _Node {          /* Tree Node */
   CLink clist;                 /* list of cluster items */
   double occ;                  /* total occupation count */
   double tProb;                /* likelihood of total cluster */
   double sProb;                /* likelihood of split cluster */
   Question *quest;             /* question used to do split */
   ILink qlist;                 /* questions can be applied to this node */
   Boolean ans;                 /* TRUE = yes, FALSE = no */
   int snum;
   MLink *macro;                /* macro used for tie */
   LabId id;                    /* name of this node */
   struct _Node *parent;        /* parent of this node */
   struct _Node *yes;           /* yes subtree */
   struct _Node *no;            /* no subtree */
   struct _Node *next;          /* doubly linked chain of */
   struct _Node *prev;          /* leaf nodes */
}Node;

typedef struct _Tree{           /* A tree */
   LabId baseId;                /* base phone name */
   ILink patList;               /* pattern list */
   int state;                   /* state (or 0 if hmm tree) */
   int size;                    /* number of non terminal nodes */
   int nLeaves;                 /* number of terminal(leaf) nodes */
   Node *root;                  /* root of tree */
   Node *leaf;                  /* chain of leaf nodes */
   struct _Tree *next;          /* next tree in list */
   IntSet streams;              /* active stream */
   int nActiveStr;              /* number of Active Streams of this tree */
}Tree;

static Tree *treeList = NULL;   /* list of trees */
static AccSum yes,no;           /* global accs for yes - no branches */
static double occs[2];          /* array[Boolean]of occupation counts */
static double cprob;            /* complete likelihood at current node */
static int numTreeClust;        /* number of clusters in tree */

/* -------------------- Tree Name Mapping routines ----------------------- */

static char treeName[PAT_LEN] = "";
static void SetTreeName(char *name) {
   strcpy(treeName,name);
   strcat(treeName,"tree");   
}

/* MapTreeName: If USEMODELNAME=FALSE, the macRoot prefix passed into
   the TB command will be used instead.   
*/
void MapTreeName(char *buf) {
   if (!useModelName) {
      strcpy(buf,treeName);
   }
}

/* CreateTree: create a new tree */
Tree *CreateTree(ILink ilist,LabId baseId,int state)
{
   Tree *tree,*t;
   char buf[PAT_LEN];
   int j;
   LabId id;
   HMMDef *hmm;
   ILink i;
   int n;
   
   tree = (Tree*) New(&questHeap,sizeof(Tree));
   tree->root = tree->leaf = NULL; tree->next = NULL; tree->size=0;
   if (treeList) { 
      for (t=treeList;t->next;t=t->next);
      t->next=tree;
   }
   else treeList=tree;
   
   /* pattern or baseId */ 
   if (usePattern) {
      strcpy(buf,baseId->name);
      ParsePattern(&tree->patList,buf);
   }
   tree->baseId = baseId;
   tree->state = state;

   /* set active stream of this tree */
   tree->streams = CreateSet(hset->swidth[0]);
   tree->nActiveStr = 0;
   for (j=1;j<=hset->swidth[0];j++) {
      if (streams.set[j]) {
         tree->nActiveStr++;
         tree->streams.set[j] = TRUE;
      }
   }

   /* check model pattern of this tree */
   if (ilist!=NULL) {
      hmm = ilist->owner;
      n=hmm->numStates;
      for (i=ilist; i!=NULL; i=i->next) {
         hmm = i->owner; strcpy(buf,HMMPhysName(hset,hmm));
         if (usePattern) {
            if (!IPatMatch(buf, tree->patList))
               HError(9999,"CreateTree: different model %s which does not match %s in item list", buf, baseId->name);
         }
         else {
            TriStrip(buf);
         MapTreeName(buf);
         id = GetLabId(buf,FALSE);
            if (!singleTree && tree->baseId != id)
               HError(-2663,"CreateTree: different base phone %s in item list", buf);
         }
         if (state>0) {
            if (hmm->svec+state != (StateElem *)i->item)
               HError(2663,"CreateTree: attempt to cluster different states");
         }
         else if (hmm->numStates != n)
            HError(2663,"CreateTree: attempt to cluster different size hmms");
      }
   }
   return tree;
}

/* CreateTreeNode: create a new tree node */
Node *CreateTreeNode(CLink clist, Node *parent)
{
   Node *n;

   n=(Node*) New(&questHeap,sizeof(Node));
   n->clist = clist;
   n->ans = FALSE;
   n->yes = n->no = NULL;
   n->prev = n->next = NULL;
   n->parent = parent;
   n->macro = NULL;
   n->quest = NULL;
   n->qlist = NULL;
   n->snum = -1;
   n->occ = 0.0;
   n->id = NULL;
   n->tProb = n->sProb = LZERO;
   return n;
}

/* MakeAccSum: make AccSum structure referring to item */
void MakeAccSum(AccSum *acc, ILink obj)
{
   int s,m;
   HMMDef *hmm;
   StateElem *se;
   StateInfo *si;
   StreamInfo *sti;
   AccSumStrE *astr;
   AccSumMixE *amix;

   if ((hmm = obj->owner)==NULL)
      HError(2690,"MakeAccSum: null hmm owner");

   acc->nStream = hmm->owner->swidth[0];
   acc->accse = (AccSumStrE *)New(&tmpHeap,acc->nStream*sizeof(AccSumStrE));
   --acc->accse;
   se = (obj->item == obj->owner)? hmm->svec+2:(StateElem *)obj->item;
   if (se == NULL)
      HError(2690,"MakeAccSum: null state elem ptr");
   si = se->info;

   for(s = 1;s <= acc->nStream;s++){
      if (streams.set[s]){
         sti = si->pdf[s].info;
         astr = acc->accse+s;
         astr->nMix = sti->nMix;
         astr->weight = (si->weights!=NULL)?si->weights[s]:1.0;
         astr->accme = (AccSumMixE *)New(&tmpHeap,sti->nMix*sizeof(AccSumMixE));
         --astr->accme;
         for(m = 1;m <= astr->nMix;m++){
            amix = astr->accme+m;
            amix->vSize = VectorSize(sti->spdf.cpdf[m].mpdf->mean);
            amix->occ = 0.0;
            amix->sum = CreateDVector(&tmpHeap, amix->vSize);
            amix->sqr = CreateDVector(&tmpHeap, amix->vSize);
            ZeroDVector(amix->sum);
            ZeroDVector(amix->sqr);
         }
      }
   }
}

/* ChkTreeObject: check that item is a valid state or hmm, 
   and make sure that MixPDF hooks are NULL */
void ChkTreeObject (ILink obj, AccSum *acc)
{
   HMMDef *hmm;
   StateElem *se;
   int j,s,m,vSize;

   hmm = obj->owner;
   if (hmm==NULL)
      HError(2690,"ChkTreeObject: null hmm owner");
   if (hmm->owner->swidth[0] != acc->nStream)
      HError(2663,"ChkTreeObject: Number of stream is differ");
   if (obj->item == obj->owner) {
      for (j=2;j<hmm->numStates;j++) {
         se = hmm->svec+j;
         for (s=1; s<=acc->nStream; s++) {
            if (streams.set[s]) {
               if (se->info->pdf[s].info->nMix != acc->accse[s].nMix)
                  HError(2663,"ChkTreeObject: Number of mixture is differ");
               for (m=1; m<=acc->accse[s].nMix; m++) {
                  if (se->info->pdf[1].info->spdf.cpdf[1].mpdf->ckind!=DIAGC)
                     HError(2663,"ChkTreeObject: TB only valid for diagonal covariace");
                  se->info->pdf[s].info->spdf.cpdf[m].mpdf->hook = NULL;
                  vSize = VectorSize(se->info->pdf[s].info->spdf.cpdf[m].mpdf->mean);
                  if (vSize != acc->accse[s].accme[m].vSize)
                     HError(2663,"ChkTreeObject: Vector Size is differ");
               }
            }
         }
      }
   }
   else {
      se = (StateElem *)obj->item;
      if (se == NULL)
         HError(2690,"ChkTreeObject: null state elem ptr");
      for (s=1;s<=acc->nStream;s++) {
         if (streams.set[s]) {
            if (se->info->pdf[s].info->nMix != acc->accse[s].nMix)
               HError(2663,"ChkTreeObject: Number of mixture is differ");
            for(m=1;m<=acc->accse[s].nMix;m++) {
               if (se->info->pdf[1].info->spdf.cpdf[1].mpdf->ckind!=DIAGC)
                  HError(2663,"ChkTreeObject: TB only valid for diagonal covariace");
               se->info->pdf[s].info->spdf.cpdf[m].mpdf->hook = NULL;
               vSize = VectorSize(se->info->pdf[s].info->spdf.cpdf[m].mpdf->mean);
               if(vSize != acc->accse[s].accme[m].vSize)
                  HError(2663,"ChkTreeObject: Vector Size is differ %d/%d",
               acc->accse[s].accme[m].vSize,vSize);
            }
         }
      }
   }
}

/* InitTreeAccs:  attach and AccSum record to the MixPDF hook and
   initialise with sum and sqr calculated from the 
   occupation counts deposited in the StateInfo hook */
void InitTreeAccs(StateElem *se)
{
   StateInfo *si;
   StreamInfo *sti;
   MixPDF *mp;
   int j, k, s, vSize;
   short nMix;
   float x, w;
   AccSum *acc;

   si=se->info ;
   for (s=1; s<=hset->swidth[0]; s++) {
      if (streams.set[s]) {
         sti = si->pdf[s].info;
         nMix = sti->nMix;
   for (j = 1; j <= nMix; j++) {
            mp = sti->spdf.cpdf[j].mpdf;
            w  = sti->spdf.cpdf[j].weight;
            vSize = VectorSize(mp->mean);
            memcpy(&x,&(sti->hook),sizeof(float));
      if (mp->hook==NULL && x>0.0) {
         acc=(AccSum*) New(&tmpHeap,sizeof(AccSum));
         mp->hook=acc;
               acc->sum = CreateDVector(&tmpHeap,vSize);
               acc->sqr = CreateDVector(&tmpHeap,vSize);
         x *= w;
               acc->occ = (double)x;
               acc->accse = NULL;
               
               for (k=1; k<=vSize; k++) {
            acc->sum[k]=mp->mean[k]*x;
            acc->sqr[k]=(mp->cov.var[k]+mp->mean[k]*mp->mean[k])*x;
         }
      }
   }
}
   }
}

/* ZeroAccSum: zero given accumulator */
void ZeroAccSum(AccSum *acc)
{
   int s,m;
   AccSumStrE *astr;
   AccSumMixE *amix;

   ZeroDVector(acc->sum);
   ZeroDVector(acc->sqr);
   acc->occ=0.0;

   if(acc->accse != NULL) {
      for(s=1;s<=acc->nStream;s++) {
         if (streams.set[s]) {
            astr = acc->accse+s;
            for(m=1;m<=astr->nMix;m++) {
               amix = astr->accme+m;
               ZeroDVector(amix->sum);
               ZeroDVector(amix->sqr);
               amix->occ=0.0;
            } 
         }
      }
   }
}

/* DisposeAccSum: dispose AccSum */
void DisposeAccSum(MemHeap *mem, AccSum *acc)
{
   int s,m;
   AccSumStrE *astr;
   AccSumMixE *amix;

   if(acc->sum != NULL)
      FreeDVector(mem,acc->sum);

   if(acc->accse != NULL) {
      for(s=1;s<=acc->nStream;s++) {
         if (streams.set[s]) {
            astr = acc->accse+s;
            for(m=1;m<=astr->nMix;m++) {
               amix = astr->accme+m;
               FreeDVector(mem,amix->sum);
            }
         }
      }
   }
}

/* AccSumProb: return log likelihood for given statistics */
double AccSumProb(AccSum *acc)
{
   double variance,occ,weight;
   double prob, x;
   int s,m,k;
   int vSize;
   DVector sqr,sum;
   AccSumStrE *astr;
   AccSumMixE *amix;

   const int S = acc->nStream;

   if (acc->occ<minLeafOcc)  /* minimum occ for each leaf node */
      return LZERO;

   prob=0.0;
   for (s=1; s<=S; s++) {
      if (streams.set[s]) {
         astr = acc->accse+s;
         for (m=1; m<=astr->nMix; m++) {
            amix = astr->accme+m;
            weight = (ignoreStrW) ? 1.0 : astr->weight;
            vSize = amix->vSize;
            sum = amix->sum;
            sqr = amix->sqr;
            occ = amix->occ;
            if (occ>0.0) {
               x = 0.0;
               for (k=1; k<=vSize; k++) {
                  variance=(sqr[k]-(sum[k]*sum[k]/occ))/occ;
         if (variance<=MINLARG) return(LZERO);
                  if (applyVFloor && variance<vf[s][k]) variance=vf[s][k];
                  x+=-0.5*occ*(1.0+log(TPI*variance));
               }
               prob+=weight*(x+occ*log(occ/acc->occ));
            }
         }
      }
   }
   return(prob);
}

/* IncSumSqr: add sums and sqrs from given state into yes or no depending
   on value of ans */
void IncSumSqr(StateInfo *si, Boolean ans, AccSum *no, AccSum *yes)
{
   AccSum *acc,*tacc;
   int s,m,k;
   int vSize;
   DVector tsum, tsqr, asum, asqr;
   Boolean first=TRUE; 

   const int S = no->nStream;
   
   for (s=1,first=TRUE; s<=S; s++) {
      if (streams.set[s]) {
         for(m=1; m<=no->accse[s].nMix; m++){
            acc = (AccSum *) si->pdf[s].info->spdf.cpdf[m].mpdf->hook;
   if (!acc) return;
   tacc =  (ans && yes != NULL) ? yes : no;
            if(first) tacc->occ+=acc->occ;
            tacc->accse[s].accme[m].occ+=acc->occ;
            vSize = tacc->accse[s].accme[m].vSize;
            tsum = tacc->accse[s].accme[m].sum;  tsqr = tacc->accse[s].accme[m].sqr;
            asum = acc->sum;                     asqr = acc->sqr;
            
            for (k=1; k<=vSize; k++) {
               tsum[k] += asum[k];
               tsqr[k] += asqr[k];
            }
         }
         first=FALSE;
      }
   }   
}

/* ClusterLogL: return log likelihood of given cluster, this is used for both
   the complete and split clusters.  In the former case, the yes accumulator
   will be NULL and only the no accumulator is used */
double ClusterLogL(CLink clist, AccSum *no, AccSum *yes, double *occs)
{
   CLink p;
   double prob;
   StateElem *se;
   int i;

   if (clist->item->item == clist->item->owner) {
      prob=0.0;
      occs[FALSE]=0.0;occs[TRUE]=0.0;
      for (i=2;i<clist->item->owner->numStates;i++) {
         ZeroAccSum(no);
         if (yes != NULL) ZeroAccSum(yes);
         for(p=clist;p!=NULL;p=p->next) {
            IncSumSqr(p->item->owner->svec[i].info,p->ans,no,yes);
         }
         prob += AccSumProb(no);
         if (yes != NULL) prob += AccSumProb(yes);
         if (occs != NULL) {
            occs[FALSE] += no->occ;
            if (yes != NULL)
               occs[TRUE] += yes->occ;
         }
      }
   }
   else {
      ZeroAccSum(no);
      if (yes != NULL) ZeroAccSum(yes);
      for(p=clist;p!=NULL;p=p->next) {
         se=(StateElem*)p->item->item;
         IncSumSqr(se->info,p->ans,no,yes);
      }
      prob = AccSumProb(no);
      if (yes != NULL) prob += AccSumProb(yes);
      if (occs != NULL) {
         occs[FALSE] = no->occ;
         occs[TRUE] = (yes != NULL) ? yes->occ : 0.0;
      }
   }
   return(prob);
}

/* SetQMTable: Set Question x Model bit table */
void SetQMTable (void)
{
   char *name,**PMTable;
   int i,j,h;
   MLink m;
   ILink p,q;
   Boolean answer;
   IPat *ip;
   Question *question;
  
   const int size=(hset->numPhyHMM>>3)+1;

   if (trace&T_BAS) {
      printf("   setting question*model table...");
      fflush(stdout);
}

   /* set pattern * model table */
   PMTable = (char **) New(&tmpHeap, nPatterns*sizeof(char *));
   
   for (i=0; i<nPatterns; i++) {
      PMTable[i] = (char *) New(&tmpHeap, size*sizeof(char));
      memset(PMTable[i], 0, size);
   }
      
#pragma omp parallel for private(m,h,i,j,name,p,ip,answer)
   for (h=0; h<MACHASHSIZE; h++) {
      for (m=hset->mtab[h]; m!=NULL; m=m->next) {
         if (m->type == 'h') {
            /* index and name of current hmm */
            i = ((HLink)(m->structure))->hIdx >> 3;  /* division by 8 */
            j = ((HLink)(m->structure))->hIdx &  7;  /* residue by 8 */
            name = m->id->name;

            /* check answers */
            for (p=pList; p!=NULL; p=p->next) {
               ip = (IPat *)p->item;
               answer = DoMatch(name, ip->pat);
               if (answer)
                  PMTable[ip->index][i] |= (1<<j);  /* set j-th bit */
            }
         }
      }
   }

   /* set question * model table */
   QMTable = (char **) New(&questHeap, nQuestions*sizeof(char *));

   for (i=0; i<nQuestions; i++) {
      QMTable[i] = (char *) New(&questHeap, size*sizeof(char));
      memset(QMTable[i], 0, size);
   }

#pragma omp parallel for private(h,i,j,q,question,p,ip)
   for (h=0; h<hset->numPhyHMM; h++) {
      i = h>>3;  j = h&7;
      for (q=qList; q!=NULL; q=q->next) {
         question = (Question *)q->item;
         for (p=question->patList; p!=NULL; p=p->next) {
	    ip = (IPat *)p->item;
	    if (PMTable[ip->index][i] & (1<<j)) {
	       QMTable[question->index][i] |= (1<<j);  /* set j-th bit */
	       break;
	    }
         }
      }
   }

   /* free PMTable */
   Dispose(&tmpHeap, PMTable);

   if (trace&T_BAS) {
      printf("done\n");
      fflush(stdout);
   }

   setQMTable = TRUE;

   return;
}

/* AnswerQuestion: set ans field in each cluster item in preparation for
   a possible split */
Boolean AnswerQuestion(Node *node, Question *q)
{
   CLink p;
   ILink i;
   int yes=0, no=0, index, j;
   MLink m;
        
   switch(reduceMem) {
   case 2:
      for (p=node->clist;p!=NULL;p=p->next) {
         m = (MLink)p->item->owner->hook;
         p->ans = QMatch(m->id->name, q);
         if (p->ans) yes++;
         else        no++;
      }
      break;
   case 1:
      index = q->index;
      if (!setQMTable) SetQMTable();
      for (p=node->clist; p!=NULL; p=p->next) {
         j = p->item->owner->hIdx & 7;  /* residue by 8 */
         p->ans = (QMTable[index][p->item->owner->hIdx>>3] & (1<<j)) ? TRUE : FALSE;
         if (p->ans) yes++;
         else        no++;
      }
      break;
   case 0:
   default: 
      /* set ans=FALSE for items in this cluster */
      for (p=node->clist;p!=NULL;p=p->next) {
         p->ans = FALSE;
         p->owner = node;
         no++;
      }
         
      /* set ans=TRUE for all items matching this question */
      for (i=q->ilist;i!=NULL;i=i->next)
         if ((p=(CLink) i->item)!=NULL && p->owner==node) {
            p->ans=TRUE;
            yes++;
            no--;
         }
   }
   
   if (yes>0 && no>0)
      return TRUE;
   else
      return FALSE;
}

/* ValidProbNode: set tProb and sProb of given node according to best
   possible question which is stored in quest field.  */
void ValidProbNode(Node *node, const double thresh, const Boolean ref)
{
   ILink i;
   Question *q, *qbest;
   double best,sProb;
   char buf[20];
   
   node->tProb = ClusterLogL(node->clist,&no,NULL,occs);
   node->occ = occs[FALSE];
   if (trace & T_TREE_BESTQ) {
      if (node->parent==NULL)
         sprintf(buf," ROOT ");
      else
         sprintf(buf,"%3d[%c]",node->parent->snum,
                 ((node->parent->yes==node)?'Y':'N'));
      
      printf("    Node %s:                  LogL=%5.3f (%.1f)\n",
             buf,node->tProb/node->occ,node->occ);
      fflush(stdout);
   }
   qbest = NULL;
   best = node->tProb;
   
   if (!ref || node->quest!=NULL) {
      for (i=(node->parent==NULL || ref) ? qList : node->parent->qlist; i!=NULL; i=i->next) {
         q = (ref) ? node->quest : (Question *)i->item;
         if (AnswerQuestion(node, q)) {
      sProb = ClusterLogL(node->clist,&no,&yes,occs);
            if (node->occ<=0.0 || (outlierThresh >= 0.0 && (occs[FALSE]<outlierThresh || occs[TRUE]<outlierThresh)))
         sProb=node->tProb;

      if (trace & T_TREE_ALLQ || 
          ((trace & T_TREE_OKQ) && (sProb-node->tProb)>thresh)) {
         printf("       Q %20s    LogL=%-7.3f  Imp = %8.2f (%.1f,%.1f)\n",
                q->qName->name,sProb/node->occ,sProb-node->tProb,
                occs[0],occs[1]);
         fflush(stdout);
      }
      if (sProb>best) {
               if (qbest!=NULL && !ref) AddItem(NULL,qbest,&node->qlist);
         best=sProb;
         qbest=q;
      }
            else if (sProb>LSMALL && !ref) 
               AddItem(NULL,q,&node->qlist);
         }
	 if (ref)
	    break;
   }
   if (trace & T_TREE_BESTQ) {
      if (qbest != NULL)
            printf("    BestQ %20s   LogL %6.3f Imp %8.2f\n",
                qbest->qName->name,best/node->occ,best-node->tProb);
      else
         printf("    BestQ [ NULL ]\n");
      fflush(stdout);
   }
   }
   node->sProb=best; node->quest=qbest;
}

/* SplitTreeNode: split the given node */
void SplitTreeNode(Tree *tree, Node *node, const Boolean ref)
{
   CLink cl,nextcl;

   if (node->quest == NULL) return;
   cprob += node->sProb - node->tProb;

   AnswerQuestion(node,node->quest);
   if (ref) {
      node->yes->next = NULL;
      node->yes->prev = NULL;
      node->no->next = NULL;
      node->no->prev = NULL;
   }
   else {
   node->yes = CreateTreeNode(NULL,node);
   node->no = CreateTreeNode(NULL,node);
   }
   node->yes->ans = TRUE;
   node->no->ans = FALSE;

   for(cl=node->clist;cl!=NULL;cl=nextcl) {
      nextcl=cl->next;
      switch(cl->ans) {
      case FALSE: 
         cl->next=node->no->clist;
         node->no->clist=cl;
         break;
      case TRUE:  
         cl->next=node->yes->clist;
         node->yes->clist=cl;
         break;
      default:
         HError(2694,"SplitTreeNode: Unspecified question result");
      }
   }
   node->clist = NULL;

   numTreeClust++;
}

/* AddLeafList: add given node into leaf list in the order of likelihood improvement */
void AddLeafList(Node *node, Tree *tree, double threshold, const Boolean parent)
{
   Node *p,*n;
   double imp = node->sProb - node->tProb;

   /* delete node->parent from leaves */
   if (parent) {
      p = node->parent;
      if (p!=NULL) {
         if (p==tree->leaf)
            tree->leaf = p->next;
         if (p->prev!=NULL)
            p->prev->next = p->next;
         if (p->next!=NULL)
            p->next->prev = p->prev;
         p->prev = p->next = NULL;
      }
   }

   /* search insert location of given node */
   p=NULL; n=tree->leaf;
   while (n != NULL && imp < n->sProb-n->tProb) {
      if (n->sProb-n->tProb < threshold) break;
      p=n; n=n->next;
   }
   
   if (p==NULL) tree->leaf = node;
   if (p!=NULL) p->next = node;
   if (n!=NULL) n->prev = node;
   
   node->prev = p;
   node->next = n;  
      
   /* Free question list of given node if imp. is less than thresh. */
   if (imp<threshold)
      FreeItems(&node->qlist);
   
   return;
}

/* FindBestSplit: find best node to split */
Node *FindBestSplit(Node *first, const double threshold)
{
   if (first->sProb - first->tProb > threshold) 
      return first;
   else
      return NULL;
}

/* MergeCost: return logL reduction if node b is merged with node a.
   On entry, atail points to last cluster item in node a clist. */
double MergeCost(Node *a, Node *b, CLink atail)
{
   double combProb;              /* combined logL */
   
   atail->next = b->clist;
   combProb = ClusterLogL(a->clist,&no,NULL,occs);
   atail->next = NULL;
   return a->tProb + b->tProb - combProb;
}

/* MergeNode: find the as yet unseen node which when merged with
   given node gives smallest reduction in LogL.  If this reduction
   is less than threshold, merge the nodes and return TRUE */
Boolean MergeNode(Node *node, const double threshold)
{
   Node *p, *min;
   CLink ctail;
   double minCost,cost;

   ctail = node->clist;         /* set ctail to last cluster item */
   if (ctail == NULL)
      HError(2690,"MergeNode: empty cluster list");
   while (ctail->next != NULL) ctail = ctail->next;

   /* find bestnode to merge */
   minCost = threshold;min=NULL;
   for (p=node->next;p!=NULL;p=p->next) {
      cost = MergeCost(node,p,ctail);
      if (trace & T_TREE_ALLM) {
         char buf1[20],buf2[20];
         if (node->parent==NULL) sprintf(buf1," ROOT ");
         else sprintf(buf1,"%3d[%c]",node->parent->snum,
                      ((node->parent->yes==node)?'Y':'N'));
         if (min->parent==NULL) sprintf(buf2," ROOT ");
         else sprintf(buf2,"%3d[%c]",min->parent->snum,
                      ((min->parent->yes==min)?'Y':'N'));
         printf("       M  %s (%.3f,%.1f) + %s (%.3f,%.1f)   Imp %.1f\n",
                buf1,node->tProb/node->occ,node->occ,
                buf2,min->tProb/min->occ,min->occ,-minCost);
      }
      if (cost < minCost) {
         minCost = cost; min = p;
      }
   }
   if (minCost < threshold) {   /* Merge nodes */
      if (trace & T_DET) {
         char buf1[20],buf2[20];
         if (node->parent==NULL) sprintf(buf1," ROOT ");
         else sprintf(buf1,"%3d[%c]",node->parent->snum,
                      ((node->parent->yes==node)?'Y':'N'));
         if (min->parent==NULL) sprintf(buf2," ROOT ");
         else sprintf(buf2,"%3d[%c]",min->parent->snum,
                      ((min->parent->yes==min)?'Y':'N'));
         printf("    BestM  %s (%.3f,%.1f) + %s (%.3f,%.1f)   Imp -%4.1f\n",
                buf1,node->tProb/node->occ,node->occ,
                buf2,min->tProb/min->occ,min->occ,minCost);
         fflush(stdout);
      }
      ctail->next = min->clist;
      node->tProb += min->tProb - minCost;
      node->occ += min->occ;
      if (min->parent->yes == min)     min->parent->yes = node;
      else if (min->parent->no == min) min->parent->no = node;
      else
         HError(2695,"MergeNode: cannot find min parent link");
      if (min->prev != NULL) min->prev->next = min->next;
      if (min->next != NULL) min->next->prev = min->prev;
      --numTreeClust;
      cprob-=minCost;
      return TRUE;
   } else
      return FALSE;
}

/* MergeLeaves: merge any pair of leaf nodes in given tree for which 
   reduction in combined LogL is less than threshold.  Note that 
   after this call, the parent links are no longer valid. */
void MergeLeaves(Tree *tree, const double threshold)
{
   Node *node;
   
   for (node=tree->leaf;node!=NULL;node=node->next)
      MergeNode(node,threshold);
}

/* GetCentroidStr: set centroid of given StreamInfo according to AccSumStrE */
void GetCentroidStr (StreamInfo *sti, AccSumStrE *astr, Vector vfloor)
{
   double v, occ, tocc=0.0;
   int m, k, vSize;
   MixPDF *mp;
   DVector sum, sqr;
   AccSumMixE *amix;
   
   for (m=1; m<=astr->nMix; m++) {
      mp = sti->spdf.cpdf[m].mpdf;
      amix = astr->accme+m;
      sum = amix->sum;
      sqr = amix->sqr;
      occ = amix->occ;
      vSize = amix->vSize;

      if (occ > 0.0) {
         for (k=1; k<=vSize; k++) {
            mp->mean[k] = (float)(sum[k]/occ);
            v = (sqr[k]-(sum[k]*sum[k]/occ))/occ;
            if (applyVFloor && v<vfloor[k])
               v = (double)vfloor[k];
            mp->cov.var[k] = (float)v;
         }
         tocc += occ; 

         FixDiagGConst(mp);  /* fix gConst */
      }
   }
   
   for (m=1; m<=astr->nMix; m++)
      sti->spdf.cpdf[m].weight = (float)(astr->accme[m].occ/tocc);
      
   return;
}
            
/* GetCentroid: set centroid of given statistics */
void GetCentroid (MLink macro, CLink clist, AccSum *acc, const int stream)
{
   int j,s;
   CLink p;
   HLink hmm;
   StateInfo *si;
   
   switch(macro->type) {
   case 'h':   /* HMM-level clustering */
      hmm = (HLink) macro->structure;
      for (j=2; j<hmm->numStates; j++) {
         /* acc should be re-calculated */
         ZeroAccSum(acc);
         for (p=clist; p!=NULL; p=p->next)
            IncSumSqr(p->item->owner->svec[j].info, FALSE, acc, NULL);
         
         si = hmm->svec[j].info;
         for (s=1; s<=acc->nStream; s++)
            GetCentroidStr(si->pdf[s].info, acc->accse+s, vf[s]);
      }
      break;
   case 's':   /* state-level clustering */
      si = (StateInfo *)macro->structure;
      for (s=1; s<=acc->nStream; s++)
         GetCentroidStr(si->pdf[s].info, acc->accse+s, vf[s]);
      break;
   case 'p':  /* stream-level clustering */
      GetCentroidStr((StreamInfo *)macro->structure, acc->accse+stream, vf[stream]);
      break;
   default:
      HError(2640,"GetCentroid: macro type %c not supported", macro->type);
   }

   return;
}

/* TieLeafNodes: tie all items in the leaf nodes of given tree */
void TieLeafNodes(Tree *tree, char *macRoot)
{
   double occ;
   ILink ilist,i;
   CLink cl;
   char clnum[20];              /* cluster number */
   int clidx;                   /* cluster index */
   char buf[MAXSTRLEN],str[MAXSTRLEN];  /* construct mac name in this */
   Node *node;
   int s,j,numItems = 0;
   LabId id = NULL;

   if (trace & T_CLUSTERS) printf("\n Nodes for %s\n",macRoot);
   cprob=0.0; occ=0.0;
   clidx = numTreeClust;
   for (node=tree->leaf;node!=NULL;node=node->next) {
      if (node->clist!=NULL) {
      cprob += ClusterLogL(node->clist,&no,NULL,occs); /*==node->tProb;*/
      occ += occs[FALSE];
         node->macro = (node->macro==NULL) ? (MLink *) New(&hmmHeap,tree->nActiveStr*sizeof(MLink)) : node->macro;
      sprintf(clnum,"%d",clidx--); /* construct macro name */
      strcpy(buf,macRoot);
      strcat(buf,clnum);
      if (trace & T_CLUSTERS) {
         char buf[20];
         if (node->parent==NULL) sprintf(buf,"ROOT");
         else sprintf(buf,"%d[%c]",node->parent->snum,
                         ((node->parent->yes==node)?'Y':'N'));
         printf("  %-6s== %-3d LogL %6.3f (%.1f) ==",buf,clidx,
                node->tProb/node->occ,node->occ);
      }
      ilist = NULL;
      for (cl=node->clist;cl!=NULL;cl=cl->next) {
         i = cl->item;
         if (trace & T_CLUSTERS)
            printf(" %s",HMMPhysName(hset,i->owner));

         i->next = ilist;
         ilist = i;
      }
      numItems += NumItems(ilist);
      if (trace & T_CLUSTERS)
          printf("\n");
         if (hset->swidth[0] == tree->nActiveStr) {  /* not stream clustering */
            if (node->id!=NULL)
               strcpy(buf,node->id->name);
            ApplyTie(ilist,buf,((ilist->item==ilist->owner)?'h':'s'));
            id = (node->id==NULL) ? GetLabId(buf,FALSE) : node->id;
            node->macro[0] = FindMacroName(hset,((ilist->item==ilist->owner)?'h':'s'),id);
            if (useLeafStats)
               GetCentroid(node->macro[0], node->clist, &no, 0);
         }
         else {
            for (s=1,j=0;s<=hset->swidth[0];s++) {
               if (tree->streams.set[s]) {
                  if (node->id==NULL) 
                     strcpy(str,buf);
                  else
                     strcpy(str,node->id->name);
                  if (tree->nActiveStr>1) {
                     sprintf(clnum,"%d",s);   /* construct macro name for each stream */
                     strcat(str,"-");
                     strcat(str,clnum);
                  }
                  id=GetLabId(str,TRUE);
                  TieLeafStream(ilist,id,s);
                  node->macro[j] = FindMacroName(hset,'p',id);
                  if (useLeafStats)
                     GetCentroid(node->macro[j], cl, &no, s);
                  j++;
      }
         }
            if (tree->nActiveStr>1)
               id=GetLabId(buf,TRUE);
      }
         if (node->id==NULL)
            node->id = id;
   
      FreeItems(&ilist);        /* free items at this node */
      node->clist=NULL;
   }
      else
         HError(-9999,"TieLeafNodes: no HMMs/states are assigned to node %d", -node->snum);
   }
}

static Node *GetNode(Node *node,int n)
{
   Node *ret;
  
   ret=NULL;
   if (node->snum==n) return(node);
   else {
      if (node->yes) {
         ret=GetNode(node->yes,n);
         if (ret) return(ret);
      }
      if (node->no) {
         ret=GetNode(node->no,n);
         if (ret) return(ret);
      }
   }
   return(NULL);
}

/* BuildTree: build a tree for objects stored in ilist and return
   the actual clusters in rlist, the number of clusters
   in numCl.  The value of threshold determines when to stop.
   macRoot is the root prefix of the name to use in the tie  */
void BuildTree (ILink ilist, double threshold, char *macRoot, char *pattern)
{
   int i,j,l,s,m,N,snum,state,numItems,numParam,count;
   char buf[PAT_LEN];
   HMMDef *hmm;
   CLink clHead,cl;
   ILink p, k;
   Question *q;
   LabId labid;
   Node *node,*n;
   Tree *tree = NULL,*t;
   static int totalItems = 0;   /* for overall statistics */
   static int totalClust = 0;
   
   SetTreeName(macRoot);
   l = hset->vecSize;
   /* Initialise Global AccSums for yes - no branches */
   yes.sum=CreateDVector(&tmpHeap,l);  yes.sqr=CreateDVector(&tmpHeap,l);
   no.sum=CreateDVector(&tmpHeap,l);   no.sqr=CreateDVector(&tmpHeap,l);
   MakeAccSum(&yes,ilist);
   MakeAccSum(&no,ilist);

   /* Check object consistency */
   hmm = ilist->owner;          /* any HMM will do */
   N = hmm->numStates;
   for(p=ilist;p!=NULL;p=p->next) 
      ChkTreeObject(p,&no);

   /* Create a new tree in tree list */
   /* Find base name and state if any */
   hmm = ilist->owner;
   if (usePattern)
      strcpy(buf,pattern);
   else {
      strcpy(buf,HMMPhysName(hset,hmm)); 
      TriStrip(buf);
   MapTreeName(buf);
   }
   labid = GetLabId(buf,TRUE);

   if (ilist->owner==ilist->item)
      state=-1;
   else
      for (j=2,state=0; j<hmm->numStates;j++)
         if (hmm->svec+j == (StateElem *)ilist->item) {
            state = j; break;
         }
   if (state==0)
      HError(2663,"BuildTree: cannot find state index");

   if (useRefTree >= 0) {
     for (t=treeList;t!=NULL;t=t->next) {
       if (strcmp(t->baseId->name,labid->name)==0 && t->state==state) {
         tree = t;
         if (trace & T_BID) {
	   printf("(using reference tree %3s.state[%d]) ",tree->baseId->name,state);
	   fflush(stdout);
         }
	 break;
       }
     }
     if (t==NULL)
       HError(2663,"BuildTree: cannot find reference tree");
   }
   else
   tree = CreateTree(ilist,labid,state);

   /* Create a single cluster, owner->hook links to cluster record */
   i=1; clHead=NULL; count=0;
   for (p=ilist;p!=NULL;p=p->next,i++) {
      if (p->owner == p->item) {
         count += p->owner->svec[2].info->stateCounter;
         for (j=2;j<p->owner->numStates;j++)
            InitTreeAccs(p->owner->svec+j);
      }
      else
         InitTreeAccs((StateElem*)p->item);
      cl=(CLink) New(&tmpHeap,sizeof(CRec));
      p->owner->hook = cl; /* HMM hook links to CRec */
      cl->item = p;  cl->ans = FALSE;
      cl->idx = i; cl->next = clHead;
      clHead = cl;
   }
   
   /* For each question make each hmm (item) point to its */
   /*  corresponding cluster member (CREC) via hmm->hook */
   for (k=qList; k!=NULL; k=k->next) {
      q = (Question *)k->item;
      for (p=q->ilist; p!=NULL; p=p->next)
         p->item = p->owner->hook;
   }
   for (p=ilist;p!=NULL;p=p->next,i++)
      p->owner->hook = FindMacroStruct(hset,'h',p->owner); /* hook MLink */

   /* Create the root of the tree */
   if (useRefTree >= 0) {
     tree->root->clist = clHead;
     tree->root->next  = NULL;
     tree->root->prev  = NULL;
   }
   else
     tree->root = CreateTreeNode(clHead,NULL);
   node = tree->leaf = tree->root;
   cprob = node->tProb = ClusterLogL(node->clist,&no,NULL,occs);
   node->occ = occs[FALSE];
   numTreeClust=1; numItems=NumItems(ilist);

   if (applyMDL) {
      numParam=0;
      for (s=1; s<=hset->swidth[0]; s++) {
         if (streams.set[s]) {
            for (m=1; m<=no.accse[s].nMix; m++)
               numParam += 2 * no.accse[s].accme[m].vSize;
            numParam += no.accse[s].nMix-1;
         }
      }
      
      if (ilist->owner == ilist->item)  /* HMM-level clustering */
         threshold = MDLfactor * 0.5 * (double)numParam * (N-2) * log(count);
      else  /* state-level or stream-level clustering */
         threshold = MDLfactor * 0.5 * (double)numParam * log(node->occ);

      if (threshold<0.0)
         threshold = 0.0;

      if (trace & T_BID) { 
         printf("based on MDL criterion, threshold=%e", threshold);
         fflush(stdout);
      }
   }
   
   if (trace & T_BID) {
      printf("\n");
      fflush(stdout);
   }

   if (trace & T_IND) {
      if (state<0) 
         printf(" Start  %4s : %d  have LogL=%6.3f occ=%4.1f\n",
                tree->baseId->name,
                numItems,cprob/node->occ,node->occ);
      else
         printf(" Start %3s.state[%d] : %d  have LogL=%6.3f occ=%4.1f\n",
                tree->baseId->name,state,numItems,cprob/node->occ,node->occ);
      fflush(stdout);
   }
   ValidProbNode(node,threshold,(useRefTree>=0)?TRUE:FALSE);

   /* Now build the actual tree by repeated node splitting */
   snum=0;
   if (useRefTree >= 0) {
      node=FindBestSplit(tree->leaf,(useRefTree==1||useRefTree==3)?0.0:threshold);
      while(node != NULL && snum < tree->size) {
	 node->id=NULL;
	 node->snum=snum++; node->quest->used=TRUE;
         if (trace & T_DET) {
            char buf[20];
            if (node->parent==NULL) sprintf(buf," ROOT ");
            else sprintf(buf,"%3d[%c]",node->parent->snum,
                         ((node->parent->yes==node)?'Y':'N'));
            printf("\n  Split %s == %-3d (%.1f)\n",
                   buf,node->snum,node->occ);
            printf("          %20s   LogL %7.3f -> %7.3f  Imp %.2f\n",
                   node->quest->qName->name,node->tProb/node->occ,
                   node->sProb/node->occ,node->sProb-node->tProb);
            fflush(stdout);
         }
         SplitTreeNode(tree,node,TRUE);
         ValidProbNode(node->yes,(useRefTree==1||useRefTree==3)?0.0:threshold,TRUE);
         ValidProbNode(node->no, (useRefTree==1||useRefTree==3)?0.0:threshold,TRUE);
   
         AddLeafList(node->yes,tree,(useRefTree==1||useRefTree==3)?0.0:threshold,TRUE);
         AddLeafList(node->no, tree,(useRefTree==1||useRefTree==3)?0.0:threshold,TRUE);

         if (trace & T_TREE_BESTQ) {
            printf("   Node %-3d  [Y] LogL %.3f (%.1f)    [N] LogL %.3f (%.1f)\n",
                   node->snum,node->yes->tProb/node->yes->occ,node->yes->occ,
                   node->no->tProb/node->no->occ,node->no->occ);
            fflush(stdout);
         }
         node = FindBestSplit(tree->leaf,(useRefTree==1||useRefTree==3)?0.0:threshold);
      }
   }

   if (useRefTree>=0) {
      if (useRefTree>=2)
         node=NULL;
      else {
         for (node=tree->leaf,j=1;node!=NULL;node=n,j++) {
            n = node->next;
            if (node->parent != NULL)
   	       node->parent->qlist = qList;
            ValidProbNode(node,threshold,FALSE);
            node->yes = NULL;
            node->no  = NULL;
            node->next= NULL;
            node->prev= NULL;
            node->id  = NULL;
            if (j>1)
   	       AddLeafList(node,tree,threshold,FALSE);
         }
	 node=FindBestSplit(tree->leaf,threshold);
      }
   }
   else
   node=FindBestSplit(tree->leaf,threshold);
   while(node != NULL) {
      node->snum=snum++; node->quest->used=TRUE;
      if (trace & T_DET) {
         char buf[20];
         if (node->parent==NULL) sprintf(buf," ROOT ");
         else sprintf(buf,"%3d[%c]",node->parent->snum,
                      ((node->parent->yes==node)?'Y':'N'));
         printf("\n  Split %s == %-3d (%.1f)\n",
                buf,node->snum,node->occ);
         printf("          %20s   LogL %7.3f -> %7.3f  Imp %.2f\n",
                node->quest->qName->name,node->tProb/node->occ,
                node->sProb/node->occ,node->sProb-node->tProb);
         fflush(stdout);
      }
      SplitTreeNode(tree,node,FALSE);
      ValidProbNode(node->yes,threshold,FALSE);
      ValidProbNode(node->no, threshold,FALSE);

      FreeItems(&node->qlist);
      
      AddLeafList(node->yes,tree,threshold,TRUE);
      AddLeafList(node->no, tree,threshold,TRUE);
      
      if (trace & T_TREE_BESTQ) {
         printf("   Node %-3d  [Y] LogL %.3f (%.1f)    [N] LogL %.3f (%.1f)\n",
                node->snum,node->yes->tProb/node->yes->occ,node->yes->occ,
                node->no->tProb/node->no->occ,node->no->occ);
         fflush(stdout);
      }
      node=FindBestSplit(tree->leaf,threshold);
   }
   tree->size=snum;
   for (p=ilist;p!=NULL;p=p->next,i++)
      p->owner->hook = NULL;
   
   /* Merge any similar leaf nodes */
   if (!applyMDL && treeMerge) {
      if (trace & T_IND) {
         if (state<0) 
            printf("\n Via    %4s : %d gives LogL=%6.3f occ=%4.1f\n",
                   tree->baseId->name,numTreeClust,
                   cprob/tree->root->occ,tree->root->occ);
         else
            printf("\n Via   %3s.state[%d] : %d gives LogL=%6.3f occ=%4.1f\n",
                   tree->baseId->name,state,numTreeClust,cprob/tree->root->occ,
                   tree->root->occ);
         fflush(stdout);
      }
      MergeLeaves(tree,threshold);
   }
   
   /* Finally assign a macro to each leaf node and do tie */
   totalItems += numItems; totalClust += numTreeClust;
   tree->nLeaves = numTreeClust;
   if (trace & T_IND) {
      if (state<0) 
         printf("\n End    %4s : %d gives LogL=%6.3f occ=%4.1f\n",
                tree->baseId->name,numTreeClust,
                cprob/tree->root->occ,tree->root->occ);
      else
         printf("\n End   %3s.state[%d] : %d gives LogL=%6.3f occ=%4.1f\n",
                tree->baseId->name,state,numTreeClust,cprob/tree->root->occ,
                tree->root->occ);
      fflush(stdout);
   }
   TieLeafNodes(tree,macRoot);
   FreeDVector(&tmpHeap,yes.sum);
   if (trace & T_BID) {
      printf("\n TB: Stats %d->%d [%.1f%%]  { %d->%d [%.1f%%] total }\n",
             numItems,numTreeClust,(float)numTreeClust*100.0/(float)numItems,
             totalItems,totalClust,(float)totalClust*100.0/(float)totalItems);
      fflush(stdout);
   }
   SetTreeName("");
}

/* AssignStructure: find shared state/stream info for given id and state/stream idx */
Ptr AssignStructure(LabId id, const int state)
{
   int len=0,stream,nTree,s,j;
   char buf[PAT_LEN];
   LabId tid = NULL;
   Tree *tree;
   Node *node;
   Boolean isYes;
   ILink tlist=NULL,i;
   MLink m = NULL;
   StateInfo *si = NULL;
   StreamElem *ste;
   
   /* First find Tree to use */
   strcpy(buf,id->name);
   if (!usePattern) {
      if (strchr(buf,'*')!=NULL || strchr(buf,'?')!=NULL)
         HError(9999,"AssignStructure: baseId %s includes pattern character", buf);
      TriStrip(buf); 
   MapTreeName(buf);
   tid = GetLabId(buf,FALSE);
   }
   
   for (tree=treeList,stream=0;tree!=NULL;tree=tree->next) {
      if ( ((usePattern && IPatMatch(buf, tree->patList)) || singleTree || (!singleTree && !usePattern && tree->baseId == tid))
           && tree->state == state ) {
         AddItem(NULL,tree,&tlist);
         stream += tree->nActiveStr;
      }
   }
   if (tlist==NULL && state<0) return(NULL);
   if (tlist==NULL)
      HError(2662,"AssignStructure: cannot find tree for %s state %d",
             id->name,state);
   if (stream!=hset->swidth[0]) 
      HError(9999,"AssignStructure: incompatible tree");

   nTree = NumItems(tlist);
   if (nTree>1) {
      si  = (StateInfo  *) New(&hmmHeap,sizeof(StateInfo));
      ste = (StreamElem *) New(&hmmHeap,hset->swidth[0]*sizeof(StreamElem));
      ste--;
      si->nUse=0; si->hook=NULL; si->stateCounter=0; si->dur=NULL; si->weights=NULL; 
      si->pdf = ste;
   }
      
   for (i=tlist;i!=NULL;i=i->next) {
      tree = (Tree *)i->item;
   /* Then move down tree until state is found */
   node = tree->root;
   if (trace & T_DET) {
      if (trace & T_TREE_ANS) printf("\n     ");
      if (state>0)
         printf(" [%d] ",state);
      fflush(stdout);
   }
   while (node->yes != NULL) {
      isYes = QMatch(id->name,node->quest);
         if (trace & T_TREE_ANS) printf("%s ",(isYes)?"yes":" no"),len+=4;
      node = (isYes)?node->yes:node->no;
   }
      if (nTree==1)   /* state-based clustering */
         m = (MLink)node->macro[0];
      else            /* stream-based clustering */
         for (s=1,j=0;s<=hset->swidth[0];s++)
            if (tree->streams.set[s])
               si->pdf[s].info = (StreamInfo *)node->macro[j++]->structure;

   if (trace & T_DET) {
      if (trace & T_TREE_ANS) printf("%*s",48-len," ");
            printf("=~%c %-10s ",((state>0)?((nTree>1)?'p':'s'):'h'),node->id->name);
      fflush(stdout);
   }
   }
   return((nTree>1)?si:m->structure);
}

/* FindProtoModel: find an allophone of given model in hList */
HLink FindProtoModel(LabId model)
{
   char phone[MAXSTRLEN], buf[MAXSTRLEN];
   int h;
   MLink q;
   HLink hmm=NULL;

   /* find an allophone of given model in hlist */
   strcpy(phone,model->name);
   TriStrip(phone);
   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next) 
         if (q->type=='h') {
            hmm = (HLink) q->structure;
            strcpy(buf,q->id->name);
            TriStrip(buf);
            if (strcmp(phone,buf)==0)
               return (hmm);
         }

   /* if no model matches to given model and usePatten or singleTree is set, 
    * return the last model in hlist */
   if (usePattern || singleTree)
      return (hmm);
   
   /* no model matches to given model */
   HError(2662,"FindProtoModel: no proto for %s in hSet",model->name);
   return hmm;
}

/* SynthStates: synthesise a HMM from state based trees */
HMMDef *SynthModel(LabId id)
{
   HMMDef *hmm,*proto;
   StateElem *se;
   int i,s,N;
   Boolean strShare;
   
   if (trace & T_DET) {
      printf("   Synth %-11s",id->name);
      fflush(stdout);
   }
   /* First find Tree to use */
   if ((hmm=(HMMDef *) AssignStructure(id,-1)) == NULL) {
      proto=FindProtoModel(id);
      hmm=(HLink) New(&hmmHeap,sizeof(HMMDef));
      N=hmm->numStates=proto->numStates;
      se=(StateElem*) New(&hmmHeap,sizeof(StateElem)*N);
      hmm->svec=se-2;hmm->owner=hset;
      hmm->dur=CloneSVector(hset->hmem,proto->dur,TRUE);
      hmm->nUse=0; hmm->hook=NULL;
      hmm->transP=CloneSMatrix(hset->hmem,proto->transP,TRUE);
      hmm->nUse = 1; hmm->hook = NULL; N = hmm->numStates;
      hmm->svec = se - 2;
      for (i=2; i<N; i++,se++) {
         se->info = (StateInfo *) AssignStructure(id,i);
         if (se->info->nUse==0) {
            strShare = FALSE;
            for (s=1; s<=hset->swidth[0]; s++) {
               if (se->info->pdf[s].info->nUse==0) 
                  HError(2695,"SynthModel: untied stream found for state %d stream %d",i,s);
               else {
                  strShare = TRUE;
                  se->info->pdf[s].info->nUse++;
               }
            }
            if (!strShare)
               HError(2695,"SynthModel: untied state found for state %d",i);
            else {
               se->info->dur = CloneSVector(hset->hmem, proto->svec[i].info->dur, TRUE);
               se->info->weights = CloneSVector(hset->hmem, proto->svec[i].info->weights, TRUE);
            }
         }
         else
            se->info->nUse++;
      }
      NewMacro(hset,fidx,'h',id,hmm);
   }
   if (trace & T_DET) {
      printf("\n");
      fflush(stdout);
   }
   return hmm;
}

/* TreeFilter: for each new model in newList, pass it thru trees and 
   add it to the currently loaded set. */
void TreeFilter(HMMSet *newSet)
{
   MLink p,q;
   HLink hmm;
   int seen,unseen,h;

   seen=unseen=0;
   for (h=0; h<MACHASHSIZE; h++)
      for (q=newSet->mtab[h]; q!=NULL; q=q->next) 
         if (q->type=='l') {
            p=FindMacroName(hset,'l',q->id);
            if (p==NULL) {
               hmm=SynthModel(q->id);
               unseen++;
               NewMacro(hset,fidx,'l',q->id,hmm);
               q->structure=hmm;
               /* Can do this as never use FindMacroStruct with newSet */
            }
            else {
               seen++;
               hmm=(HLink) p->structure;
               hmm->nUse++;
               q->structure=hmm;
            }
         }
   for (h=0; h<MACHASHSIZE; h++)
      for (q=newSet->mtab[h]; q!=NULL; q=q->next) 
         if (q->type=='h') {
            p=FindMacroName(newSet,'l',q->id);
            q->structure=p->structure;
         }
}

void DownTree(Node *node,Node **array) 
{
   if (node->yes) {
      if (node->snum>=0)
         array[node->snum]=node;
      DownTree(node->yes,array);
      DownTree(node->no,array);
   }
}

/* PrintQuestions: print questions for Tree-based clustering */
void PrintQuestions(FILE *file)
{
   Question *q;
   ILink i,p;
  
   for (i=qList;i!=NULL;i=i->next) {
      q = (Question *)i->item;
      if (q->used) {
         fprintf(file,"QS %s ",q->qName->name);
         fprintf(file,"{ %s",ReWriteString(((IPat *)q->patList->item)->pat,NULL,DBL_QUOTE));
         for (p=q->patList->next;p!=NULL;p=p->next)
            fprintf(file,",%s",ReWriteString(((IPat *)p->item)->pat,NULL,DBL_QUOTE));
      fprintf(file," }\n");
   }
   }
   fprintf(file,"\n");
}

/* PrintTree: print decision-tree */
void PrintTree(Tree *tree, FILE *file)
{
   Node *node, **array;
   int i;
   
      if (tree->size==0) {
      fprintf(file,"   %-45s\n\n", 
              ReWriteString(tree->root->id->name,NULL,DBL_QUOTE));
      }
      else {
         array=(Node**) New(&tmpHeap,sizeof(Node*)*tree->size);
         for (i=0;i<tree->size;i++) array[i]=NULL;
         DownTree(tree->root,array);
         fprintf(file,"{\n");
         for (i=0;i<tree->size;i++) {
            node=array[i];
            if (node==NULL) 
               HError(2695,"ShowTreesCommand: Cannot find node %d",i);
            if (node->yes==NULL || node->no==NULL) 
               HError(2695,"ShowTreesCommand: Node %d has no children",i);
         fprintf(file," %3d %-45s ",-i,
                 node->quest->qName->name);
            if (node->no->yes) fprintf(file,"  %5d    ",-node->no->snum);
         else fprintf(file," %15s ",
                      ReWriteString(node->no->id->name,NULL,DBL_QUOTE));
            if (node->yes->yes) fprintf(file,"  %5d    ",-node->yes->snum);
         else fprintf(file," %15s ",
                      ReWriteString(node->yes->id->name,NULL,DBL_QUOTE));
            fprintf(file,"\n");
         fflush(file);
         }
         fprintf(file,"}\n\n");
         Dispose(&tmpHeap,array);
      }
   }

/* ShowTrees: print a summary of currently constructed trees */
void ShowTreesCommand(void)
{
   Tree *tree;
   int i,s;
   FILE *file;
   char fn[MAXFNAMELEN];
   Boolean isPipe;
  
   ChkedAlpha("ST trees file name",fn);         /* get name of trees file */
   if (trace & T_BID) {
      printf("\nST %s\n Writing current questions/trees to file\n",fn);
      fflush(stdout);
   }
   if ((file=FOpen(fn,NoOFilter,&isPipe))==NULL)
      HError(2611,"ShowTreesCommand: Cannot open trees file %s",fn);
   
   PrintQuestions(file);
 
   for (tree=treeList;tree!=NULL;tree=tree->next) {
      if (tree->state>0)                       
         fprintf(file,"%s[%d]",
                 ReWriteString(tree->baseId->name,NULL,ESCAPE_CHAR),
                 tree->state);
      else
         fprintf(file,"%s",tree->baseId->name);
      if (tree->nActiveStr == hset->swidth[0])
         fprintf(file,"\n");
   else {
         fprintf(file,".stream[");         /* output active stream of current tree */
         for (s=1,i=1;s<=hset->swidth[0];s++)
            if (tree->streams.set[s])
                fprintf(file,"%d%s",s,(i++<tree->nActiveStr)?",":"]\n");
      }
      PrintTree(tree,file);
      }
   FClose(file,isPipe);
   }

float GetNodeOcc (MLink macro, char type)
{
   float occ;
   
   if (!occStatsLoaded || macro==NULL)
      return 0.0;
   
   switch(type) {
   case 's': memcpy(&occ,&(((StateInfo  *)macro->structure)->hook),sizeof(float)); break;
   case 'p': memcpy(&occ,&(((StreamInfo *)macro->structure)->hook),sizeof(float)); break;
   }
   
   return occ;
}    

Tree *LoadTree(char *name,Source *src)
{
   int n,nt,num_no,num_yes,s,nStream=1;
   char type;
   LabId lab_no,lab_yes,qname;
   Tree *tree;
   Node *node;
   char buf[MAXSTRLEN],bname[PAT_LEN],index[PAT_LEN],mname[MAXSTRLEN],*p;
   Boolean strTree=FALSE;
  
   strcpy(bname,name);
   if (strstr(bname,"stream")!=NULL) {
      strcpy(index,bname);
      *(strstr(bname,"stream")-1)='\0';
      strTree = TRUE;
   }

   if (name[strlen(bname)-1]==']' && strrchr(bname,'[')!=NULL) {
      p=strchr(bname,'[');
      *p++=0;
      if (sscanf(p,"%d",&n)!=1)
         HError(2660,"LoadTree: Cannot parse tree state %s %s",name,p);
      type='s';
   }
   else
      n=-1,type='h';

   
   if (GetLabId(bname,TRUE)==NULL)
      HError(2660,"LoadTree: Cannot parse tree name %s (base-phone)",name);

   tree=CreateTree(NULL,GetLabId(bname,FALSE),n);
   nt=0;tree->size=0;tree->nLeaves=0;
   tree->root=CreateTreeNode(NULL,NULL);
   tree->root->snum=0;

   if (strTree) {
     p = strrchr(index,'[')+1;

     while (*p==' ') p++;
     s = atoi(p);
     if (s<=0 || s>hset->swidth[0])
       HError(9999,"LoadTree: stream index out of range");
     tree->streams.set[s] = TRUE;
     while (*p!=',' && *p!=']') p++;
     while (*p==' ') p++;

     while (*p==',') {
        p++;
        s = atoi(p);
        if (s<=0 || s>hset->swidth[0])
           HError(9999,"LoadTree: stream index out of range");
        tree->streams.set[s] = TRUE;
        nStream++;
        while (*p!=',' && *p!=']') p++;
        while (*p==' ') p++;
     }
     
     if (*p != ']')
       HError(9999,"LoadTree: stream index ] expected");
     
     if (IsFullSet(tree->streams)) {
        type = 's';
        strTree = FALSE;
     }
     else {
        type = 'p';
     }
   }
   else {
      nStream = hset->swidth[0];
      for (s=1;s<=hset->swidth[0];s++)
         tree->streams.set[s] = TRUE;
   }
   tree->nActiveStr=nStream;

   ReadString(src,buf);
   if (strcmp(buf,"{")) {
      tree->root->macro = (MLink *) New(&hmmHeap,nStream*sizeof(MLink));
      if (strTree && nStream>1) {
         for (s=1,n=0;s<=hset->swidth[0];s++)
            if (tree->streams.set[s]) {
               strcpy(mname,buf);
               sprintf(index,"%d",s);   /* construct macro name for each stream */
               strcat(mname,"-");
               strcat(mname,index);
               if ((tree->root->macro[n++]=FindMacroName(hset,type,GetLabId(mname,FALSE)))==NULL)
                  if (useRefTree < 0)
                     HError(-2661,"LoadTree: Macro %s not recognised",mname);
            }
      }
      else{
         if ((tree->root->macro[0]=FindMacroName(hset,type,GetLabId(buf,FALSE)))==NULL)
            if (useRefTree < 0)
               HError(-2661,"LoadTree: Macro %s not recognised",buf);
      }
      tree->nLeaves=1;
      tree->root->id = GetLabId(buf, TRUE);
      tree->root->occ = GetNodeOcc(tree->root->macro[0],type);
      tree->leaf = tree->root;
   }
   else {
      while (ReadString(src,buf),strcmp("}",buf)!=0) {
         tree->size++;
         sscanf(buf,"%d",&n);

         ReadString(src,buf);
         qname=GetLabId(buf,FALSE);
         if ((qname==NULL)?TRUE:qname->aux==0)
            HError(2661,"LoadTree: Question %s not recognised",buf);

         if ((node=GetNode(tree->root,-n))==NULL)
            HError(2661,"LoadTree: Node %d not in tree",n);

         node->quest=(Question *) qname->aux;
         node->quest->used=(useRefTree<0)?TRUE:FALSE;
         node->yes=CreateTreeNode(NULL,node);
         node->yes->ans=TRUE;
         node->no=CreateTreeNode(NULL,node);
         node->no->ans=FALSE;

         ReadString(src,buf);
         if (sscanf(buf,"%d",&num_no)==1)
            lab_no=NULL;
         else {
            lab_no=GetLabId(buf,TRUE);
            node->id = lab_no;
         }

         ReadString(src,buf);
         if (sscanf(buf,"%d",&num_yes)==1)
            lab_yes=NULL;
         else {
            lab_yes=GetLabId(buf,TRUE);
            node->id = lab_yes;
         }

         if (lab_no) {
            node->no->id = lab_no;
            node->no->macro = (MLink *) New(&hmmHeap,nStream*sizeof(MLink));
            if (strTree && nStream>1) {   /* stream share */
               for (s=1,n=0;s<=hset->swidth[0];s++)
                  if (tree->streams.set[s]) {
                     strcpy(mname,lab_no->name);
                     sprintf(index,"%d",s);   /* construct macro name for each stream */
                     strcat(mname,"-");
                     strcat(mname,index);
                     if ((node->no->macro[n++]=FindMacroName(hset,type,GetLabId(mname,FALSE)))==NULL)
                        if (useRefTree < 0)
                           HError(-2661,"LoadTree: Macro %s not recognised",mname);
                  }
            }
            else{
               if ((node->no->macro[0]=FindMacroName(hset,type,lab_no))==NULL) 
                  if (useRefTree < 0)
                     HError(-2661,"LoadTree: Macro %s not recognised",lab_no->name);
            }
            node->no->snum=--nt;
            node->no->next=tree->leaf;
            if (tree->leaf) tree->leaf->prev=node->no;
            node->no->prev=NULL;
            node->no->occ=GetNodeOcc(node->no->macro[0],type);
            tree->leaf=node->no;
            tree->nLeaves++;
         }
         else 
            node->no->snum=-num_no,node->no->macro=NULL;

         if (lab_yes) {
            node->yes->id = lab_yes;
            node->yes->macro = (MLink *) New(&hmmHeap,nStream*sizeof(MLink));
            if (strTree && nStream>1) {   /* stream share */
               for (s=1,n=0;s<=hset->swidth[0];s++)
                  if (tree->streams.set[s]) {
                     strcpy(mname,lab_yes->name);
                     sprintf(index,"%d",s);   /* construct macro name for each stream */
                     strcat(mname,"-");
                     strcat(mname,index);
                     if ((node->yes->macro[n++]=FindMacroName(hset,type,GetLabId(mname,FALSE)))==NULL)
                        if (useRefTree < 0)
                           HError(-2661,"LoadTree: Macro %s not recognised",mname);
                  }
            }
            else {
               if ((node->yes->macro[0]=FindMacroName(hset,type,lab_yes))==NULL) 
                  if (useRefTree < 0)
                     HError(-2661,"LoadTree: Macro %s not recognised",lab_yes->name);
            }
            node->yes->snum=--nt;
            node->yes->next=tree->leaf;
            if (tree->leaf) tree->leaf->prev=node->yes;
            node->yes->prev=NULL;
            node->yes->occ=GetNodeOcc(node->yes->macro[0],type);
            tree->leaf=node->yes;
            tree->nLeaves++;
         }
         else
            node->yes->snum=-num_yes,node->yes->macro=NULL;
      }
   }
   return(tree);
}

void LoadTreesCommand(void)
{
   Source src;
   char qname[MAXSTRLEN],buf[PAT_LEN],info[PAT_LEN];
   char fn[MAXSTRLEN];
   ILink ilist=NULL;
   char type;
   char *pattern;
  
   ChkedAlpha("LT trees files name",fn);        /* get name of trees file */
   if (trace & T_BID) {
      printf("\nLT %s\n Loading questions/trees from file\n",fn);
      fflush(stdout);
   }
   if(InitSource(fn,&src,NoFilter)<SUCCESS)
      HError(2610,"LoadTreesCommand: Can't open file %s", fn);

   while(ReadStringWithLen(&src,info,PAT_LEN)) {
      if (strcmp(info,"QS")!=0)
         break;
      ReadString(&src,qname);
    
      if (useRefTree >= 0 && reduceMem == 0) {
	 ilist=NULL;
	 type='h';
	 pattern = PItemList(&ilist,&type,hset,&src,NULL,0,0,(trace&T_ITM)?TRUE:FALSE);

	 if (trace & T_QST) {
	    printf("\nQS %s %s Define question\n",qname,pattern);
	    fflush(stdout);
	 }
	 if (ilist==NULL && (trace & T_QST))
	    HError(-2631,"QuestionCommand: No items for question %s\n",qname);
	 else {
	    LoadQuestion(qname,ilist,pattern);
	    if (trace & T_QST) {
	       printf(" QS: Refers to %d of %d models\n",
		      NumItems(ilist),hset->numLogHMM);
	       fflush(stdout);
	    }
	 }
      }
      else {
         ReadLine(&src,buf);
      LoadQuestion(qname,NULL,buf);
   }
   }
   /* comment out for no-question tree
   if (qList==NULL)
      HError(2660,"LoadTreesCommand: Questions Expected");
   */

   do {
      LoadTree(info,&src);
   }
   while (ReadStringWithLen(&src,info,PAT_LEN));
   CloseSource(&src);
}

/* ----------------- CM - Convert HMMSets to PDF --------------- */
void ConvertModelsCommand(void)
{
   Boolean out[SMAX], first;
   int i, j, k, s, v, vSize, sSize, idx, nMix;
   float weight, mean, vari;
   Tree *tree;
   FILE *file = NULL;
   char dn[MAXSTRLEN], fn[MAXSTRLEN], head[MAXSTRLEN], ext[MAXSTRLEN];
   Node *leaf, **array;
   StreamInfo *sti;
   Boolean isPipe;

   ChkedAlpha("CM output directory name", dn);
   if (trace & T_BID) {
      printf("\nCM %s\n Convert current HMMSet into the hts_engine format\n",dn);
      fflush(stdout);
   }

   /* check hset is valid for converting */
   if (hset->hsKind==DISCRETEHS)
      HError(9999,"ConvertModels: Only continuous HMMSet is supported");
   if (treeList==NULL)
      HError(2655,"ConvertModels: No trees loaded - use LT command");

   /* if parent/adapt transforms have been set, apply them */
   if (hset->curXForm==NULL && hset->parentXForm!=NULL)
      ApplyHMMSetXForm(hset, hset->parentXForm, TRUE);
   if (hset->curXForm!=NULL)
      ApplyHMMSetXForm(hset, hset->curXForm, TRUE);

   /* start conversion */
   strcpy(head,"pdf.");

   for (s=1; s<=hset->swidth[0]; s++) {
      nMix = MaxMixInSetS(hset, s);
      if ((hset->msdflag[s] && nMix>2) || (!hset->msdflag[s] && nMix>1)) 
         HError(9999,"ConvertModels: Only single mixture system is supported");
      out[s]=FALSE;
}

   for (s=1; s<=hset->swidth[0]; s++) {
      if (!out[s]) {
         /* output vector size and number of pdfs for each tree */
         first=TRUE; vSize=0; sSize=0;
         for (i=2; i<=MaxStatesInSet(hset)+1; i++) {
            for (tree=treeList; tree!=NULL; tree=tree->next) {
               if (tree->state==i && tree->streams.set[s] ) {
                  for (j=1; j<=hset->swidth[0]; j++)
                     if (tree->streams.set[j]) {
                        out[j]=TRUE;
                        vSize += hset->swidth[j];
                        sSize++;
                     }
                  if (first) {
                     /* write header */
                     sprintf(ext,"%d",s);
                     MakeFN(head,dn,ext,fn);
                     if ((file=FOpen(fn,NoOFilter,&isPipe))==NULL)
                        HError(2611,"ConvertModels: Cannot create output file %s",fn);
                     if(hset->msdflag[s])
                        k=1;
                     else
                        k=0;
                     WriteInt(file, &k, 1, TRUE);
                     WriteInt(file, &sSize, 1, TRUE);
                     WriteInt(file, &vSize, 1, TRUE); 
                     first=FALSE;
                  }
                  WriteInt(file, &tree->nLeaves, 1, TRUE);
               }
            }
         }

         /* output pdf (mixture weight, mean vector and covariance matrix) */
         for (i=2; i<=MaxStatesInSet(hset)+1; i++) {
            for (tree=treeList; tree!=NULL; tree=tree->next) {
               if (tree->state==i && tree->streams.set[s] ) {
                  /* store leaf node to array */
                  array = (Node **) New(&tmpHeap, tree->nLeaves*sizeof(Node *));
                  array--;
                  
                  for (leaf=tree->leaf; leaf!=NULL; leaf=leaf->next) {
                     idx = atoi(strrchr(leaf->id->name,'_')+1);
                     array[idx] = leaf;
                  }
                  
                  /* output array */
                  for (j=1; j<=tree->nLeaves; j++) {
                     for (k=0; k<tree->nActiveStr; k++) {
                        if (IsFullSet(tree->streams))
                           sti = ((StateInfo *)array[j]->macro[0]->structure)->pdf[k+1].info; /* state tying */
                        else
                           sti = (StreamInfo *)array[j]->macro[k]->structure;                 /* stream tying */
                        vSize = VectorSize(sti->spdf.cpdf[1].mpdf->mean);
                        for (v=1; v<=VectorSize(sti->spdf.cpdf[1].mpdf->mean); v++) {
                           switch (sti->spdf.cpdf[1].mpdf->ckind) {
                           case DIAGC:
                              vari = sti->spdf.cpdf[1].mpdf->cov.var[v];
                              break;
                           case INVDIAGC:
                              vari = 1.0/sti->spdf.cpdf[1].mpdf->cov.var[v];
                              break;
                           case FULLC:
                              vari = 1.0/sti->spdf.cpdf[1].mpdf->cov.inv[v][v];  /* only diagonal elements are used */
                              break;
                           default:
                              HError(999,"ConvertModels: not supported CovKind");
                           }
                           mean = sti->spdf.cpdf[1].mpdf->mean[v];
                           /* output mean & variance value */
                           WriteFloat(file, &mean, 1, TRUE);
                           WriteFloat(file, &vari, 1, TRUE);
                        }
                        /* for multi space probability distribution */
                        if (sti->nMix>1) {
                           if (vSize!=hset->swidth[sti->stream])
                              HError(9999,"ConvertModels: Vector size of the first mixture is not equal to stream width (%d, %d)", vSize, hset->swidth[sti->stream]);
                           /* output space weight */
                           weight = sti->spdf.cpdf[1].weight;
                           WriteFloat(file, &weight, 1, TRUE);
                           weight = 1-weight;
                           WriteFloat(file, &weight, 1, TRUE);
                        }
                     }
                  }
                  array++;
                  Dispose(&tmpHeap, array);
               }
            }
         }
         if (!first)
            FClose(file,isPipe);
      }
   }

   saveHMMSet = FALSE;
}

/* ----------------- CT - Convert Trees for synthesizer module --------------- */

void ConvertTreesCommand(void)
{
   Boolean out[SMAX], first;
   int i, j, s;
   Tree *tree;
   FILE *file = NULL;
   char dn[MAXSTRLEN], fn[MAXSTRLEN], head[MAXSTRLEN], ext[MAXSTRLEN];
   Boolean isPipe;
   
   ChkedAlpha("CT output directory ", dn);
   if (trace & T_BID) {
      printf("\nCT %s\n Convert current questions/trees into the hts_engine format\n",dn);
      fflush(stdout);
   }
   
   if (treeList==NULL)
      HError(2655,"ConvertTrees: No trees loaded - use LT command");

   strcpy(head,"trees.");
   for (s=1; s<=hset->swidth[0]; s++)
      out[s]=FALSE;

   for (s=1; s<=hset->swidth[0]; s++) {
      if (!out[s]) {      
         first = TRUE;
         for (i=2; i<=MaxStatesInSet(hset)+1; i++) {
            for (tree=treeList; tree!=NULL; tree=tree->next) {
               if (tree->state==i && tree->streams.set[s]) {
                  if (first) {
                     sprintf(ext,"%d",s);
                     MakeFN(head,dn,ext,fn);
                     if ((file=FOpen(fn,NoOFilter,&isPipe))==NULL)
                        HError(2611,"ConvertTrees: Cannot open file %s",fn);
                     PrintQuestions(file);
                     first = FALSE;
                  }
                  fprintf(file,"%s[%d]\n",
                          ReWriteString(tree->baseId->name,NULL,ESCAPE_CHAR),
                          tree->state);

                  PrintTree(tree, file);
                  
                  for (j=1; j<=hset->swidth[0]; j++)
                     if (tree->streams.set[j])
                        out[j]=TRUE;
               }
            }
         }
         if (!first)
            FClose(file,isPipe);
   }
   }

   saveHMMSet = FALSE;
}

/* --------- SM - Output KLD-based state mapping table ---------- */

static StreamInfo **DumpStream[SMAX];   /* StreamInfo list per stream index */
static int DStreamNum[SMAX];    /* number of StreamInfo per stream index */
static int *DStateIndexList[SMAX];      /* index of StreamInfo per stream index */

#define MAX_KLD_SCORE 1.0E10;

/* DumpAllStreams: dump all streams */
void DumpAllStreams(HMMSet * hset_L1)
{
   HMMScanState hss_L1;
   int s;

   /* count the total number of stream for each state */
   memset(DStreamNum, 0, sizeof(int) * SMAX);
   NewHMMScan(hset_L1, &hss_L1);
   do {
      while (GoNextState(&hss_L1, TRUE)) {
         while (GoNextStream(&hss_L1, TRUE)) {
            DStreamNum[hss_L1.s]++;
         }
      }
   } while (GoNextHMM(&hss_L1));
   EndHMMScan(&hss_L1);

   /* alloc memory for dumped streams */
   for (s = 1; s <= hset_L1->swidth[0]; s++)
      DumpStream[s] = (StreamInfo **) New(&tmpHeap, sizeof(StreamInfo *) * (DStreamNum[s] + 1));
   for (s = 1; s <= hset_L1->swidth[0]; s++)
      DStateIndexList[s] = (int *) New(&tmpHeap, sizeof(int) * (DStreamNum[s] + 1));

   /* dump all stream */
   memset(DStreamNum, 0, sizeof(int) * SMAX);
   NewHMMScan(hset_L1, &hss_L1);
   do {
      while (GoNextState(&hss_L1, TRUE)) {
         while (GoNextStream(&hss_L1, TRUE)) {
            DStreamNum[hss_L1.s]++;
            DumpStream[hss_L1.s][DStreamNum[hss_L1.s]] = hss_L1.sti;
            DStateIndexList[hss_L1.s][DStreamNum[hss_L1.s]] = hss_L1.i;
         }
      }
   } while (GoNextHMM(&hss_L1));
   EndHMMScan(&hss_L1);
}

/* PerformStateMapping: output state mapping table */
void PerformStateMapping(HMMSet * hset_L1, HMMSet * hset_L2, char *out_table)
{
   FILE *fp;
   int i = 0, s = 1, j;
   float min, tmp;
   Boolean isPipe;

   HMMScanState hss_L2;
   StreamInfo *sti = NULL;
   MLink ml_L2, ml_L1;

   /* Dump all streams for L1 model */
   DumpAllStreams(hset_L1);

   if ((fp = FOpen(out_table, NoOFilter, &isPipe)) == NULL)
      HError(6601, "PerformStateMapping: Can't open file %s", out_table);

   /* Map each mixture of L2 model to one mixture of L1 model */
   NewHMMScan(hset_L2, &hss_L2);
   do {
      while (GoNextState(&hss_L2, TRUE)) {
         while (GoNextStream(&hss_L2, TRUE)) {
            /* KLD mapping */
            sti = NULL;
            min = MAX_KLD_SCORE;
            for (j = 1; j <= DStreamNum[hss_L2.s]; j++) {
               if (hss_L2.sti->nMix == DumpStream[hss_L2.s][j]->nMix) {
                  tmp = CalStrKLDist(hss_L2.sti, DumpStream[hss_L2.s][j]);
                  if (tmp < min) {
                     min = tmp;
                     sti = DumpStream[hss_L2.s][j];
                  }
               }
            }

            /* output */
            ml_L2 = FindMacroStruct(hset_L2, 'p', hss_L2.sti);
            ml_L1 = FindMacroStruct(hset_L1, 'p', sti);
            if (ml_L2)
               fprintf(fp, "%s ", ReWriteString(ml_L2->id->name, NULL, ESCAPE_CHAR));
            else {
               HError(-999, "PerformStateMapping: Can't find macro name of L2");
               fprintf(fp, "%s_%d_%d ",
                       ReWriteString(HMMPhysName(hss_L2.hset, hss_L2.hmm), NULL, ESCAPE_CHAR),
                       hss_L2.i, hss_L2.s);
            }
            if (ml_L1)
               fprintf(fp, "%s\n", ReWriteString(ml_L1->id->name, NULL, ESCAPE_CHAR));
            else {
               HError(-999, "PerformStateMapping: Can't find macro name of L1");
               fprintf(fp, "%s",
                       ReWriteString(HMMPhysNameFromStreamInfo(hset_L1, sti, &i, &s), NULL, ESCAPE_CHAR));
               fprintf(fp, "_%d_%d\n", i, s);
            }
         }
      }
   } while (GoNextHMM(&hss_L2));
   EndHMMScan(&hss_L2);

   FClose(fp, isPipe);
}

/* StateMappingCommand: state mapping */
void StateMappingCommand()
{
   HMMSet hset_L2;
   char out_table[MAXFNAMELEN];
   char in_mmf[MAXFNAMELEN];
   char in_list[MAXFNAMELEN];

   /* initialise */
   ChkedAlpha("SM output mapping table", out_table);
   ChkedAlpha("SM input mmf file", in_mmf);
   ChkedAlpha("SM input list", in_list);

   if (trace & T_BID) {
      printf("\nSM %s %s %s\n Output state mapping table\n", out_table, in_mmf, in_list);
      fflush(stdout);
   }

   CreateHMMSet(&hset_L2, &hmmHeap, TRUE);
   AddMMF(&hset_L2, in_mmf);

   /* load */
   if (MakeHMMSet(&hset_L2, in_list) < SUCCESS)
      HError(2321, "StateMappingCommand: MakeHMMSet failed");
   if (LoadHMMSet(&hset_L2, hmmDir, hmmExt) < SUCCESS)
      HError(2321, "StateMappingCommand: LoadHMMSet failed");
   SetParmHMMSet(&hset_L2);
   if (hset_L2.ckind == INVDIAGC)
      ConvDiagC(&hset_L2, TRUE);

   /* state mapping */
   PerformStateMapping(hset, &hset_L2, out_table);

   saveHMMSet = FALSE;
}

/* ----------------- TR - Set Trace Level Command --------------- */

/* SetTraceCommand: set the trace level */
void SetTraceCommand(void)
{
   int m;

   m = ChkedInt("Trace level",0,31);
   if (trace & T_BID) {
      printf("\nTR %d\n Adjusting trace level\n",m);
      fflush(stdout);
   }
   if (m & T_MEM)
      PrintAllHeapStats();
   trace = ((cmdTrace&0xfff0) | (m&0xf));
}

/* ----------------- RN - Renaming HMMSet Command --------------- */

/* RenameHMMSetIdCommand: Change the HMM Set identifier */
void RenameHMMSetIdCommand(void)
{
   char buf[MAXSTRLEN];

   ChkedAlpha("RN Rename the HMM Set identifier",buf);
   hset->hmmSetId=CopyString(hset->hmem,buf);

}

/* -------------------- CL - Clone Command ---------------------- */

void SwapLists(HMMSet *set,HMMSet *list)
{
   MLink q;
   int h;

   /* First delete old HMM names */
   for (h=0; h<MACHASHSIZE; h++)
      for (q=set->mtab[h]; q!=NULL; q=q->next)
         if (q->type=='l' || q->type=='h')
            DeleteMacro(set,q);
   /* Then add new ones */
   for (h=0; h<MACHASHSIZE; h++)
      for (q=list->mtab[h]; q!=NULL; q=q->next) 
         if (q->type=='l' || q->type=='h')
            NewMacro(set,fidx,q->type,q->id,q->structure);
}

/* CloneCommand: clone the current HMM set to make the target
   set.  The 'basename' of every target hmm must be in the
   current HMMlist. */
void CloneCommand(void)
{
   char buf[MAXSTRLEN];
   HMMSet *tmpSet;
   HLink oldHMM,newHMM;
   MLink p,q;
   int h;
   
   ChkedAlpha("CL hmmlist file name",buf);
   if (trace & T_BID) {
      printf("\nCL %s\n Cloning current hmms to produce new set\n",buf);
      fflush(stdout);
   }
   tmpSet=(HMMSet*) New(&hmmHeap,sizeof(HMMSet));
   CreateHMMSet(tmpSet,&hmmHeap,TRUE);
   if(MakeHMMSet(tmpSet,buf)<SUCCESS)
      HError(2628,"CloneCommand: MakeHMMSet failed");
   
   for (h=0; h<MACHASHSIZE; h++)
      for (q=tmpSet->mtab[h]; q!=NULL; q=q->next) 
         if (q->type=='l') {
            if ((p=FindMacroName(hset,'l',q->id))==NULL)
               oldHMM=FindBaseModel(hset,q->id,baseMono);
            else
               oldHMM=(HLink) p->structure;
            newHMM=(HLink) q->structure;
            CloneHMM(oldHMM,newHMM,TRUE);
            newHMM->nUse=1;
         }
   SwapLists(hset,tmpSet);
}

/* -------------------- DP - Duplicate Command ---------------------- */

SVector DupSVector(SVector v)
{
   SVector w;

   if (v==NULL) return(NULL);
   if (GetUse(v)>0) {   /* vector is shared */
      if ((w=(SVector) GetHook(v))==NULL)
         HError(2692,"DupSVector: could not find replacement vector macro");
      IncUse(w);
   } else {                     /* vector not shared so Dup it */
      w = CloneSVector(&hmmHeap,v,FALSE);
   }
   return(w);
}

SMatrix DupSMatrix(SMatrix m)
{
   SMatrix w;

   if (m==NULL) return(NULL);
   if (GetUse(m)>0) {   /* matrix is shared */
      if ((w=(SMatrix) GetHook(m))==NULL)
         HError(2692,"DupSMatrix: could not find replacement matrix macro");
      IncUse(w);
   } else {                     /* matrix not shared so Dup it */
      w = CloneSMatrix(&hmmHeap,m,FALSE);
   }
   return(w);
}

STriMat DupSTriMat(STriMat m)
{
   STriMat w;

   if (m==NULL) return(NULL);
   if (GetUse(m)>0) {   /* TriMat is shared */
      if ((w=(STriMat) GetHook(m))==NULL)
         HError(2692,"DupSTriMat: could not find replacement TriMat macro");
      IncUse(w);
   } else {                     /* TriMat not shared so Dup it */
      w = CloneSTriMat(&hmmHeap,m,FALSE);
   }
   return(w);
}

/* DupMixPDF: return a Dup of given MixPDF */
MixPDF *DupMixPDF(MixPDF *s, Boolean frc)
{
   MixPDF *t;                   /* the target */
   int vSize;
   
   if (s->nUse>0 && !frc) {     /* shared struct so just return ptr to it */
      if ((t=(MixPDF *) s->hook)==NULL)
         HError(2692,"DupMixPDF: could not find dup mixpdf macro");
      ++t->nUse;
      return t;
   }
   vSize = VectorSize(s->mean);
   t = (MixPDF*) New(&hmmHeap,sizeof(MixPDF));
   t->nUse = 0; t->hook = NULL;
   t->ckind=s->ckind; t->gConst = s->gConst;
   t->mean=DupSVector(s->mean);
   switch(s->ckind) {
   case DIAGC:
   case INVDIAGC:
      t->cov.var = DupSVector(s->cov.var); break;
   case LLTC:
   case FULLC:
      t->cov.inv = DupSTriMat(s->cov.inv); break;
   case XFORMC:
      t->cov.xform = DupSMatrix(s->cov.xform); break;
   }
   return t;
}

/* DupStream: return a Dup of given stream */
StreamInfo *DupStream(StreamInfo *s, Boolean frc)
{
   int m,M;
   StreamInfo *t;
   MixtureElem *sme,*tme;

   if (s->nUse>0 && !frc) {     /* shared struct so just return ptr to it */
      if ((t=(StreamInfo *) s->hook)==NULL)
         HError(2692,"DupStream: could not find dup streamInfo macro");
      ++t->nUse;
      return t;
   }
   M = s->nMix;
   t = (StreamInfo *) New(&hmmHeap,sizeof(StreamInfo));
   t->nUse = 0; t->hook = NULL; 
   t->nMix = M; t->stream = s->stream;

   tme = (MixtureElem*) New(&hmmHeap,sizeof(MixtureElem)*M);
   t->spdf.cpdf = tme-1;
   sme = s->spdf.cpdf + 1;
   for (m=1; m<=M; m++,sme++,tme++) {
      tme->weight = sme->weight;
      if (tme->weight > MINMIX)
         tme->mpdf = DupMixPDF(sme->mpdf,FALSE);
      else
         tme->mpdf = NULL;
   }
   return t;
}

/* DupState: return a Dup of given State */
StateInfo *DupState(StateInfo *s, Boolean frc)
{
   StateInfo *t;                /* the target */
   StreamElem *tste,*sste;
   int st;
   
   if (s->nUse>0 && !frc) {    /* shared struct so just return ptr to it */
      if ((t=(StateInfo *) s->hook)==NULL)
         HError(2692,"DupState: could not find dup stateInfo macro");
      ++t->nUse;
      return t;
   }
   t = (StateInfo*) New(&hmmHeap,sizeof(StateInfo));
   t->nUse = 0; t->hook = NULL;
   tste = (StreamElem*) New(&hmmHeap,sizeof(StreamElem)*hset->swidth[0]);
   t->pdf = tste-1; 
   sste = s->pdf + 1;
   for (st=1; st<=hset->swidth[0]; st++,tste++,sste++) {
      tste->info = DupStream(sste->info,FALSE);
   }
   t->dur = DupSVector(s->dur);
   t->weights = DupSVector(s->weights);
   return t;
}

/* DupHMM: if src hasnt been Dupd before then just return
   a pointer to it, otherwise copy it.  The hook field of
   the HMM def is used to flag that a first copy has already 
   been taken */
HMMDef *DupHMM(HMMDef *src)
{
   HMMDef *tgt;
   StateElem *s,*t;
   int i;
  
   tgt = (HLink) New(&hmmHeap,sizeof(HMMDef));
   t = (StateElem*) New(&hmmHeap,sizeof(StateElem)*(src->numStates-2));
   tgt->owner = src->owner;
   tgt->numStates = src->numStates;
   tgt->dur = DupSVector(src->dur);
   tgt->transP = DupSMatrix(src->transP);
   tgt->svec = t-2; s = src->svec+2;
   for (i=2; i<tgt->numStates; i++,s++,t++) {
      if (hset->hsKind==TIEDHS || hset->hsKind==DISCRETEHS)
         t->info = CloneState(hset, s->info, FALSE);
      else
         t->info = DupState(s->info,FALSE);
   }
   tgt->hook = NULL; tgt->nUse = 1;
   src->hook = tgt;
   return tgt;
}

void DupMacro(MLink ml,LabId labid)
{
   Vector iv,ov;
   Matrix im,om;
   MixPDF *imp,*omp;
   StateInfo *isi,*osi;
   StreamInfo *isti,*osti;
   Ptr ostct=NULL;
    
   switch(ml->type) {
   case 's':
      isi=(StateInfo *) ml->structure;
      osi=DupState(isi,TRUE);
      ostct=osi;
      isi->hook=osi;
      break;
   case 'p':
      isti=(StreamInfo *) ml->structure;
      osti=DupStream(isti,TRUE);
      ostct=osti;
      isti->hook=osti;
      break;
   case 'm':
      imp=(MixPDF *) ml->structure;
      omp=DupMixPDF(imp,TRUE);
      ostct=omp;
      imp->hook=omp;
      break;
   case 'c':
   case 'i':
      im=(Matrix) ml->structure;
      om=DupSTriMat(im);
      ostct=om;
      SetHook(ml->structure,ostct);
      break;
   case 't':
   case 'x':
      im=(Matrix) ml->structure;
      om=DupSMatrix(im);
      ostct=om;
      SetHook(ml->structure,ostct);
      break;
   case 'v':
   case 'u':
   case 'w':
   case 'd':
      iv=(Vector) ml->structure;
      ov=DupSVector(iv);
      ostct=ov;
      SetHook(ml->structure,ostct);
      break;
   }
   if (trace & (T_DET | T_MAC)) {
      printf("   Dup Macro: ~%c:%-20s (from %s)\n",
             ml->type,labid->name,ml->id->name);
      fflush(stdout);
   }
   NewMacro(hset,fidx,toupper(ml->type),labid,ostct);
}

/* DuplicateCommand: clone the current HMM set to make the target
   set.  The 'basename' of every target hmm must be in the
   current HMMlist. */
void DuplicateCommand(void)
{
   char buf[MAXSTRLEN],name[MAXSTRLEN],*p,id[MAXSTRLEN],what[MAXSTRLEN];
   MLink ml;
   HLink hmm,newHMM;
   Ptr str;
   LabId labid;
   int i,N,h,nphmm=0,nlhmm=0;
   
   ChkedAlpha("DP macro types",what);
   N = ChkedInt("No. Duplicates",1,INT_MAX);
   if (trace & (T_BID | T_MAC)) {
      printf("\nDP %s %d 'id1' ..\n Duplicating current HMMSet\n",what,N);
      fflush(stdout);
   }

   ResetHooks(hset,NULL);
   occStatsLoaded = TRUE;  /* We just over-wrote them */
   
   for (i=1;i<=N;i++) {
      ChkedAlpha("DP macro name extension",id);
      if (trace & T_IND) {
         printf("  Duplicating for id == '%s'\n",id);
         fflush(stdout);
      }
    
      for (h=0; h<MACHASHSIZE; h++)
         for (ml=hset->mtab[h]; ml!=NULL; ml=ml->next) {
            if (ml->type=='*' || isupper((int) ml->type)) continue;
            if (ml->type=='l' || ml->type=='h') str=NULL;
            else if (strchr(what,ml->type)) str=NULL;
            else str=ml->structure;
            SetMacroHook(ml,str);
            if (ml->type=='m' || ml->type=='s' || !strchr(what,ml->type))
               continue;
            strcpy(buf,ml->id->name);
            strcat(buf,id);
            DupMacro(ml,GetLabId(buf,TRUE));
         }
      if (strchr(what,'m'))
         for (h=0; h<MACHASHSIZE; h++)
            for (ml=hset->mtab[h]; ml!=NULL; ml=ml->next) {
               if (ml->type!='m') continue;
               strcpy(buf,ml->id->name);
               strcat(buf,id);
               DupMacro(ml,GetLabId(buf,TRUE));
            }
      if (strchr(what,'p'))
         for (h=0; h<MACHASHSIZE; h++)
            for (ml=hset->mtab[h]; ml!=NULL; ml=ml->next) {
               if (ml->type!='p') continue;
               strcpy(buf,ml->id->name);
               strcat(buf,id);
               DupMacro(ml,GetLabId(buf,TRUE));
            }
      if (strchr(what,'s'))
         for (h=0; h<MACHASHSIZE; h++)
            for (ml=hset->mtab[h]; ml!=NULL; ml=ml->next) {
               if (ml->type!='s') continue;
               strcpy(buf,ml->id->name);
               strcat(buf,id);
               DupMacro(ml,GetLabId(buf,TRUE));
            }
      
      for (h=0; h<MACHASHSIZE; h++)
         for (ml=hset->mtab[h]; ml!=NULL; ml=ml->next)
            if (ml->type=='l' || ml->type=='h') {
               strcpy(buf,ml->id->name);
               if ((p=strchr(buf,'+'))!=NULL) {
                  *p++=0;
                  sprintf(name,"%s%s+%s",buf,id,p);
               }
               else
                  sprintf(name,"%s%s",ml->id->name,id);
               labid = GetLabId(name,TRUE);
               hmm = (HLink) ml->structure;

               if (hmm->hook==NULL) {
                  newHMM = DupHMM(hmm);
                  hmm->hook = newHMM;
               }
               else newHMM = (HLink) hmm->hook;
               
               NewMacro(hset,fidx,toupper(ml->type),labid,newHMM);
               if (trace & T_DET) {
                  printf("   Dup Model: %s\n",labid->name);
                  fflush(stdout);
               }
               if (ml->type=='h') nphmm++;
               else nlhmm++;
            }
   }
   for (h=0; h<MACHASHSIZE; h++)
      for (ml=hset->mtab[h]; ml!=NULL; ml=ml->next) {
         if (ml->type=='*') continue;
         ml->type=tolower(ml->type);
         SetMacroHook(ml,NULL);
      }
   if (trace & T_BID) {
      printf(" DP: HMMSet Duplicated for %d ids for %d / %d new models\n",
             N,nlhmm,nphmm);
      fflush(stdout);
   }
}

/* -------------------- MT - Make Triphone Command --------------- */

/* -------------------- Triphone Recording ------------------ */

typedef struct {
   HLink left,right;            /* physical names of constituent biphones */
   MLink ml;
}TriRec;

static int triSize;        /* num items in triTab */
static TriRec *triTab;     /* array[0..triSize-1] of TriRec; */

/* SameTriphone: return the existing HMMEntry for left+right, if any */
MLink SameTriphone(HLink left, HLink right)
{
   int i;
   TriRec *tr;
   
   for (i=0,tr=triTab;i<triSize;i++,tr++)
      if (tr->left == left && tr->right==right)
         return tr->ml;
   return NULL;
}

/* RecordTriphone: in next free slot of triTab */
void RecordTriphone(HLink left, HLink right, MLink ml)
{
   triTab[triSize].left = left;
   triTab[triSize].right = right;
   triTab[triSize].ml = ml;
   ++triSize;
}

/* MakeTriCommand: make each phone in given list from currently
   loaded models.  If phone is not a triphone then it must 
   be already loaded in which case it is just cloned.
   Otherwise, the triphone is synthesised from left and right
   biphones which must already be loaded.  The synthesis for
   A-B+C is to clone B+C then replace state 2 by that of
   A-B (this is intended for 3 state models).
   */
void MakeTriCommand(void)
{
   char fn[MAXFNAMELEN], *s;
   HMMSet *tmpSet;
   HLink hmm,left,right;
   MLink q,match;
   int h,mcount=0,tcount=0;
   
   ChkedAlpha("MT hmmlist file name",fn);
   if (trace & T_BID) {
      printf("\nMT %s\n Converting current biphones to triphones\n",fn);
      fflush(stdout);
   }
   tmpSet=(HMMSet*) New(&hmmHeap,sizeof(HMMSet));
   CreateHMMSet(tmpSet,&hmmHeap,TRUE);
   if(MakeHMMSet(tmpSet,fn)<SUCCESS)
      HError(2628,"MakeTriCommand: MakeHMMSet failed");
   
   triTab=(TriRec*) New(&tmpHeap,tmpSet->numLogHMM*sizeof(TriRec));
   triSize = 0;
   for (h=0; h<MACHASHSIZE; h++)
      for (q=tmpSet->mtab[h]; q!=NULL; q=q->next) {
         if (q->type != 'l') continue;
         s = q->id->name;
         if (strchr(s,'-') != NULL && strrchr(s,'+') != NULL ) { /*triphone*/
            right = FindBaseModel(hset,q->id,baseRight);
            left = FindBaseModel(hset,q->id,baseLeft);
            hmm=(HLink) q->structure;
            if ((match = SameTriphone(left,right)) == NULL) {
               RecordTriphone(left,right,q);
               CloneHMM(right,hmm,TRUE);
               tcount++;
               hmm->svec[2].info=CloneState(hset,left->svec[2].info,TRUE);
            } else {            /* this triphone is already made */
               hmm = (HLink) (q->structure = match->structure);
               hmm->nUse++;
            }
         } 
         else {                 /* not a triphone so just clone it */
            hmm = FindBaseModel(hset,q->id,baseNorm);
            CloneHMM(hmm, (HLink) q->structure, TRUE);
            mcount++;
         }
      }
   SwapLists(hset,tmpSet);
   if (trace & T_BID) {
      printf(" MT: %d distinct triphones, %d non-triphones\n",
             tcount,mcount);
      fflush(stdout);
   }
}

/* ----------------- AT/RT - Add/Remove Transition Command ---------------- */

/* EditTransMat: add/rem a transition in list of transP */
void EditTransMat(Boolean adding)
{
   ILink t,ilist = NULL;        /* list of items to tie */
   char type = 't';             /* type of items must be t */
   int i,j,k,use;
   float prob=0.0,sum,x;
   SMatrix tran;
   int N,nedit=0;
   Vector row;
   HMMDef *hmm;
   
   i = ChkedInt("From state",1,maxStates-1);
   j = ChkedInt("To state",2,maxStates);
   if (adding) prob = ChkedFloat("AT transition prob",0.0,0.999999);
   else prob = 0.0;
   if (trace & T_BID) {
      if (adding)
         printf("\nAT %d %d %.3f {}\n Adding transitions to transP\n",
                i,j,prob);
      else
         printf("\nRT %d %d {}\n Removing transitions from transP\n",i,j);
      fflush(stdout);
   }
   PItemList(&ilist,&type,hset,&source,NULL,0,0,(trace&T_ITM)?TRUE:FALSE);
   if (ilist == NULL) {
      HError(-2631,"EditTransMat: No trans mats to edit!");
      return;
   }
   for (t=ilist; t!=NULL; t=t->next) {
      hmm = t->owner;
      tran = hmm->transP;
      use = GetUse(tran);
      if (IsSeen(use)) continue;
      Touch(&use);SetUse(tran,use);
      N = hmm->numStates;
      if (i<1 || i>N-1 || j<2 || j>N)
         HError(2632,"EditTransMat: States (%d->%d) out of range for %s",
                i,j,HMMPhysName(hset,hmm));
      row = tran[i];
      row[j] = prob;
      sum = 0.0;
      for (k=1;k<=N;k++)
         if (k!=j) {
            x = row[k];
            row[k]=(x<LSMALL)?0.0:exp(x);
            sum += row[k];
         }
      sum /= 1.0-row[j];
      for (k=1;k<=N;k++) {
         x = row[k];
         if (k!=j) x /= sum;
         row[k] = (x<MINLARG)?LZERO:log(x);
      }
      nedit++;
   }
   ClearSeenFlags(hset,CLR_HMMS);
   FreeItems(&ilist);
   if (trace & T_BID) {
      printf(" %cT: %d transP matrices adjusted\n",((adding)?'A':'R'),nedit);
      fflush(stdout);
   }
}

/* -------------------- MU - Mix Up Command ---------------------- */

/* MixUpCommand: increase num mixtures in itemlist, note
   that this also restores defunct mixtures.  When a mix is
   split, 1 is added to the hook of the mpdf struct and this 
   is used to weight against repeated splitting of the same
   mixture */
void MixUpCommand(void)
{
   ILink i,ilist = NULL;        /* list of items to mixup */
   char type = 'p';             /* type of items must be p */
   int ch,j,m,trg,M,mDefunct,n2fix,totm=0,totM=0;
   StreamElem *ste;
   StreamInfo *sti;
   HMMDef *hmm;
   char *hname;
   
   SkipWhiteSpace(&source);
   ch = GetCh(&source);
   if (ch=='+') {
      trg = -ChkedInt("Mix increment",1,INT_MAX);
   }
   else if (ch=='*') {
      trg = ChkedInt("Mix multiply",1,INT_MAX);
   }
   else {
      UnGetCh(ch,&source);
      trg = ChkedInt("Mix target",1,INT_MAX);
   }

   if (trace & T_BID) {
      if (trg<0)
         printf("\nMU +%d {}\n Mixup by %d components per stream\n",-trg,-trg);
      else if (ch=='*')
         printf("\nMU *%d {}\n Mixup by %d components per stream\n",trg,trg);
      else
         printf("\nMU %d {}\n Mixup to %d components per stream\n",trg,trg);
      fflush(stdout);
   }

   if (hset->hsKind==TIEDHS || hset->hsKind==DISCRETEHS)
      HError(2640,"MixUpCommand: MixUp only possible for continuous models");
   PItemList(&ilist,&type,hset,&source,NULL,0,0,(trace&T_ITM)?TRUE:FALSE);
   if (ilist == NULL) {
      HError(-2631,"MixUpCommand: No mixtures to increase!");
      return;
   }
   SetGCStats();
   for (i=ilist; i!=NULL; i=i->next) {
      ste = (StreamElem *)i->item;
      sti = ste->info;
      M = sti->nMix;
      if (M>0) sti->nMix = -M;
   }
   for (i=ilist; i!=NULL; i=i->next) {
      hmm = i->owner; 
      ste = (StreamElem *)i->item;  sti = ste->info;  M = sti->nMix;
      if (M<0) {
         M = sti->nMix = -M;
         hname = HMMPhysName(hset,hmm);
         for (j=1; j<=M; j++)   /* reset the split counters */
            sti->spdf.cpdf[j].mpdf->hook = (void *) 0;
         mDefunct = CountDefunctMix(sti);
         if (trg<0) m=M-trg;
         else if (ch=='*') m=M*trg;
         else m=trg;
         if (m > M-mDefunct) {
            n2fix = m-M+mDefunct;
            if (n2fix>mDefunct) n2fix = mDefunct;
            if (n2fix>0) {
               FixDefunctMix(hname,sti,n2fix);
            }
         }
         else n2fix=0;
         if (trace & T_IND) {
            if (n2fix>0)
               printf("  %12s : mixup %d -> %d (%d defunct restored)\n",
                      hname,M,m,n2fix);
            else
               printf("  %12s : mixup %d -> %d\n",hname,M,m);
         }
         if (m>maxMixes) maxMixes = m;
         if (m>M) UpMix(hname,sti,M,m);
         totm+=sti->nMix; totM+=M;
         for (j=1; j<=sti->nMix; j++) /* restore the hooks */
            sti->spdf.cpdf[j].mpdf->hook = NULL;
      }
   }
   FreeItems(&ilist);
   if (trace & T_BID) {
      printf(" MU: Number of mixes increased from %d to %d\n",
             totM,totm);
      fflush(stdout);
   }
}

/* -------------------- TI - Tie Command ---------------------- */

/* TieCommand: tie all components in following itemlist */
void TieCommand(void)
{
   ILink ilist = NULL;          /* list of items to tie */
   char type = ' ';             /* type of items to tie */
   char macName[MAXSTRLEN];     /* name of macro to use */
   
   ChkedAlpha("TI macro name",macName);
   if (strlen(macName) > 20 )
      HError(-2639,"TieCommand: %s is rather long for a macro name!",macName);
   PItemList(&ilist,&type,hset,&source,NULL,0,0,(trace&T_ITM)?TRUE:FALSE);
   if (trace & (T_BID | T_MAC)) {
      printf("\nTI %s {}\n Tie items\n",macName);
      fflush(stdout);
   }
   ApplyTie(ilist,macName,type);
   FreeItems(&ilist);
}

/* ----------------- DM - Delete Macro Command ---------------- */

void DeleteMacroCommand(void)
{
   char type, macName[MAXSTRLEN];
   LabId macId;
   
   type = GetCh(&source);
   ChkedAlpha("DM macro name", macName);
   macId = GetLabId(macName,FALSE);
   
   if (trace & (T_BID | T_MAC)) {
      printf("\nDM {}\n Delete given macro from model-set\n");
      fflush(stdout);
   }

   if (macId==NULL)
      HError(9999,"DeleteMacroCommand: cannot find macro ~%c %s", type, macName);

   DeleteMacro(hset, FindMacroName(hset,type,macId));
   
   if (trace & T_BID) {
      printf(" DM: Macro ~%c %s was deleted\n", type, macName);
      fflush(stdout);
   }
   
   return;
}

/* ------------------ MM - Tie All Command -------------------- */

/* MakeIntoMacrosCommand: tie all components in following itemlist */
void MakeIntoMacrosCommand(void)
{
   ILink ilist = NULL;          /* list of items to tie */
   char type = ' ';             /* type of items to tie */
   char macName[MAXSTRLEN],buf[MAXSTRLEN]; /* name of macro to use */
   ItemRec itemp,*i;
   MixPDF *mp;
   StateInfo *si;
   StateElem *se;
   HMMDef *hmm;
   int use,n=0;
   
   ChkedAlpha("MM macro name",macName);
   if (trace & (T_BID | T_MAC)) {
      printf("\nMM %s {}\n Make each item into macro\n",macName);
      fflush(stdout);
   }
   if (strlen(macName) > 20 )
      HError(-2639,"MakeIntoMacrosCommand: %s is rather long for a macro name!",macName);
   PItemList(&ilist,&type,hset,&source,NULL,0,0,(trace&T_ITM)?TRUE:FALSE);
   if (type=='p' || type=='h')
      HError(2640,"MakeIntoMacros: Cannot convert PDFs or HMMs into macro");
   for (i=ilist;i;i=i->next) {
      se=(StateElem*)i->item; si=(StateInfo *)i->item; 
      mp=(MixPDF *)i->item; hmm=(HMMDef*)i->owner;
      switch(type) {
      case 's': use=se->info->nUse; break;
      case 'x': use=GetUse(mp->cov.xform); break;
      case 'v': use=GetUse(mp->cov.var); break;
      case 'i': use=GetUse(mp->cov.inv); break;
      case 'c': use=GetUse(mp->cov.inv); break;
      case 'u': use=GetUse(mp->mean); break;
      case 't': use=GetUse(hmm->transP); break;
      case 'm': use=mp->nUse; break;
      case 'w': use=GetUse(si->weights); break;
      case 'd': use=GetUse(si->dur); break;
      default : use=-1; break;
      }
      if (use==0) {
         itemp.owner=i->owner;
         itemp.item=i->item;
         itemp.next=NULL;
         sprintf(buf,"%s%d",macName,++n);
         ApplyTie(&itemp,buf,type);
      }
   }
   if (trace & T_BID) {
      printf(" MM: Made %d ~%c macros for %d items\n",
             n,type,NumItems(ilist));
      fflush(stdout);
   }
   FreeItems(&ilist);
}

/* -------------------- UT - Untie Command ---------------------- */

/* UntieCommand: untie all components in following itemlist */
void UntieCommand(void)
{
   ILink ilist = NULL;          /* list of items to untie */
   char type = ' ';             /* type of items to untie */
   
   if (trace & (T_BID | T_MAC)) {
      printf("\nUT {}\n Untie previously tied structures\n");
      fflush(stdout);
   }
   PItemList(&ilist,&type,hset,&source,NULL,0,0,(trace&T_ITM)?TRUE:FALSE);
   if (ilist==NULL) {
      HError(-2631,"UntieCommand: No items to untie");
      return;
   }
   switch (type) {
   case 's': UntieState(ilist); break;
   case 'p': UntiePDF(ilist);   break;
   case 'm': UntieMix(ilist);   break;
   case 't': UntieTrans(ilist); break;
   case 'u': UntieMean(ilist);  break;
   case 'v': UntieVar(ilist);   break;
   case 'i': UntieInv(ilist);   break;
   case 'x': UntieXform(ilist); break;
   case 'h':
      HError(2640,"UntieCommand: Joined HMMs cannot be untied");
   default:
      HError(2640,"UntieCommand: Cannot untie %c's",type);
   }
   if (trace & (T_BID | T_MAC)) {
      printf(" UT: Untied %d ~%c structures\n",NumItems(ilist),type);
      fflush(stdout);
   }
   FreeItems(&ilist);
}

/* -------------------- JO - Set Join Size Command ---------------- */

/* JoinSizeCommand: set number of tied mixtures */
void JoinSizeCommand(void)
{
   joinSize = ChkedInt("JoinSize",1,INT_MAX);
   joinFloor = ChkedFloat("JoinFloor",0.0,FLOAT_MAX);
   if (trace & T_BID) {
      printf("\nJO %d %.2f\n Set size and floor for pdf tying\n",
             joinSize,joinFloor);
      fflush(stdout);
   }
}

/* ----------- TC/NC - Threshold/Number Cluster Command ---------------------- */

/* ClusterCommand: cluster all components in following itemlist 
   and then tie each cluster.  The i'th cluster
   is called macroi.  If macro = 'check' then
   just the cluster info is output and no tying
   is done.  If nCluster is set then clustering
   converges when the required number is reached
   otherwise a threshold for ming is used */
void ClusterCommand(Boolean nCluster)
{
   ILink ilist = NULL;          /* list of items to tie */
   char type = ' ';             /* type of items to tie */
   char macName[MAXSTRLEN];     /* name of macro to use */
   int numItems,numClust=1;     /* num clusters required */
   float thresh = 1.0E15;
   static int totalItems = 0;   /* for overall statistics */
   static int totalClust = 0;
   
   if (nCluster)
      numClust = ChkedInt("No. clusters",1,INT_MAX);
   else
      thresh = ChkedFloat("Cluster threshold",0.0,FLOAT_MAX);
   ChkedAlpha((char *) ((nCluster) ? "NC macro name" : "TC macro name"), macName);
   if (trace & (T_BID | T_CLUSTERS)) {
      if (nCluster) 
         printf("\nNC %d %s {}\n Cluster items into N groups\n",
                numClust,macName);
      else
         printf("\nTC %.2f %s {}\n Cluster items until threshold exceeded\n",
                thresh,macName);
      fflush(stdout);
   }
   if (strlen(macName) > 20 )
      HError(-2639,"ClusterCommand: %s is rather long for a macro name",
             macName);
   PItemList(&ilist,&type,hset,&source,NULL,0,0,(trace&T_ITM)?TRUE:FALSE);
   if (ilist==NULL) {
      HError(-2631,"ClusterCommand: No items to cluster for %s\n",macName);
      return;
   }
   numItems = NumItems(ilist);
   Clustering(ilist, &numClust, thresh, type, macName);
   totalItems += numItems; totalClust += numClust;
   if (trace & T_BID) {
      printf("\n %cC: Stats %d->%d [%.1f%%] { %d->%d [%.1f%%] total }\n",
             (nCluster?'N':'T'),numItems,numClust,numClust*100.0/numItems,
             totalItems,totalClust,totalClust*100.0/totalItems);
      fflush(stdout);
   }
}

/* ------------- LS/RO - Load Stats/Remove Outliers Set-Up Command ----------- */

/* LoadStatsCommand: loads occupation counts for hmms */
void LoadStatsCommand(void)
{
   char statfile[MAXFNAMELEN];
   
   ChkedAlpha("LS stats file name",statfile);
   if (trace & T_BID) {
      printf("\nLS %s\n",statfile);
      printf(" Loading state occupation stats\n");
      fflush(stdout);
   }
   LoadStatsFile(statfile, hset, (trace&T_BID)?TRUE:FALSE);
   occStatsLoaded = TRUE;
}


/* RemOutliersCommand: used in conjunction with NC/TC.  After clustering
   any group with a total occupation count less than the specified 
   threshold is merged with the closest group
   */
void RemOutliersCommand(void)
{
   char statfile[MAXFNAMELEN];
   int ch;
   
   outlierThresh = ChkedFloat("Outlier Occupancy Threshold",0.0,FLOAT_MAX);
   if (trace & T_BID) {
      printf("\nRO %.2f ''\n",outlierThresh);
      printf(" Setting outlier threshold for clustering\n");
      fflush(stdout);
   }
   do {
      ch = GetCh(&source);
      if (ch=='\n') break;
   }
   while(ch!=EOF && isspace(ch));
   UnGetCh(ch,&source);
   if (ch!='\n') { /* Load Stats File */
      ChkedAlpha("RO stats file name",statfile);
      if (trace & T_BID) {
         printf(" RO->LS %s\n",statfile);
         printf("  and loading state occupation stats\n");
         fflush(stdout);
      }
      if (occStatsLoaded)
         HError(-2655,"RemOutliersCommand: Stats already loaded");
      else
         LoadStatsFile(statfile,hset,(trace&T_BID)?TRUE:FALSE);
      occStatsLoaded = TRUE;
   }
}

/* ---------------- CO - Compact HMM Set Command ---------------- */

/* CompactCommand: Find all sets of identical HMM's in current
   hmm set and merge each into a single physical hmm */
void CompactCommand(void)
{
   ILink seen=NULL,i;
   MLink q;
   HLink hmm;
   int h;
   Boolean seenMean,seenVar;
   char fn[MAXFNAMELEN];
   
   ChkedAlpha("CO hmmlist file name",fn);  /* get name of new hmm list */
   if (trace & T_BID) {
      printf("\nCO %s\n Create compact HMMList\n",fn);
      fflush(stdout);
   }
   
   ResetHooks(hset,"h");
   seenMean=seenVar=FALSE;
   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next)  {
         if (q->type=='p') seenMean=seenVar=TRUE;
         if (q->type=='m') seenMean=seenVar=TRUE;
         if (q->type=='u') seenMean=TRUE;
         if (q->type=='v' || q->type=='i' || q->type=='c' || q->type=='x')
            seenVar=TRUE;
         if (seenMean && seenVar) break;
      }
   if (seenMean && seenVar) equivState=TRUE;
   else equivState=FALSE;

   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next) 
         if (q->type=='h') {
            hmm=(HLink) q->structure;
            for (i=seen;i!=NULL;i=i->next)
               if (hmm->transP==i->owner->transP && EquivHMM(hmm,i->owner)) break;
            if (i!=NULL) {
               hmm->hook=i->owner;
               i->owner->nUse++;
               DeleteMacro(hset,q);
               if (trace & T_DET)
                  printf("  %12s == %s\n",q->id->name,
                         ((MLink)i->item)->id->name);
            }
            else {
               if (trace & T_DET)
                  printf("  %12s ++\n",q->id->name);
               AddItem(hmm,q,&seen);
            }
         }

   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next) 
         if (q->type=='l') {
            hmm=(HLink) q->structure;
            if (hmm->hook!=NULL && hmm->hook!=hmm) {
               NewMacro(hset,fidx,'L',q->id,hmm->hook);
               DeleteMacro(hset,q);
            }
         }
   
   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next) 
         if (q->type=='L') {
            hset->numLogHMM++;hset->numMacros--;
            q->type='l';
         }
   ResetHooks(hset,"h");
   
   if(SaveHMMList(hset,fn)<SUCCESS)
      HError(2611,"CompactCommand: SaveHMMList failed");
   if (trace & T_BID) {
      printf(" CO: HMMs %d Logical %d Physical\n",
             hset->numLogHMM,hset->numPhyHMM);
      fflush(stdout);
   }
}


/* ------------------ SS - SplitStream Command ------------------ */

/* SplitStreamCommand: split input vectors into n independent data streams */
void SplitStreamCommand(Boolean userWidths)
{
   int s,next,S,V,ewidth,vchk,i,nedit=0;
   int epos[4];
   char buf[20],c=' ';
   HLink hmm=NULL;
   HMMScanState hss;
   short swidth[SMAX];
   ParmKind pk;
   Vector v;
   Boolean simple=TRUE,vfSet,hasE,hasN,hasD,hasA,has0;

   S = ChkedInt("No. streams",2,SMAX-1);
   V = hset->vecSize;  pk = hset->pkind;
   swidth[0] = S;
   if (userWidths)
      for (s=1;s<=S;s++)
         swidth[s] = ChkedInt("Stream width",1,V);
   if (trace & (T_BID | T_MAC | T_SIZ)) {
      if (userWidths) {
         printf("\nSU %d",S);
         for (s=1;s<=S;s++) printf(" %d",swidth[s]);
         printf("\n splitting into user defined stream widths\n");
      }
      else
         printf("SS %d\n splitting into standard stream widths\n",S);
      fflush(stdout);
   }
   if (hset->hsKind==TIEDHS || hset->hsKind==DISCRETEHS)
      HError(2640,"SplitStreamCommand: Only implemented for continuous");
   if (hset->swidth[0] != 1)
      HError(2640,"SplitStreamCommand: Cannot split multiple streams");
   
   /* Calculate Stream widths */
   hasN=HasNulle(pk);
   if (!userWidths) {
      hasD=HasDelta(pk);hasA=HasAccs(pk);
      hasE= HasEnergy(pk); has0 = HasZeroc(pk);
      if (hasE && has0)
         HError(2641,"SplitStreamCommand: Source data cant have _0 and _E");
      hasE = (hasE||has0) ? TRUE:FALSE;     /* dont care which it is any more */
      if (!hasD && !hasE)
         HError(2641,"SplitStreamCommand: Source data must have _D or _E");
      if (S==2) {
         if (hasA)
            HError(2641,"SplitStreamCommand: need 3 streams min when _A");  
         if (hasD) {
            swidth[1] = V / 2;
            swidth[2] = V - swidth[1];
         } else {
            swidth[1] = V -1; swidth[2] = 1;
         }
      }
      else if (S==3) {
         if (!hasD)
            HError(2641,"SplitStreamCommand: Source data must have _D");
         if (!hasE && !hasA)
            HError(2641,"SplitStreamCommand: Source data must have _E or _A");
         if (hasA) {
            swidth[1] = V / 3;
            swidth[2] = swidth[3] = (V - swidth[1]) / 2;
         } else {
            if (hasN) ewidth=1; else ewidth=2;
            swidth[1] = swidth[2] = (V-ewidth) / 2;
            swidth[3] = ewidth;
         }
      }
      else if (S==4) {
         if (!(hasE && hasA))
            HError(2641,"SplitStreamCommand: Source data must have _E and _A");
         if (hasN) ewidth=2; else ewidth=3;
         swidth[1] = swidth[2] = swidth[3] = (V-ewidth) / 3;
         swidth[4] = ewidth;
      }
      else
         HError(2641,"SplitStreamCommand: cant manage %d streams",S);
      if (trace & (T_BID | T_SIZ)) {
         printf(" Calculated %d stream widths: ",S);
         for (s=1;s<=S;s++) printf(" %d",swidth[s]);
         printf("\n");
         fflush(stdout);
      }
      simple = (!hasE || (swidth[S]<2) || (S==2) || (hasA && S==3)) ? TRUE:FALSE;
   }
   
   /* Get varFloor */
   SetVFloor(hset,vf,-1.0);
   for (i=1,vfSet=TRUE;i<=V;i++)
      if (vf[1][i]<0.0) {
         vfSet=FALSE;
         break;
      }

   /* Check Widths consistent with VecSize */
   vchk = 0;
   hset->swidth[0]=swidth[0];
   for (i=1; i<=S; i++) {
      vchk += swidth[i];
      hset->swidth[i]=swidth[i];
   }
   if (vchk != V) {
      if (userWidths)
         HError(2630,"SplitStreamCommand: Sum of stream widths = %d, vecSize=%d",vchk,V);     
      else
         HError(2696,"SplitStreamCommand: Bug in stream width allocation");
   
   }
   /* Do Splitting */
   NewHMMScan(hset,&hss);
   while(GoNextState(&hss,FALSE)) {
      SplitStreams(hset,hss.si,simple,(nedit==0)?TRUE:FALSE);
      if (trace & (T_SIZ | T_DET)) {
         if (nedit==0) printf(" For model.state["),hmm=NULL;
         if (hmm!=hss.hmm)
            printf("]\n  %12s.state",hss.mac->id->name),c='[',hmm=hss.hmm;
         printf("%c%d",c,hss.i);c=',';
         fflush(stdout);
      }
      nedit++;
   }
   if (trace & (T_SIZ | T_DET))
      printf("]\n"),fflush(stdout);
   EndHMMScan(&hss);
   /* Now do varFloor */
   if (vfSet) {
      if (trace & (T_SIZ | T_DET | T_MAC))
         printf(" For varFloor\n"),fflush(stdout);
      for (i=0; i<3; i++) epos[i] = 0;
      v=vf[1];
      DeleteMacroStruct(hset,'v',v);
      for (s=1,next=1;s<=S;s++) {
         sprintf(buf,"varFloor%d",s);
         if (simple || s<S) 
            vf[s] = SliceVector(v,next,next+swidth[s]-1);
         else
            vf[s] = ChopVector(v,epos[0],epos[1],epos[2]);
         NewMacro(hset,fidx,'v',GetLabId(buf,TRUE),vf[s]);
         if (trace & (T_DET | T_MAC)) {
            printf("   varFloor Macro: ~m:%s created [%d]\n",buf,swidth[s]);
         }
         next += swidth[s];
         if (!simple && !(hasN && s==1))
            epos[s-(hasN?2:1)] = next++;
      }
      SetVFloor(hset,vf,minVar);
   }
         
      
   badGC = TRUE;
   if (trace & (T_BID | T_SIZ)) {
      printf(" S%c: %d state's stream widths adjusted\n",
             (userWidths?'U':'S'),nedit);
      fflush(stdout);
   }
}

/* ---------------- SW - Set Stream Width Command ---------------- */

/* SetStreamWidthCommand: change width of stream s to n */
void SetStreamWidthCommand(void)
{
   int s, n, orign, nedit=0;
   char c=' ';
   HMMScanState hss;
   HLink hmm=NULL;
   MixPDF *mp;

   s = ChkedInt("Stream",1,SMAX-1);
   n = ChkedInt("Stream width",1,INT_MAX);
   if (trace & (T_BID | T_SIZ | T_MAC)) {
      printf("\nSW %d %d\n Changing stream width\n",s,n);
      fflush(stdout);
   }
   if (hset->swidth[s]==n) return;
   NewHMMScan(hset,&hss);
   if (!(hss.isCont || (hss.hset->hsKind == TIEDHS))) {
         HError(2640,"SetStreamWidthCommand: Can only resize continuous and tied systems");      
   }
   while(GoNextMix(&hss,FALSE)) {
      if (hss.s!=s) continue;
      mp = hss.me->mpdf;
      mp->mean=ResizeSVector(hset,mp->mean,n,'u',0.0);
      switch(mp->ckind) {
      case DIAGC:
         mp->cov.var=ResizeSVector(hset,mp->cov.var,n,'v',1.0);
         break;
      case FULLC:
         mp->cov.inv=ResizeSTriMat(hset,mp->cov.inv,n,'i',1.0);
         break;
      default: 
         HError(2640,"SetStreamWidthCommand: Can only resize DIAGC or FULLC");
      }
      if (trace & (T_SIZ | T_DET)) {
         if (nedit==0) printf(" For model.state["),hmm=NULL;
         if (hmm!=hss.hmm)
            printf("]\n  %12s.state",hss.mac->id->name),c='[',hmm=hss.hmm;
         printf("%c%d",c,hss.i);c=',';
         fflush(stdout);
      }
      nedit++;
   }
   if (trace & (T_SIZ | T_DET))
      printf("]\n"),fflush(stdout);
   EndHMMScan(&hss);
   /* Now varFloor */
   SetVFloor(hset,vf,minVar);
   if (FindMacroStruct(hset,'v',vf[s])!=NULL) {
      if (trace & T_BID)
         printf(" Resizing varFloor\n");
      ResizeSVector(hset,vf[s],n,'v',0.0);
   }
   badGC = TRUE;
   orign = hset->swidth[s];
   hset->swidth[s]=n;
   hset->vecSize += n - orign;
   
   if (trace & (T_BID | T_SIZ)) {
      printf(" SW: Stream width changed for %d mixes\n",nedit);
      fflush(stdout);
   }
}

/* ---------------- SK - Set Sample Kind Command ---------------- */

/* SetSampKindCommand: change skind of all loaded models */
void SetSampKindCommand(void)
{
   char s[MAXSTRLEN];
   ParmKind pk;
   
   ChkedAlpha("SK sample kind",s);
   if (trace & T_BID) {
      printf("\nSK %s\n Set parameter kind of currently loaded HMM set\n",s);
      fflush(stdout);
   }
   pk = Str2ParmKind(s);
   hset->pkind=pk;
}

/* ---------------- HK - Set HMMSet Kind Command ---------------- */

static int nconv;

static int cmpMix(const void *v1,const void *v2)
{
   MixPDF *m1,*m2;
   MLink ml1,ml2;
   int i1,i2;
   
   m1=*(MixPDF**)v1;m2=*(MixPDF**)v2;
   ml1=FindMacroStruct(hset,'m',m1);
   ml2=FindMacroStruct(hset,'m',m2);
   if (ml1==NULL || ml2==NULL) return(m1-m2);
   i1=strlen(ml1->id->name);
   i2=strlen(ml2->id->name);
   if (i1==i2)
      return(strcmp(ml1->id->name,ml2->id->name));
   else
      return(i1-i2);
}
   
void CreateTMRecs(void)
{
   HMMScanState hss;
   TMixRec *p;
   MixPDF **mpp;
   TMProb *tm;
   LabId labid;
   char buf[80];
   int s,M;
   
   for (s=1;s<=hset->swidth[0];s++)
      hset->tmRecs[s].nMix=0;
   NewHMMScan(hset,&hss);
   while(GoNextMix(&hss,FALSE))
      hset->tmRecs[hss.s].nMix++;
   EndHMMScan(&hss);
   
   for (s=1;s<=hset->swidth[0];s++) {
      M = hset->tmRecs[s].nMix;
      p = hset->tmRecs+s;
      sprintf(buf,"%s_%d_",tiedMixName,s);
      labid=GetLabId(buf,TRUE);
      p->mixId = labid;
      mpp = (MixPDF **)New(hset->hmem,sizeof(MixPDF *) * M);
      p->mixes = mpp-1;
      tm = (TMProb *)New(hset->hmem,sizeof(TMProb)*M);
      p->probs = tm-1;
      p->topM = 1;
   }
   NewHMMScan(hset,&hss);
   while(GoNextMix(&hss,FALSE)) {
      p = hset->tmRecs+hss.s;
      p->mixes[p->topM]=hss.me->mpdf;
      p->topM++;
   }
   for (s=1;s<=hset->swidth[0];s++) {
      p = hset->tmRecs+s;
      qsort(p->mixes+1,p->topM-1,sizeof(MixPDF*),cmpMix);
   }
   EndHMMScan(&hss);
}

/* CalcTMWeights: Fix the weights of me using the old stream info */
Vector CalcTMWeights(int s, StreamInfo *sti, double tFloor)
{
   DVector v;
   Vector tpdf;
   MixtureElem *sme;
   double w,p,wSum,fSum,maxW,floor,lFloor;
   int m,sm,M;
   
   M = hset->tmRecs[s].nMix;

   v = CreateDVector(&tmpHeap,M);
   tpdf = CreateVector(hset->hmem,M);

   maxW = LZERO;
   if (M==sti->nMix) {
      for (m=1;m<=M;m++) {  /* copy weights */
         for (sm=1,sme=sti->spdf.cpdf+1;sm<=sti->nMix;sm++,sme++)
            if (sme->mpdf==hset->tmRecs[s].mixes[m]) break;
         if (sm>sti->nMix)
            HError(2693,"CalcTMWeights: Cannot find matching mixture");
         if (sme->weight>MINMIX)
            w = v[m] = log(sme->weight);
         else
            w = v[m] = LZERO;
         if (w > maxW) maxW = w; 
      }
   }
   else {
      for (m=1;m<=M;m++) {  /* set weights to SOutP(owner) */
         w=LZERO;
         for (sm=1,sme=sti->spdf.cpdf+1;sm<=sti->nMix;sm++,sme++) {
            if (sme->weight>MINMIX || hset->msdflag[s]) {
               p = MOutP(hset->tmRecs[s].mixes[m]->mean,sme->mpdf);
               w = LAdd(w,log(sme->weight)+p);
            }
         }
         if (w > maxW) maxW = w; 
         v[m] = w;
      }
   }
   wSum = 0.0; fSum = 0.0; 
   floor = tFloor*MINMIX; 
   if (floor>0.0)
      lFloor = log(floor*M);
   else
      lFloor = LZERO;
   for (m=1,sm=0;m<=M;m++) {  /* make all weights > floor */
      w = v[m]-maxW;
      if (w < lFloor) {
         v[m] = floor; fSum += floor;sm++;
      } else {
         w = exp(w); v[m] = w;
         wSum += w;
      }
   }
   if (fSum>=0.9)
      HError(2634,"CalcTMWeights: Join Floor too high");
   if (wSum==0.0)
      HError(2691,"CalcTMWeights: No positive weights");
   wSum /= (1-fSum);            /* finally normalise */
   fSum=0.0;
   for (m=1;m<=M;m++) {
      w = v[m];
      if (w>floor) w /= wSum;
      tpdf[m]=w;
      fSum+=w;
   }
   if (trace & T_DET) {
      printf(" Sum == %.3f [%d floored]\n",fSum,sm);
      fflush(stdout);
   }
   nconv++;
   FreeDVector(&tmpHeap,v);
   return(tpdf);
}


void ConvertCont2Tied(void)
{
   Vector tpdf;
   MLink q;
   StreamInfo *sti;
   HLink hmm;
   LabId labid;
   char buf[80];
   int h,i,j,s,M;

   /* Create the TMRecs */
   CreateTMRecs();

   /* Rebuild every stream element */
   /* Can't use Scan because will break */
   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next) 
         if (q->type=='h') {
            hmm=(HLink) q->structure;
            for (j=2;j<hmm->numStates;j++) {
               for (s=1;s<=hset->swidth[0];s++) {
                  sti=hmm->svec[j].info->pdf[s].info;
                  if (IsSeen(sti->nUse)) continue;
                  if (trace & T_DET)
                     printf("   For  %-12s[%d].stream[%d]:  ",
                            HMMPhysName(hset,hmm),j,s);
                  M = hset->tmRecs[s].nMix;
                  tpdf=CalcTMWeights(s,sti,joinFloor);
                  sti->spdf.tpdf=tpdf;
                  sti->nMix=M;
                  Touch(&sti->nUse);
               }
            }
         }
   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next) 
         if (q->type=='h') {
            hmm=(HLink) q->structure;
            for (j=2;j<hmm->numStates;j++)
               for (s=1;s<=hset->swidth[0];s++) {
                  sti=hmm->svec[j].info->pdf[s].info;
                  Untouch(&sti->nUse);
               }
         }
                                  
   /* Purge any mix macros */
   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next) 
         if (q->type=='m')
            DeleteMacro(hset,q);

   /* Give the tied mixture records names */
   for (s=1;s<=hset->swidth[0];s++) {
      M = hset->tmRecs[s].nMix;
      for (i=1;i<=M;i++) {
         sprintf(buf,"%s%d",hset->tmRecs[s].mixId->name,i);
         labid=GetLabId(buf,TRUE);
         q=NewMacro(hset,fidx,'m',labid,hset->tmRecs[s].mixes[i]);
         SetMacroUse(q,1);
      }
   }
}

/* SetHSetKindCommand: change the hset kind of the current HMM set  */
void SetHSetKindCommand(void)
{
   char s[MAXSTRLEN];
   HSetKind hk;
   
   ChkedAlpha("HK HMMSet kind",s);
   if (trace & T_BID) {
      printf("\nHK %s\n Set HMMSet kind of currently loaded HMM set\n",s);
      fflush(stdout);
   }
   if (strcmp(s,"PLAINHS")==0) hk=PLAINHS;
   else if (strcmp(s,"SHAREDHS")==0) hk=SHAREDHS;
   else if (strcmp(s,"TIEDHS")==0) hk=TIEDHS;
   else if (strcmp(s,"DISCRETEHS")==0) hk=DISCRETEHS;
   else {
      hk=PLAINHS;
      HError(2632,"SetHSetKindCommand: Unknown HMMSet kind %s",s);
   }

   if (hset->hsKind==hk)
      return;
   nconv=0;
   switch(hset->hsKind) {
   case PLAINHS:
      switch(hk) {
      case SHAREDHS:
         break;
      case TIEDHS:
         ConvertCont2Tied();
         break;
      case DISCRETEHS:
         HError(2640,"SetHSetKindCommand: conversion to %s not implemented",s);
         break;
      }
      break;
   case SHAREDHS:
      switch(hk) {
      case PLAINHS:
         /* Should untie all tied parameters */
         HError(2640,"SetHSetKindCommand: conversion to %s not implemented",s);
         break;
      case TIEDHS:
         ConvertCont2Tied();
         break;
      case DISCRETEHS:
         HError(2640,"SetHSetKindCommand: conversion to %s not implemented",s);
         break;
      }
      break;
   case TIEDHS:
      switch(hk) {
      case PLAINHS:
         /* Should copy each tm to each slot */
      case SHAREDHS:
         /* Should make macros from tm and copy to slots */
      case DISCRETEHS:
         /* Should make code-book and discretise system */
         HError(2640,"SetHSetKindCommand: conversion to %s not implemented",s);
      }
      break;
   case DISCRETEHS:
      switch(hk) {
      case PLAINHS:
         /* Should make macros from code book */
         /* Should copy each tm to each slot */
      case SHAREDHS:
         /* Should make macros from code book */
         /* Should make macros from tm and copy to slots */
      case TIEDHS:
         /* Should make tmrecs from code book */
         HError(2640,"SetHSetKindCommand: conversion to %s not implemented",s);
      }
      break;
   }

   hset->hsKind=hk;
   if (trace & (T_BID | T_SIZ)) {
      printf(" HK: Rebuilt %d streams\n",nconv);
      fflush(stdout);
   }
}

/* ------------------ RM - Remove Mean Command ------------------ */

/* RemMean: subtract src from tgt */
void RemMean(Vector src, Vector tgt)
{
   int i,n;

   n = VectorSize(src);
   i = VectorSize(tgt);
   if (n != i)
      HError(2630,"RemMean: vector sizes incompatible %d vs %d",n,i);
   for (i=1; i<=n; i++)
      tgt[i] -= src[i];
}

/* RemMeansCommand: remove mean given by state 1, mix 1 of given
   hmm def, if all then remove means from energy too */
void RemMeansCommand(void)
{
   HMMScanState hss;
   HMMSet tmpSet;
   HLink hmm;
   MLink ml;
   Vector sv[SMAX];
   int s,nedit=0;
   char buf[MAXSTRLEN];

   ChkedAlpha("RM hmm file name",buf);
   if (trace & T_BID) {
      printf("\nRM %s\n Subracting HMM Mean from HMMSet means\n",buf);
      fflush(stdout);
   }

   CreateHMMSet(&tmpSet,&hmmHeap,TRUE);
   if(MakeOneHMM(&tmpSet,buf)<SUCCESS)
      HError(2628,"RemMeansCommand: MakeOneHMM failed");
   if(LoadHMMSet(&tmpSet,NULL,NULL)<SUCCESS)
      HError(2628,"RemMeansCommand: LoadHMMSet failed");

   if (hset->hsKind==TIEDHS || hset->hsKind==DISCRETEHS)
      HError(2640,"RemMeansCommand: HMM is not continuous");
   if (hset->swidth[0] != tmpSet.swidth[0])
      HError(2630,"RemMeansCommand: num streams incompatible");
   if (hset->pkind != tmpSet.pkind)
      HError(2630,"RemMeansCommand: different parameter kinds");
   ml = FindMacroName(&tmpSet,'h',GetLabId(buf,FALSE));
   hmm = (HLink) ml->structure;
   for (s=1; s<=hset->swidth[0]; s++) {
      if (hset->swidth[s] != tmpSet.swidth[s])
         HError(2630,"RemMeansCommand: stream %d different size",s);
      sv[s] = hmm->svec[2].info->pdf[s].info->spdf.cpdf[1].mpdf->mean;
   }
   NewHMMScan(hset,&hss);
   while(GoNextMix(&hss,FALSE)) {
      RemMean(sv[hss.s],hss.me->mpdf->mean);
      nedit++;
   }
   EndHMMScan(&hss);
   if (trace & (T_BID | T_SIZ)) {
      printf(" RM: Mean subtracted from %d mixes\n",nedit);
      fflush(stdout);
   }
}

/* --------------- QS - Load Question Set Commands ------------ */

/* QuestionCommand: question is an item list of model names */
void QuestionCommand(void)
{
   char qName[MAXSTRLEN];

   ChkedAlpha("QS question name",qName);
   
   /* get copy of original item list whilst parsing it */
   if (reduceMem!=0) {
      char pattern[PAT_LEN]; 
      ReadLine(&source, pattern);
      if (trace & T_QST) {
         printf("\nQS %s %s Define question\n",qName,pattern);
         fflush(stdout);
      }
      LoadQuestion(qName,NULL,pattern);
      setQMTable=FALSE;
   }
   else {
      ILink ilist=NULL;
      char type='h';
      char *pattern = PItemList(&ilist,&type,hset,&source,NULL,0,0,(trace&T_ITM)?TRUE:FALSE);

   if (trace & T_QST) {
      printf("\nQS %s %s Define question\n",qName,pattern);
      fflush(stdout);
   }
      if (ilist==NULL && (trace & T_QST))
      HError(-2631,"QuestionCommand: No items for question %s\n",qName);
   else {
      LoadQuestion(qName,ilist,pattern);
      if (trace & T_QST) {
         printf(" QS: Refers to %d of %d models\n",
                NumItems(ilist),hset->numLogHMM);
         fflush(stdout);
      }
   }
   }
}

/* --------------- TB - Tree Based Clustering Command ------------ */

void TreeBuildCommand(void)
{
   ILink ilist = NULL;          /* list of items to tie */
   char type = ' ';             /* type of items to tie */
   char macName[MAXSTRLEN];     /* name of macro to use */
   char *pattern, *p;           /* pattern of items to use */
   float thresh = 0.0;
   int s;

   ClearSet(streams);
   
   thresh = ChkedFloat("Tree build threshold",0.0,FLOAT_MAX);
   ChkedAlpha("TB macro name",macName);
   
   if (useRefTree<0 && treeList!=NULL && thisCommand!=lastCommand)
      HError(2640,"TreeBuildCommand: TB commands must be in sequence");
   if (!occStatsLoaded)
      HError(2655,"TreeBuildCommand: No stats loaded - use LS command");
   if (strlen(macName) > 20 )
      HError(-2639,"TreeBuildCommand: %s is rather long for a macro name",
             macName);

   pattern = PItemList(&ilist,&type,hset,&source,&streams,0,0,(trace&T_ITM)?TRUE:FALSE);

   if (trace & (T_BID | T_MAC | T_CLUSTERS | T_TREE)) {
      printf("\nTB %.2f %s %s\n Tree based clustering ",thresh,macName,pattern);
      fflush(stdout);
   }
   
   /* extract specified model pattern */ 
   if (type != 'h') {
      if ((p = strchr(pattern, '.')) == NULL)
         HError(9999, "TreeBuild: Specified pattern does not include any dot");
      *p = '}'; *(p+1) = '\0';
   }
      
   if (ilist==NULL) {
      HError(-2631,"TreeBuildCommand: No items to cluster for %s\n",macName);
      return;
   }

   if (type!='h' && type!='s' && type!='p')
      HError(2640,"TreeBuildCommand: Type %c not implemented",type);

   if (type=='h' || type=='s')
      for (s=1;s<=hset->swidth[0];s++)
         streams.set[s] = TRUE;
   
   /* Do Tree based clustering */
   BuildTree(ilist, (double)thresh, macName, pattern);
}

/* ------------- AU - Add Unseen Triphones Command --------- */

void AddUnseenCommand(void)
{
   char newListFn[MAXFNAMELEN];     /* name of hmmList containing unseen models */
   HMMSet newSet;
   int oldL = hset->numLogHMM;
   int oldP = hset->numPhyHMM;

   ChkedAlpha("AU hmmlist file name",newListFn);
   if (trace & (T_BID | T_TREE_ANS)) {
      printf("\nAU %s\n Creating HMMset using trees to add unseen triphones\n",
             newListFn);
      fflush(stdout);
   }
   if (treeList == NULL)
      HError(2662,"AddUnseenCommand: there are no existing trees");
   CreateHMMSet(&newSet,&hmmHeap,TRUE);
   if(MakeHMMSet(&newSet,newListFn)<SUCCESS)
      HError(2628,"AddUnseenCommand: MakeHMMSet failed");

   TreeFilter(&newSet);
   SwapLists(hset,&newSet);
   if (trace & T_BID) {
      printf(" AU: %d Log/%d Phys created from %d Log/%d Phys\n",
             hset->numLogHMM,hset->numPhyHMM,oldL,oldP);
      fflush(stdout);
   }
}

/* ------------ IT - Impose Tree Command ---------- */

void ImposeTreeCommand (void)
{
   char buf[MAXSTRLEN],fn[MAXFNAMELEN],macName[MAXSTRLEN];
   int h,i,j,snum,totalItems=0,totalClust=0;
   float occ;
   Boolean shrinkTree=FALSE;
   MLink q;
   HLink hmm;
   LabId tid=NULL;
   Node *node;
   Boolean isYes=FALSE;
   CLink cl=NULL;
   Tree *tree;
   ILink ilist=NULL;
   
   void ShrinkTree(Tree *tree, Node *node, int *snum, const float threshold);
   
   ChkedAlpha("IT trees file name",fn);  /* get name of trees file */

   if (trace & T_BID) {
      printf("\nIT\n Clustering while imposing previously constructed tree-based tying structure\n");
      fflush(stdout);
   }

   if (hset->hsKind!=PLAINHS)
      HError(9999,"ImposeTreeCommand: only PLAINHS is supported");
   
   if (!occStatsLoaded)
      HError(2672, "ImposeTreeCommand: Use LoadStats (LS <whatever-dir/stats>) before doing this.");
   
   if (treeList == NULL)
      HError(2662,"ImposeTreeCommand: there are no existing trees");
   
   for (tree=treeList; tree!=NULL; tree=tree->next) {
      /* set active stream flag */
      ClearSet(streams);
      for (i=1; i<=tree->streams.nMembers; i++) 
         streams.set[i] = tree->streams.set[i];
      
      /* assign HMM states to leaf nodes */
      for (h=0; h<MACHASHSIZE; h++) {
         for (q=hset->mtab[h]; q!=NULL; q=q->next) {
            if (q->type=='h') {
               hmm=(HLink) q->structure;
               
               /* find tree to use */
               strcpy(buf,q->id->name);
               if (!usePattern) {
                  if (strchr(buf,'*')!=NULL || strchr(buf,'?')!=NULL)
                     HError(9999,"ImposeTreeCommand: baseId %s includes pattern character", buf);
                  TriStrip(buf); 
                  MapTreeName(buf);
                  tid = GetLabId(buf,FALSE);
               }
               
               if (((usePattern && IPatMatch(buf, tree->patList)) 
                 || singleTree 
                 || (!singleTree && !usePattern && tree->baseId == tid))) {  /* check this model matches to this tree */    
        
                  /* traverse tree */
                  node = tree->root;  
                  occ=hmm->svec[2].info->stateCounter; node->occ+=occ;  /* acc occ to shrink tree, use occ of HMM instead of state */
                  while (node->yes != NULL) {
                     isYes = QMatch(q->id->name,node->quest);
                     node = (isYes) ? node->yes : node->no;
                     node->occ += occ;
                  }
   
                  /* make clist */
                  j = tree->state;
                  if (j<0) {
                     AddItem(hmm, hmm, &ilist);
                     for (j=2; j<hmm->numStates; j++)
                       InitTreeAccs(hmm->svec+j);
                  }
                  else {
                     AddItem(hmm, hmm->svec+j, &ilist);
                     InitTreeAccs(hmm->svec+j);
                  }           
                  cl=(CLink) New(&tmpHeap,sizeof(CRec));
                  hmm->hook = cl;
                  cl->item = ilist;  cl->ans = FALSE;
                  cl->idx = -1;
                  cl->next = node->clist;
                  node->clist = cl;
                  totalItems++;
               }
            }
         }
      }
      
      /* get macroname */
      for (node=tree->leaf; node->next!=NULL; node=node->next);
      strcpy(macName,node->id->name);
      macName[strlen(macName)-1] = '\0';
      
      /* shrink tree to avoid empty leaf node */
      for (node=tree->leaf,numTreeClust=0; node!=NULL; node=node->next) {
         if (node->clist==NULL) {
            tree->leaf=NULL; numTreeClust=0; snum=0;
            ShrinkTree(tree,tree->root,&snum,0.1);
            if (trace & T_BID) {
               if (tree->state>0)
                  printf(" Tree %s.state[%d]",ReWriteString(tree->baseId->name,NULL,ESCAPE_CHAR),tree->state);
               else
                  printf(" Tree %s",tree->baseId->name);
               if (tree->nActiveStr==hset->swidth[0])
                  printf("\n");
               else {
                  printf(".stream[");         /* output active stream of current tree */
                  for (i=1,j=1; j<=hset->swidth[0]; j++)
                     if (tree->streams.set[j])
                        printf("%d%s",j,(i++<tree->nActiveStr)?",":"]\n");
               }
               printf("  %d leaf nodes were pruned into %d nodes\n\n", tree->nLeaves, numTreeClust);
               fflush(stdout);
            }
            tree->nLeaves=numTreeClust; tree->size=snum;
            
            /* reset node->id && node->macro */
            for (node=tree->leaf; node!=NULL; node=node->next) {
               node->id = NULL;  node->macro = NULL;
            }
            /* turn on output tree flag */
            shrinkTree = TRUE;
            break;
         }
         else
            numTreeClust++;
      }
   
      /* tie leaf nodes */
      no.sum=CreateDVector(&tmpHeap,hset->vecSize);
      no.sqr=CreateDVector(&tmpHeap,hset->vecSize);
      MakeAccSum(&no,ilist);
      
      TieLeafNodes(tree,macName);
      
      FreeDVector(&tmpHeap,no.sum);
      ilist = NULL;
      totalClust+=numTreeClust;
   }
   if (trace & T_BID) {
      printf("\n IT: Stats %d->%d [%.1f%%] total\n", totalItems, totalClust,(float)totalClust*100.0/(float)totalItems);
      fflush(stdout);
   }
   
   
   /* if any trees are shrinked, output trees to fn */ 
   if (shrinkTree) {
      int s;
      FILE *file;
      Boolean isPipe;
      
      if (trace & T_BID) {
         printf("\nIT %s\n Writing new trees to file\n",fn);
         fflush(stdout);
      }
      if ((file=FOpen(fn,NoOFilter,&isPipe))==NULL)
         HError(2611,"ShowTreesCommand: Cannot open trees file %s",fn);
      
      PrintQuestions(file);
    
      for (tree=treeList;tree!=NULL;tree=tree->next) {
         if (tree->state>0)                       
            fprintf(file,"%s[%d]",
                    ReWriteString(tree->baseId->name,NULL,ESCAPE_CHAR),
                    tree->state);
         else
            fprintf(file,"%s",tree->baseId->name);
         if (tree->nActiveStr == hset->swidth[0])
            fprintf(file,"\n");
         else {
            fprintf(file,".stream[");         /* output active stream of current tree */
            for (s=1,i=1;s<=hset->swidth[0];s++)
               if (tree->streams.set[s])
                   fprintf(file,"%d%s",s,(i++<tree->nActiveStr)?",":"]\n");
         }
         PrintTree(tree,file);
      }
      FClose(file,isPipe);
   }
   
   return;
}

/* ------------- UF - Use File Command --------- */

void UseCommand(void)
{
   MILink mil;
   char newMMF[MAXFNAMELEN];   /* Name of MMF file to store macros in */

   ChkedAlpha("UF file name",newMMF);
   if (trace & T_BID) {
      printf("\nUF %s\n Using MMF file to store new macros",newMMF);
      fflush(stdout);
   }
   mil=AddMMF(hset,newMMF);
   mil->isLoaded=TRUE; /* Just to make sure we actually save to file */
   fidx=mil->fidx;
}

/* -------------------------- FA Command ------------------------- */
void FloorAverageCommand(void)
{
   HMMScanState hss;
   StateInfo *si   ;
   StreamInfo *sti;
   SVector var , mean;
   DVector *varAcc;
   double occAcc[SMAX];
   float weight;
   float occ,occs;
   int l=0,k,i,s,S;
   float varScale;
   Boolean singleMix;

   /*  need stats for operation */
   if (!occStatsLoaded)
      HError(1,"FloorAverageCommand: stats must be loaded before calling FA");
   varScale = ChkedFloat("Variance Scale Value",0.0,FLOAT_MAX);
   if (hset->hsKind==TIEDHS || hset->hsKind==DISCRETEHS)
      HError(2640,"FloorAverageCommand: Only possible for continuous models");
   if (hset->ckind != DIAGC)
      HError(2640,"FloorAverageCommand: Only implemented for DIAGC models");
   S= hset->swidth[0];

   /* allocate accumulators */
   varAcc = (DVector*)New(&gstack,(S+1)*sizeof(DVector));
   for (s=1;s<=S;s++) {
      varAcc[s] = CreateDVector(&gstack,hset->swidth[s]);
      ZeroDVector(varAcc[s]);
   }
   NewHMMScan(hset,&hss);
   for (s=1;s<=S;s++) occAcc[s] = 0.0;
   while(GoNextState(&hss,FALSE)) {
      si = hss.si;
      while (GoNextStream(&hss,TRUE)) {
         s=hss.s;
         l=hset->swidth[s];
         sti = hss.sti;
         memcpy(&occ,&(sti->hook),sizeof(float));
         /* single/multiple mix check */
         for (i=1,k=0; i<=hss.M; i++) {
            if (hset->swidth[s] == VectorSize(sti->spdf.cpdf[i].mpdf->mean))  /* MSD check */
               k++;
         }
         singleMix = (k==1) ? TRUE : FALSE;
         /* accumulate statistics */
         if ( !singleMix ) { 
            occs = 0.0;
            for (k=1;k<=l;k++) { /* loop over k-th dimension */
               double    rvar  = 0.0;
               double    rmean = 0.0;
               for (i=1; i<= hss.M; i++) {
                  if (VectorSize(sti->spdf.cpdf[i].mpdf->mean) == l) {  /* MSD check */
                  weight = sti->spdf.cpdf[i].weight;
                  var  = sti->spdf.cpdf[i].mpdf->cov.var;
                  mean = sti->spdf.cpdf[i].mpdf->mean;
                  
                  rvar  += weight * ( var[k] + mean[k] * mean[k] );
                  rmean += weight * mean[k];
                  if (k==1) occs += weight * occ;
                  }
               }
               rvar -= rmean * rmean;
               varAcc[s][k] += rvar * occs ;
            }
            occAcc[s] += occs;
         }
         else { /* single mix (single space matches the stream weight) */
            for (i=1; i<=hss.M; i++) {
               if (VectorSize(sti->spdf.cpdf[i].mpdf->mean) == l) {
                  weight = sti->spdf.cpdf[i].weight;
                  var    = sti->spdf.cpdf[i].mpdf->cov.var;
            for (k=1;k<=l;k++) 
                     varAcc[s][k] += occ * weight * var[k];
                  occAcc[s] += occ * weight;
               }
            }
         }
      }
   }
   EndHMMScan(&hss);
   /* normalisation */
   for (s=1;s<=S;s++)
      for (k=1;k<=hset->swidth[s];k++)
         varAcc[s][k] /= occAcc[s];
   /* set the varFloorN macros */
   for (s=1; s<=S; s++){
      int size;
      LabId id;
      SVector v;
      char mac[MAXSTRLEN];
      MLink m;
      sprintf(mac,"varFloor%d",s);
      id = GetLabId(mac,FALSE);
      if (id != NULL  && (m=FindMacroName(hset,'v',id)) != NULL){
         v = (SVector)m->structure;
         SetUse(v,1);
         /* check vector sizes */
         size = VectorSize(v);
         if (size != hset->swidth[s])
            HError(7023,"FloorAverageCommand: Macro %s has vector size %d, should be %d",
                   mac,size,hset->swidth[s]);
      }
      else { /* create new macro */
         id = GetLabId(mac,TRUE);
         v = CreateSVector(hset->hmem,hset->swidth[s]);
         NewMacro(hset,hset->numFiles,'v',id,v);
      }
      /* scale and store */
      for (k=1;k<=hset->swidth[s];k++)
         v[k] = varAcc[s][k]*varScale;
   }
   /* and apply floors */
   if (applyVFloor)
       ApplyVFloor(hset);
   else
       HError(-7023,"FloorAverageCommand: variance floors have not been applied to mixes");

   for (s=S; s>=1; s--)
      FreeDVector(&gstack, varAcc[s]);
   Dispose(&gstack,varAcc);
}

/* -------------------------- FV Command ------------------------- */
void FloorVectorCommand(void)
{
   /* read macro file as generated by HCompV 
       and copy floor vectors to model set */
   char fn[MAXFNAMELEN],mac[MAXSTRLEN],buf[MAXSTRLEN];
   Source src;
   int s,S,n,i;
   char c;
   LabId id;
   SVector v;
   MLink m;

   ChkedAlpha("FV file containing variance floor value(s)",fn);
   if (trace & T_BID) {
      printf("\nFV: loading variance floor(s) from file %s\n",fn);
      fflush(stdout);
   }
   /* check model set */
   if (hset->hsKind==TIEDHS || hset->hsKind==DISCRETEHS)
      HError(2640,"FloorVectorCommand: Only possible for continuous models");
   S=hset->swidth[0];

   /* load variance floor macro */
   if(InitSource(fn, &src, NoFilter)<SUCCESS)
      HError(2610,"FloorVectorCommand: Can't open file");
   do {
      while ( (c=GetCh(&src)) != '~' && c!= EOF);
      c = tolower(GetCh(&src));
      if (c == 'v' ) { 
         /* found variance vector */
         if (!ReadString(&src,buf))
            HError(2613,"FloorVectorCommand: cannot parse file %s",fn);
         strcpy(mac,"varFloor");
         s=atoi(buf+strlen(mac));
         buf[strlen(mac)]='\0';
         if (strcmp(mac,buf)!=0) /* not a varFloor */
            continue;
         if(s<1||s>S)
            HError(2613,"FloorVectorCommand: undefined stream %d in HMM set",s);
         sprintf(mac,"%s%d",mac,s);
         if (trace & T_BID) {
            printf("loading vector %s\n",mac);
         }
          
         /* obtain vector */
         id = GetLabId(mac,FALSE);
         if (id != NULL  && (m=FindMacroName(hset,'v',id)) != NULL){
            v = (SVector)m->structure;
            SetUse(v,1);
         }
         else { /* create new macro */
            id = GetLabId(mac,TRUE);
            v = CreateSVector(hset->hmem,hset->swidth[s]);
            NewMacro(hset,hset->numFiles,'v',id,v);
         }
         /* load vector */
         if (!ReadString(&src,buf))
            HError(2613,"FloorVectorCommand: cannot parse file %s",fn);
         for (n=strlen(buf),i=0;i<n;i++)
            buf[i]=tolower(buf[i]);
         if (strcmp("<variance>",buf)!=0)
            HError(2613,"FloorVectorCommand: variance vector expected",fn);
         if (!ReadInt(&src,&n,1,FALSE))
            HError(2613,"FloorVectorCommand: cannot parse file %s",fn);
         if (n!=hset->swidth[s])
            HError(2613,"FloorVectorCommand: stream width mismatch (stream %d)",s);
         if (!ReadFloat(&src,v+1,n,FALSE))
            HError(2613,"FloorVectorCommand: cannot load variance vector");
      }
   } while(c!=EOF);
   CloseSource(&src);

   /* and apply floors */
   if (applyVFloor)
       ApplyVFloor(hset);
   else
       HError(-7023,"FloorVectorCommand: variance floors have not been applied to mixes");
}

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define ABS(a) ((a)>0?(a):-(a))

/* Sets size of state... */
int SetSize(char *hname, StreamInfo *sti /*nMix must be +ve*/, int tgt)
{  /*returns nDefunct*/
   int nDefunct=0,sign = (sti->nMix > 0 ? 1 : -1); 
   int m, M;
   sti->nMix *= sign; M=sti->nMix;

   /* (1) remove defunct mixture compoonents */
   for (m=1; m<=M; m++)
      if (sti->spdf.cpdf[m].weight <= MINMIX) {
         MixPDF *mp=sti->spdf.cpdf[m].mpdf;
         if (mp->nUse) mp->nUse--; /* decrement MixPDF use */
         sti->spdf.cpdf[m] = sti->spdf.cpdf[M]; M--; 
         nDefunct++;
      }
   /* (2) get to desired size.. */
   sti->nMix = M;
   if(tgt < M) DownMix(hname,sti,tgt,TRUE);
   else if(tgt > M) UpMix(hname, sti, M, tgt);
   sti->nMix *= sign;
   return nDefunct;
}


/* ------------- PS Command --------------- */

void PowerSizeCommand(void)
{
   int ch;
   int nDefunct=0,m=0,M=0,maxM=0,newM=0,newMIdeal=0,nTooBig=0;  int S=0;
   float minweight = 1;
   float sum=0; float factor;

   HMMScanState hss;
   float AvgNumMix = ChkedFloat("AvgNumMix",1.1, 1000);
   float Power = ChkedFloat("Power",0.1, 10);
   float NumItsRemaining = 1.0;   /* A float but will normally be an int.
                                     Set this to e.g. 4,3,2,1 in succession
                                     for 4 its of up-mixing. */
   
   do { 
      ch = GetCh(&source);
      if (ch=='\n') break;
   } while(ch!=EOF && isspace(ch));
   UnGetCh(ch,&source);
   if (ch!='\n') { /* Get nIts remaining... */
      NumItsRemaining = ChkedFloat("NumItsRemaining",1, 100);
   }


   if(!occStatsLoaded)
      HError(2672, "ControlSizeCommand: Use LoadStats (LS <whatever-dir/stats>) before doing this.");

   printf("Running PowerSize command, tgt #mix/state = %f, power of #frames=%f, NumItsRemaining=%f\n",
          AvgNumMix,Power,NumItsRemaining); fflush(stdout);

   NewHMMScan(hset,&hss);
   while(GoNextStream(&hss,FALSE)){
      float stateocc;
      memcpy(&stateocc,&(hss.si->hook),sizeof(float));
      sum += exp(log(MAX(stateocc,MINLARG)) * Power);
      S++; M+=hss.M;  maxM=MAX(maxM,hss.M); 
      for (m=1;m<=hss.M;m++){ minweight = MIN(hss.ste->info->spdf.cpdf[m].weight, minweight); }
   }  /*count states. */
   EndHMMScan(&hss);

   if(minweight > 0){
      printf("Min weight in this HMM set is %f, perhaps because weights were floored\n"
             "If so, use HERest with -w 0 or no -w option to get unfloored weights.\n", minweight);
   }
   factor = AvgNumMix / (sum/S);  

   NewHMMScan(hset,&hss);
   while(GoNextStream(&hss,FALSE)){
      float stateocc; float tgt; int itgt;
      memcpy(&stateocc,&(hss.si->hook),sizeof(float));
      tgt = factor * exp(log(MAX(stateocc,MINLARG)) * Power);
      itgt = (int)(tgt+0.5); 
      if(itgt<1) itgt=1;

      newMIdeal += itgt;
      if(itgt > ABS(hss.M) && NumItsRemaining!=1.0){
         /* If NumItsRemaining>0, then don't mix up all at once. */
         int nummix = ABS(hss.M);
         int newitgt = (int) (0.5 +  nummix * exp( 1.0/NumItsRemaining * log(itgt/(float)nummix)));
         if(newitgt <= nummix)   newitgt = nummix+1;
         itgt = newitgt;
      }
      newM += itgt;
      if(itgt > hss.M*2) nTooBig++;
      nDefunct += SetSize(HMMPhysName(hset,hss.hmm), hss.ste->info, itgt);
   }
   EndHMMScan(&hss);
   
   if(nTooBig>0) HError(-2631/*?*/, "%d(/%d) mixtures are more than doubled in size; you may want to mix up \n"
                        "in multiple iterations as: PS #mix power n, where n = nIts remaining (e.g. 3,2,1)", nTooBig, S);
   printf("Initial %d mixes, final %d mixes, avg %f/state, nDefunct=%d\n",M,newM,newM/(float)S,
          nDefunct);
   if(NumItsRemaining!=1.0)
      printf("Aiming for final nMixes = %d, nIts remaining = %f\n",newMIdeal, (NumItsRemaining-1.0));  
}


/* -------------------------- MD Command ------------------------- */

void MixDownCommand(void)
{
   ILink i,ilist = NULL;		/* list of items to mixdown */
   char type = 'p';		        /* type of items must be p */
   int trg;
   int m,M;
   int totm=0,totM=0;
   StreamElem *ste;
   StreamInfo *sti;
   HMMDef *hmm;
   char *hname;
   int nDefunct,nDefunctMix;
    
   trg = ChkedInt("MD: Mix Target",1,INT_MAX);
   if (trace & T_BID){
      printf("Executing Mix Down Command to %d\n",trg);
      fflush(stdout);
   }
   /* check compatibility with model set */
   if (hset->hsKind==TIEDHS || hset->hsKind==DISCRETEHS)
      HError(2640,"MixDownCommand: MixDown only possible for continuous models");

   /* get itemlist */
   PItemList(&ilist,&type,hset,&source,NULL,0,0,(trace&T_ITM)?TRUE:FALSE);
   if (ilist == NULL) {
      HError(-2631,"MixDownCommand: No mixtures to decrease!");
      return;
   }
   /* touch all mixture models to avoid duplicates */
   nDefunct = nDefunctMix= 0; 
   for (i=ilist; i!=NULL; i=i->next) {
      ste = (StreamElem *)i->item;
      sti = ste->info;
      M = sti->nMix;
      if (M>0) sti->nMix = -M;
   }

   /* mix down all pdf's */
   nDefunct = nDefunctMix= 0; 
   for (i=ilist; i!=NULL; i=i->next){
      hmm = i->owner;
      ste = (StreamElem *)i->item; 
      sti = ste->info; 
      M = sti->nMix;
      if (M<0) { 
         /* mix not yet processed */
         M = sti->nMix = -M;
         hname = HMMPhysName(hset,hmm);
         /* remove defunct mixture compoonents */
         for (m=1; m<=M; m++)
            if (sti->spdf.cpdf[m].weight <= MINMIX) {
               MixPDF *mp=sti->spdf.cpdf[m].mpdf;
               if (mp->nUse) mp->nUse--; /* decrement MixPDF use */
               sti->spdf.cpdf[m] = sti->spdf.cpdf[M]; M--; 
               nDefunct++;
            }
         if(sti->nMix>M) {
            if ( trace & T_DET) 
               printf("MD %12s : %d out of %d mixes defunct\n",hname,sti->nMix-M,sti->nMix);
            nDefunctMix++;
            sti->nMix = M;
         }
         /* downmix if necessary */
         if (trg<M) {
            DownMix(hname,sti,trg,TRUE);
            totm+=trg;
         }
         else 
            totm+=M;
         totM+=M;
      }
   }
   FreeItems(&ilist);
   if (trace & T_BID) {
      if (nDefunct>0)
         printf("MD: deleted %d defunct mix comps in %d pdfs \n",nDefunct,nDefunctMix);
      else
         printf("MD: no defunct mixcomps found \n");
      printf("MD: Number of mixes decreased from %d to %d\n",totM,totm);
      fflush(stdout);
   }
}

/* ------------- FC - FullCovar Command ----------------- */

void FullCovarCommand(void)
{
   HMMScanState hss;
   int i,j,u,z;
   SVector var;
   STriMat invcov;
   MLink ml;

   if (hset->hsKind==TIEDHS || hset->hsKind==DISCRETEHS)
      HError(2640,"FullCovarCommand: Only possible for continuous models");
   if (hset->ckind != DIAGC)
      HError(2640,"FullCovarCommand: Only implemented for DIAGC models");

   ConvDiagC(hset,TRUE);

   NewHMMScan(hset,&hss);
   while(GoNextMix(&hss,FALSE)) {
      var = hss.me->mpdf->cov.var;
      hss.me->mpdf->ckind = FULLC;
      u = GetUse(var);
      if (u==0 || !IsSeen(u)) {
         z = VectorSize(var);
         invcov = CreateSTriMat(&hmmHeap,z);
         /* ZeroTriMat(invcov); */
         for (i=1; i<=z; i++) {
            for (j=1; j<i; j++)
               invcov[i][j] = 0.0;
            invcov[i][i] = var[i];
         } 
         if (u!=0) {
            ml = FindMacroStruct(hset,'v',var);
            DeleteMacro(hset,ml);                  /* this needs to delete */
            NewMacro(hset,fidx,'i',ml->id,invcov); /* the macro name also */
            TouchV(var);
            SetUse(invcov,u);
            SetHook(var,invcov);
         }
      }
      else  /* a macro that we have already seen */
         invcov = GetHook(var);
      hss.me->mpdf->cov.inv = invcov;
   }
   EndHMMScan(&hss);
   hset->ckind = FULLC;
}

/* ------------- DV - DiagVar Command ----------------- */

void DiagVarCommand (void)
{
   HMMScanState hss;
   int i,u,z;
   SVector var;
   STriMat inv;
   MLink ml;

   if (hset->hsKind==TIEDHS || hset->hsKind==DISCRETEHS)
      HError(2640,"DiagVarCommand: Only possible for continuous models");
   if (hset->ckind != FULLC)
      HError(2640,"DiagVarCommand: Only implemented for FULLC models");

   NewHMMScan(hset,&hss);
   while(GoNextMix(&hss,FALSE)) {
      inv = hss.me->mpdf->cov.inv;
      hss.me->mpdf->ckind = INVDIAGC;
      u = GetUse(inv);
      if (u==0 || !IsSeen(u)) {
         z = TriMatSize(inv);
         var = CreateSVector(&hmmHeap,z);
         for (i=1; i<=z; i++)
            var[i] = inv[i][i];
         if (u!=0) {
            ml = FindMacroStruct(hset,'i',inv);
            DeleteMacro(hset,ml);                 /* this needs to delete */
            NewMacro(hset,fidx,'v',ml->id,var);   /* the macro name also */
            TouchV(inv);
            SetUse(var,u);
            SetHook(inv,var);
         }
      }
      else  /* a macro that we have already seen */
         var = GetHook(inv);
      hss.me->mpdf->cov.var = var;
   }
   EndHMMScan(&hss);
   hset->ckind = DIAGC;
   
   ConvDiagC(hset,TRUE);  /* inv -> var */
   FixAllGConsts(hset);   /* fix gConsts */
}

/* ------------- PR - ProjectCommand   ----------------- */

void ProjectCommand(void)
{
   AdaptXForm *xform;

   xform = hset->semiTied;
   if ((xform == NULL) || (hset->projSize == 0))
      HError(999,"Can not only project with semitied XForm and PROJSIZE>0");

   /* check that this is a reasonable command to run */
   if (xform->bclass->numClasses != 1)
      HError(999,"Can only store as input XForm with global transform");
   if (hset->swidth[0] != 1) 
      HError(999,"Can only store as input XForm with single stream");
   if (hset->xf != NULL)
      HError(999,"Can not store as input XForm if HMMSet already has an input XForm");

   UpdateProjectModels(hset,newDir);
}

/* -----Regression Class Clustering Tree Building Routines -------------- */

/* linked list of components that are grouped into a cluster */
typedef struct _CoList {

   struct _CoList *next;     /* next component in the linked list */
   char   *hmmName;          /* Physical hmm name for this component */
   int    state;             /* state containing this component */
   int    stream;            /* stream for this component */
   int    mix;               /* mixture for this component */
   MixPDF *mp;               /* actual component */

} CoList;
  
/* node information in the regression class tree */
typedef struct {

   int vSize;                /* vector size of this cluster */
   Vector aveMean;           /* node cluster mean */
   Vector aveCovar;          /* node cluster variance */
   double clusterScore;      /* node cluster score */
   double clustAcc;          /* accumulates in this cluster */
   int  nComponents;         /* number of components in this cluster */
   short  nodeIndex;         /* node index number */
   CoList *list;             /* linked list of the mixture components */
   Boolean pureVecSize;
   Boolean pureStream;     

} RNode;

/* GetRNode: return the node of a tree */
RNode *GetRNode (RegNode *n) 
{
   return ((RNode *) n->info);
}

/* CreateClusterLink: Create a link for a cluster linked list */
CoList *CreateClusterLink(char *s, int state, int stream, int mix, 
                          MixPDF *mp) 
{
   CoList *c;

   c = (CoList *) New(&tmpHeap, sizeof(CoList));
   c->next = NULL;
   if (s == NULL)
      HError(2670, "CreateClusterLink: Physical name is NULL!?!");
   c->hmmName = s;
   c->state = state;
   c->stream = stream;
   c->mix = mix;
   c->mp = mp;

   return c;

}

/* PureVecSize: check dimensionality in list */
Boolean PureVecSize (CoList *list) 
{
   CoList *c;
   int vSize;
   
   if (list==NULL)
      return TRUE;
      
   vSize = VectorSize(list->mp->mean);

   for (c=list->next; c!=NULL; c=c->next)
      if (VectorSize(c->mp->mean)!=vSize)
         return FALSE;

   return TRUE;
}

/* PureStream: check stream in list */
Boolean PureStream (CoList *list) 
{
   CoList *c;
   int stream;
   
   if (list==NULL)
      return TRUE;
      
   stream = list->stream;

   for (c=list->next; c!=NULL; c=c->next)
      if (c->stream != stream)
         return FALSE;

   return TRUE;
}

/* MaxVecSize: maximum dimensionality in list */
int MaxVecSize (CoList *list) 
{
   CoList *c;
   int vSize;
   
   if (list==NULL)
      return 0;
      
   vSize = VectorSize(list->mp->mean);

   for (c=list->next; c!=NULL; c=c->next)
      if (VectorSize(c->mp->mean) > vSize)
         vSize = VectorSize(c->mp->mean);

   return vSize;
}

/* Create a tree node for use in the regression tree */
RNode *CreateRegTreeNode(CoList *list, int vSize) 
{
   RNode *n;

   n = (RNode *) New(&tmpHeap, sizeof(RNode));
   n->vSize = vSize;
   n->aveMean  = CreateVector(&tmpHeap, vSize);
   n->aveCovar = CreateVector(&tmpHeap, vSize);
   ZeroVector(n->aveMean);
   ZeroVector(n->aveCovar);
   n->list = list;
   n->nComponents = 0;
   n->clusterScore = 0.0;
   n->nodeIndex = -1;
   n->pureStream = FALSE;
   n->pureVecSize = FALSE;

   return n;
}

/* FreeRegTreeNode: free RNode */
void FreeRegTreeNode (RNode *r)
{
   FreeVector(&tmpHeap, r->aveCovar);
   FreeVector(&tmpHeap, r->aveMean);
   Dispose(&tmpHeap, r);
}

void PrintNodeInfo(RNode *n) 
{
   int i;

   printf("For cluster %d there are:\n", n->nodeIndex);
   printf("%d components with %f occupation count;\n  Cluster score of %e\n",
          n->nComponents, n->clustAcc, n->clusterScore);
   printf("MEAN:-\n");
   for (i=1; i<=n->vSize; i++)
      printf("%e ", n->aveMean[i]);
   printf("\nCOVARIANCE:-\n");
   for (i=1; i<=n->vSize; i++)
      printf("%e ", n->aveCovar[i]);
   printf("\n");
   fflush(stdout);
}


double Distance (Vector v1, Vector v2) 
{  
   int k, vSize;
   double x, dist = 0.0;

   vSize = VectorSize(v1);
   if (vSize==0)
      return LZERO;

   for (k = 1; k <= vSize; k++) {
      x = (double)(v1[k] - v2[k]);
      x *= x;
      dist += x;
   }

   return (sqrt(dist));
}

double CalcNodeScore (RNode *n) 
{
   double score = 0.0;
   Vector mean;
   CoList *c;
   AccSum *acc;

   if (!(n->pureVecSize && n->pureStream))
      return -LZERO;
      
   for(c = n->list; c != NULL; c = c->next) {
      mean = c->mp->mean;
      acc  = (AccSum *) c->mp->hook;
      if (acc != NULL)
         score += Distance(mean, n->aveMean) * acc->occ ;
   }

   return(score);
}

/* CalcClusterDistribution: calculation of the cluster distribution -> aveMean aveCovar */
void CalcClusterDistribution (RNode *n) 
{
   CoList *c;
   AccSum *acc;
   int k, s;
   double occ;
   DVector sum, sqr;
  
   sum = CreateDVector(&gstack, n->vSize);
   sqr = CreateDVector(&gstack, n->vSize);
   occ = 0.0;
   ZeroDVector(sum);
   ZeroDVector(sqr);
   s = n->list->stream;

   for (c = n->list; c != NULL; c = c->next) {
      /* scaled(mean) and scaled(mean*mean + covar) already stored in mp->hook */
      acc = (AccSum *) c->mp->hook;
      if (acc != NULL) {
         /* check pure stream and vecSize */
         if (n->pureVecSize && n->pureStream) {      
            for (k=1; k<=n->vSize; k++) {
            sum[k] += acc->sum[k];
            sqr[k] += acc->sqr[k];
         }
         }
         /* count only 1 stream */
         if (c->stream == s)
         occ += acc->occ;
      }
      else
         HError(-2671, "CalcClusterDistribution: Accumulates are non existant for hmm %s", 
                c->hmmName);
   }

   /* now calculate mean and covariance for this cluster */
   for (k=1; k<=n->vSize; k++) {
      n->aveMean[k]  = (n->pureVecSize && n->pureStream) ? (float)(sum[k] / occ) : 0.0;
      n->aveCovar[k] = (n->pureVecSize && n->pureStream) ? (float)((sqr[k] - sum[k]*sum[k]/occ) / occ) : 1.0;
   }
   n->clustAcc = occ;

   FreeDVector(&gstack, sqr);
   FreeDVector(&gstack, sum);
}
    
/* InitRegTree: Initialise a regression tree root node -- will also separate out
   speech and non-speech sounds */
RegTree *InitRegTree (HMMSet *hset, ILink ilist) 
{  
  HMMScanState hss;
  CoList *listSp=NULL,*listNonSp=NULL,*cs, *cn;
  RNode *r1=NULL, *r2=NULL, *rNode=NULL;
  RegNode *root;
  RegTree *rTree;
  int vSize, nComponentsSp = 0, nComponentsNonSp = 0;
  StreamElem *ste;
  ILink p=NULL;

  if (!occStatsLoaded)
    HError(2672, "InitRegTree: Building regression classes must load the stats file!\n");
  
  SetSet(streams);

  NewHMMScan(hset,&hss);

  if (!(hss.isCont || (hss.hset->hsKind == TIEDHS))) {
     HError(2673,"InitRegTree: Can only resize continuous featured system");
  }

  cs = cn = NULL;
  do {
    while (GoNextState(&hss,TRUE)) {
      InitTreeAccs(hss.se);
      while (GoNextStream(&hss,TRUE)) {
	/* go through item list to see if non-speech sound */
	if (ilist != NULL)
	  for (p=ilist; p != NULL; p=p->next) {
	    ste = (StreamElem *) p->item;
	    if (ste == hss.ste)
	      break;
	  }
	while (GoNextMix(&hss,TRUE)) {
	  if (p == NULL) {
	    if (cs == NULL) {
	      cs = CreateClusterLink(HMMPhysName(hset, hss.hmm), 
				     hss.i, hss.s, hss.m, hss.mp);
	      listSp = cs;
	    }
	    else {
	      cs->next = CreateClusterLink(HMMPhysName(hset, hss.hmm), 
					   hss.i, hss.s, hss.m, hss.mp);
	      cs = cs->next;
	    }
	    nComponentsSp += 1;
	  }
	  else {
	    if (cn == NULL) {
	      cn = CreateClusterLink(HMMPhysName(hset, hss.hmm), 
				     hss.i, hss.s, hss.m, hss.mp);
	      listNonSp = cn;
	    }
	    else {
	      cn->next = CreateClusterLink(HMMPhysName(hset, hss.hmm), 
					   hss.i, hss.s, hss.m, hss.mp);
	      cn = cn->next;
	    }
	    nComponentsNonSp += 1;
	  }  
	}
      }
    }
  } while (GoNextHMM(&hss));
  EndHMMScan(&hss);

  /* create an instance of the node */
  cs = NULL; cn = NULL;
  vSize = MaxVecSize(listSp);
  if (vSize <= 0)
    HError(2673, "InitRegTree: Problem with vector Size = %d", vSize);
  r1 = CreateRegTreeNode(listSp, vSize);  
  r1->nodeIndex = 1;
  r1->nComponents = nComponentsSp;
  r1->pureVecSize = PureVecSize(r1->list);
  r1->pureStream  = PureStream (r1->list);
  CalcClusterDistribution(r1);
  r1->clusterScore = CalcNodeScore(r1);
   
  /* create an instance of the tree */
  rTree = (RegTree *) New(&tmpHeap, sizeof(RegTree));
  rTree->bclass = NULL;
  rTree->numNodes = 0;
  rTree->numTNodes = 0;
  root = rTree->root = (RegNode *) New(&tmpHeap, sizeof(RegNode));
  /* is there a speech/sil split at the root */
  if (nComponentsNonSp > 0) {
    /* reset the root node */
    root->nodeIndex = 1;
    rNode = CreateRegTreeNode(listSp, vSize);  /* use listSp for dummy */
    rNode->nodeIndex = 1;
    rNode->nComponents = nComponentsNonSp + nComponentsSp;
    rNode->pureVecSize = r1->pureVecSize;
    rNode->pureStream  = r1->pureStream;
    root->info = rNode;
    root->numChild = 2;
    root->child = (RegNode **)New(&tmpHeap,3*sizeof(RegNode *));
    root->child[1] = (RegNode *) New(&tmpHeap, sizeof(RegNode));
    root->child[2] = (RegNode *) New(&tmpHeap, sizeof(RegNode));
    /* set-up information for first child */
    vSize = MaxVecSize(listNonSp);
    r2 = CreateRegTreeNode(listNonSp, vSize);
    r2->nodeIndex = 2;
    r2->nComponents = nComponentsNonSp;
    r2->pureVecSize = PureVecSize(r2->list);
    r2->pureStream  = PureStream (r2->list);
    CalcClusterDistribution(r2);
    r2->clusterScore = CalcNodeScore(r2);
    root->child[1]->info = r2;
    root->child[1]->numChild = 0;
    root->child[1]->child = NULL;
    /* set-up information for second child */
    r1->nodeIndex = 3;
    root->child[2]->info = r1;
    root->child[2]->numChild = 0;
    root->child[2]->child = NULL;
  }
  else {
    root->info = r1;
    root->numChild = 0;
    root->child = NULL;
  }
  return(rTree);
}


void PerturbMean(Vector mean, Vector covar, float pertDepth) 
{  
   float x;
   int k, vSize;

   vSize = VectorSize(mean);
   for(k = 1; k <= vSize; k++) {
      x = pertDepth * covar[k];
      mean[k] += x;
   }

}

void CalcDistance (CoList *list, RNode *ch1, RNode *ch2)
{
   int k, vSize;
   CoList *c;
   AccSum *acc;
   double score1, score2;
   Vector mean;
   DVector sum1, sum2;

   vSize = MaxVecSize(list);

   sum1 = CreateDVector(&gstack, vSize);
   ZeroDVector(sum1);
   sum2 = CreateDVector(&gstack, vSize);
   ZeroDVector(sum2);

   ch1->clusterScore = ch2->clusterScore = 0.0;
   ch1->clustAcc = ch2->clustAcc = 0.0;
  
   for(c = list; c != NULL; c = c->next) {
      mean = c->mp->mean;
      acc  = (AccSum *) c->mp->hook;
      score1 = Distance(mean, ch1->aveMean);
      score2 = Distance(mean, ch2->aveMean);
      if (score1 < score2) {
         if (acc != NULL) {
            ch1->clustAcc += acc->occ;
            ch1->clusterScore += (acc->occ * score1);
            for (k = 1; k <= vSize; k++)
               sum1[k] += acc->sum[k];
         }
      }
      else {
         if (acc != NULL) {
            ch2->clustAcc += acc->occ;
            ch2->clusterScore += (acc->occ * score2);
            for (k = 1; k <= vSize; k++)
               sum2[k] += acc->sum[k];
         }
      }
   }

   for (k = 1; k <= vSize; k++) {
      ch1->aveMean[k] = (float)(sum1[k] / ch1->clustAcc);
      ch2->aveMean[k] = (float)(sum2[k] / ch2->clustAcc);
   }

   FreeDVector(&gstack, sum2);
   FreeDVector(&gstack, sum1);
}

void CreateChildNodes (RNode *parent, RNode *ch1, RNode *ch2) 
{
   CoList *c, *c1, *c2, *list;
   double score1, score2;
   int numLeft, numRight, s, v;
   Vector mean;

   list = parent->list;
   c1 = c2 = NULL;
   numLeft = numRight = 0;
   s = list->stream;
   v = VectorSize(list->mp->mean);

   for(c = list; c != NULL; c = c->next) {
      if (parent->pureStream && parent->pureVecSize) {
         /* both stream & vecSize in this list are pure */
      mean = c->mp->mean;
      score1 = Distance(mean, ch1->aveMean);
      score2 = Distance(mean, ch2->aveMean);
      if (score1 < score2) {
         if (c1 == NULL)
            c1 = ch1->list = c;
         else {
            c1->next = c;
            c1 = c1->next;
         }
            numLeft++;
         }
         else {
            if (c2==NULL)
               c2 = ch2->list = c;
            else {
               c2->next = c;
               c2 = c2->next;
            }
            numRight++;
         }
      }
      else if (!parent->pureStream) {
         /* stream is not pure */
         if (c->stream == s) {
            if (c1==NULL)
               c1 = ch1->list = c;
            else {
               c1->next = c;
               c1 = c1->next;
            }
            numLeft++;
         }
         else {
            if (c2==NULL)
               c2 = ch2->list = c;
            else {
               c2->next = c;
               c2 = c2->next;
            }
            numRight++;
         }
      }
      else {
         if (VectorSize(c->mp->mean) == v) {
            if (c1==NULL)
               c1 = ch1->list = c;
            else {
               c1->next = c;
               c1 = c1->next;
            }
            numLeft++;
      }
      else {
         if (c2 == NULL)
            c2 = ch2->list = c;
         else {
            c2->next = c;
            c2 = c2->next;
         }
            numRight++;
         }
      }
   }

   ch1->nComponents = numLeft;
   ch2->nComponents = numRight;
   if (c1 != NULL)
      c1->next = NULL;
   if (c2 != NULL)
      c2->next = NULL;
}

void ClusterChildren (RNode *parent, RNode *ch1, RNode *ch2) 
{ 
   const double thresh = 1.0e-12;
   int iter=0;
   double oldDistance, newDistance=0.0 ;

   if (parent->pureStream && parent->pureVecSize) {
   do {
      iter+=1;
      if (iter >= MAX_ITER)
         break;
         CalcDistance(parent->list, ch1, ch2);
      if (iter == 1)
         oldDistance = ((ch1->clusterScore + ch2->clusterScore) / 
            (ch1->clustAcc + ch2->clustAcc)) + thresh + 1;
      else
         oldDistance = newDistance ;
      newDistance = (ch1->clusterScore + ch2->clusterScore) / 
         (ch1->clustAcc + ch2->clustAcc) ;
      if (trace & T_CLUSTERS) {
         if (iter == 1)
            printf("Iteration %d: Distance = %e\n", iter, newDistance);
         else
               printf("Iteration %d: Distance = %e, Delta = %e\n", iter, newDistance, oldDistance - newDistance);
         fflush(stdout);
      }
   } while ((oldDistance - newDistance) > thresh);

   if (trace & T_CLUSTERS)
      printf("Cluster 1: Score %e, Occ %e\t Cluster 2: Score %e, Occ %e\n",
             ch1->clusterScore,ch1->clustAcc, ch2->clusterScore,ch2->clustAcc);
   }

   CreateChildNodes(parent, ch1, ch2);
}

static void GetTreeVector(RegNode **nVec, RegNode *t) 
{  
  RNode *n;
  int i;

  if (t->numChild>0) {
    for (i=1;i<=t->numChild;i++) 
      GetTreeVector(nVec,t->child[i]);     
  }
  n = GetRNode(t);
  nVec[n->nodeIndex] = t;
}

void PrintRegTree(FILE *f, RegTree *t, int nNodes, char* rname, char* bname) 
{  
   RegNode **tVec;
   RNode *n;
   int i,j;

   tVec = (RegNode **) New(&gstack, nNodes*sizeof(RegNode *));
   --tVec;
   GetTreeVector(tVec, t->root);
   fprintf(f,"~r %s\n",ReWriteString(rname,NULL,DBL_QUOTE));
   
   /* print base classes */
   fprintf(f,"<BASECLASS>~b %s\n",ReWriteString(bname,NULL,DBL_QUOTE));
   for (i=1;i<=nNodes;i++) {
     n = GetRNode(tVec[i]);
     if (tVec[i]->numChild > 0) {
       fprintf(f, "<NODE> %d 2 ", n->nodeIndex);
       for (j=1;j<=tVec[i]->numChild;j++) {
	 n = GetRNode(tVec[i]->child[j]);
	 n = GetRNode(tVec[n->nodeIndex]);
	 fprintf(f, "%d ", n->nodeIndex);
	 t->numNodes++;
       }
       fprintf(f,"\n");
     } else {
       t->numTNodes++;
       /* always one base class */
       fprintf(f, "<TNODE> %d 1 %d\n",n->nodeIndex,t->numTNodes);
     }
   }
   
   tVec++;
   Dispose(&gstack, tVec);
}

void PrintBaseClass(FILE *f, RegTree *t, int nNodes, char *bname) 
{  
  CoList *c;
  RNode *n;
  int i, class=0;
  RegNode **tVec;
  
  tVec = (RegNode **) New(&gstack, nNodes*sizeof(RegNode *));
  --tVec;
  GetTreeVector(tVec, t->root);
  fprintf(f,"~b %s\n",ReWriteString(bname,NULL,DBL_QUOTE));
  fprintf(f,"<MMFIDMASK> %s\n",mmfIdMask);
  fprintf(f,"<PARAMETERS> MIXBASE\n");
   /* put <StreamInfo> for multiple streams system */
   if (hset->swidth[0]>1) {
      fprintf(f,"<STREAMINFO> ");
      WriteShort(f, hset->swidth, 1, FALSE);
      for (i=1; i<=hset->swidth[0]; i++)
         WriteShort(f, hset->swidth+i, 1, FALSE);
      fprintf(f,"\n");
   }
  fprintf(f,"<NUMCLASSES> %d\n",t->numTNodes);

  for (i=1;i<=nNodes;i++) {
    /* print out the information from the baseclasses */
    if (tVec[i]->numChild == 0) {
      class++;
      n = GetRNode(tVec[i]);
      fprintf(f,"<CLASS> %d {", class);
      for (c = n->list; c != NULL; c= c->next) {
	if (c!=n->list) fprintf(f,",");
	if (hset->swidth[0] == 1) /* single stream case */
	  fprintf(f,"%s.state[%d].mix[%d]", c->hmmName, c->state, c->mix);
	else 
	  fprintf(f,"%s.state[%d].stream[%d].mix[%d]", c->hmmName, c->state, c->stream, c->mix);

      }
      fprintf(f,"}\n");
    }
  }
  
  tVec++;
  Dispose(&gstack, tVec);
}

RegNode *FindBestTerminal (RegNode *t, double *score, RegNode *best) 
{
  RNode *n;
  double nodeScore;
  int i;

  if (t != NULL) {
    if (t->numChild>0) {
      for (i=1;i<=t->numChild;i++)
        best = FindBestTerminal(t->child[i], score, best);
    } else {
      n = GetRNode(t);
      nodeScore = n->clusterScore;
      if (!n->pureVecSize || !n->pureStream || ((*score < nodeScore) && (n->nComponents > n->vSize*3))) {
	*score = nodeScore;
	best = t;
      }
    }
  }
  return(best);
}


int BuildRegClusters (RegNode *rtree, const int nTerminals, Boolean nonSpeechNode) 
{
  RegNode *t;
  RNode *ch1, *ch2, *n, *r;   /* temporary children */  
  double score;
  int s, k, index, nStrTerminals[SMAX];
  IntSet strTerminals;

  /* prepare children nodes for clustering */
  r = GetRNode(rtree);
  ch1 = CreateRegTreeNode(NULL, r->vSize);
  ch2 = CreateRegTreeNode(NULL, r->vSize);

  /* initialise # of terminals in each stream */
  strTerminals = CreateSet(hset->swidth[0]);
  for (s=1; s<=hset->swidth[0]; s++)
    nStrTerminals[s] = 0;
  if (r->pureStream && r->pureVecSize && r->vSize>0)
    nStrTerminals[r->list->stream] = (nonSpeechNode) ? 2 : 1; 

  /* the artificial first split of speech/non-speech has been made */
  if (nonSpeechNode)
    index = 4;
  else
    index = 2;

  k = index/2;
  while (!IsFullSet(strTerminals)) { 
    score = 0.0;
    /* find the best (worst scoring) terminal to split 
       based on score and number of examples */
    t = FindBestTerminal(rtree, &score, rtree);

    /* check to see if there are no more nodes to split */
    if ((k > 1 || score == 0.0) && t == rtree) {
      printf("No more nodes to split..\n");
      fflush(stdout);
      break;
    }
    r = GetRNode(t);
      
    /* check to see whether this node should be split or not */
    if (r->pureStream && r->pureVecSize && r->vSize>0) {
      if (nStrTerminals[r->list->stream]>=nTerminals) {  /* no more splitting */
        r->clusterScore = 0.0;
        AddMember(strTerminals, r->list->stream);
        continue;
      }
         
      /* decrement nStrTerminals[s] */
      nStrTerminals[r->list->stream]--;
    }

    if (trace & T_BID) {
      printf("Splitting Node %d, score %e\n", r->nodeIndex, score);
         if (!r->pureStream) 
            printf("(Stream splitting)\n");
         else if (!r->pureVecSize)
            printf("(MSD splitting)\n");
         else
            printf("(Stream=%d, vSize=%d)\n", r->list->stream, r->vSize);
      fflush(stdout);
    }

    /* copy parent node distribution to children */
    ZeroVector(ch1->aveMean);  ZeroVector(ch2->aveMean);
    ZeroVector(ch1->aveCovar); ZeroVector(ch2->aveCovar);

    CopyRVector(r->aveMean,  ch1->aveMean,  r->vSize);
    CopyRVector(r->aveMean,  ch2->aveMean,  r->vSize);
    CopyRVector(r->aveCovar, ch1->aveCovar, r->vSize);
    CopyRVector(r->aveCovar, ch2->aveCovar, r->vSize);
    
    PerturbMean(ch1->aveMean, ch1->aveCovar, 0.2);
    PerturbMean(ch2->aveMean, ch2->aveCovar, -0.2);
    
    /* cluster children of r into ch1 and ch2 */
    ClusterChildren(r, ch1, ch2);
  
    /* Currently always build a binary regression tree */
    t->numChild = 2;
    t->child = (RegNode **)New(&tmpHeap,3*sizeof(RegNode *));
    /* now create new left child node and store the clusters */
    t->child[1] = (RegNode *) New(&tmpHeap, sizeof(RegNode));
    t->child[1]->info = (Ptr) CreateRegTreeNode(NULL, MaxVecSize(ch1->list));
    t->child[1]->numChild = 0;
    t->child[1]->child = NULL;

    t->child[2] = (RegNode *) New(&tmpHeap, sizeof(RegNode));
    t->child[2]->info = (Ptr) CreateRegTreeNode(NULL, MaxVecSize(ch2->list));
    t->child[2]->numChild = 0;
    t->child[2]->child = NULL;

    /* store the clusters */
    n = GetRNode(t->child[1]);
    n->list = ch1->list;
    n->pureVecSize = PureVecSize(n->list);
    n->pureStream  = PureStream (n->list);
    CalcClusterDistribution(n);
    n->clusterScore = CalcNodeScore(n);
    n->nComponents  = ch1->nComponents;
    n->nodeIndex    = index++;
    if (n->pureStream && n->pureVecSize && n->vSize>0)
      nStrTerminals[n->list->stream]++;

    n = GetRNode(t->child[2]);
    n->list = ch2->list;
    n->pureVecSize = PureVecSize(n->list);
    n->pureStream  = PureStream (n->list);
    CalcClusterDistribution(n);
    n->clusterScore = CalcNodeScore(n);
    n->nComponents  = ch2->nComponents;
    n->nodeIndex    = index++;
    if (n->pureStream && n->pureVecSize && n->vSize>0)
      nStrTerminals[n->list->stream]++;
      
    k++;
  }
   
  if (trace&T_BID) {
    for (s=1; s<=hset->swidth[0]; s++)
      printf("stream %d: #terminals=%d\n", s, nStrTerminals[s]);
    fflush(stdout);
  }
   
  /* free strTerminals */
  FreeSet(strTerminals);
      
  return(index-1);   
} 

void RegClassesCommand(void) 
{
  char ch;
  char buf[MAXSTRLEN], fname[MAXFNAMELEN], bname[MAXSTRLEN], tname[MAXSTRLEN];
  char macroname[MAXSTRLEN], fname2[MAXFNAMELEN];
  RegTree *regTree;
  char type = 'p';             /* type of items must be p (pdfs) */
  ILink ilist=NULL;            /* list of items that are non-speech sounds */
  int nTerminals, nNodes;   
  FILE *f;
  Boolean isPipe;

  nTerminals = ChkedInt("Maximum number of terminal nodes is 256",0,256);
  ChkedAlpha("RC regression trees identifier",buf);         
  if (trace & T_BID) {
    printf("\nRC %d %s\n Building regression tree with %d terminals (%d streams)\n",
           nTerminals, buf, nTerminals, hset->swidth[0]);
    printf("Creating regression class tree with ident %s.tree and baseclass %s.base\n",
	   buf, buf);
    fflush(stdout);
  }
  do {
    ch = GetCh(&source);
    if (ch=='\n') break;
  }
  while(ch!=EOF && isspace((int) ch));
  UnGetCh(ch,&source);
  if (ch != '\n')
    PItemList(&ilist,&type,hset,&source,NULL,0,0,(trace&T_ITM)?TRUE:FALSE);

  regTree = InitRegTree(hset, ilist);
  if (ilist != NULL) 
    nNodes = BuildRegClusters(regTree->root, nTerminals, TRUE);
  else
    nNodes = BuildRegClusters(regTree->root, nTerminals, FALSE);

  /* now store the baseclasses and regression classes */
  MakeFN(buf,newDir,"tree",fname);
  /* ar527: sometimes an output filter does exist
  if ((f=FOpen(fname,NoOFilter,&isPipe)) == NULL){
    HError(999,"RC: Cannot create output file %s",fname);
  }*/
  if ((f=FOpen(fname,HMMDefOFilter,&isPipe)) == NULL){
    HError(999,"RC: Cannot create output file %s",fname);
  }
  MakeFN(buf,NULL,"tree",tname);
  MakeFN(buf,NULL,"base",bname);
  PrintRegTree(f,regTree,nNodes,tname,bname);
  FClose(f,isPipe);  
  MakeFN(buf,newDir,"base",fname2);
  /* ar527: sometimes an output filter does exist
  if ((f=FOpen(fname2,NoOFilter,&isPipe)) == NULL){
    HError(999,"RC: Cannot create output file %s",bname);
  }*/
  if ((f=FOpen(fname2,HMMDefOFilter,&isPipe)) == NULL){
    HError(999,"RC: Cannot create output file %s",bname);
  }
  PrintBaseClass(f,regTree,nNodes,bname); 
  FClose(f,isPipe);  

  /* Create the macros so they can be stored with the models */
  LoadBaseClass(hset,NameOf(bname,macroname),fname2);  
  LoadRegTree(hset,NameOf(fname,macroname),fname);  

  /* 
     In practice the above commands are wasteful, but useful to check
     macronames. Turn off model storage instead.
  */
  saveHMMSet = FALSE;
}


/* ----------------- InputXForm Command -------------------- */

void InputXFormCommand()
{
   char fn[MAXFNAMELEN], macroname[MAXSTRLEN];
   InputXForm *xf;

   ChkedAlpha("IX input Xform file name",fn); 
   if (trace & T_BID) {
      printf("\nIX %s\n Setting HMMSet input XForm\n", fn);
      fflush(stdout);
   }
   xf = LoadInputXForm(hset, NameOf(fn,macroname), fn);
   xf->nUse++;
   hset->xf = xf;
}


/* ----------------- ParentXForm Command -------------------- */

void ParentXFormCommand()
{
   char fn[MAXFNAMELEN], macroname[MAXSTRLEN];
   AdaptXForm *xf;
   
   ChkedAlpha("PX parent Xform file name", fn); 
   if (trace & T_BID) {
      printf("\nPX %s\n Setting HMMSet parent XForm\n", fn);
      fflush(stdout);
   }
   xf = LoadOneXForm(hset,NameOf(fn,macroname),fn);
   SetParentXForm(hset, NULL, xf);
}

/* ----------------- AdaptXForm Command -------------------- */

typedef struct _StateMapping {
   char *l1;
   char *l2;
   struct _StateMapping *next;
   struct _StateMapping *prev;
} StateMapping;

typedef struct _StateMappingTable {
   struct _StateMapping *head;
   struct _StateMapping *tail;
} StateMappingTable;

void StateMappingTableInitialize(StateMappingTable * smt)
{
   smt->head = NULL;
   smt->tail = NULL;
}

void StateMappingTablePush(StateMappingTable * smt, char *l1, char *l2)
{
   StateMapping *sm;

   if (l1 == NULL || l2 == NULL)
      return;

   sm = New(&tmpHeap, sizeof(StateMapping));
   sm->l1 = CopyString(&tmpHeap, l1);
   sm->l2 = CopyString(&tmpHeap, l2);
   sm->next = NULL;
   sm->prev = NULL;

   if (smt->head == NULL) {
      sm->prev = NULL;
      smt->head = sm;
   } else {
      sm->prev = smt->tail->next;
      smt->tail->next = sm;
   }
   smt->tail = sm;
}

void StateMappingTableClear(StateMappingTable * smt)
{
   StateMapping *s1, *s2;

   s1 = smt->tail;
   while (s1 != NULL) {
      s2 = s1->prev;
      Dispose(&tmpHeap, s1);
      s1 = s2;
   }
   StateMappingTableInitialize(smt);
}

/* PerformMapAdapt: Apply xform to L2 model using state mapping table */
void PerformMapAdapt(HMMSet * hset_L1, HMMSet * hset_L2, StateMappingTable * smt)
{
   HMMScanState hss_L2;
   MixPDF *mp_L2 = NULL;
   MixPDF *mp_L1 = NULL;
   MLink ml_L2 = NULL;
   MLink ml_L1 = NULL;
   LabId id_L1 = NULL;
   StreamInfo *sti_L1 = NULL;
   int m;
   StateMapping *s;
   Boolean find;

   /* Attach XForm information into L2 model */
   AttachXFormInfo(hset_L2);

   NewHMMScan(hset_L2, &hss_L2);
   do {
      while (GoNextState(&hss_L2, TRUE)) {
         while (GoNextStream(&hss_L2, TRUE)) {
            ml_L2 = FindMacroStruct(hset_L2, 'p', hss_L2.sti);
            find = FALSE;
            for (s = smt->head; s != NULL; s = s->next) {
               if (strcmp(ml_L2->id->name, s->l2) == 0) {
                  id_L1 = GetLabId(s->l1, FALSE);
                  ml_L1 = FindMacroName(hset_L1, 'p', id_L1);
                  sti_L1 = (StreamInfo *) ml_L1->structure;

                  for (m = 1; m <= hss_L2.sti->nMix; m++) {
                     mp_L2 = hss_L2.sti->spdf.cpdf[m].mpdf;
                     mp_L1 = sti_L1->spdf.cpdf[m].mpdf;
                     if (VectorSize(mp_L2->mean) != VectorSize(mp_L1->mean))
                        HError(6611, "PerformMapAdapt: Only support same vector size");
                     if (VectorSize(mp_L2->mean) > 0) { /* if vector size = 0 then skip */
                        ApplyMapXForm(mp_L2, mp_L1, hset_L1->curXForm, TRUE);
                        hset_L2->ckind = hss_L2.mp->ckind;
                     }
                  }
                  find = TRUE;
                  break;
               }
            }
            if (find == FALSE)
               HError(-6611, "PerformMapAdapt: Can't find state mapping table for %s", ml_L2->id->name);
         }
      }
   }
   while (GoNextHMM(&hss_L2));
   EndHMMScan(&hss_L2);

   if (hset_L2->ckind == FULLC) {
      NewHMMScan(hset_L2, &hss_L2);
      while (GoNextMix(&hss_L2, FALSE)) {
         hss_L2.mp->ckind = FULLC;
      }
      EndHMMScan(&hss_L2);
   }
}

void AdaptXFormCommand()
{
   char fn[MAXFNAMELEN], macroname[MAXSTRLEN];
   AdaptXForm *xf;
   char c;
   char mapping[MAXFNAMELEN];
   char mmf_L2[MAXFNAMELEN];
   char list_L2[MAXFNAMELEN];
   char out_mmf[MAXFNAMELEN];
   HMMSet hset_L2;
   StateMappingTable smt;
   char name_L1[MAXSTRLEN];
   char name_L2[MAXSTRLEN];
   FILE *fp;
   Boolean is_pipe = FALSE;

   ChkedAlpha("AX adapt Xform file name", fn);

   /* check next args */
   do {
      c = GetCh(&source);
      if (c == '\n')
         break;
   }
   while (c != EOF && isspace(c));
   UnGetCh(c, &source);

   if (c == '\n' || c == EOF) {
      /* apply xform */
      if (trace & T_BID) {
         printf("\nAX %s\n Setting HMMSet adapt XForm\n", fn);
         fflush(stdout);
      }
      xf = LoadOneXForm(hset, NameOf(fn, macroname), fn);
      SetXForm(hset, &xfInfo, xf);
   } else {
      /* apply xform to hset and another one */
      ChkedAlpha("State mapping list", mapping);
      ChkedAlpha("L2 mmf", mmf_L2);
      ChkedAlpha("L2 model list", list_L2);
      ChkedAlpha("Output mmf", out_mmf);

      if (trace & T_BID) {
         printf("\nAX %s %s %s %s %s\n Apply XForm to HMMSet and another one\n", fn, mapping, mmf_L2, list_L2, out_mmf);
         fflush(stdout);
      }

      /* load state mapping table */
      StateMappingTableInitialize(&smt);
      fp = FOpen(mapping, NoFilter, &is_pipe);
      if (fp == NULL)
         HError(999, "AdaptXFormCommand: Can't open state mapping table");
      while (fscanf(fp, "%s%s", name_L2, name_L1) == 2)
         StateMappingTablePush(&smt, name_L1, name_L2);
      FClose(fp, is_pipe);

      /* L1 model */
      SetParmHMMSet(hset);
      if (hset->ckind == INVDIAGC)
         ConvDiagC(hset, TRUE);

      /* L2 model */
      CreateHMMSet(&hset_L2, &hmmHeap, TRUE);
      AddMMF(&hset_L2, mmf_L2);
      if (MakeHMMSet(&hset_L2, list_L2) < SUCCESS)
         HError(2321, "AdaptXFormCommand: MakeHMMSet failed");
      if (LoadHMMSet(&hset_L2, hmmDir, hmmExt) < SUCCESS)
         HError(2321, "AdaptXFormCommand: LoadHMMSet failed");
      SetParmHMMSet(&hset_L2);
      if (hset_L2.ckind == INVDIAGC)
         ConvDiagC(&hset_L2, TRUE);

      /* apply xform to L1 model */
      xf = LoadOneXForm(hset, NameOf(fn, macroname), fn);
      SetXForm(hset, &xfInfo, xf);

      /* apply xform to L2 model and save HMMSet */
      PerformMapAdapt(hset, &hset_L2, &smt);

      /* save L2 model */
      ConvDiagC(&hset_L2, TRUE);
      SaveInOneFile(&hset_L2, out_mmf);
      if (SaveHMMSet(&hset_L2, NULL, NULL, NULL, inBinary) < SUCCESS)
         HError(2611, "PerformMapAdapt: SaveHMMSet failed");

      /* free state mapping table */
      StateMappingTableClear(&smt);
      saveHMMSet = FALSE;
   }
}

/* ----------------- ReOrderFeatures Command -------------------- */

void ReOrderFeaturesCommand()
{
   int i,j,vSize,nr,nc;
   char *mac="varFloor1"; /* single stream assumed */
   MLink m;
   LabId id;
   Vector tmpm,tmpv,mean,var;
   Matrix tmpxf,xform;
   IntVec frv;
   HMMScanState hss;

   vSize = hset->vecSize;
   frv = CreateIntVec(&gstack,vSize);
   for (i=1; i<=vSize; i++)
      frv[i] = ChkedInt("Feature index",1,vSize);
   tmpm = CreateVector(&gstack,vSize);
   tmpv = CreateVector(&gstack,vSize);
   NewHMMScan(hset,&hss);
   if (!(hss.isCont || (hss.hset->hsKind == TIEDHS))) {
         HError(2690,"ReOrderFeaturesCommand: only continuous feature systems supported");
   }
   while(GoNextMix(&hss,FALSE)) {
      if (hss.mp->ckind != DIAGC)
         HError(2690,"ReOrderFeaturesCommand: CovKind not DIAGC");
      mean = hss.mp->mean;
      var = hss.mp->cov.var;
      for (i=1; i<=vSize; i++) {
	 tmpm[i] = mean[frv[i]];
	 tmpv[i] = var[frv[i]];
      }
      CopyVector(tmpm,mean);
      CopyVector(tmpv,var);
   }
   EndHMMScan(&hss);
   id = GetLabId(mac,FALSE);
   if (id != NULL  && (m=FindMacroName(hset,'v',id)) != NULL) {
      var = (Vector)m->structure;
      for (i=1; i<=vSize; i++)
	 tmpv[i] = var[frv[i]];
      CopyVector(tmpv,var);
   } else
      HError(2623,"ReOrderFeaturesCommand: variance floor macro %s not found",mac);
   if (hset->xf != NULL) {
      xform = hset->xf->xform->xform[1];
      nc = NumCols(xform);
      nr = hset->xf->xform->vecSize;
      if (nr != vSize)
	 HError(2623,"ReOrderFeaturesCommand: Num rows %d in InputXForm does not match the vecsize %d",nr,vSize);
      tmpxf = CreateMatrix(&gstack,nr,nc);
      for (i=1; i<=nr; i++)
	 for (j=1; j<=nc; j++)
	    tmpxf[i][j] = xform[frv[i]][j];
      CopyMatrix(tmpxf,xform);
      if (trace & T_BID) {
	 printf(" RF: Transform re-ordered according to %d, %d, %d, ...\n",frv[1],frv[2],frv[3]);
	 fflush(stdout);
      }
      FreeMatrix(&gstack, tmpxf);
   }
   if (trace & T_BID) {
      printf(" RF: Parameters re-ordered according to %d, %d, %d, ...\n",frv[1],frv[2],frv[3]);
      fflush(stdout);
   }
   
   FreeVector(&gstack, tmpv);
   FreeVector(&gstack, tmpm);
   FreeIntVec(&gstack,frv);
}

/* ----------------- JoinModel Command -------------------- */

void JoinModelByState (ILink ilist, HMMSet *srcSet, HMMSet *dstSet)
{
   HLink hmm;
   ILink p;
   MLink mSrc, mDst;
   StateElem *se;
   StateInfo *si;
   int j;
   int state, N;

   hmm = ilist->owner;
   N = hmm->numStates;

   for (j=2,state=0; j<hmm->numStates; j++)
      if (hmm->svec+j==(StateElem *)ilist->item) {
         state = j;
         break;
      }

   if (state==0)
      HError(2663,"JoinModelByState: cannot find state index");

   for (p=ilist; p!=NULL; p=p->next) {
      se = (StateElem *)p->item;
      si = se->info;

      mSrc = FindMacroStruct(srcSet,'s',si);
      if (mSrc!=NULL) {  /* shared state */
         mDst = FindMacroName(dstSet, 's', mSrc->id);

         if (mDst==NULL) {
            mDst = NewMacro(dstSet, mSrc->fidx, 's', mSrc->id, mSrc->structure);
            si = (StateInfo *) mDst->structure;

            if (trace&T_DET) {
               printf(" Adding macro(state=%d) %s\n", state, mSrc->id->name);
               fflush(stdout);
            }
         }
         else {  /* already existed, update si */
            si = (StateInfo *) mDst->structure;
         }
      }
      else {  /* not a shared state */
         if (trace&T_DET) {
            mSrc = FindMacroStruct(srcSet,'h',p->owner);
            HError(-2663,"JoinModelByState: not a shared stream (state=%d) in macro %s\n", state, mSrc->id->name);
         }
      }

      /* then handle state itself */
      mSrc = FindMacroStruct(srcSet, 'h', p->owner);
      mDst = FindMacroName  (dstSet, 'h', mSrc->id);

      (((HLink)mDst->structure)->svec + state)->info = si;
   }

   return;
}

void JoinModelByStream (ILink ilist, HMMSet *srcSet, HMMSet *dstSet)
{
   HLink hmm;
   ILink p;
   MLink mSrc, mDst;
   StateElem *se;
   StateInfo *si;
   StreamElem *ste;
   StreamInfo *sti;
   int j, s;
   int state, N;

   hmm = ilist->owner;
   N = hmm->numStates;

   state = 0;
   if (ilist->owner==ilist->item) {  /* never go here */
      HError(-2663,"JoinModelByStream: Join Model on model level is not supported.\n");
      state = -1;
      return;
   }
   else {
      for (j=2,state=0; j<hmm->numStates; j++)
         if (hmm->svec+j==(StateElem *)ilist->item) {
            state = j;
            break;
         }
   }

   if (state==0)
      HError(2663,"JoinModelByStream: cannot find state index");

   for (p=ilist; p!=NULL; p=p->next) {
      se = (StateElem *) p->item;
      si = se->info;

      for(s=1; s<=srcSet->swidth[0]; s++) {
         if (streams.set[s]) {
            /* Merge this stream from srcSet to dstSet */
            ste = si->pdf+s;
            sti = ste->info;

            /* First we handle shared macro */
            mSrc = FindMacroStruct(srcSet,'p',sti);
            if (mSrc!=NULL) {  /* shared stream */
               mDst = FindMacroName(dstSet, 'p', mSrc->id);

               if (mDst==NULL) {
                  mDst = NewMacro(dstSet, mSrc->fidx, 'p', mSrc->id, mSrc->structure);
                  sti = (StreamInfo *)mDst->structure;
                  if (trace&T_DET) {
                     printf(" Adding macro (state=%d, stream=%d) %s\n", state, s, mSrc->id->name);
                     fflush(stdout);
                  }
               }
               else {  /* already existed, update sti */
                  sti = (StreamInfo *)mDst->structure;
               }
            }
            else {
               /* Not a shared stream */
               if (trace & T_DET) {
                  mSrc = FindMacroStruct(srcSet,'h',p->owner);
                  HError(-2663,"JoinModelByStream: not a shared stream(state=%d, stream=%d) in macro %s\n", state, s, mSrc->id->name);
               }
            }

            /* Then we handle stream itself */
            mSrc = FindMacroStruct(srcSet, 'h', p->owner);
            mDst = FindMacroName  (dstSet, 'h', mSrc->id);

            (((((HLink)mDst->structure)->svec + state)->info)->pdf+s)->info = sti;
         }
      }
   }

   return;
}

void JoinModelCommand (void)
{
   HMMSet tmpSet;
   HMMSet * tmpset;
   char buf[MAXSTRLEN];

   ILink ilist=NULL;  /* list of items to tie */
   char type=' ';     /* type of items to tie */
   char *pattern,*p;  /* pattern of items to use */
   int s;

   ChkedAlpha("JM MMF file name", buf);

   CreateHMMSet(&tmpSet,&hmmHeap,TRUE);
   tmpset = &tmpSet;
   if(MakeHMMSet(&tmpSet,hmmListFn)<SUCCESS)
      HError(2628,"JoinModelCommand: MakeHMMSet failed");
   AddMMF(&tmpSet,buf);
   if(LoadHMMSet(&tmpSet,NULL,NULL)<SUCCESS)
      HError(2628,"JoinModelCommand: LoadHMMSet failed");

   if (hset->swidth[0]!=tmpset->swidth[0])
      HError(2630,"JoinModelCommand: num streams incompatible");
   if (hset->pkind!=tmpset->pkind)
      HError(2630,"JoinModelCommand: different parameter kinds");
   if (hset->numLogHMM!=tmpset->numLogHMM)
      HError(2630,"JoinModelCommand: different number of logical HMMs");
   if (hset->numPhyHMM!=tmpset->numPhyHMM)
      HError(2630,"JoinModelCommand: different number of physical HMMs");
   ClearSet(streams);
   pattern = PItemList(&ilist,&type,tmpset,&source,&streams,0,0,(trace&T_ITM)?TRUE:FALSE);

   if (trace&T_BID) {
      printf("JM %s %s\n", buf, pattern);
      fflush(stdout);
   }

   /* extract specified model pattern */
   if (type!='h') {
      if ((p=strchr(pattern, '.'))==NULL)
         HError(9999, "JoinModelCommand: Specified pattern does not include any dot");
      *p = '}'; *(p+1) = '\0';
   }

   if (ilist==NULL) {
      HError(-2631,"JoinModelCommand: No items to cluster\n");
      return;
   }

   if (type=='h' || type=='s')
      for (s=1;s<=hset->swidth[0];s++)
         streams.set[s] = TRUE;

   switch(type) {
   case 's':
      JoinModelByState(ilist, tmpset, hset); break;
   case 'p':
      JoinModelByStream(ilist, tmpset, hset); break;
   case 'h':
   default:
      HError(2663,"JoinModelCommand: type %c is not supported", type);
      break;
   }

   if (CheckHSet(hset)<SUCCESS){
      ResetHMMSet(hset);
      HRError(7031,"LoadHMMSet: Invalid HMM data");
      return;
   }

   SetIndexes(hset);
   SetCovKindUsage(hset);
   SetParmHMMSet(hset);

   return;
}

/* ----------------- DR - Convert decision trees to a regression tree for adaptation --------------- */

void AssignHMMsToNodes (Tree *tree)
{
   char buf[MAXSTRLEN];
   int h;
   MLink q;
   HLink hmm;
   LabId tid=NULL;
   Node *node;
   Boolean isYes=FALSE;

   for (h=0; h<MACHASHSIZE; h++)
      for (q=hset->mtab[h]; q!=NULL; q=q->next)
         if (q->type=='h') {
            hmm=(HLink) q->structure;

            /* First find Tree to use */
            strcpy(buf,q->id->name);
            if (!usePattern) {
               if (strchr(buf,'*')!=NULL || strchr(buf,'?')!=NULL)
                  HError(9999,"AssignHMMsToNodes: baseId %s includes pattern character", buf);
               TriStrip(buf); 
               MapTreeName(buf);
               tid = GetLabId(buf,FALSE);
            }
            
            if (((usePattern && IPatMatch(buf, tree->patList)) 
              || singleTree 
              || (!singleTree && !usePattern && tree->baseId == tid))) {  /* check this model matches to this tree */    
     
               /* traverse tree */
               node = tree->root;
               while (node->yes != NULL) {
                  isYes = QMatch(q->id->name,node->quest);
                  node = (isYes) ? node->yes : node->no;
               }

               /* originally node->qlist is used to store active questions in a node, 
                * but here we use it to store HMM/macro list which is assigned to the node */
               AddItem(hmm,q,&node->qlist);
            }
         }
   
   return;                    
}

/* NumSpaces: return number of spaces in the s-th stream */
int NumSpaces (IMatrix vecSize, const int s)
{
   int j;
   
   for (j=1; j<=maxMixes && vecSize[s][j]>=0; j++);
   
   return j-1;
}     

/* InitDec2Reg: initialization for decision trees to a regression tree conversion */
void InitDec2Reg (ILink ***st, IMatrix *vs)
{
   int j, m, s;
   int vSize, state;
   StreamInfo *sti;
   HLink hmm;
   Tree *tree;
   Node *node;
   ILink p, strTree[SMAX], **spaceTree;
   IMatrix vecSize;
   HMMScanState hss;
   Boolean added;
   
   const int maxM = maxMixes;
   
   /* initialize strTree and spaceTree */
   spaceTree = (ILink **) New(&tmpHeap, hset->swidth[0]*sizeof(ILink *));
   spaceTree--;
   for (s=1; s<=hset->swidth[0]; s++) {
      spaceTree[s] = (ILink *) New(&tmpHeap, maxM*sizeof(ILink));
      spaceTree[s]--;
      for (m=1; m<=maxM; m++) 
         spaceTree[s][m] = NULL;
      strTree[s] = NULL;
   }
   
   /* set vector size of spaces in each stream */
   vecSize = CreateIMatrix(&tmpHeap, hset->swidth[0], maxM);
   for (s=1; s<=hset->swidth[0]; s++) 
      for (m=1; m<=maxM; m++)
         vecSize[s][m] = -1;  
   
   NewHMMScan(hset, &hss);
   while (GoNextMix(&hss,FALSE)) {
      vSize = VectorSize(hss.mp->mean);
      for (m=1; m<=maxM; m++) {
         if (vSize==vecSize[hss.s][m])
            break;
         if (vecSize[hss.s][m]==-1) {
            vecSize[hss.s][m] = vSize;
            break;
         }
      }
   }
   EndHMMScan(&hss);
   
   /* assign HMM lists to terminal nodes of decision trees 
    * and cluster trees to streams */   
   for (tree=treeList; tree!=NULL; tree=tree->next) {
      AssignHMMsToNodes(tree);
      for (s=1; s<=hset->swidth[0]; s++) 
         if (tree->streams.set[s])
            AddItem(NULL, tree, &strTree[s]);
   }
   
   /* compose tree list for each space of each stream */      
   for (s=1; s<=hset->swidth[0]; s++) {
      for (j=1; j<=maxM && vecSize[s][j]!=-1; j++) {
         for (p=strTree[s]; p!=NULL; p=p->next) {
            tree = (Tree *) p->item;
            added = FALSE;
            for (node=tree->leaf; node!=NULL; node=node->next) {
               hmm = node->qlist->owner;
               
               for (state=((tree->state>0)?tree->state:2); state<=((tree->state>0)?tree->state:hmm->numStates-1); state++) {
                  sti = hmm->svec[state].info->pdf[s].info;
            
                  for (m=1; m<=sti->nMix; m++) {
                     if (VectorSize(sti->spdf.cpdf[m].mpdf->mean)==vecSize[s][j]) {
                        AddItem(NULL, tree, &spaceTree[s][j]);
                        added = TRUE;
                        break;
                     }
                  }   
                  if (added) break;
               }
               if (added) break;
            }
         }
      }
   }
   
   /* free itemlist for stream tree lists */
   for (s=1; s<=hset->swidth[0]; s++) 
      FreeItems(&strTree[s]);
   
   if (trace & T_BID) {
      for (s=1; s<=hset->swidth[0]; s++) {
         printf(" Stream %d has %d space(s)\n", s, NumSpaces(vecSize, s));
         for (j=1; j<=NumSpaces(vecSize, s); j++) {
            printf("  space %d: vSize=%d #trees=%d\n",j,vecSize[s][j],NumItems(spaceTree[s][j]));
         }
         printf("\n");
      }
   }
   
   *vs = vecSize;
   *st = spaceTree;
                   
   return;
}

/* ResetDec2Reg: free objects used in Dec2Reg */
void ResetDec2Reg (ILink ***st, IMatrix *vs)
{
   int m, s;
   Tree *tree;
   Node *node;
   ILink **spaceTree = *st;
   IMatrix vecSize = *vs;
   
   for (tree=treeList; tree!=NULL; tree=tree->next)
      for (node=tree->leaf; node!=NULL; node=node->next)
         FreeItems(&node->qlist);
         
   FreeIMatrix(&tmpHeap, vecSize);
   
   for (s=hset->swidth[0]; s>0; s--) {
      for (m=1; m<=maxMixes; m++)
         FreeItems(&spaceTree[s][m]);
      spaceTree[s]++;
      Dispose(&tmpHeap, spaceTree[s]);
   }
   spaceTree++;
   Dispose(&tmpHeap, spaceTree);
                   
   return;
}

void PrintNodeBaseClass (FILE *f, ILink ilist, const int state, const int stream, const int class, const int vSize)
{
   int m,j;
   ILink c;
   MLink macro;
   HLink hmm;
   StreamInfo *sti; 
   Boolean first=TRUE,firstmix;
   
   fprintf(f,"<CLASS> %d {", class);
   
   for (c=ilist; c!=NULL; c=c->next) {
      macro = (MLink) c->item;
      hmm = c->owner;
      for (j=((state>0)?state:2); j<=((state>0)?state:hmm->numStates-1); j++) {
         sti = hmm->svec[j].info->pdf[stream].info;
         firstmix=TRUE;
         for (m=1; m<=sti->nMix; m++) {
            if (VectorSize(sti->spdf.cpdf[m].mpdf->mean)==vSize) {
               if (!IsSeen(sti->spdf.cpdf[m].mpdf->nUse)) {
                  /* print baseclass */
                  if (firstmix && !first) fprintf(f,",");
                  if (hset->swidth[0] == 1) { /* single stream case */
                     if (firstmix) {
                        fprintf(f,"%s.state[%d].mix[%d", macro->id->name, j, m);
                        firstmix=FALSE;
                     }
                     else
                        fprintf(f,",%d", m);
                  }
                  else {
                     if (firstmix) { 
                        fprintf(f,"%s.state[%d].stream[%d].mix[%d", macro->id->name, j, stream, m);
                        firstmix=FALSE;
                     }
                     else
                        fprintf(f,",%d", m);
                  }
                  first = FALSE;
                  Touch(&sti->spdf.cpdf[m].mpdf->nUse);
               }
            }
         }
         if (!firstmix)
            fprintf(f,"]");
      }
   }
   fprintf(f,"}\n");
   
   return;
}

CLink GetChildCList (Node *node)
{
   CLink p;
   
   /* yes node */
   if (node->yes!=NULL) {
      if (node->clist==NULL)
         node->clist = GetChildCList(node->yes);
      else {
         for (p=node->clist; p->next!=NULL; p=p->next);
         p->next = GetChildCList(node->yes);
      }
   }
   /* no node */
   if (node->no!=NULL) {
      if (node->clist==NULL)
         node->clist = GetChildCList(node->no);
      else {
         for (p=node->clist; p->next!=NULL; p=p->next);
         p->next = GetChildCList(node->no);
      }
   }
   
   return node->clist;
}

void ShrinkTree(Tree *tree, Node *node, int *snum, const float threshold)
{
   if (node->yes!=NULL && node->occ>threshold && node->yes->occ>0.0 && node->no->occ>0.0) {
      node->snum = *snum; (*snum)++;
      ShrinkTree(tree, node->yes, snum, threshold);
      ShrinkTree(tree, node->no,  snum, threshold);
   }
   else {
      /* link leaf list */
      node->next=tree->leaf;
      if (tree->leaf) tree->leaf->prev=node;
      node->prev=NULL;
      tree->leaf=node;
         
      /* initialize variables */
      node->clist = GetChildCList(node);
      node->yes = node->no = NULL;
      node->quest = NULL;
      
      /* update # of clusters count */
      numTreeClust++;
   }
   
   return;
}

void DecTrees2RegTreeCommand(void)
{
   char buf[MAXSTRLEN], fname1[MAXFNAMELEN], fname2[MAXFNAMELEN], tname[MAXSTRLEN], bname[MAXSTRLEN], macroname[MAXSTRLEN];
   int i, j, s, snum;
   int bias, tindex, tnodeindex, tmpindex, nclass;
   FILE *f1=NULL, *f2=NULL;
   Boolean isPipe1, isPipe2, zeroOcc;
   Tree *tree;
   Node *node, *leaf, **array;
   ILink p, **spaceTree;
   IMatrix vecSize;
   
   ChkedAlpha("DR regression tree identifier", buf);
   
   if (trace & T_BID) {
      printf("\nDR: Convert decision trees to a regression tree\n");
      fflush(stdout);
   }

   if (treeList==NULL)
      HError(2655,"DecTrees2RegTreeCommand: No decision trees loaded - use LT command first");
   
   if (shrinkOccThresh!=NULL) {
      if (!occStatsLoaded)
         HError(2655,"DecTrees2RegTreeCommand: Shrinking tree requires stats - use LS command");
   
      for (tree=treeList; tree!=NULL; tree=tree->next) {
         /* get stream index */
         for (s=1; s<=hset->swidth[0]; s++) 
            if (tree->streams.set[s]) break;

         /* update occ counts of all nodes */
         for (leaf=tree->leaf,zeroOcc=TRUE; leaf!=NULL; leaf=leaf->next) {
            if (tree->nLeaves > 1) {
            for (node=leaf; node->parent!=NULL; node=node->parent) {
               node->parent->occ += leaf->occ;
               if (leaf->occ>0.0)
                  zeroOcc = FALSE;
            }
            } else {
               if (leaf->occ>0.0)
                  zeroOcc = FALSE;
            }
         }
         if (zeroOcc)
            HError(9999,"DecTree2RegTree: Load stats file before loading tree files.");
      
         /* shrink tree */
         tree->leaf=NULL; numTreeClust=0; snum=0;  
         ShrinkTree(tree,tree->root,&snum,shrinkOccThresh[s]);
         if (trace & T_BID) {
            if (tree->state>0)
               printf(" Tree %s.state[%d]",ReWriteString(tree->baseId->name,NULL,ESCAPE_CHAR),tree->state);
            else
               printf(" Tree %s",tree->baseId->name);
            if (tree->nActiveStr==hset->swidth[0])
               printf("\n");
            else {
               printf(".stream[");         /* output active stream of current tree */
               for (i=1,j=1; j<=hset->swidth[0]; j++)
                  if (tree->streams.set[j])
                     printf("%d%s",j,(i++<tree->nActiveStr)?",":"]\n");
            }
            printf("  %d leaf nodes were pruned into %d nodes (occ thresh=%.1f)\n\n",
                   tree->nLeaves, numTreeClust, shrinkOccThresh[s]);
            fflush(stdout);
         }
         tree->nLeaves=numTreeClust; tree->size=snum;
      }
   }
   
   /* initialization */
   InitDec2Reg(&spaceTree, &vecSize);
   
   /* now store the baseclasses and regression classes */
   MakeFN(buf,newDir,"tree",fname1);
   if ((f1=FOpen(fname1,NoOFilter,&isPipe1)) == NULL) {
      HError(999,"DecTrees2RegTreeCommand: Cannot create output file %s",fname1);
   }
   MakeFN(buf,newDir,"base",fname2);
   if ((f2=FOpen(fname2,NoOFilter,&isPipe2)) == NULL) {
      HError(999,"DecTrees2RegTreeCommand: Cannot create output file %s",fname2);
   }
   MakeFN(buf,NULL,"tree",tname);
   MakeFN(buf,NULL,"base",bname);
   
   /* print baseclass and tree names to regression tree file */
   fprintf(f1,"~r %s\n",ReWriteString(tname,NULL,DBL_QUOTE));
   fprintf(f1,"<BASECLASS>~b %s\n",ReWriteString(bname,NULL,DBL_QUOTE));
   
   /* print baseclass macro name and header to regression class file */
   fprintf(f2,"~b %s\n",ReWriteString(bname,NULL,DBL_QUOTE));
   fprintf(f2,"<MMFIDMASK> %s\n",mmfIdMask);
   fprintf(f2,"<PARAMETERS> MIXBASE\n");
   
   /* print <StreamInfo> if multiple stream */
   if (hset->swidth[0]>1) {
      fprintf(f2,"<STREAMINFO> ");
      WriteShort(f2, hset->swidth, 1, FALSE);
      for (i=1; i<=hset->swidth[0]; i++)
         WriteShort(f2, hset->swidth+i, 1, FALSE);
      fprintf(f2,"\n");
   }
   
   
   /* first output root node of the regression tree (separate streams) */
   bias=1; tmpindex=2;
   fprintf(f1,"<NODE> %d %d", bias++, hset->swidth[0]);
   for (s=1; s<=hset->swidth[0]; s++)
      fprintf(f1, " %d", tmpindex++);
   fprintf(f1, "\n");
   fflush(f1);
   
   /* secondly output nodes for each stream (separate MSD spaces) */
   tmpindex = bias + hset->swidth[0];
   for (s=1; s<=hset->swidth[0]; s++) {
      fprintf(f1,"<NODE> %d %d", bias++, NumSpaces(vecSize, s));
      for (i=1; i<=NumSpaces(vecSize, s); i++)
         fprintf(f1," %d", tmpindex++);
      fprintf(f1,"\n");
   }     
   fflush(f1);
   
   /* thirdly output nodes for each space (separate decision trees) 
    * and number of baseclasses (leaf nodes) */
   tmpindex = bias;
   nclass = 0;
   for (s=1; s<=hset->swidth[0]; s++)
      tmpindex += NumSpaces(vecSize, s);
   for (s=1; s<=hset->swidth[0]; s++) {
      for (i=1; i<=NumSpaces(vecSize, s); i++) {
         fprintf(f1,"<NODE> %d %d", bias++, NumItems(spaceTree[s][i]));
         for (p=spaceTree[s][i]; p!=NULL; p=p->next) {
            tree = (Tree *) p->item;
            fprintf(f1," %d", tmpindex);
            tmpindex += tree->size + tree->nLeaves;
            nclass += tree->nLeaves;
         }
         fprintf(f1,"\n");
      }
   }
   fprintf(f2,"<NUMCLASSES> %d\n",nclass);
   fflush(f1);
   fflush(f2);
   
   /* finally output each decision tree */
   tindex=1;
   for (s=1; s<=hset->swidth[0]; s++) {  /* process multiple streams */
      for (j=1; j<=NumSpaces(vecSize, s); j++) {
         for (p=spaceTree[s][j]; p!=NULL; p=p->next) {
            tree = (Tree *)p->item; 
            if (tree->size==0) {
               fprintf(f1, "<TNODE> %d 1 %d\n", bias++, tindex);
               PrintNodeBaseClass(f2, tree->root->qlist, tree->state, s, tindex, vecSize[s][j]);
               tindex++;
            }
            else {
               array=(Node**) New(&tmpHeap,sizeof(Node*)*tree->size);
               for (i=0; i<tree->size; i++)  array[i]=NULL;
               DownTree(tree->root,array);
         
               tnodeindex = bias+tree->size;
               tmpindex = tindex;
               for (i=0; i<tree->size; i++) {
                  node=array[i];
                  if (node==NULL) 
                     HError(2695,"DecTrees2RegTreeCommand: Cannot find node %d",i);
                  if (node->yes==NULL || node->no==NULL) 
                     HError(2695,"DecTrees2RegTreeCommand: Node %d has no children",i);
               
                  fprintf(f1,"<NODE> %d 2", bias+i);
               
                  /* no node */
                  if (node->no->yes)
                     fprintf(f1," %d", bias+node->no->snum);
                  else {
                     PrintNodeBaseClass(f2, node->no->qlist, tree->state, s, tmpindex++, vecSize[s][j]);
                     fprintf(f1," %d", tnodeindex++);
                  }
                  /* yes node */
                  if (node->yes->yes)
                     fprintf(f1," %d\n", bias+node->yes->snum);
                  else {
                     PrintNodeBaseClass(f2, node->yes->qlist, tree->state, s, tmpindex++, vecSize[s][j]);
                     fprintf(f1," %d\n", tnodeindex++);
                  }
               }
         
               /* output terminal nodes */
               for (i=bias+tree->size; i<tnodeindex; i++)
               fprintf(f1, "<TNODE> %d 1 %d\n", i, tindex++);  
               bias += tree->size+tree->nLeaves;
            
               Dispose(&tmpHeap,array);
            }
            fflush(f1);
            fflush(f2);
         }
      }
   }
   ClearSeenFlags(hset,CLR_ALL);
   
   ResetDec2Reg(&spaceTree, &vecSize);
   
   FClose(f2,isPipe2);  
   FClose(f1,isPipe1);  
   
   if (trace & T_BID) {
      printf(" Conversion finished.\n");
      printf(" A regression tree with %d nodes (#terminal nodes=%d) and its baseclass were generated.\n", bias, nclass);
      fflush(stdout);
   }
    
   /* Create the macros so they can be stored with the models */
   LoadBaseClass(hset,NameOf(bname,macroname),fname2);  
   LoadRegTree(hset,NameOf(tname,macroname),fname1);  
}

/* -------------------- Comment Command -------------------- */

void CommentCommand()
{
   if (trace > 0) {
      char comment[MAXSTRLEN];
      
      /* read this line from source */
      ReadLine(&source, comment);
      
      /* output comment */
      fflush(stdout);
      printf("\n// %s",comment);
      fflush(stdout);
   }
   else
      /* skip this line */
      SkipLine(&source);
}

/* -------------------- Initialisation --------------------- */
 
void Initialise(char *hmmListFn)
{
  
   CreateHeap(&questHeap,"Question Heap",MSTAK,1,1.0,8000,200000);
   CreateHeap(&tmpHeap,"Temporary Heap",MSTAK,1,1.0,40000,1600000);


   if(MakeHMMSet(&hSet,hmmListFn)<SUCCESS)
      HError(2628,"Initialise: MakeHMMSet failed");
   if (noAlias) ZapAliases(); 
   if(LoadHMMSet(&hSet,hmmDir,hmmExt)<SUCCESS)
      HError(2628,"Initialise: LoadHMMSet failed");

   maxStates = MaxStatesInSet(hset);
   maxMixes = MaxMixInSet(hset);

   streams = CreateSet(SMAX);

   if (trace != 0) {
      printf("HHEd\n");
      printf(" %d/%d Models Loaded [%d states max, %d mixes max]\n",
             hset->numLogHMM,hset->numPhyHMM,maxStates,maxMixes);
      fflush(stdout);
   }

   SetVFloor(hset, vf, minVar);
}

/* -------------------- Top Level of Editing ---------------- */


static int  nCmds = 52;

static char *cmdmap[] = {"AT","RT","SS","CL","CM","CO","CT","JO","MU","TI","UF","NC","SM",
                         "TC","UT","MT","SH","SU","SW","SK",
                         "RC",
                         "RO","RM","RN","RP",
                         "LS","QS","TB","TR","AU","GQ","MD","ST","LT",
                         "MM","DP","HK","FC","DV","FA","FV","IX","PX","AX","PS","PR","DR","DM","IT","JM","//","" };

typedef enum           { AT=1,RT , SS , CL , CM , CO , CT , JO , MU , TI , UF , NC , SM ,
                         TC , UT , MT , SH , SU , SW , SK ,
                         RC ,
                         RO , RM , RN , RP ,
                         LS , QS , TB , TR , AU , GQ , MD , ST , LT ,
                         MM , DP , HK , FC , DV , FA , FV , IX , PX , AX , PS , PR , DR , DM , IT , JM , XX }
cmdNum;

/* CmdIndex: return index 1..N of given command */
int CmdIndex(char *s)
{
   int i;
   
   for (i=1; i<=nCmds; i++)
      if (strcmp(cmdmap[i-1],s) == 0) return i;
   return 0;
}

/* DoEdit: edit the loaded HMM Defs using given command file */
void DoEdit(char * editFn)
{
   int prevCommand = 0,c1,c2;
   char cmds[] = "  ";
  
   /* Open the Edit Script File */
   if(InitSource(editFn,&source,NoFilter)<SUCCESS)
      HError(2610,"DoEdit: Can't open file %s", editFn);
      
   /* Apply each Command */
   for (;;) {
      SkipWhiteSpace(&source);
      if( (c1 = GetCh(&source)) == EOF) break;
      if( (c2 = GetCh(&source)) == EOF) break;
      cmds[0] = c1; cmds[1] = c2;
      if (!isspace(GetCh(&source))) 
         HError(2650,"DoEdit: White space expected after command %s",cmds);
      thisCommand = CmdIndex(cmds);
      if ((thisCommand != lastCommand && prevCommand != TR &&
           thisCommand != SH && thisCommand != ST) || thisCommand == TR)
         trace = cmdTrace;
      switch (thisCommand) {
      case AT: EditTransMat(TRUE); break;
      case RT: EditTransMat(FALSE); break;
      case SS: SplitStreamCommand(FALSE); break;
      case CL: CloneCommand(); break;
      case CM: ConvertModelsCommand(); break;
      case CO: CompactCommand(); break;
      case CT: ConvertTreesCommand(); break;
      case JO: JoinSizeCommand(); break;
      case MU: MixUpCommand(); break;
      case MD: MixDownCommand(); break;
      case TI: TieCommand(); break;
      case NC: ClusterCommand(TRUE); break;
      case SM: StateMappingCommand(); break;
      case TC: ClusterCommand(FALSE); break;
      case UT: UntieCommand(); break;
      case MT: MakeTriCommand(); break;
      case SH: ShowHMMSet(); break;
      case SU: SplitStreamCommand(TRUE); break;
      case SW: SetStreamWidthCommand(); break;
      case SK: SetSampKindCommand(); break;
      case HK: SetHSetKindCommand(); break;
      case RC: RegClassesCommand(); break;
      case RM: RemMeansCommand(); break;
      case RN: RenameHMMSetIdCommand(); break;
      case RP: ReOrderFeaturesCommand(); break;
      case RO: RemOutliersCommand(); break;
      case LS: LoadStatsCommand(); break;
      case QS: QuestionCommand(); break;
      case TB: TreeBuildCommand(); break;
      case TR: SetTraceCommand(); break;
      case AU: AddUnseenCommand(); break;
      case ST: ShowTreesCommand(); break;
      case LT: LoadTreesCommand(); break;
      case MM: MakeIntoMacrosCommand(); break;
      case DM: DeleteMacroCommand(); break;
      case DP: DuplicateCommand(); break;
      case IX: InputXFormCommand(); break;
      case PX: ParentXFormCommand(); break;
      case AX: AdaptXFormCommand(); break;
      case UF: UseCommand(); break;
      case FA: FloorAverageCommand(); break;
      case FC: FullCovarCommand(); break;
      case DV: DiagVarCommand(); break;
      case FV: FloorVectorCommand(); break;
      case PS: PowerSizeCommand(); break;
      case PR: ProjectCommand(); break;
      case DR: DecTrees2RegTreeCommand(); break;
      case IT: ImposeTreeCommand (); break;
      case JM: JoinModelCommand(); break;
      case XX: CommentCommand(); break;
      default: 
         HError(2650,"DoEdit: Command %s not recognised",cmds);
      }
      if (thisCommand != TR && thisCommand != SH && thisCommand != ST)
         lastCommand = thisCommand;
      prevCommand = thisCommand;
   }
   CloseSource(&source);
   
   if (saveHMMSet) {
      /* Save the Edited HMM Files */
      if (trace & T_BID) {
         printf("\nSaving new HMM files ...\n");
         fflush(stdout);
      }
      if (badGC) {
      FixAllGConsts(hset);         /* in case any bad gConsts around */
      badGC=FALSE;
      }
      PurgeMacros(hset);

      if (mmfFn!=NULL)
         SaveInOneFile(hset,mmfFn);
      if(SaveHMMSet(&hSet,newDir,newExt,NULL,inBinary)<SUCCESS)
         HError(2611,"DoEdit: SaveHMMSet failed");
   }

   if (trace & T_BID) {
      printf("Edit Complete\n");
      fflush(stdout);
   }
}


/* ----------------------------------------------------------- */
/*                        END:  HHEd.c                         */
/* ----------------------------------------------------------- */



