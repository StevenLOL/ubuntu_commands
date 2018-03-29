
# matplotlib 图例中文乱码, how to display Chinese chars in matplotlib




## Download chinese true type font
## Copy to your matplotlib/mpl-data/fonts/ttf/
if you have no idear where matplotlib is:
```
#python
import matplotlib
print(matplotlib.__file__)
```
## clean cache
```
  rm -rf ~/.matplotlib/*.cache
  #or
  rm -rf ~/.cache/matplotlib/*.cache
```
 
 # usage
 ```
import matplotlib.pyplot as plt
plt.rcParams['font.sans-serif']=['SimHei']
plt.subplots(figsize=(18, 10)) 
plt.title('中文')
plt.show()
 ```
 Now you should see chinese chars instead of '口口' in the title.
 
 ### ref https://www.zhihu.com/question/25404709
 
