# Torch (Lua-based ML framework)

## 1. What is it?

Torch is a scientific-computing framework with a LuaJIT backend and a rich ML library (`torch`, `nn`, `cutorch`). Predecessor of PyTorch. This note covers installing the classic `torch/distro`.

## 2. What is it for?

- Building/running neural nets in Lua (common in older speech/NLP research codebases).

## 3. How to download / install

```bash
git clone https://github.com/torch/distro.git ~/torch --recursive
cd ~/torch
bash install-deps
./install.sh
```

Docs: http://torch.ch/docs/getting-started.html · https://github.com/torch/distro

## 4. How to use

If ZeroBrane/luarocks needs `image`, set the Lua paths:
```bash
export TORCH_DIR=$HOME/torch
export LUA_PATH="$TORCH_DIR/install/share/lua/5.1/?.lua;$TORCH_DIR/install/share/lua/5.1/?/init.lua;$TORCH_DIR/install/share/luajit-2.1.0-alpha/?.lua"
export LUA_CPATH="$TORCH_DIR/install/lib/lua/5.1/?.so"
```

### Torch with Lua 5.2
```bash
git clone https://github.com/torch/distro.git ~/torch --recursive
cd torch
./clean.sh
sudo TORCH_LUA_VERSION=LUA52 ./install.sh
sudo luarocks install tds
sudo luarocks install lua-zlib
sudo apt-get install libreadline-dev
```

If luarocks breaks:
```bash
wget https://luarocks.org/releases/luarocks-2.4.1.tar.gz
tar zxpf luarocks-2.4.1.tar.gz
cd luarocks-2.4.1
./configure; sudo make bootstrap
sudo luarocks install luasocket
```

### ZeroMQ (for distributed torch)
```bash
# download zeromq 3.2.5, then:
./autogen.sh && ./configure && make -j 4
make check && make install && sudo ldconfig
luarocks install https://raw.github.com/Neopallium/lua-zmq/master/rockspecs/lua-zmq-scm-1.rockspec
```

## 5. Pitfalls

- **Torch (Lua) is legacy** — new projects should use PyTorch (Python).
- **`install-deps` changes your system** (apt installs, env changes); review before running.
- **Lua 5.2 build** needs `libreadline-dev` or luarocks steps fail.
- **Linker**: after installing zeromq run `sudo ldconfig`.
- **`TORCH_LUA_VERSION=LUA52`** must be set for the 5.2 variant; default is LuaJIT 5.1.
