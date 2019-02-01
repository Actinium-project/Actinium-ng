macOS Build Instructions and Notes
====================================
The commands in this guide should be executed in a Terminal application.
The built-in one is located in `/Applications/Utilities/Terminal.app`.

Preparation
-----------
Install the macOS command line tools:

`xcode-select --install`

When the popup appears, click `Install`.

Then install [Homebrew](https://brew.sh).

Dependencies
----------------------

    brew install automake berkeley-db4 libtool boost miniupnpc openssl pkg-config protobuf python qt libevent zmq qrencode

See [dependencies.md](dependencies.md) for a complete overview.

If you want to build the disk image with `make deploy` (.dmg / optional), you need RSVG:

    brew install librsvg

Make the Homebrew OpenSSL headers visible to the configure script  (do ```brew info openssl``` to find out why this is necessary, or if you use Homebrew with installation folders different from the default).

    export LDFLAGS+=-L/usr/local/opt/openssl/lib
    export CPPFLAGS+=-I/usr/local/opt/openssl/include

Berkeley DB
-----------
It is recommended to use Berkeley DB 4.8. If you have to build it yourself,
you can use [the installation script included in contrib/](/contrib/install_db4.sh)
like so:

```shell
./contrib/install_db4.sh .
```

from the root of the repository.

**Note**: You only need Berkeley DB if the wallet is enabled (see [*Disable-wallet mode*](/doc/build-osx.md#disable-wallet-mode)).

Build Actinium Core
------------------------

1. Clone the Actinium Core source code:

        git clone https://github.com/Actinium-project/Actinium-ng
        cd bitcoin

2.  Build Actinium Core:

    Configure and build the headless Actinium Core binaries as well as the GUI (if Qt is found).

    You can disable the GUI build by passing `--without-gui` to configure.

        ./autogen.sh
        ./configure --disable-shared --with-gui=qt5
        make

3.  It is recommended to build and run the unit tests:

        make check

4.  You can also create a .dmg that contains the .app bundle (optional):

        make deploy

Disable-wallet mode
--------------------
When the intention is to run only a P2P node without a wallet, Actinium Core may be compiled in
disable-wallet mode with:

    ./configure --disable-wallet

In this case there is no dependency on Berkeley DB 4.8.

Mining is also possible in disable-wallet mode using the `getblocktemplate` RPC call.

Running
-------

Actinium Core is now available at `./src/bitcoind`

Before running, you may create an empty configuration file:

    touch "/Users/${USER}/Library/Application Support/Actinium/Actinium.conf"

    chmod 600 "/Users/${USER}/Library/Application Support/Actinium/Actinium.conf"

The first time you run bitcoind, it will start downloading the blockchain. This process could take many hours, or even days on slower than average systems.

You can monitor the download process by looking at the debug.log file:

    tail -f $HOME/Library/Application\ Support/Actinium/debug.log

Other commands:
-------

    ./src/bitcoind -daemon # Starts the bitcoin daemon.
    ./src/Actinium-cli --help # Outputs a list of command-line options.
    ./src/Actinium-cli help # Outputs a list of RPC commands when the daemon is running.

Notes
-----

* Tested on OS X 10.10 Yosemite through macOS 10.13 High Sierra on 64-bit Intel processors only.

* Building with downloaded Qt binaries is not officially supported.
