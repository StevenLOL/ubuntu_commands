# Mono / MonoDevelop (.NET on Linux)

## 1. What is it?

**Mono** is an open-source implementation of Microsoft's .NET Framework, letting you run/compile C# and other .NET apps on Linux. **MonoDevelop** is the cross-platform IDE for it (now succeeded by Visual Studio for Mac). This folder also holds build scripts.

## 2. What is it for?

- Running .NET applications on Ubuntu.
- Developing C# apps in MonoDevelop.
- Hosting ASP.NET apps under Apache (`mod_mono`).

## 3. How to download / install

**Runtime + IDE (apt):**
```bash
sudo apt-get install mono-runtime mono-complete monodevelop \
  build-essential automake checkinstall intltool \
  mono-addins-utils gtk-sharp2 gnome-sharp2
```
(See `runtime-install.sh` in this folder.)

**MonoDevelop 7.1 via Flatpak** (see `readme.md` snippet):
```bash
sudo add-apt-repository ppa:alexlarsson/flatpak
sudo apt update && sudo apt install flatpak
flatpak install --user --from https://download.mono-project.com/repo/monodevelop.flatpakref
flatpak run com.xamarin.MonoDevelop
```
Ref: https://www.monodevelop.com/download/linux/

**Build MonoDevelop from source** (see `mono-developer-git-clone.sh`):
```bash
sudo git clone github.com/mono/monodevelop.git
cd monodevelop
sudo git submodule init && sudo git submodule update
sudo apt-get install mono-gmcs monodevelop
```

## 4. How to use

```bash
mcs hello.cs            # compile C#
mono hello.exe          # run it
```

ASP.NET on Apache uses `mod_mono` (see `mono_on_apache.md`).

## 5. Pitfalls

- **Mono is largely legacy** — Microsoft's own .NET (now ".NET") runs on Linux natively; prefer it for new work.
- **`mono-gmcs` is obsolete** (the old compiler); modern Mono uses `mcs`/`csc`.
- **Flatpak path**: the `readme.md` Flatpak line uses `develop.mono-project.com` via `alexlarsson/flatpak` PPA — verify it's still live.
- **`Task.sh`** in this folder is unrelated (a Kaldi/sgmm script); ignore it for Mono.
