Repository Tools
---------------------

### [Developer tools](/contrib/devtools) ###
Specific tools for developers working on this repository.
Contains the script `github-merge.py` for merging GitHub pull requests securely and signing them using GPG.

### [Verify-Commits](/contrib/verify-commits) ###
Tool to verify that every merge commit was signed by a developer using the above `github-merge.py` script.

### [Linearize](/contrib/linearize) ###
Construct a linear, no-fork, best version of the blockchain.

### [Qos](/contrib/qos) ###

<<<<<<< HEAD
A Linux bash script that will set up traffic control (tc) to limit the outgoing bandwidth for connections to the Actinium network. This means one can have an always-on bitcoind instance running, and another local bitcoind/Actinium-qt instance which connects to this node and receives blocks from it.
=======
A Linux bash script that will set up traffic control (tc) to limit the outgoing bandwidth for connections to the Bitcoin network. This means one can have an always-on bitcoind instance running, and another local bitcoind/bitcoin-qt instance which connects to this node and receives blocks from it.
>>>>>>> 86e0a33f5c382513d5179e3fdf158baf952d7e2f

### [Seeds](/contrib/seeds) ###
Utility to generate the pnSeed[] array that is compiled into the client.

Build Tools and Keys
---------------------

### Packaging ###
The [Debian](/contrib/debian) subfolder contains the copyright file.

All other packaging related files can be found in the [bitcoin-core/packaging](https://github.com/bitcoin-core/packaging) repository.

### [Gitian-descriptors](/contrib/gitian-descriptors) ###
<<<<<<< HEAD
Files used during the gitian build process. For more information about gitian, see the [the Actinium Core documentation repository](https://github.com/bitcoin-core/docs).

### [Gitian-keys](/contrib/gitian-keys)
PGP keys used for signing Actinium Core [Gitian release](/doc/release-process.md) results.
=======
Files used during the gitian build process. For more information about gitian, see the [the Bitcoin Core documentation repository](https://github.com/bitcoin-core/docs).

### [Gitian-keys](/contrib/gitian-keys)
PGP keys used for signing Bitcoin Core [Gitian release](/doc/release-process.md) results.
>>>>>>> 86e0a33f5c382513d5179e3fdf158baf952d7e2f

### [MacDeploy](/contrib/macdeploy) ###
Scripts and notes for Mac builds. 

### [Gitian-build](/contrib/gitian-build.py) ###
Script for running full Gitian builds.

Test and Verify Tools 
---------------------

### [TestGen](/contrib/testgen) ###
<<<<<<< HEAD
Utilities to generate test vectors for the data-driven Actinium tests.

### [Verify Binaries](/contrib/verifybinaries) ###
This script attempts to download and verify the signature file SHA256SUMS.asc from actinium.org.
=======
Utilities to generate test vectors for the data-driven Bitcoin tests.

### [Verify Binaries](/contrib/verifybinaries) ###
This script attempts to download and verify the signature file SHA256SUMS.asc from bitcoin.org.
>>>>>>> 86e0a33f5c382513d5179e3fdf158baf952d7e2f
