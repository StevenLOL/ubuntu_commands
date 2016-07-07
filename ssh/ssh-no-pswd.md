
ssh-keygen -t rsa
ssh steven@155.69.149.215 mkdir -p .ssh
cat .ssh/id_rsa.pub | ssh steven@155.69.149.215 'cat >> .ssh/authorized_keys'
ssh steven@155.69.149.215




### for git hub or bitbucket

#1 generate ssh key by
```
ssh-keygen
```
There will be two files: one private key and one public key (*.pub)
#2 upload public to public...
then copy the text in *.pub and to github or bitbucket

#3 load private key locally:

```
ssh-agent bash
eval ssh-agent -s
ssh-add PATH_TO_YOUR_PRIVATE_KEY
```







https://confluence.atlassian.com/bitbucket/set-up-ssh-for-git-728138079.html
Set up SSH for Git
If you came to this page because you don't have SSH set up, then you have been using the secure hypertext transfer protocol (HTTPS) to communicate between your local system and Bitbucket Cloud. When you use HTTPS, you authenticate (supply a username and password) each time you take an action that requires a connection with Bitbucket. Who wants to do that? This page shows you how to use secure shell (SSH) to communicate with the Bitbucket server and avoid having to manually type a password all the time.
Set up SSH
Setting up an SSH identity can be prone to error. Allow yourself some time, perhaps as much as an hour depending on your experience, to complete this page. If you run into issues, check out Troubleshoot SSH Issues for extra information that may help you along. You can even skip this whole page and continue to use HTTPS if you want.
The following sections cover how to set up SSH for Git.
  Set up SSH for Windows
  Set up SSH for Mac OS/Linux
To use SSH with Bitbucket, you create an SSH identity. An identity consists of a private and a public key which together are a key pair. The private key resides on your local computer and the public you upload to your Bitbucket account. Once you upload a public key to your account, you can use SSH to connect with repositories you own and repositories owned by others, provided those other owners give your account permissions. By setting up SSH between your local system and the Bitbucket server, your system uses the key pair to automate authentication; you won't need to enter your password each time you interact with your Bitbucket repository.
There are a few important concepts you need when working with SSH identities and Bitbucket.
You cannot reuse an identity's public key across accounts. If you have multiple Bitbucket accounts, you must create multiple identities and upload their corresponding public keys to each individual account. 
You can associate multiple identities with a Bitbucket account. You would create multiple identities for the same account if, for example, you access a repository from a work computer and a home computer. You might create multiple identities if you wanted to execute DVCS actions on a repository with a script  the script would use a public key with an empty passphrase, allowing it to run without human intervention.
RSA (R. Rivest, A. Shamir, L. Adleman are the originators) and digital signature algorithm (DSA) are key encryption algorithms. Bitbucket supports both types of algorithms. You should create identities using whichever encryption method is most comfortable and available to you.
Step 1. Ensure you have an SSH client installed
SSH is most likely included with your version of Mac OS or Linux. To make sure, do the following to verify your installation:
From your terminal window, enter the following command, which identifies the version of SSH you have installed.
If SSH is installed, you see something similar to the following:
$ ssh -v 
OpenSSH_5.6p1, OpenSSL 0.9.8r 8 Feb 2011
usage: ssh [-1246AaCfgKkMNnqsTtVvXxYy] [-b bind_address] [-c cipher_spec]
           [-D [bind_address:]port] [-e escape_char] [-F configfile]
           [-I pkcs11] [-i identity_file]
           [-L [bind_address:]port:host:hostport]
           [-l login_name] [-m mac_spec] [-O ctl_cmd] [-o option] [-p port]
[-R [bind_address:]port:host:hostport] [-S ctl_path]
           [-W host:port] [-w local_tun[:remote_tun]]
           [user@]hostname [command]>
