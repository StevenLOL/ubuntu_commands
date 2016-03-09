
https://github.com/tensorflow/tensorflow
https://github.com/tensorflow/tensorflow/blob/master/tensorflow/g3doc/get_started/os_setup.md

install TensorFlow:

# Ubuntu/Linux 64-bit, GPU enabled:
$ sudo pip install --upgrade https://storage.googleapis.com/tensorflow/linux/gpu/tensorflow-0.7.1-cp27-none-linux_x86_64.whl


Test the TensorFlow installation

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/cuda/lib64"
export CUDA_HOME=/usr/local/cuda


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


##cuDNN## 
*Download untar and copy files to cuda path*
The NVIDIA CUDA Deep Neural Network library (cuDNN) is a GPU-accelerated library of primitives for deep neural networks
https://developer.nvidia.com/cudnn

