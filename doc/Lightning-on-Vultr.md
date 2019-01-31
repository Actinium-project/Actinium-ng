## How to compile and run a Actinium Lightning node on a vServer using Vultr

This howto describes the basic steps to setup an Actinium Lightning node on a vServer offered from hosting companies like [vultr.com](https://www.vultr.com). 
Any other hosting provider with similiar setup should work as long as it is the same Operating system and meets the minimal standards like ssh access.

### Prequesites:

You can adminster the server from any operating system on your PC, as long as you have  a SSH client. Ideally, you already have some experience with the Linux bash, but even as a beginner to Linux, it should be possible for you to setup the server.
Both Windows and macOS offer ssh clients, [putty](https://www.putty.org/)  being the most popular for Windows. 
You need to own some ACM to fund the channels with your node, so purchase some at exchanges who [offer ACM.](https://coinmarketcap.com/currencies/actinium/#markets) 



### Basic setup of the vServer:

Register at [vultr.com](https://www.vultr.com)  and send in an initial funding to create a Vultr Cloud Computer vServer (VC2) instance. I have chosen the minimum of 10$ to cover the costs for 2 months. Chose a location that fits you best and as Operating System you should chose **Ubuntu 18.04 x64**.

I think the 5$ package with 25 GB SSD storage, 1GB RAM and 1TB bandwith the most suitable for a low price to do the job, though there might be the need for higher RAM, especially while compiling - but to that later.
Finally fill in  a Server hostname at point 7 and click **deploy now**.

The server will be up and running in a short time, you can check it´s status at the menu **Servers**. Click on the server caption or the three horizontal dots to view the server details of your server. Copy your IP address and password for root and save it, you will need it later.
I will use "serverIP" for your IP address.

- Connect to the server using ssh (or putty in Windows)
~~~bash
ssh root@serverIP
~~~
- Initial update of the software using apt and cleanup to save space:
~~~bash
apt update -y
apt upgrade -y
apt clean
~~~

- 	Your vServer only has root available, we need to create a user as we do not want to run the daemons with root powers, we will create a user **foo** but you should chose your own username, always in lowercase and we will provide that user with sudo power.  The process is self-explaining, just use a strong password and write it down somewhere, maybe fill in the full name, all other fields can be left empty.

~~~bash
adduser foo
usermod -a -G sudo foo
~~~
- Additionally we will create a swapfile, because I ran out of RAM during compilation, you could prevent that step by purchasing a vServer with more RAM. You can skip this step if you have purchased a vServer with 2 GB RAM or higher.
~~~bash
fallocate -l 1G /swapfile
chmod 600 /swapfile
mkswap /swapfile
swapon /swapfile
echo "/swapfile swap swap defaults 0 0" >> /etc/fstab
~~~

- You have to restart your server. You can either do it via [Vultr Webpage](https://my.vultr.com) or through ssh:

~~~bash
reboot
~~~
- If everything went like expected, your server should be up and running again. Remember you can always restart from scratch by deleting the server instance and creating a new one at the vultr website.
  From now on, we will ssh only with the user **foo** and your chosen password.
~~~bash
ssh foo@serverIP
~~~


### Compilation and configuration of Actinium-ng:
- Now we install all necessary packages for compilation, add a ppa for Berkeley DB4.8 and install it:
~~~bash
sudo apt install -y \
build-essential libtool autotools-dev git automake pkg-config bsdmainutils python3 \
libssl-dev libevent-dev libboost-system-dev libboost-filesystem-dev \
libboost-chrono-dev libboost-test-dev libboost-thread-dev zlib1g-dev \
libgmp-dev libsqlite3-dev python net-tools

sudo apt install software-properties-common
sudo add-apt-repository ppa:bitcoin/bitcoin
sudo apt update
sudo apt install libdb4.8-dev libdb4.8++-dev
~~~
- Now we setup our basic working directories. Depending on the username you have chosen, it will be /home/**foo** in our case.
Then we will clone our first project from git -  Actinium-ng:

~~~bash
cd ~
mkdir src
cd src
git clone https://github.com/Actinium-project/Actinium-ng.git
cd Actinium-ng
./autogen.sh
./configure --without-gui --disable-shared --with-miniupnpc=no
~~~
- If no error message was displayed, we can go on and start the make process. It will take a lot of time to compile. 

~~~
make
~~~
- Occasionally you might face some linker errors with seccomp & cap. If that is the case, use this configure line instead: 

  ```bash
  ./configure LIBS="-lcap -lseccomp" --without-gui --disable-shared --with-miniupnpc=no
  
  make
  ```

- If the compile process has finished without any error, it is time to install the binaries:
~~~
sudo make install
~~~
- The standard working directory for Actinium-ng is ~/.actinium. As we wish to configure our own configuration, we will create that directory and place a config file for Actinium-ng in it:
~~~bash
cd ~
mkdir .actinium
cd .actinium
nano Actinium.conf
~~~
- An empty file will appear, which we have to fill with the following text in the next block. 
  Set **youruser** and **yourpass** to a user/pass combination you would like. Do not use your ssh credentials ! To save press CTRL+o and CTRL+x
~~~bash
rest=1
port=4334
txindex=1
rpcuser=youruser
rpcpassword=yourpass
rpcport=2300
rpcallowip=127.0.0.1
shrinkdebugfile=1
debug=0
torenabled=0
~~~
- Now it´s time to test the daemon and the cli:
~~~bash
Actiniumd
~~~
- You should see a daemon output now, just watch the lines for any errors. To stop the daemon, login with another ssh shell and do a:
~~~bash
Actinium-cli -getinfo
Actinium-cli stop
~~~

### Compilation and configuration of acm-lightning:

- We change back to our compilation directory ~/src and start pulling and compiling acm-lightning:
~~~
cd ~/src
git clone https://github.com/Actinium-project/acm-lightning.git
cd acm-lightning
./configure
make
sudo make install
~~~
- Now we have to place a config file for Lightning into the default working dir and match rpcuser and rpcpassword like in the Actinium.conf file we placed at ~/.actinium:
~~~bash
cd ~
mkdir .lightning
cd .lightning
nano conf
~~~
- Place the following textblock in the file, adapt **rpcuser**, **rpcpassword** like configured in Actinium.conf and configure other variables so that it matches your installation. **alias** can be chosen with your personal caption. **log-file** path and **lightning-dir**  have to be adapted to your username. Just exchange **foo** with your username.
  Save with CTRL+o and CTRL+x.
~~~bash
alias=MY-LIGHTNING-NODE
rgb=008100
network=actinium
bitcoin-cli=/usr/local/bin/Actinium-cli
bitcoin-rpcuser=youruser
bitcoin-rpcpassword=yourpass
bitcoin-rpcport=2300
bitcoin-rpcconnect=127.0.0.1
log-prefix=acm-lightning
log-file=/home/foo/.lightning/lightning.log
lightning-dir=/home/foo/.lightning
daemon
~~~
- As we stopped Actiniumd above, we have to restart it first. Whenever we log out from the secure shell, the started programs within the bash would quit too. To prevent this, we will use the flag **-daemon** to start Actiniumd as daemon in the background.
  You will not be able to see any output to stdout, because Actiniumd will run as background process, however you always will be able to examine the logfile in the directory .actinium

```bash
Actiniumd -daemon
```

- We have to invoke the location of the config file for lightningd whenever we start it. lightningd will run  as background process automatically. Additionally we want the status of the Lightning daemon displayed:

~~~bash
lightningd --conf=$HOME/.lightning/conf
lightning-cli getinfo
~~~

If you have configured the daemon's logfile like in the example above you can now check its contents with 

```bash
cat $HOME/.lightning/lightning.log
```


You should see an output similar to this:

>```
>2018-08-30T11:02:03.044Z acm-lightning Creating database
>2018-08-30T11:02:03.085Z lightning_hsmd(88437): HSM: created new hsm_secret file
>2018-08-30T11:03:06.630Z acm-lightning Server started with public key 
>0228de6714942ee30fc6fbff581b7589177d1a0f79e24a480698e28a43d12db02c, alias MY-ACM-LIGHTNING-NODE 
>(color #008100) and lightningd v0.6.1rc1-3-g81a4145-modded
>```

### Connecting to other nodes and funding of channels:

**lightning-cli getinfo** will display your IP and the port the Lightning daemon runs at. For example:

>```
>{
>"id": "0228de6714942ee30fc6fbff581b7589177d1a0f79e24a480698e28a43d12db02c", 
>"alias": "MY-ACM-LIGHTNING-NODE", 
>"color": "008100", 
>"address": [
>{
>"type": "ipv4", 
>"address": "1.2.3.4", 
>"port": 9735
>}
>], 
>"binding": [
>{
>"type": "ipv6", 
>"address": "::", 
>"port": 9735
>}
>], 
>"version": "v0.6.1rc1-3-g81a4145-modded", 
>"blockheight": 79289, 
>"network": "actinium"
>}
>```

To have a deeper understanding of how lightning works, read the documents at [the Actinium github page](https://github.com/Actinium-project) and read the plenty articles on the web.

- Initial setup of a wallet address:
~~~bash
# Returns an address <address>
lightning-cli newaddr
~~~
Take your preferred [Actinium wallet](https://actinium.org/#wallets) or exchange and send initial ACM to the above given address. A **lightning-cli listfunds** will show your funding status under "outputs". You will eventually see a "**Confirmed**" showing the ACM arrived.

- Once lightningd has funds, we can connect to a node and open a channel. Let's assume the remote node is accepting connections at **ip** (and optional **port**, if not 9735) and has the node ID **node_id** :
~~~bash
lightning-cli connect <node_id> <ip> [<port>]
lightning-cli fundchannel <node_id> <amount_in_satoshis>
~~~
You will find all availabe **node_id** and **ip:port** at the [Official Actinium Lightning node list](https://github.com/Actinium-project/Actinium/wiki/Official-Lightning-Nodes) and visualized including the unofficial ones at [The Lightning node explorer](https://ln-explorer.actinium.org/).
Once you have established a channel to another node, it will be displayed in the Node explorer.

