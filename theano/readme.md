
TECH LOGIC

    Machine Learning Resources for Beginners and Beginners++
    I have been learning ML for sometime now and I have spent some time on finding what are some of the good resources for ML.  For Beginners...
    Compiling mex files on 64bit Linux(UBUNTU) using MATLAB 2012
    I could not setup the compiler for myself to create mex files. So I searched the internet, going through many blogs and posts. Finally I go...
    Unsupervised Feature Learning and Deep Learning Resources
    Unsupervised feature learning and deep learning has been fascinating to me recently and here are some interesting links and tutorials.  F...
    Probabilistic programming
    This is nice a nice post about probabilistic programming. http://radar.oreilly.com/2013/04/probabilistic-programming.html http://tm.durus...
    closed-form expression for nth term of the Fibonacci sequence
    Generally we all know what a Fibonacci series is. I had 1st seen it in my computer science class where I used the recursive relation to fin...

Tuesday, March 31, 2015
Setting up Theano on Ubuntu 14.04
After 2 days of non-stop struggle, I was able to make theano work with my GPU NVIDIA GeForce GTX 860M.

TRY AT YOUR OWN RISK!!

Settings which worked are as follows.
DONOT install bumblebee

Run Following command to see if your NVIDIA GPU device is being detected. If not, this BLOG wont help. Sorry.

    lspci | grep -i NVIDIA


Install the following:
Driver: NVIDIA 340.76
Cuda : 6.5 toolkit

    Switch to NVIDIA card (If you dont have this command try to get it, without changing the graphics drivers, conflicting drivers can be blacklisted, see next point).

        prime-switch nvidia

    Blacklist other driver which can create conflicts:

    create /etc/modprobe.d/blacklist-file-drivers.conf File with blacklisted drivers. Use command ubuntu-drivers devices to get a list of nvidia drivers:

        blacklist nvidia-349
        blacklist nvidia-346
        blacklist xserver-xorg-video-nouveau 

    To list all installed Graphics Drivers (Useful while blacklisting drivers)   

        ubuntu-drivers devices

    Note: DONOT blacklist nvidia-340

    Make sure the following command works without error.

        nvidia-modprobe
        nvidia-settings
        nvidia-smi

     Find /usr/local/cuda-6.5/samples/1_Utilities/deviceQuery folder for your system.

    use make command to create executable. 

        cd /usr/local/cuda-6.5/samples/1_Utilities/deviceQuery/
        sudo make  

    run /usr/local/cuda-6.5/samples/1_Utilities/deviceQuery/deviceQuery

    You should get the following results: 

        deviceQuery, CUDA Driver = CUDART, CUDA Driver Version = 6.5, CUDA Runtime Version = 6.0, NumDevs = 1, Device0 = GeForce GTX 860M

        Result = PASS

    Install theano from GIT (First install dependencies from theano website):

        sudo apt-get install python-numpy python-scipy python-dev python-pip python-nose g++ libopenblas-dev git

        sudo pip uninstall theano 
        sudo pip install git+git://github.com/Theano/Theano.git


In /usr/local/cuda-6.5/targets/x86_64-linux/include/host_config.h, cuda-6.5 supports gcc-4.8 g++-4.8 so one needs to install these and make links to gcc and g++ respectively.
example:  sudo ln -s /usr/bin/gcc-4.8 /usr/local/cuda/bin/gcc

    Create ~/.theanorc File with following content:

    [global]
    floatX = float32
    device = gpu

    [nvcc]
    fastmath = True

    [cuda]
    root=/usr/local/cuda-6.5/

     Make a python file to test gpu say test.py: 

        from theano import function, config, shared, sandbox
        import theano.tensor as T
        import numpy
        import time

        vlen = 10 * 30 * 768  # 10 x #cores x # threads per core
        iters = 1000

        rng = numpy.random.RandomState(22)
        x = shared(numpy.asarray(rng.rand(vlen), config.floatX))
        f = function([], T.exp(x))
        print f.maker.fgraph.toposort()
        t0 = time.time()
        for i in xrange(iters):
            r = f()
        t1 = time.time()
        print 'Looping %d times took' % iters, t1 - t0, 'seconds'
        print 'Result is', r
        if numpy.any([isinstance(x.op, T.Elemwise) for x in f.maker.fgraph.toposort()]):
            print 'Used the cpu'
        else:
            print 'Used the gpu'


    Once you run the above python file:

        sudo python test.py

    You might get an error:

        Failed to compile cuda_ndarray.cu: libcublas.so.6.5: cannot open shared object file: No such file or directory

    So you need to locate and configure that path:

        locate libcublas.so.6.5

    Add the following library path to ~/.bashrc (Second path has libcublas.so.6.5)

        sudo ldconfig /usr/local/cuda-6.5/lib64/
        sudo ldconfig /usr/local/cuda-6.5/targets/x86_64-linux/lib/

        export LD_LIBRARY_PATH=/usr/local/cuda-6.5/targets/x86_64-linux/lib:$LD_LIBRARY_PATH

        export PATH=/usr/local/cuda-6.5/bin:$PATH
        export PATH=/usr/local/cuda-6.5/targets/x86_64-linux/lib:$PATH

    Now run the gpu code.

        sudo python test.py

    I get roughly 10x speedup with GPU.

I screwed up many times with different versions of drivers, if you do the same. You might not get a login screen (black screen). The use ctrl + Alt + F1 to goto command mode.
Remove  xorg.conf
sudo rm /etc/X11/xorg.conf

sudo service lightdm stop
sudo service lightdm start

You possibly have got you login screen. You might also be trapped in loop, where login screen asks for password and goes in and again login screen asks for password. I hope you dont get this situation.
(I added:
nvidia-modprobe  
prime-switch intel
to end of ~/.bashrc because there were login loop issues with got resolved by this. I am not sure but it works.)

Although I have written this for my record but I hope it might help someone. :)

Hope this helps.



Posted by Devendra Kumar at 3:08 PM Email ThisBlogThis!Share to TwitterShare to FacebookShare to Pinterest
Labels: cuda, cuda 6.5, drivers, gpu, theano, ubuntu 14.04
No comments:

Post a Comment

Newer Post Older Post Home
Subscribe to: Post Comments (Atom)
Search This Blog
	
Archive

    ▼  2015 (5)
        ►  May (1)
        ▼  March (1)
            Setting up Theano on Ubuntu 14.04
        ►  February (2)
        ►  January (1)

    ►  2014 (6)

    ►  2013 (10)

    ►  2012 (12)

Followers
Share It
Total Pageviews
26840
About Me
My Photo

Devendra Kumar

View my complete profile
	
	
Picture Window template. Powered by Blogger.

