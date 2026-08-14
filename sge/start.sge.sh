# Sun Grid Engine (SGE) cluster setup

## 1. What is it / What is it for?

Sun Grid Engine (SGE, packaged as `gridengine` on Debian/Ubuntu) is a job-scheduler / batch-queue system for HPC clusters. This note installs a master + exec nodes, submits jobs, and monitors them with `qstat`/`qmon`. **`start.sge.sh` is a one-node quick-start.**


- Distributing batch jobs across cluster nodes.
- Queuing, resource accounting, and monitoring via `qsub`/`qstat`/`qmon`.

## 2. How to download / install

**Single node (start.sge.sh):**
```bash
sudo apt-get install gridengine-client gridengine-common gridengine-master gridengine-qmon
sudo apt-get install gridengine-exec

sudo nano /var/lib/gridengine/wslg/common/act_qmaster   # set master hostname

sudo /etc/init.d/gridengine-exec start
sudo /etc/init.d/gridengine-master start
```

**Multi-node cluster** (from the detailed note):
1. Java is required (historically Sun Java 6):
   ```bash
   sudo apt-get install python-software-properties
   sudo add-apt-repository ppa:ferramroberto/java
   sudo apt-get update && sudo apt-get install sun-java6-jre
   ```
2. Edit `/etc/hosts` on every node (master = `grid00`, exec = `grid01`/`grid02`):
   ```text
   127.0.0.1 localhost.localdomain localhost
   <SERVER_IP> grid00.linuxidc.com grid00
   <SERVER_IP> grid01.linuxidc.com grid01
   <SERVER_IP> grid02.linuxidc.com grid02
   ```
3. On master: `sudo apt-get install gridengine-client gridengine-common gridengine-master`.
4. On exec nodes: `sudo apt-get install gridengine-exec`.
5. On master, register exec hosts: `qconf -ah grid01` / `qconf -ah grid02`.
6. On each exec node: `echo "grid00.linuxidc.com" > /var/lib/gridengine/default/common/act_qmaster` then start `gridengine-exec`.
7. Verify with `qhost`.

## 3. How to use

```bash
qsub hello_world.sh &     # submit a job
qstat -f                  # show queue state
qmod                      # modify jobs/queue
qmon                      # GUI monitor (needs X11 fonts)
```

If `qmon` fonts are missing:
```bash
cd /usr/share/fonts/X11/100dpi/ ; sudo mkfontdir
cd /usr/share/fonts/X11/75dpi/  ; sudo mkfontdir
xset fp+ /usr/share/fonts/X11/75dpi
```

