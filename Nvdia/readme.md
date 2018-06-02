The best way to install CUDA or Nvidia driver is through its deb package

1) download deb pakage
https://developer.nvidia.com/cuda-downloads/

and install through
>sudo dpkag -i your_downloaded_package

This will add PPA to your software source 
then:
>sudo apt-get update
>sudo apt-get install cuda

2)few thins to note
2.1) you may need to uninstall your driver before insall CUDA
2.2) There is a cudnn,  cuDNN is our library for Deep Learning frameworks, and can be downloaded separately from the cuDNN home page.
https://developer.nvidia.com/cudnn
It will make your DNN training faster, to install just copy * to your cuda lib folder

3)Wat? You want the cuda 7.0 ?

https://developer.nvidia.com/cuda-toolkit-70
wget http://developer.download.nvidia.com/compute/cuda/7_0/Prod/local_installers/cuda_7.0.28_linux.run


4) nvidia-smi -l 1
