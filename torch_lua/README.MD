https://github.com/torch/distro




git clone https://github.com/torch/distro.git ~/torch --recursive 
cd ~/torch; bash install-deps;
./install.sh


ZeroBrane require 'image' ...

export TORCH_DIR=$HOME/torch

export LUA_PATH="$TORCH_DIR/install/share/lua/5.1/?.lua;$TORCH_DIR/install/share/lua/5.1/?/init.lua;$TORCH_DIR/install/share/luajit-2.1.0-alpha/?.lua"

export LUA_CPATH="$TORCH_DIR/install/lib/lua/5.1/?.so"


