REF http://www.open-open.com/lib/view/open1455241083495.html

鍩轰簬docker鐨刢affe鐜鎼缓
  2016-02-12 09:39:09 鍙戝竷
鎮ㄧ殑璇勪环: 	
     
	4.0 	
     1鏀惰棌

鏉ヨ嚜锛?http://blog.csdn.net//chenriwei2/article/details/50250685

寮曡█

涓轰粈涔堣鐢╠ocker锛?鐔熸倝docker 鍏跺疄宸茬粡寰堜箙浜嗭紝鑷粠瀹炰範鐨勬椂鍊欒寰楀畠鏄竴涓埄鍣ㄤ互鏉ヤ竴鐩存病鏈夋満浼氫娇鐢ㄥ畠锛岃繖鍑犲ぉ鍦ㄦ姌鑵綾affe鐜涓紝瑙夊緱鏄椂鍊欑敤涓奃ocker浜嗐€?

闇€姹傦細鏋勫缓涓€涓嫭绔嬬殑瀹瑰櫒锛岄噷闈㈠畨瑁呬簡caffe 鐨勬墍鏈変緷璧栵紝鍦ㄩ渶瑕佽窇浠ｇ爜鐨勬椂鍊欙紝鐩存帴杩愯瀹冦€?

浼樼偣锛氬彲浠ヨВ鍐冲悇绉嶄緷璧栭棶棰橈紝姣斿杩欎釜杞欢闇€瑕佸畨瑁単cc 4.7 锛岃€屽彟澶栦竴涓渶瑕佸畨瑁単cc 4.8 绛夌瓑浜掓枼鐨勭幆澧冮渶姹傘€?
Docker 瀹夎

docker 鐨勫畨瑁呭拰鍩烘湰浣跨敤锛屽彲浠ュ弬鐓ф垜涓婇潰鐨勪袱绡囧崥瀹細瀹夎绡?鍜?浣跨敤绡囥€?
鏋勫缓闀滃儚

鏋勫缓闀滃儚鏈変袱绉嶆柟娉曪細

1锛屽啓Dockerfile,浼樼偣鏄究浜庡叡浜紱

2锛屼粠瀹瑰櫒涓璫ommit锛屼紭鐐规槸绠€鍗曟柟渚匡紝浣嗘槸涓嶆柟渚垮叡浜€?

鐢变簬瀹夎caffe鐜闇€瑕佷紬澶氱殑渚濊禆鍖咃紝鑰屼笖鐢变簬缃戠粶鏂归潰鐨勫師鍥狅紝杩欎簺渚濊禆鍖呭線寰€涓嶈兘涓€娆″瑁呭ソ锛屼负浜嗘柟渚胯捣瑙侊紝杩欓噷鐩存帴鍚姩涓€涓熀鏈殑瀹瑰櫒锛岀劧鍚庝粠閲岄潰瀹夎渚濊禆鍖呫€?

杩欒竟閫夋嫨鍩烘湰鐨剈buntu:14.04 鏉ュ紑濮嬫瀯寤虹幆澧冿紝
1锛屽惎鍔ㄥ鍣細

sudo docker run --dns 8.8.8.8 --dns 8.8.4.4 --name ubuntu_caffe -i -t ubuntu:14.04 /bin/bash

闇€瑕佹洿鏀筪ns锛屾槸鍥犱负濡傛灉娌℃湁鐨勮瘽锛屽鍣ㄤ笂涓嶄簡缃戙€?
2锛屼緷璧栧寘瀹夎

杩涘叆瀹瑰櫒涔嬪悗锛屾墍鏈夌殑鎿嶄綔灏辫窡鏅€氱殑涓€涓牱浜嗐€?
棣栧厛瀹夎涓€浜涘熀鏈殑宸ュ叿锛?

apt-get install wget
apt-get install unzip
apt-get install python-pip

