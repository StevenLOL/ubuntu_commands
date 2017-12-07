#ipython notebook
```
sudo apt-get purge ipython  #this ipython doesn't support ipython 4.0 format
sudo pip install ipython -U
sudo pip install jupyter -U
sudo pip install notebook -U
ipython notebook --ip='your ip address' ---pylab=inline --port=7777
or
ipython notebook younotbook
```
If Error no extn 
```
then sudo apt-get install --reinstall python-setuptools
```
#to dispay a plot in ipython
```
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
```
REF http://stackoverflow.com/questions/2801882/generating-a-png-with-matplotlib-when-display-is-undefined


#Show running time of cell
```
%%time
```

#ipython  remote access
## Via password
REF http://blog.csdn.net/suzyu12345/article/details/51037905
```
In [1]: from IPython.lib import passwd
In [2]: passwd()
Enter password:
Verify password:
Out[2]: ‘sha1:67c9e60bb8b6:9ffede0825894254b2e042ea597d771089e11aed‘
```
```
openssl req -x509 -nodes -days 365 -newkey rsa:1024 -keyout mycert.pem -out mycert.pem
```

Start notebook nbserver
```
ipython profile create nbserver
mv mycert.pem .ipython
```
Create Conifugre file
```
c = get_config()

# Kernel config
c.IPKernelApp.pylab = 'inline'  # if you want plotting support always

# Notebook config
c.NotebookApp.certfile = u'/home/zhenyu/.ipython/mycert.pem'
c.NotebookApp.ip = '*'
c.NotebookApp.open_browser = False
c.NotebookApp.password = u'sha1:42dd2962e4eb:4e258d7a934d8971e4b26b460ab27276a9d082b0'
# It's a good idea to put it on a known, fixed port
c.NotebookApp.port = 9999
```
Restart nbserver
```
ipython notebook --profile=nbserver
```


## via SSH tunneling

REF https://www.digitalocean.com/community/tutorials/how-to-set-up-a-jupyter-notebook-to-run-ipython-on-ubuntu-16-04

```
ssh -L 8000:localhost:8888 your_server_username@your_server_ip
```
In the remote server, the jupyter is running locally (so it is not open to public) on port 8888, and in the client side one can open the remote jupyter via port 8000 on local machine.


#install python2 or python3 kernel

```
python3 -m pip install jupyterhub notebook ipykernel
python3 -m ipykernel install # register Python 3 kernel (not technically necessary at this point, but a good idea)
python2 -m pip install ipykernel
python2 -m ipykernel install # register Python 2 kernel
```
or 
```
ipython3 kernelspec install-self
ipython2 kernelspec install-self
```
