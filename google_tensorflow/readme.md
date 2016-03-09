#REF#
https://github.com/tensorflow/tensorflow
https://github.com/tensorflow/tensorflow/blob/master/tensorflow/g3doc/get_started/os_setup.md

#install TensorFlow:#

~~~
Ubuntu/Linux 64-bit, GPU enabled:
$ sudo pip install --upgrade https://storage.googleapis.com/tensorflow/linux/gpu/tensorflow-0.7.1-cp27-none-linux_x86_64.whl
~~~

~~~
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/cuda/lib64"
export CUDA_HOME=/usr/local/cuda
~~~
##Test the TensorFlow installation###


python

import tensorflow as tf
hello = tf.constant('Hello, TensorFlow!')
sess = tf.Session()
print(sess.run(hello))
#Hello, TensorFlow!
a = tf.constant(10)
b = tf.constant(32)
print(sess.run(a + b))


#You may need these#
sudo apt-get install build-essential
sudo apt-get install gfortran


#About cuda and cudnn#

##Download cuda deb from nvidia##

https://developer.nvidia.com/cuda-downloads
It is better download the network deb file.
sudo dpkg -i cuda_7.5_whatever.deb
sudo apt-get update
sudo apt-get install cuda-toolkit-7.5
sudo apt-get install cuda-drivers

###Test cuda installation###
nivdia-smi
In python
import gensim

##cuDNN ## 


The NVIDIA CUDA Deep Neural Network library (cuDNN) is a GPU-accelerated library of primitives for deep neural networks

https://developer.nvidia.com/cudnn

**Download and untar then copy files to cuda path**

cp ./include/* /usr/local/cuda-7.5/include/

cp ./lib64/* /usr/local/cuda-7.5/lib64/




