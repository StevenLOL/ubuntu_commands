# 在某国，github 经常打不开，有人说是DNS被污染了，这时候需要配置 hosts 文件来链接 github


## How to change hosts ?



## 1 下载安装 SwitchHosts

在Github刚好不抽风的时候，赶紧下载 SwitchHosts， https://github.com/oldj/SwitchHosts/releases
新建配置文件，选择远程 指定URL为：https://cdn.jsdelivr.net/gh/521xueweihan/GitHub520@main/hosts

则 会从云端拉取最新 github的配置，如：

···
# GitHub520 Host Start
140.82.113.25                 alive.github.com
140.82.113.25                 live.github.com
185.199.108.154               github.githubassets.com
140.82.112.21                 central.github.com
185.199.108.133               desktop.githubusercontent.com
185.199.108.153               assets-cdn.github.com
185.199.108.133               camo.githubusercontent.com
185.199.108.133               github.map.fastly.net
199.232.69.194                github.global.ssl.fastly.net
140.82.113.4                  gist.github.com
185.199.108.153               github.io
140.82.112.3                  github.com
192.0.66.2                    github.blog
140.82.113.5                  api.github.com
185.199.108.133               raw.githubusercontent.com
185.199.108.133               user-images.githubusercontent.com
185.199.108.133               favicons.githubusercontent.com
185.199.108.133               avatars5.githubusercontent.com
185.199.108.133               avatars4.githubusercontent.com
185.199.108.133               avatars3.githubusercontent.com
185.199.108.133               avatars2.githubusercontent.com
185.199.108.133               avatars1.githubusercontent.com
185.199.108.133               avatars0.githubusercontent.com
185.199.108.133               avatars.githubusercontent.com
140.82.112.9                  codeload.github.com
52.217.49.132                 github-cloud.s3.amazonaws.com
52.217.173.121                github-com.s3.amazonaws.com
52.216.96.243                 github-production-release-asset-2e65be.s3.amazonaws.com
52.217.174.129                github-production-user-asset-6210df.s3.amazonaws.com
52.216.143.172                github-production-repository-file-5c1aeb.s3.amazonaws.com
185.199.108.153               githubstatus.com
64.71.144.202                 github.community
23.100.27.125                 github.dev
185.199.108.133               media.githubusercontent.com


# Update time: 2021-11-19T00:05:31+08:00
# Update url: https://raw.hellogithub.com/hosts
# Star me: https://github.com/521xueweihan/GitHub520
# GitHub520 Host End
···
## 2 firfox + livehosts

手工添加上述内容

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
