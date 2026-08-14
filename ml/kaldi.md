svn cleanup kaldi-trunk
svn co https://svn.code.sf.net/p/kaldi/code/trunk kaldi-trunk


cd tools
make -j 4
cd ..
sudo apt-get install libatlas-dev libatlas-base-dev
./configure
make depend -j 10
make -j 10




<USER>@<HOSTNAME>:~$ cat .profile
# ~/.profile: executed by the command interpreter for login shells.
# This file is not read by bash(1), if ~/.bash_profile or ~/.bash_login
# exists.
# see /usr/share/doc/bash/examples/startup-files for examples.
# the files are located in the bash-doc package.

# the default umask is set in /etc/profile; for setting the umask
# for ssh logins, install and configure the libpam-umask package.
#umask 022

# if running bash
if [ -n "$BASH_VERSION" ]; then
    # include .bashrc if it exists
    if [ -f "$HOME/.bashrc" ]; then
	. "$HOME/.bashrc"
    fi
fi

# set PATH so it includes user's private bin if it exists
if [ -d "$HOME/bin" ] ; then
    PATH="$HOME/bin:$PATH"
fi
PATH=<YOUR_HOME>/apps/getmfcc/utils:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/util:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/bin:$PATH     
PATH=<YOUR_HOME>/kaldi-trunk/src/fgmmbin:$PATH  
PATH=<YOUR_HOME>/kaldi-trunk/src/gmmbin:$PATH      
PATH=<YOUR_HOME>/kaldi-trunk/src/kwsbin:$PATH  
PATH=<YOUR_HOME>/kaldi-trunk/src/nnet2bin:$PATH  
PATH=<YOUR_HOME>/kaldi-trunk/src/onlinebin:$PATH  
PATH=<YOUR_HOME>/kaldi-trunk/src/sgmmbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/featbin:$PATH  
PATH=<YOUR_HOME>/kaldi-trunk/src/fstbin:$PATH   
PATH=<YOUR_HOME>/kaldi-trunk/src/ivectorbin:$PATH  
PATH=<YOUR_HOME>/kaldi-trunk/src/latbin:$PATH  
PATH=<YOUR_HOME>/kaldi-trunk/src/nnetbin:$PATH   
PATH=<YOUR_HOME>/kaldi-trunk/src/sgmm2bin:$PATH
<USER>@<HOSTNAME>:~$ 
















PATH=<YOUR_HOME>/apps/getmfcc/utils:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/utils:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/bin:$PATH     
PATH=<YOUR_HOME>/kaldi-trunk/src/fgmmbin:$PATH  
PATH=<YOUR_HOME>/kaldi-trunk/src/gmmbin:$PATH      
PATH=<YOUR_HOME>/kaldi-trunk/src/kwsbin:$PATH  
PATH=<YOUR_HOME>/kaldi-trunk/src/nnet2bin:$PATH  
PATH=<YOUR_HOME>/kaldi-trunk/src/onlinebin:$PATH  
PATH=<YOUR_HOME>/kaldi-trunk/src/sgmmbin:$PATH
PATH=<YOUR_HOME>/kaldi-trunk/src/featbin:$PATH  
PATH=<YOUR_HOME>/kaldi-trunk/src/fstbin:$PATH   
PATH=<YOUR_HOME>/kaldi-trunk/src/ivectorbin:$PATH  
PATH=<YOUR_HOME>/kaldi-trunk/src/latbin:$PATH  
PATH=<YOUR_HOME>/kaldi-trunk/src/nnetbin:$PATH   
PATH=<YOUR_HOME>/kaldi-trunk/src/sgmm2bin:$PATH



===================old
if fatal error: clapack.h

sudo apt-get install libatlas-base-dev
sudo apt-get --purge remove liblapack-dev liblapack3 liblapack3gf
export CPLUS_INCLUDE_PATH=/usr/include/atlas

#https://github.com/SciRuby/nmatrix/wiki/Installation