If you have  ssh  installed, the terminal returns version information. 
If you don't have ssh installed, install it now.
List the contents of your ~/.ssh directory.
If you don't have an .ssh directory, don't worry, you'll create it the next section. If you have a .ssh directory or you may see something like this:
$ ls -a ~/.ssh 
known_hosts
If you have defined a default identity, you'll see the two id_* files:
$ ls -a ~/.ssh 
.        ..        id_rsa        id_rsa.pub    known_hosts
In this case, the default identity used RSA encryption (id_rsa.pub). If you want to use an existing default identity for your Bitbucket account, skip the next section and go to start the ssh-agent and load your keys.
Step 2. Set up your default identity
By default, the system adds keys for all identities to the /Users/<yourname>/.ssh directory on Mac OSX, or /home/<yourname>/.ssh on Linux. This procedure creates a default identity. If you have a default identity and you want to use it for Bitbucket, skip this step and go to start the ssh-agent and load your keys. If you have an existing default identity but you forgot the passphrase, you can also use this procedure to overwrite your default identity and create a fresh one.
Want to Use Multiple Identities?
You can create multiple SSH identities. Doing this is an advanced topic and beyond the scope of this tutorial. For information on how to create multiple identities, see Configure multiple SSH identities for GitBash, Mac OSX, & Linux.
Use the following procedure to create a new default identity.
Open a terminal in your local system.
Enter ssh-keygen at the command line.
The command prompts you for a file where you want to save the key. If the .ssh directory doesn't exist, the system creates one for you.
$ ssh-keygen 
Generating public/private rsa key pair.
Enter file in which to save the key (/Users/emmap1/.ssh/id_rsa):
Press the Enter or Return key to accept the default location.
Enter and re-enter a passphrase when prompted.
Unless you need a key for a process such as script, you should always provide a passphrase. The command creates your default identity with its public and private keys. The whole interaction will look similar to the following:
$ ssh-keygen 
Generating public/private rsa key pair.
Enter file in which to save the key (/Users/emmap1/.ssh/id_rsa):
Created directory '/Users/emmap1/.ssh'.
Enter passphrase (empty for no passphrase):
Enter same passphrase again:
Your identification has been saved in /Users/emmap1/.ssh/id_rsa.
Your public key has been saved in /Users/emmap1/.ssh/id_rsa.pub.
The key fingerprint is:
4c:80:61:2c:00:3f:9d:dc:08:41:2e:c0:cf:b9:17:69 emmap1@myhost.local
The key's randomart image is:
+--[ RSA 2048]----+
|*o+ooo.          |
|.+.=o+ .         |
|. *.* o .        |
| . = E o         |
|    o . S        |
|   . .           |
|     .           |
|                 |
|                 |
+-----------------+
List the contents of ~/.ssh to view the key files.
$ ls -a ~/.ssh
Step 3. Start the ssh-agent and load your keys
If you are running OSX 10.6.8 or later you can skip this step.  The OSX 10.6.8 system asks for your connection parameters the first time you try to establish a SSH connection.  Then, it automatically starts the ssh-agent for you. If you don't have OSX 10.6.8 or are running another Linux operating system, do the following:
Open a terminal window and enter the ps -e | grep [s]sh-agent command to see if the agent is running.
$ ps -e | grep [s]sh-agent 
 9060 ?? 0:00.28 /usr/bin/ssh-agent -l
If the agent isn't running, start it manually with the following command:
$ ssh-agent /bin/bash
Load your new identity into the ssh-agent management program using the ssh-add command.
$ ssh-add ~/.ssh/id_rsa 
Enter passphrase for /Users/emmap1/.ssh/id_rsa:
Identity added: /Users/emmap1/.ssh/id_rsa (/Users/emmpa1/.ssh/id_rsa)
Use the ssh-add command to list the keys that the agent is managing.
$ ssh-add -l 
2048 7a:9c:b2:9c:8e:4e:f4:af:de:70:77:b9:52:fd:44:97 /Users/manthony/.ssh/id_rsa (RSA)
Step 4. Install the public key on your Bitbucket account
From Bitbucket Cloud, choose avatar > Settings from the application menu. 
The system displays the Account settings page.
Click SSH keys.
The SSH Keys page displays. If you have any existing keys, those appear on this page.
Back in your terminal window, copy the contents of your public key file.
For example, in Linux you can cat the contents.
$ cat ~/.ssh/id_rsa.pub
In Mac OSX, the following command copies the output to the clipboard:
$ pbcopy < ~/.ssh/id_rsa.pub
Back in your browser, enter a Label for your new key, for example, Default public key.
Paste the copied public key into the SSH Key field
