# Latest:

http://torch.ch/docs/getting-started.html




https://github.com/torch/distro




git clone https://github.com/torch/distro.git ~/torch --recursive 
cd ~/torch; bash install-deps;
./install.sh


ZeroBrane require 'image' ...

export TORCH_DIR=$HOME/torch

export LUA_PATH="$TORCH_DIR/install/share/lua/5.1/?.lua;$TORCH_DIR/install/share/lua/5.1/?/init.lua;$TORCH_DIR/install/share/luajit-2.1.0-alpha/?.lua"

export LUA_CPATH="$TORCH_DIR/install/lib/lua/5.1/?.so"


# Torch with LUA5.2
```

git clone https://github.com/torch/distro.git ~/torch --recursive 
cd torch
./clean.sh

sudo TORCH_LUA_VERSION=LUA52 ./install.sh
sudo luarocks install tds
sudo luarocks install lua-zlib
sudo apt-get install libreadline-dev

```

# In case luarocks breaks, just reinstall it via

```
$ wget https://luarocks.org/releases/luarocks-2.4.1.tar.gz
$ tar zxpf luarocks-2.4.1.tar.gz
$ cd luarocks-2.4.1
$ ./configure; sudo make bootstrap
$ sudo luarocks install luasocket
$ lua
Lua 5.3.3 Copyright (C) 1994-2016 Lua.org, PUC-Rio
> require "socket"

#ref https://luarocks.org/#quick-start
```


# install zeromq
## install zeromq packages
Download and install [Legacy Stable Release 3.2.5](https://github.com/zeromq/zeromq3-x/releases/download/v3.2.5/zeromq-3.2.5.tar.gz) from http://zeromq.org/intro:get-the-software .
```
./autogen.sh && ./configure && make -j 4
make check && make install && sudo ldconfig
```

## install lua-zmq
```
luarocks install https://raw.github.com/Neopallium/lua-zmq/master/rockspecs/lua-zmq-scm-1.rockspec
```