鏂板缓璺緞锛屽悗闈㈡墍鏈夌殑caffe浠ｇ爜閮芥斁鍦ㄨ繖閲?

mkdir <YOUR_HOME>/Caffe
cd <YOUR_HOME>/Caffe

opencv 鐨勫畨瑁咃細
github涓婇潰鏈変竴涓竴閿畨瑁呯殑锛屼絾鏄湪瀹瑰櫒閲岄潰涓€閿畨瑁呬笉浜嗭紝鍚庨潰鎴戞槸鎶婂畠閲岄潰鐨勮剼鏈竴鍙ヤ竴鍙ョ殑鎼炲嚭鏉ユ墽琛屻€?

#opencv涓€鍙ヤ竴鍙ョ殑杩愯
arch=$(uname -m)
if [ "$arch" == "i686" -o "$arch" == "i386" -o "$arch" == "i486" -o "$arch" == "i586" ]; then flag=1; else flag=0; fi
echo "Installing OpenCV 2.4.9"
mkdir OpenCV
cd OpenCV
sudo apt-get -y install libopencv-dev
sudo apt-get -y install build-essential checkinstall cmake pkg-config yasm
sudo apt-get -y install libtiff4-dev libjpeg-dev libjasper-dev
sudo apt-get -y install libavcodec-dev libavformat-dev libswscale-dev libdc1394-22-dev libxine-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev libv4l-dev
sudo apt-get -y install python-dev python-numpy
sudo apt-get -y install libtbb-dev
sudo apt-get -y install libqt4-dev libgtk2.0-dev
sudo apt-get -y install libfaac-dev libmp3lame-dev libopencore-amrnb-dev libopencore-amrwb-dev libtheora-dev libvorbis-dev libxvidcore-dev
wget http://archive.ubuntu.com/ubuntu/pool/multiverse/f/faac/faac_1.28-6.debian.tar.gz
vi /etc/hosts
ifconfig
sudo apt-get -y install x264 v4l-utils ffmpeg
wget -O OpenCV-2.4.9.zip http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/2.4.9/opencv-2.4.9.zip/download
unzip OpenCV-2.4.9.zip
cd opencv-2.4.9
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_TBB=ON -D BUILD_NEW_PYTHON_SUPPORT=ON -D WITH_V4L=ON -D INSTALL_C_EXAMPLES=ON -D INSTALL_PYTHON_EXAMPLES=ON -D BUILD_EXAMPLES=ON -D WITH_QT=ON -D WITH_OPENGL=ON ..
make -j4
sudo make install
sudo sh -c 'echo "/usr/local/lib" > /etc/ld.so.conf.d/opencv.conf'
sudo ldconfig
cd ..

caffe 鍜宲ython渚濊禆鍖咃細

sudo apt-get install libprotobuf-dev libleveldb-dev libsnappy-dev libopencv-dev libhdf5-serial-dev protobuf-compiler
sudo apt-get install --no-install-recommends libboost-all-dev
sudo apt-get install libgflags-dev libgoogle-glog-dev liblmdb-dev
sudo apt-get install libatlas-base-dev

瀹夎cuda锛?
杩欓噷闈釜灏忔妧宸у氨鏄紝濡備綍浠巋ost 涓绘満瀵煎叆鏁版嵁鍒板鍣ㄤ腑锛屽彲浠ヤ娇鐢ㄥ涓嬪懡浠わ紙鍦╤ost涓婅繍琛岋級

sudo cp cuda_7.5.18_linux.run /var/lib/docker/aufs/mnt/92562f161e51994949dd8496360265e5d54d32fbe301d693300916cd56d4e0a2<CONTAINER_HOME>/Caffe

sudo cp cudnn-7.0-linux-x64-v3.0-prod.tgz /var/lib/docker/aufs/mnt/92562f161e51994949dd8496360265e5d54d32fbe301d693300916cd56d4e0a2<CONTAINER_HOME>/Caffe

