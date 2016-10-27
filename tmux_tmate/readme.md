
#Tmux
```
      tmux
      tmux list-sessions
      tmux attach-session -t #
      #enable mouse model: in ~/.tmux.conf
      setw -g mode-mouse on
      or setw -g mouse on
      then tmux source-file ~/.tmux.conf
      #when in tmux
      #switch and list tmux session
      ctrl-b s
      #detach session
      ctrl-b d
```      
      
      
#Tmate
```
sudo apt-get install software-properties-common && \
sudo add-apt-repository ppa:tmate.io/archive    && \
sudo apt-get update                             && \
sudo apt-get install tmate
```
show log/message again:

press prefix + ~ to list all previous tmux/tmate messages
press prefix + : to get tmux prompt. Then type show-messages command to get the same results as in 1).

prefix+b then pagedown/pageup


##Attach tmate session from loacal:
```
tmate -S /tmp/tmate-1002/x9fOaz attach
```
