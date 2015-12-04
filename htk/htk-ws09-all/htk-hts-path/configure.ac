# ----------------------------------------------------------- 
#                                                             
#                          ___                                
#                       |_| | |_/   SPEECH                    
#                       | | | | \   RECOGNITION               
#                       =========   SOFTWARE                  
#                                                             
#                                                             
# ----------------------------------------------------------- 
#         Copyright: Cambridge University
#          1995-2009 Engineering Department
#                    http://htk.eng.cam.ac.uk
#                    http://mi.eng.cam.ac.uk
#                 
#   Use of this software is governed by a License Agreement   
#    ** See the file License for the Conditions of Use  **    
#    **     This banner notice must not be removed      **    
#                                                             
# ----------------------------------------------------------- 
# File: configure.ac source file for generating configure script
# ----------------------------------------------------------- 

#  *** THIS IS A MODIFIED VERSION OF HTK ***                        #
# ----------------------------------------------------------------- #
#           The HMM-Based Speech Synthesis System (HTS)             #
#           developed by HTS Working Group                          #
#           http://hts.sp.nitech.ac.jp/                             #
# ----------------------------------------------------------------- #
#                                                                   #
#  Copyright (c) 2001-2011  Nagoya Institute of Technology          #
#                           Department of Computer Science          #
#                                                                   #
#                2001-2008  Tokyo Institute of Technology           #
#                           Interdisciplinary Graduate School of    #
#                           Science and Engineering                 #
#                                                                   #
# All rights reserved.                                              #
#                                                                   #
# Redistribution and use in source and binary forms, with or        #
# without modification, are permitted provided that the following   #
# conditions are met:                                               #
#                                                                   #
# - Redistributions of source code must retain the above copyright  #
#   notice, this list of conditions and the following disclaimer.   #
# - Redistributions in binary form must reproduce the above         #
#   copyright notice, this list of conditions and the following     #
#   disclaimer in the documentation and/or other materials provided #
#   with the distribution.                                          #
# - Neither the name of the HTS working group nor the names of its  #
#   contributors may be used to endorse or promote products derived #
#   from this software without specific prior written permission.   #
#                                                                   #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            #
# CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       #
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          #
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          #
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS #
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          #
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   #
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     #
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON #
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   #
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    #
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           #
# POSSIBILITY OF SUCH DAMAGE.                                       #
# ----------------------------------------------------------------- #

# After editing this file, run "autoconf" to regenerate "configure".

AC_INIT(HTS,2.2, hts-users@sp.nitech.ac.jp)
AC_COPYRIGHT(Copyright 2003-2009 University of Cambridge)
AC_COPYRIGHT(Copyright 2001-2011 Nagoya Institute of Technology)
AC_COPYRIGHT(Copyright 2001-2008 Tokyo Institute of Technology)

AC_PREFIX_DEFAULT(/usr/local/HTS-2.2beta)

dnl Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

dnl Check for ar
AN_MAKEVAR([AR], [AC_PROG_AR])
AN_PROGRAM([ar], [AC_PROG_AR])
AC_DEFUN([AC_PROG_AR], [AC_CHECK_TOOL(AR, ar, :)])
AC_PROG_AR

dnl Checks for libraries.
AC_CHECK_LIB([X11], [main])
AC_CHECK_LIB([m], [main],,AC_MSG_ERROR(Cannot find libm))

dnl Checks for FORTRAN
AC_PROG_F77
if test -z "$F77"; then
	LBFGS=''
	FLIBS=''
else
	LBFGS='lbfgs.o'
	AC_F77_LIBRARY_LDFLAGS
	CFLAGS="$CFLAGS -D_HAS_FORTRAN"
fi
AC_SUBST(FLIBS, $FLIBS)
AC_SUBST(LBFGS, $LBFGS)

dnl Checks for header files.
AC_CONFIG_HEADERS
AC_PATH_X
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h fcntl.h float.h limits.h malloc.h memory.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_VOLATILE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MEMCMP
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_HEADER_STDBOOL
AC_CHECK_FUNCS(floor gettimeofday memmove memset modf pow socket sqrt strchr strcspn strrchr strspn strstr strtol)
AC_CHECK_FUNCS(strdup)


AC_CANONICAL_HOST
AC_C_BIGENDIAN
AC_CHECK_SIZEOF(int *)

dnl Traditional HTK build?
dnl if enabled this will install in $bindir.$host_cpu
AC_ARG_ENABLE(trad-htk,
		AS_HELP_STRING([--enable-trad-htk],
		[HTK installed into $prefix/bin.$cpu and installed when compiled]))

dnl Build HDecode
AC_ARG_ENABLE(hdecode,
		AS_HELP_STRING([--enable-hdecode],
		[build large vocabulary recognition system (HDecode)]))

dnl Build LM Tools
AC_ARG_ENABLE(hlmtools,
		AS_HELP_STRING([--enable-hlmtools],
		[build Language Modelling tools]))

