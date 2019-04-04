(note: this is a temporary file, to be added-to by anybody, and moved to
release-notes at release time)

Actinium Core version *version* is now available from:

  <https://bitcoincore.org/bin/bitcoin-core-*version*/>

This is a new major version release, including new features, various bugfixes
and performance improvements, as well as updated translations.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/Actinium-project/Actinium-ng/issues>

To receive security and update notifications, please subscribe to:

  <https://bitcoincore.org/en/list/announcements/join/>

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over `/Applications/Actinium-Qt` (on Mac)
or `Actiniumd`/`Actinium-qt` (on Linux).

The first time you run version 0.15.0, your chainstate database will be converted to a
new format, which will take anywhere from a few minutes to half an hour,
depending on the speed of your machine.

Note that the block database format also changed in version 0.8.0 and there is no
automatic upgrade code from before version 0.8 to version 0.15.0. Upgrading
directly from 0.7.x and earlier without redownloading the blockchain is not supported.
However, as usual, old wallet versions are still supported.

Downgrading warning
-------------------

The chainstate database for this release is not compatible with previous
releases, so if you run 0.15 and then decide to switch back to any
older version, you will need to run the old release with the `-reindex-chainstate`
option to rebuild the chainstate data structures in the old format.

If your node has pruning enabled, this will entail re-downloading and
processing the entire blockchain.

Compatibility
==============

Bitcoin Core is supported and extensively tested on operating systems using
the Linux kernel, macOS 10.10+, and Windows 7 and newer.  It is not recommended
to use Bitcoin Core on unsupported systems.

Actinium Core should also work on most other Unix-like systems but is not
frequently tested on them.

From 0.17.0 onwards, macOS <10.10 is no longer supported.  0.17.0 is
built using Qt 5.9.x, which doesn't support versions of macOS older than
10.10.  Additionally, Actinium Core does not yet change appearance when
macOS "dark mode" is activated.

In addition to previously-supported CPU platforms, this release's
pre-compiled distribution also provides binaries for the RISC-V
platform.

Notable changes
===============

Updated RPCs
------------

Note: some low-level RPC changes mainly useful for testing are described in the
Low-level Changes section below.

* The `sendmany` RPC had an argument `minconf` that was not well specified and
  would lead to RPC errors even when the wallet's coin selection would succeed.
  The `sendtoaddress` RPC never had this check, so to normalize the behavior,
  `minconf` is now ignored in `sendmany`. If the coin selection does not
  succeed due to missing coins, it will still throw an RPC error. Be reminded
  that coin selection is influenced by the `-spendzeroconfchange`,
  `-limitancestorcount`, `-limitdescendantcount` and `-walletrejectlongchains`
  command line arguments.


Low-level changes
=================

Example item
------------

Credits
=======

Thanks to everyone who directly contributed to this release:


As well as everyone that helped translating on [Transifex](https://www.transifex.com/bitcoin/bitcoin/).
