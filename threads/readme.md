
Ref http://stackoverflow.com/questions/39381974/how-to-set-the-max-thread-a-python-script-could-use-when-calling-from-shell?noredirect=1#comment66096808_39381974

taskset -c 1-3 python yourProgram.py

In this case the threads 1-3 (3 in total) will be used. Any parallelization invoked by your program will share those resources.

For a solution that fits your exact problem you should better identify which part of the code parellelizes. For instance, if it is due to numpy routines, you could limit it by using:

OMP_NUM_THREADS=4 python yourProgram.py
