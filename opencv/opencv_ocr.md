
ubuntu安装tesseract 进行OCR识别

    博客分类： ocr 

 

之前使用 sudo apt-get install tesseract-ocr 安装的tesseract-ocr有问题，不能使用psm参数。决定手动编译安装。下面参考别人的安装过程。
安装所需的库
复制代码

sudo apt-get install libpng12-dev
sudo apt-get install libjpeg62-dev
sudo apt-get install libtiff4-dev

sudo apt-get install gcc
sudo apt-get install g++
sudo apt-get install automake

复制代码

 

pytesser 调用了 tesseract，因此需要安装 tesseract，安装 tesseract 需要安装 leptonica，否则编译tesseract 的时候出现 "configure: error: leptonica not found"。

 

以下都是解压编译安装的老步骤：

./configure
make -j4
sudo make install

 
下载安装leptonica

http://www.leptonica.org/download.html 或者

http://code.google.com/p/leptonica/downloads/list

最新的是leptonica-1.69.tar.bz2

 
下载安装tesseract

http://code.google.com/p/tesseract-ocr/

最新的是 tesseract-ocr-3.02.02.tar.gz

 
下载安装 tesseract 的语言数据包

http://code.google.com/p/tesseract-ocr/downloads/list

最新的是 tesseract-ocr-3.01.eng.tar.gz

解压tessdata目录下的文件（9个）到 "/usr/local/share/tessdata"目录下

注意：这个网址下载到的只有一个，不能用，使用中会报错，http://tesseract-ocr.googlecode.com/files/eng.traineddata.gz

 
下载安装 pytesser

http://code.google.com/p/pytesser/

最新的是 pytesser_v0.0.1.zip 

 
测试pytesser

到pytesser的安装目录，创建一个test.py，python test.py 查看结果。
复制代码

from pytesser import *
#im = Image.open('fnord.tif')
#im = Image.open('phototest.tif')
#im = Image.open('eurotext.tif')
im = Image.open('fonts_test.png')
text = image_to_string(im)
print text

复制代码

tesseract 目录还有其他tif文件，也可以复制过来测试，上面测试的tif，png文件正确识别出文字。

 

pytesser的验证码识别能力较低，只能对规规矩矩不歪不斜数字和字母验证码进行识别。测试了几个网站的验证码，显示 Empty page，看来用它来识别验证码是无望了。

测试发现提高对比度后再识别有助于提高识别准确率。

enhancer = ImageEnhance.Contrast(im)
im = enhancer.enhance(4)

 
错误记录

1.遇到这个错误

$ tesseract foo.png bar
tesseract: error while loading shared libraries: libtesseract_api.so.3 cannot open shared object file: No such file or directory

 

You need to update the cache for the runtime linker. The following should get you up and running:

$ sudo ldconfig

 

2. 使用遇到如下错误

Tesseract Open Source OCR Engine v3.02.02 with Leptonica
Error in findTiffCompression: function not present
Error in pixReadStreamTiff: function not present
Error in pixReadStream: tiff: no pix returned
Error in pixRead: pix not read
Unsupported image type.

坑爹居然是leptonica1.69的bug，参考：https://bugs.mageia.org/show_bug.cgi?id=10411#c4，坑爹的是居然没有找到组新修复版，不得已我使用leptonica-1.68，可以了。

 

安装成功的提示：

yunpeng@yunpeng-duitang:~/下载/tesseract-temp$ tesseract
Usage:tesseract imagename outputbase [-l lang] [-psm pagesegmode] [configfile...]

pagesegmode values are:
0 = Orientation and script detection (OSD) only.
1 = Automatic page segmentation with OSD.
2 = Automatic page segmentation, but no OSD, or OCR
3 = Fully automatic page segmentation, but no OSD. (Default)
4 = Assume a single column of text of variable sizes.
5 = Assume a single uniform block of vertically aligned text.
6 = Assume a single uniform block of text.
7 = Treat the image as a single text line.
8 = Treat the image as a single word.
9 = Treat the image as a single word in a circle.
10 = Treat the image as a single character.
-l lang and/or -psm pagesegmode must occur before anyconfigfile.

Single options:
  -v --version: version info
  --list-langs: list available languages for tesseract engine

 

参考：

http://www.oschina.net/question/54100_59400

http://ubuntuforums.org/showthread.php?p=10248384

====================================================================================

 

使用pytesser遇到问题：
Traceback (most recent call last):
  File "test.py", line 15, in <module>
    pytesser.image_to_string(data)
  File "/home/jose/Documentos/Geek/pytesser.py", line 31, in image_to_string
    call_tesseract(scratch_image_name, scratch_text_name_root)
  File "/home/jose/Documentos/Geek/pytesser.py", line 21, in call_tesseract
    proc = subprocess.Popen(args)
  File "/usr/lib/python2.7/subprocess.py", line 672, in __init__
    errread, errwrite)
  File "/usr/lib/python2.7/subprocess.py", line 1213, in _execute_child
    raise child_exception
OSError: [Errno 2] No such file or directory

参考：http://code.google.com/p/pytesser/issues/detail?id=2

需要安装tesseract-ocr
sudo apt-get install tesseract-ocr


使用： tesseract test.jpg -l  eng
Tesseract是图盲，默认情况下只能看得懂未压缩的TIFF图像，如果直接用tesseract处理其它格式的图片，会报错如下：
name_to_image_type:Error:Unrecognized image type:test.jpg
IMAGE::read_header:Error:Can't read this image type:test.jpg
tesseract:Error:Read of file failed:test.jpg

所以我们需要用ImageMagick来转换图片格式。
安装ImageMagick:
sudo apt-get install imagemagick

使用imagemagick转换图片格式：
convert -compress none -depth 8 -alpha off  test.jpg  test.tif

使用tesseract,结果保存在out.txt中
tesseract test.tif out

问题：如何让tesseract只识别字母或者数字？
编辑：sudo vi /usr/share/tesseract-ocr/tessdata/configs/digits
tessedit_char_whitelist abcdefghijklmnopqrstuvwxyz0123456789
执行：
tesseract 1.tif out nobatch digits


附录:
Usage:tesseract imagename outputbase [-l lang] [-psm pagesegmode] [configfile...]
pagesegmode values are:
0 = Orientation and script detection (OSD) only.
1 = Automatic page segmentation with OSD.
2 = Automatic page segmentation, but no OSD, or OCR
3 = Fully automatic page segmentation, but no OSD. (Default)
4 = Assume a single column of text of variable sizes.
5 = Assume a single uniform block of vertically aligned text.
6 = Assume a single uniform block of text.
7 = Treat the image as a single text line.
8 = Treat the image as a single word.
9 = Treat the image as a single word in a circle.
10 = Treat the image as a single character.
-l lang and/or -psm pagesegmode must occur before anyconfigfile.


tesseract imagename outputbase [-l lang] [-psm pagesegmode] [configfile...]

tesseract    图片名  输出文件名 -l 字库文件 -psd pagesegmode 配置文件
例如：
tesseract code.jpg result  -l chi_sim -psd 7 nobatch
-l chi_sim 表示用简体中文字库（需要下载中文字库文件，解压后，存放到tessdata目录下去,字库文件扩展名为.raineddata，简体中文字库文件名为:  chi_sim.traineddata）
-psd 7 表示告诉tesseract code.jpg图片是一行文本  这个参数可以减少识别错误率.  默认为 3
configfile 参数值为tessdata\configs 和  tessdata\tessconfigs 目录下的文件名
