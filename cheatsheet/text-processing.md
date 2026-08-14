# Text Processing cheatsheet

Quick reference for string ops, search/sort, line-by-line file processing, looping over folders, splitting data + multithreading, random pick, and scripting.

## String
```
# substring
stringZ=abcABC123ABCabc
echo ${stringZ:7}                            # 23ABCabc
echo ${stringZ:7:3}                          # 23A
# replace '\t' with ' ':
cat a.txt | tr '\t' ' '
# or sed 's/old_string/new_string/g'
# replace ' ' with '':
cat a.txt | sed 's/ //g'
# first column of a csv/tsv:
cat ./myfile.tsv | awk '{print $1}'
# extract strings (incl. binary) from a file:
strings a.exe
strings ./001.txt --encoding={s,S}   # unicode strings
strings ./001.txt --encoding={s,S} > your_output_file
```

## Search / sort
```
cat ./spk2utt | cut -d ' ' -f 1 > spk2gender   # cut first column
sort -nk 12                                  # sort by column 12 (numeric)
sort -r flist.txt > rflist.txt
cat hub-140-f.data | sort -k 5,5 -k 2,2 -k 3,3n > hub-140-f.data.sorted
grep Mean exp/...scoring/*.txt.sys | sort -nk 12
grep WER exp/tri3b/.../wer_* | sort -nk 2
find . -name "words.txt"
grep -r word *                               # search text in files
find ./exp/... -name "*.sys" | xargs grep Mean | sort -nk 11
grep someword -R ./
grep 'pattern1\|pattern2' filename
grep -E 'Tech|Sales' employee.txt
egrep 'pattern1|pattern2' filename
grep -E 'pattern1.*pattern2' filename         # and
grep -v 'pattern1' filename                   # not
locate libname   # sudo apt install plocate then updatedb
```

## Read a file and process line by line
```
FILE=wav.list
ext=wav
outputfolder=seg_remove_cm
cat $FILE | while read line; do
    paste -d"," *.txt                        # paste files horizontally
    echo "Processing $line"
    nameOnly=$(echo $line | awk -F / '{ print $7 }')
    nameOnly=${nameOnly%.$ext}
    echo "NAME= $nameOnly"
    java -Xmx2048m -classpath ./batch.jar \
        edu.cmu.sphinx.tools.feature.FeatureFileDumper \
        -config ./frontend.config.xml -name cepstraFrontEnd \
        -i $line -o ./mfcc/$nameOnly.mfcc
done
```

## Loop over a folder, process one file at a time
```
# eg0: rename .html -> .txt
for file in *.html; do
   mv "$file" "$(basename "$file" .html).txt"
done
# eg1:
for file in ./input/gt_ctm/*.ctm
do
    echo "Processing $file"
    nameOnly=$(echo $file | awk -F "/" '{print $NF}')
    nameOnly=${nameOnly%.ctm}
    python my_validation.py "./output/gt_rttm/sorted_$nameOnly.ctm" "./output/gt_rttm/$nameOnly.rttm"
done
# eg2: loop over wav files
for ifile in <YOUR_HOME>/.../utts/*.wav
do
    show=`basename $ifile .wav`
    /usr/bin/java -Xmx2024m -jar ./LIUM_SpkDiarization-8.4.1.jar \
        --fInputMask=$ifile --sOutputMask=<YOUR_HOME>/.../$show.seg ZHIZHENGTEST &
done
# eg3: loop over an array
names=( Jennifer Tonya Anna Sadie )
for name in ${names[@]}; do echo $name; done
```

## Split data + multithreading
```
mkdir -p $outdir
split -d -n l/15 <YOUR_HOME>/.../magor_gt_merge_nodev.test.lst ${tid}
for x in ./${tid}*
do
    echo $x
    ./getgender.sh $x $outdir &
done
```

## Pick random files
```
find ./audio -type f | shuf -n 10
```

## Scripting
```
# a.sh 13 213 323   ->  inside a.sh: $1 $2 $3 ; & runs in background
v1=$1 v2=$2
pid=$(fuser - tcp 139 | awk '{print $1}')   # capture a script's output
```
