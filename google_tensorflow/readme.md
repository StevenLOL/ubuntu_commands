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
~~~
import tensorflow as tf
hello = tf.constant('Hello, TensorFlow!')
sess = tf.Session()
print(sess.run(hello))
#Hello, TensorFlow!
a = tf.constant(10)
b = tf.constant(32)
print(sess.run(a + b))
~~~

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

~~~
cp ./include/* /usr/local/cuda-7.5/include/
cp ./lib64/* /usr/local/cuda-7.5/lib64/
~~~

sudo pip install protobuf -I
#Errors that may occur#
##[No module named extern](https://github.com/pypa/pip/issues/3551)##
~~~
apt-get install --reinstall python-setuptools
~~~
##[ImportError: No module named protobuf ](https://github.com/tensorflow/tensorflow/issues/1336)##

~~~
##repeat till get error
sudo pip uninstall tensorflow
sudo pip uninstall protobuf
~~~




#BUILD and  instal android demo

In tensorflow root,open the WORKSPACE (Line 24+) and setup android sdk and ndk eg:
```
# Uncomment and update the paths in these entries to build the Android demo.
android_sdk_repository(
    name = "androidsdk",
    api_level = 23,
    build_tools_version = "23.0.1",
    # Replace with path to Android SDK on your system
    path = "/home/steven/Android/Sdk",
)
#
android_ndk_repository(
    name="androidndk",
    path="/home/steven/Downloads/android-ndk-r13b",
    api_level=21)
```

Then in the andoid folder:
```
bazel build //tensorflow/examples/android:tensorflow_demo
```

To install 
```
/home/steven/Android/Sdk/platform-tools/adb install -r -g ./tensorflow_demo.apk
```