dnl Disable building for X
dnl i.e. use HGraf.null, and don't build HSLab
AC_ARG_ENABLE(hslab,
		AS_HELP_STRING([--disable-hslab],
		[don't build HSLab]))

dnl Build LV Recognition
AC_ARG_ENABLE(htkbook,
		AS_HELP_STRING([--enable-htkbook],
		[build HTK book]))


dnl Use -Wall if using gcc
case "$CC" in
	 gcc*)
		CFLAGS="-Wall -Wno-switch $CFLAGS"
		;;		
esac
dnl Use -Wall if using g77
case "$F77" in
	g77*)
		FFLAGS="-Wall $FFLAGS"
	;;
esac


dnl customise for htk comilpation
trad_bin_dir=$host
case "$host_cpu" in
	 powerpc)
	 	host=darwin
		trad_bin_dir=$host
		;;
esac

case "$host" in
		*x86_64*linux*)
				CFLAGS="-ansi -D_SVID_SOURCE -DOSS_AUDIO -D'ARCH=\"$host_cpu\"' $CFLAGS"
				FFLAGS="$FFLAGS"
				AC_MSG_CHECKING(whether to compile in 32-bit or 64-bit mode)
				if test x$ac_cv_sizeof_int_p = x"4"; then
					AC_MSG_RESULT(32-bit)
				LDFLAGS="-L/usr/X11R6/lib $LDFLAGS"
				elif test x$ac_cv_sizeof_int_p = x"8"; then
					AC_MSG_RESULT(64-bit)
					LDFLAGS="-L/usr/X11R6/lib64 $LDFLAGS"
				fi
				ARCH=linux
				trad_bin_dir=linux
		;;
		*linux*)
				CFLAGS="-ansi -D_SVID_SOURCE -DOSS_AUDIO -D'ARCH=\"$host_cpu\"' $CFLAGS"
				FFLAGS="$FFLAGS"
				LDFLAGS="-L/usr/X11R6/lib $LDFLAGS"
				ARCH=linux
				trad_bin_dir=linux
		;;
		*pc-solaris*)
				CFLAGS="-ansi -I/usr/openwin/include -O2 -DSOLARIS16_AUDIO -DBSD_COMP -D'ARCH=\"$host_cpu\"' $CFLAGS"
				FFLAGS="-O2 $FFLAGS"
				LDFLAGS="-L/usr/openwin/lib -ldl -lsocket -lnsl -lintl $LDFLAGS"
		;;
		*sun*)
				CFLAGS="-ansi -I/usr/openwin/include -O2 -DSOLARIS16_AUDIO -DBSD_COMP -D'ARCH=\"$host_cpu\"' $CFLAGS"
				FFLAGS="-O2 $FFLAGS"
				LDFLAGS="-L/usr/openwin/lib -ldl -lsocket -lnsl -lintl $LDFLAGS"
		;;
		*sgi*)
				CFLAGS="-O -DSGI_AUDIO  -D'ARCH=\"$host_cpu\"' $CFLAGS"
				FFLAGS="$FFLAGS"
				LDFLAGS="-laudio -lmalloc $LDFLAGS"
		;;
		*hp*)
				CFLAGS="+e -O -I/usr/include/X11R5 -I/usr/include/audio -I/opt/audio/include -D_HPUX_SOURCE -Aa -DHPUX_AUDIO -DUSE_TIMER  -D'ARCH=\"$host_cpu\"' $CFLAGS"
				FFLAGS="$FFLAGS"
				LDFLAGS="-L/opt/audio/lib -lAlib $LDFLAGS"
		;;
		*cygwin)
				CFLAGS="-ansi -DCYGWIN -D'ARCH=\"ASCII\"' $CFLAGS"
				FFLAGS="$FFLAGS"
				LDFLAGS="-L/usr/X11R6/lib $LDFLAGS"
				ARCH=ASCII
				CPU=cygwin
				BINARY_EXTENSION=.exe
		;;				
		i386*darwin*)
				CFLAGS="-ansi -g -O2 -DNO_AUDIO -D'ARCH=\"darwin\"' -I/usr/include/malloc $CFLAGS"
				FFLAGS="-gdwarf-2 -O2 $$FFLAGS"
				LDFLAGS="-L/usr/X11R6/lib $LDFLAGS"
				ARCH=darwin
				Objcopy=echo
				PRILF="-x"
				CPU=darwin
				SHRLF="-shared"
				LIBEXT=dylib
		;;				
		*darwin*)
				CFLAGS="-ansi -g -O2 -DNO_AUDIO -D'ARCH=\"darwin\"' -I/usr/include/malloc $CFLAGS"
				FFLAGS="-gdwarf-2 -O2 $$FFLAGS"
				LDFLAGS="-L/usr/X11R6/lib $LDFLAGS"
				ARCH=darwin
				Objcopy=echo
				PRILF="-x"
				CPU=darwin
				SHRLF="-shared"
				LIBEXT=dylib
		;;				
