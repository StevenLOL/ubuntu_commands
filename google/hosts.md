# 在某国，github 经常打不开，有人说是DNS被污染了，这时候需要配置 hosts 文件来链接 github


## How to change hosts ?

## 1 firfox + livehosts

## 2 下载安装 SwitchHosts

在Github刚好不抽风的时候，赶紧下载 SwitchHosts， 常用切 host 软件。
其中 URL 填写：https://cdn.jsdelivr.net/gh/521xueweihan/GitHub520@main/hosts

# reCAPTCHA打不开的解决方法


reCAPTCHA是广泛使用的验证码，但由于一些原因在某国无法使用。

观察使用reCAPTCHA的网站，发现验证码无法加载是因为无法加载如下JavaScript文件：

    https://www.google.com/recaptcha/api.js

仅此而已。同时我们知道，该JavaScript文件的另一个地址是：

    https://recaptcha.net/recaptcha/api.js

使用火狐 插件 Gooreplacer

将 https://www.google.com/recaptcha/api.js  重定向 为 https://recaptcha.net/recaptcha/api.js 即可




# ref： 
https://www.jianshu.com/p/7dce995dda9e
https://www.cnblogs.com/mouseleo/p/11896578.html
