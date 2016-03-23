#ipython notebook

sudo apt-get purge ipython  #this ipython doesn't support ipython 4.0 format
sudo pip install ipython -U
sudo pip install jupyter -U
sudo pip install notebook -U
ipython notebook --ip='your ip address' ---pylab=inline --port=7777
or
ipython notebook younotbook

%If Error no extn 
then sudo apt-get install --reinstall python-setuptools

#to dispay a plot in ipython

#coding=utf-8
%matplotlib inline
from gensim.models import word2vec,Word2Vec
from gensim import corpora, models
import matplotlib
matplotlib.use('Agg') 
import matplotlib.pyplot as plt


plt.clf()
plt.plot(YOURDATA,label=u'水煮鱼')
plt.show()

REF http://stackoverflow.com/questions/2801882/generating-a-png-with-matplotlib-when-display-is-undefined

