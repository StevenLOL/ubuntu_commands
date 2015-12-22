The best way to install CUDA or Nvidia driver is through is deb package

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
It will make your DNN training faster, to install copy * to your

