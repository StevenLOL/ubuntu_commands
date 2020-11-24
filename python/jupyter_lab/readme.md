
https://www.cnblogs.com/jscs/p/13724589.html

```
jupyter lab --ip 0.0.0.0 --allow-root

```


if tqdm not show up 
HBox(children=(HTML(value=''), FloatProgress(value=0.0, max=1992.0), HTML(value='')))
One have to install widgetsnbextension
```
jupyter nbextension enable --py widgetsnbextension 
jupyter labextension install @jupyter-widgets/jupyterlab-manager
```




