
1.1 read from ark, first thing first you need to know what data it is
	copy-vector ark:vad_NIST_SRE08_3summed.model.8.ark ark,t:- > vad.ark.t.text
1.2 write back to ark
	copy-vector ark:vad.ark.t.text ark:- >001.ark
	
	copy-feats scp:./data/ngmm/feats.scp ark,t:- | head


feats-to-dim ark:./input.ark ark,t:- | head -n 1


feats select 0 index



copy-vector ark:./ivector.1.ark ark,t: |sed -e 's/\[\|\]//g'





#Read/write Kaldi features

##Read Kaldi MFCC features

###Raw Feature location

Most Kaldi features are stored in mfcc folder with an extension of ark or scp.

####The ark file

The ark stores raw features, its size is normally in few hundred MBs.

Eg: 20 dimensional MFCC features matrix is stored in the ark file like following:

UtteranceID1 [d1 d2 d3 d4 d5 .. d20\n d1 d2 d3 d4 d5 .. d20\n d1 d2 d3 d4 d5 .. d20\n ...]\n

UtteranceID2 [d1 d2 d3 d4 d5 .. d20\n d1 d2 d3 d4 d5 .. d20\n ]\n

Where \n means new line.

To view raw feature, type the following command in the terminal
```
copy-feats ark:./abc.ark ark,t:
```
This command means copy the feature from input source (ark:source) to output target (ark,t:target),here we leave "target" empty so the feature will print to the terminal.
Following two commands will dump the features to text file
```
copy-feats ark:./abc.ark ark,t: > a.txt
copy-feats ark:./abc.ark ark,t:a.txt
```
And dump to binary file:
```
copy-feats ark:./abc.ark ark:a.bin
```


####The scp file

It is often saw a scp file which describes the content of an ark file.

The scp is only a text file, with following format:

UtteranceID1 arkLocation1:offset1 

UtteranceID2 arkLocation2:offset2




Following two commands will give same results
```
copy-feats scp:./abc.scp ark,t:
copy-feats ark:./abc.ark ark,t:
```
####Features in the data folder

feats.scp and vad.scp are two feature descriptors in the Kaldi data folder.


##Write kaldi MFCC features

One can write kaldi feature to the ark follow the given text format. However most script in Kaldi require its scp file, one way to create scp file is:
```
copy-feats ark:./abc.ark ark,scp:b.ark,b.scp
```


##Read/write kaldi iVector 
Read/write kaldi iVector is similar to read/write MFCC feature, only replace the copy-feats with copy-vector command.

###Read Kaldi iVector
The MFCC feature is matrix like data structure, the iVector is sorted in the ark file too, but in a vector format:

UtteranceID1 [d1 d2 d3 d4 d5 .. d400]\n

UtteranceID2 [d1 d2 d3 d4 d5 .. d400]\n

Similar to read MFCC feature,following command will read the iVector:
```
copy-vector ark:./ivector.1.ark ark,t:
copy-vector ark:./ivector.1.ark ark,t:YOUR_TEXT_FILE

```

###Write Kaldi iVector

Once you know the iVector format, you can write it back easily.
Following command will convert your ark file to Kaldi format, and generate a scp file.

```
copy-vector ark,t:./your.ark ark,t:

```