sudo cp caffe-master.zip /var/lib/docker/aufs/mnt/92562f161e51994949dd8496360265e5d54d32fbe301d693300916cd56d4e0a2<CONTAINER_HOME>/Caffe

鍏朵腑锛岄偅涓€闀夸覆鏁板瓧锛屾槸浣犵殑瀹瑰櫒鐨勫畬鍏╥d锛屽彲浠ョ敤鍛戒护

docker inspect -f   '{{.Id}}' ubuntu_caffe #ubuntu_caffe 鏄鍣ㄧ殑鍚嶅瓧

./cuda_*_linux.run -extract=`pwd`
./NVIDIA-Linux-x86_64-*.run -s --no-kernel-module
./cuda-linux64-rel-*.run -noprompt

瀹夎cudnn锛?

tar -xvf cudnn-7.0-linux-x64-v3.0-prod.tgz 
cp cuda/include/cudnn.h /usr/local/cuda/include/
cp cuda/lib64/* /usr/local/cuda/lib64/

瀹夎caffe锛?

cd caffe-15.12.07/
cp Makefile.config.example Makefile.config
vi Makefile.config
make all
make test

瀹夎caffe 鐨刾ython缁戝畾

cd python/

apt-get install python-pip
for req in $(cat requirements.txt); do pip install $req; done

sudo apt-get install python-numpy python-scipy python-matplotlib ipython ipython-notebook python-pandas python-sympy python-nose

涓嬮潰杩欏嚑涓寘闇€瑕佹墜鍔ㄨ嚜宸卞幓瑁咃紝鐢ㄤ笂闈㈢殑鍛戒护涓€鐩磋涓嶅ソ銆?

apt-get install python-scipy
for req in $(cat requirements.txt); do pip install $req; done
pip install -U scikit-learn
for req in $(cat requirements.txt); do pip install $req; done
pip install scikit-image
for req in $(cat requirements.txt); do pip install $req; done

鐒跺悗

make pycaffe

鏈€鍚庯紝
鏂板缓涓€涓矾寰勶紝浠ヤ究浜庣鐩樻槧灏?

mkdir /media/crw/MyBook

鎵€鏈夌殑鐜閮藉仛濂戒簡涔嬪悗锛屽氨鍙互commit 浜?

docker commit 92562f161e519 my-ubuntu-caffe

杩欐牱锛屽氨鍦ㄦ湰鍦扮幆澧冧腑锛屽埗浣滃ソ浜嗕竴涓浆濂絚affe gpu鐜鐨勫鍣ㄤ簡銆?
鍚姩caffe鍚姩瀹瑰櫒锛?

sudo docker run -ti \
    --device /dev/nvidia0:/dev/nvidia0 \     --device /dev/nvidiactl:/dev/nvidiactl \     --device /dev/nvidia-uvm:/dev/nvidia-uvm \     -v /media/crw/MyBook:/media/crw/MyBook \     my-ubuntu-caffe  /bin/bash

1锛屾樉鍗＄洿閫氫娇鐢紝
2锛屾枃浠舵槧灏勶紝鎸傝浇涓€涓猦ost涓绘満鐨勭鐩樺埌瀹瑰櫒璺緞涓嬶紝杩欓噷璁剧疆涓虹浉鍚岋紝鍙互鍑忓皯涓€浜涗笉蹇呰鐨勯夯鐑︺€?
杩愯caffe妯″瀷璁粌

浼氭湁鎻愮ず璇存壘涓嶅埌cuda鍟ョ殑锛岃缃竴涓嬬幆澧冨彉閲忓嵆鍙€?

$ export CUDA_HOME=/usr/local/cuda
$ export LD_LIBRARY_PATH=${CUDA_HOME}/lib64
$ PATH=${CUDA_HOME}/bin:${PATH}
$ export PATH

cd /media/crw/MyBook/Experience/FaceRecognition/Softmax/try3_3
./train.sh

Done~
