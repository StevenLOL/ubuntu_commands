
#install openvpn

```
sudo apt-get install openvpn
```

#login via web interface and download connection profiles, it maybe a file named *.ovpn


Then in your client side , if you are using UBUNTU:

```
sudo openvpn ~/client.ovpn
```

To check your new IP:

https://www.whatismyip.com/


```
    openvpn /etc/openvpn/client.ovpn > /dev/null &



    /etc/rc.local
```

Ref: http://www.linuxidc.com/Linux/2013-06/86562.htm
