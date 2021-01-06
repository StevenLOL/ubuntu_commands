#ipython notebook
```
# sudo apt-get purge ipython  #this ipython doesn't support ipython 4.0 format
sudo apt install python-dev   # else error "fatal error: Python.h: No such file or directory"
sudo pip install ipython -U
sudo pip install jupyter -U
sudo pip install notebook -U

# sudo pip install "ipython==5.5.0" jupyter notebook -U  ## for python 2.7 , the ipython 6.0+ only support python 3.3 +
ipython notebook --ip='your ip address' ---pylab=inline --port=7777
or
ipython notebook younotbook
```
If Error no extn 
```
then sudo apt-get install --reinstall python-setuptools
```
https://www.reddit.com/r/IPython/comments/4z26o7/setup_and_configure_jupyter_how_to_set_up_a/
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


# Show running time of cell
```
%%time
```
## auto reload libs
```
%load_ext autoreload 
%autoreload 2
```

#ipython  remote access
## Via password
REF http://blog.csdn.net/suzyu12345/article/details/51037905
```
and then we create a configuration file with the following:
jupyter notebook --generate-config

The output given will be the path where this file has been created, /.jupyter/jupyter_notebook_config.py. 
Now, in order for users to have a password, users must open python in the terminal, enter the commands
>>> from notebook.auth import passwd
>>> passwd()
Enter password:
Verify password:
'sha1:d66351142f0a:9eea3d7f99e434a6837f5e73af18d03cf0353392'
>>> ctrl+z

copy the hashed password, and paste it in the configuration file at 
c.NotebookApp.password = u''
 

```


## install extensions
```
pip install jupyter_nbextensions_configurator jupyter_contrib_nbextensions -i https://pypi.douban.com/simple
jupyter contrib nbextension install --user --skip-running-check
jupyter nbextensions_configurator enable --user
jupyter nbextension enable

[Tools] 分享几个实用的 jupyter notebook 扩展功能
https://blog.csdn.net/su_2018/article/details/89378015?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-3.edu_weight&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-3.edu_weight#%C2%A01.%E4%BB%A3%E7%A0%81%E5%AF%BC%E8%88%AA%E5%8A%9F%E8%83%BD
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
Save notebook to python script

```
jupyter nbconvert --to script ./0001.NN.baseline.GridSearch.Manual.ipynb
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
