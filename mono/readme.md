Install monodevelop 7.1 via flatpak
```
sudo add-apt-repository ppa:alexlarsson/flatpak
sudo apt update
sudo apt install flatpak

flatpak install --user --from https://download.mono-project.com/repo/monodevelop.flatpakref
flatpak run com.xamarin.MonoDevelop
```
ref:http://www.monodevelop.com/download/linux/
