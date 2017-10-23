# sox
sox is a command line tool for audio file format conversion, and audio recoding/play.
## install
```
sudo apt-get install lame
sudo apt-get install libsox-fmt-mp3
sudo apt-get install sox
```
## format conversion

```
#covnert mp3 to wav format, resampleing frequency 16khz , mono channel

sox my-audio.mp3 -r 16000 -c 1 mywav.wav
``

## others

sudo apt-get install libasound2-plugins libasound2-python libsox-fmt-all

REF: http://quadloops.com/installing-sox-in-ubuntu-12-04-and-working-with-ogg-and-vorbis-formats/
