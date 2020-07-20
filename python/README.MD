


# Code completion in interactive console (python)

```
sudo pip install readline rlcompleter3
```
Create a file .pythonrc
```
# ~/.pythonrc
# enable syntax completion
try:
    import readline
except ImportError:
    print("Module readline not available.")
else:
    import rlcompleter
    readline.parse_and_bind("tab: complete")
```
then in your .bashrc file, add

```
export PYTHONSTARTUP=~/.pythonrc
```
REF:

http://stackoverflow.com/questions/246725/how-do-i-add-tab-completion-to-the-python-shell


## More packages @ 
```
https://www.lfd.uci.edu/~gohlke/pythonlibs/
```


# the whl
The whl file is a zip file.