esac

dnl define MAXSTRLEN if users want to change it to other value
AC_ARG_VAR([MAXSTRLEN],[max length of strings (default=1024)])
if test -n "$MAXSTRLEN" -a $MAXSTRLEN!=1024; then
	AC_MSG_CHECKING(user-defined maximum length of strings)
	CFLAGS="$CFLAGS -DMAXSTRLEN=$MAXSTRLEN"
	AC_MSG_RESULT($MAXSTRLEN)
fi

dnl define MAXFNAMELEN if users want to change it to other value
AC_ARG_VAR([MAXFNAMELEN],[max length of file names (default=1024)])
if test -n "$MAXFNAMELEN" -a $MAXFNAMELEN!=1024; then
	AC_MSG_CHECKING(user-defined maximum length of file names)
	CFLAGS="$CFLAGS -DMAXFNAMELEN=$MAXFNAMELEN"
	AC_MSG_RESULT($MAXFNAMELEN)
fi

dnl define PAT_LEN if users want to change it to other value
AC_ARG_VAR([PAT_LEN],[max length of patterns (default=2048)])
if test -n "$PAT_LEN" -a $PAT_LEN!=2048; then
	AC_MSG_CHECKING(user-defined maximum length of file names)
	CFLAGS="$CFLAGS -DPAT_LEN=$PAT_LEN"
	AC_MSG_RESULT($PAT_LEN)
fi

dnl define SMAX if users want to change it to other value
AC_ARG_VAR([SMAX],[max number of streams (default=30)])
if test -n "$SMAX" -a $SMAX!=30; then
	AC_MSG_CHECKING(user-defined maximum number of streams)
	CFLAGS="$CFLAGS -DSMAX=$SMAX"
	AC_MSG_RESULT($SMAX)
fi


dnl Process customised options
make_all=htktools
make_install=install-htktools
build_notes="Build notes: "
dnl Tweak bindir and libdir for backwards compatibility 
dnl with previous versions of HTK
case "$enable_trad_htk" in
     yes)
	bindir=${bindir}.${trad_bin_dir}
	libdir=${libdir}.${trad_bin_dir}
	;;
esac
case "$enable_hlmtools" in
     yes)
	make_all="$make_all hlmtools"
	make_install="$make_install install-hlmtools"
	build_notes="$build_notes Language Modelling tools will be built."
	;;
     *)
	build_notes="$build_notes The Language Modelling tools will not be built. You can build them manually later by running 'make hlmtools install-hlmtools'"
	;;
esac
case "$enable_hdecode" in
     yes)
	make_all="$make_all hdecode"
	make_install="$make_install install-hdecode"
	build_notes="$build_notes Large Vocabulary recogniser HDecode will be built."
	LIBLV="HTKLiblv.a"
	;;
     *)
	build_notes="$build_notes HDecode will not be built. You can build it manually later by running 'make hdecode install-hdecode'"
	LIBLV=""
	;;
esac
case "$enable_htkbook" in
	 yes)
	 make_all="$make_all book"
	 make_install="$make_install install-book"
	 build_notes="$build_notes The HTK Book will be built."
	 ;;
esac
case "$enable_hslab" in
     no)
	HSLAB=
	HGRAF=HGraf.null.o
	build_notes="$build_notes HSLab will not be built.  If you want to change this then run 'configure' again with the '--enable-hslab' option."
	;;
     *)
	HSLAB=HSLab
	HGRAF=HGraf.o
	build_notest="$build_notes GUI tool HSLab will be built."
	;;
esac
AC_SUBST(TRADHTK, $enable_trad_htk)
AC_SUBST(TRADHTKBIN, $trad_bin_dir)
AC_SUBST(make_all, $make_all)
AC_SUBST(make_install, $make_install)
AC_SUBST(HSLAB, $HSLAB)
AC_SUBST(HGRAF, $HGRAF)
AC_SUBST(BINARY_EXTENSION, $BINARY_EXTENSION)
AC_SUBST(LIBLV)

AC_OUTPUT(HTKLib/Makefile HTKTools/Makefile HLMLib/Makefile HLMTools/Makefile HTKLVRec/Makefile Makefile)

case "$enable_trad_htk" in
     yes)
cat<<EOF
**************************************************

HTK is now ready to be built.

Type "make all" to build and install the HTK
libraries and tools.

The tools will be installed in ${prefix}/`basename ${bindir}`

`echo $build_notes | fmt -w 50`

**************************************************
EOF
	;;
	*)
cat<<EOF
**************************************************

HTK is now ready to be built.

Type "make all" to build the HTK libraries
and tools.

Then "make install" to install them.

The tools will be installed in ${prefix}/`basename ${bindir}`

`echo $build_notes | fmt -w 50`

**************************************************
EOF
	;;
esac
