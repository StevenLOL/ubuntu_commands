# tmux / cron / Docker startup cheatsheet

Quick reference for keeping long tasks alive (nohup/tmux), scheduling (cron), starting tasks at boot, and an example Docker auto-start.

## Detach a terminal from a remote (survive broken pipe)
```
# an ssh task that runs for hours can die with "broken pipe"; use nohup or tmux:
aaron@localpc$ ssh root@remoteserver
root@remoteserver# nohup ./run_eval24hr_M_SAD_SD.sh </dev/null &
nohup ./local/run_dnn.sh </dev/null &
nohup some_command > nohup2.out &
tmux
tmux list-sessions
tmux attach-session -t #
# enable mouse mode in ~/.tmux.conf:
setw -g mouse on
tmux source-file ~/.tmux.conf
# inside tmux:
ctrl-b s    # switch / list sessions
ctrl-b d    # detach session
```

## Run a task on a schedule (cron)
```
crontab -e
01 * * * * python <YOUR_HOME>/Dropbox/workspace/python_lib/utli/remote_script/<HOSTNAME>loop.py
```

## Run a task at system startup
```
sudo nano /etc/rc.local
# REF: http://askubuntu.com/questions/9853/how-can-i-make-rc-local-run-on-startup
cp myscript.sh /etc/init.d/
sudo update-rc.d myscript.sh defaults 90
# to remove:
sudo update-rc.d -f myscript.sh remove
```

## Example: start a Docker image at startup
```
sudo cat /etc/init.d/runserver.sh
```
```bash
#!/bin/bash
nvidia-docker run -p 30011:30011 -p 30012:30012 -tdi -v <YOUR_HOME>/face_release/:/data/ <YOUR_DOCKERHUB>/nvida_docker_caffe_tensorflow_keras_scikit_learn /data/runme.indocker.sh
```
```
sudo cat <YOUR_HOME>/face_release/runme.indocker.sh
```
```bash
#!/bin/bash
export PATH=/usr/bin:/usr/local/bin:/bin:/sbin:/usr/sbin:/usr/local/sbin:/usr/local/cuda/bin:$PATH
export PYTHONIOENCODING=utf-8
export C_INCLUDE_PATH=/usr/local/cuda-8.0/include/
export CPLUS_INCLUDE_PATH=/usr/local/cuda-8.0/include/
export CUDA_HOME=/usr/local/cuda
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/usr/local/cuda/lib64
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/downloads/cuda-8.0/lib64
export PYTHONPATH=/root/downloads/caffe/build/install/python:$PYTHONPATH
export CUDA_CUDA_LIBRARY=/usr/local/cuda/lib

cd /data/development_face_cut_and_embedding_v1_facenet
nohup python face_cut_server.py &
cd /data/facenet/src/
python face_v_server_extract_features_tf.py
```
