# 文本处理 / Text Processing

## 字符串 / string
```
#substring
stringZ=abcABC123ABCabc
echo ${stringZ:7}                            # 23ABCabc
echo ${stringZ:7:3}                          # 23A
# Three characters of substring
#replace '\t' with ' ':
cat a.txt | tr '\t' ' '
or sed 's/old_string/new_string/g'
#replace ' ' with '':
cat a.txt | sed 's/ //g'
#get the first row of a csv or tsv file:
cat ./myfile.tsv | awk '{print $1}'
#get strings in any files eg text/binary files
strings a.exe
strings ./001.txt --encoding={s,S}   # get the unicode string
strings ./001.txt --encoding={s,S} >  your_output_file
```

## 搜索 / 排序 / search sort
```
cat ./spk2utt | cut -d ' ' -f 1 > spk2gender #cut the first coloum
sort -nk 12 sort by 12 colloums
sort -r flist.txt > rflist.txt
cat hub-140-f.data | sort -k 5,5 -k 2,2 -k 3,3n > hub-140-f.data.sorted    #sort by 5,2,3 3 in digit format
grep Mean exp/sgmm2_5a/decode_graph_tg222pr_24h.new.crf_fmllr/scoring/*.txt.sys | sort -nk 12
grep Mean exp/sgmm2_5a/decode_graph_tg222pr_24h.new.crf/scoring/*.txt.sys | sort -nk 12
grep WER exp/tri3b/decode_tgpr_eval24hr.si/wer_* | sort -nk 2
#search
find . -name "words.txt"
#search text in files
grep -r word *
#find and grep
find ./exp/tri3b/decode_tgpr_eval6hr_rttm14Aug13/ -name "*.sys" | xargs grep Mean | sort -nk 11
#search include sub directory
grep someword -R ./
grep 'pattern1\|pattern2' filename  #or
grep -E 'Tech|Sales' employee.txt   #or
egrep 'pattern1|pattern2' filename  #or
grep -E 'pattern1.*pattern2' filename #and
grep -v 'pattern1' filename           #not
locate libname # sudo apt install locate then updatedb
```

## 逐行读文件处理 / read text file and process line by lines
```
FILE=wav.list
ext=wav
outputfolder=seg_remove_cm
cat $FILE | while read line; do
#cat file horizontally
paste -d"," *.txt
    echo "Processing $line"
    nameOnly=$(echo $line | awk -F / '{ print $7 }')
    nameOnly=${nameOnly%.$ext}
    echo "NAME= $nameOnly"
    java -Xmx2048m -classpath ./batch.jar \
        edu.cmu.sphinx.tools.feature.FeatureFileDumper \
        -config ./frontend.config.xml \
        -name cepstraFrontEnd \
        -i $line \
        -o ./mfcc/$nameOnly.mfcc
done
```

## 遍历文件夹逐个处理 / read a folder and process file one by one
```
#loop and find names
eg0:
for file in *.html; do
   mv "$file" "$(basename "$file" .html).txt"
done

eg1:
for file in ./input/gt_ctm/*.ctm
do
    echo "Processing $file"
    nameOnly=$(echo $file | awk -F "/" '{print $NF}')
    nameOnly=${nameOnly%.ctm}
    python my_validation.py "./output/gt_rttm/sorted_$nameOnly.ctm" "./output/gt_rttm/$nameOnly.rttm"
done

#eg2: loop dir
for ifile in <YOUR_HOME>/Dropbox/workspace/magor/magor_allinone/5.sv/ivector/utts/*.wav
do
    show=`basename $ifile .wav`
    echo $show
    /usr/bin/java -Xmx2024m -jar ./LIUM_SpkDiarization-8.4.1.jar \
        --fInputMask=$ifile --sOutputMask=<YOUR_HOME>/Dropbox/workspace/magor/magor_allinone/5.sv/ivector/utts_gender/$show.seg ZHIZHENGTEST &
done

#eg3: loop vars
names=( Jennifer Tonya Anna Sadie )
for name in ${names[@]}
do
    echo $name
done
```

## 拆分数据 + 多线程 / split data and multithreading
```
mkdir -p $outdir
split -d -n l/15 <YOUR_HOME>/apps/getgender/lium/magor_gt_merge_nodev.test.lst ${tid}
for x in ./${tid}*
do
    echo $x
    ./getgender.sh $x $outdir &
done
```

## 随机抽取文件 / pick random files
```
find ./audio -type f | shuf -n 10
```

## 脚本参数 / scripting
```
#a.sh 13 213 323
#in a.sh  $1 $2 $3 & will run in background
#in a.sh v1=$1 v2=$2
pid=$(fuser - tcp 139 | awk '{print $1}')  # get output of a script
```
