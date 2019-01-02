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

Mining
------

- Calls to `getblocktemplate` will fail if the segwit rule is not specified.
  Calling `getblocktemplate` without segwit specified is almost certainly
  a misconfiguration since doing so results in lower rewards for the miner.

Command line option changes
---------------------------

The `-enablebip61` command line option (introduced in Actinium Core 0.17.0) is
used to toggle sending of BIP 61 reject messages. Reject messages have no use
case on the P2P network and are only logged for debugging by most network
nodes. The option will now by default be off for improved privacy and security
as well as reduced upload usage. The option can explicitly be turned on for
local-network debugging purposes.

Documentation
-------------

- A new short
  [document](https://github.com/Actinium-project/Actinium-ng/blob/master/doc/JSON-RPC-interface.md)
  about the JSON-RPC interface describes cases where the results of an
  RPC might contain inconsistencies between data sourced from different
  subsystems, such as wallet state and mempool state.  A note is added
  to the [REST interface documentation](https://github.com/Actinium-project/Actinium-ng/blob/master/doc/REST-interface.md)
  indicating that the same rules apply.

- A new [document](https://github.com/Actinium-project/Actinium-ng/blob/master/doc/bitcoin-conf.md)
  about the `Actinium.conf` file describes how to use it to configure
  Actinium Core.

- A new document introduces Actinium Core's BIP174
  [Partially-Signed Actinium Transactions (PSBT)](https://github.com/Actinium-project/Actinium-ng/blob/master/doc/psbt.md)
  interface, which is used to allow multiple programs to collaboratively
  work to create, sign, and broadcast new transactions.  This is useful
  for offline (cold storage) wallets, multisig wallets, coinjoin
  implementations, and many other cases where two or more programs need
  to interact to generate a complete transaction.

- The [output script descriptor](https://github.com/Actinium-project/Actinium-ng/blob/master/doc/descriptors.md)
  documentation has been updated with information about new features in
  this still-developing language for describing the output scripts that
  a wallet or other program wants to receive notifications for, such as
  which addresses it wants to know received payments.  The language is
  currently used in the `scantxoutset` RPC and is expected to be adapted
  to other RPCs and to the underlying wallet structure.

Build system changes
--------------------

- A new `--disable-bip70` option may be passed to `./configure` to
  prevent Actinium-Qt from being built with support for the BIP70 payment
  protocol or from linking libssl.  As the payment protocol has exposed
  Actinium Core to libssl vulnerabilities in the past, builders who don't
  need BIP70 support are encouraged to use this option to reduce their
  exposure to future vulnerabilities.

Deprecated or removed RPCs
--------------------------

- The `signrawtransaction` RPC is removed after being deprecated and
  hidden behind a special configuration option in version 0.17.0.

- The 'account' API is removed after being deprecated in v0.17.  The
  'label' API was introduced in v0.17 as a replacement for accounts.
  See the [release notes from v0.17](https://github.com/Actinium-project/Actinium-ng/blob/master/doc/release-notes/release-notes-0.17.0.md#label-and-account-apis-for-wallet)
  for a full description of the changes from the 'account' API to the
  'label' API.

- The `addwitnessaddress` RPC is removed after being deprecated in
  version 0.13.0.

- The wallet's `generate` RPC method is deprecated and will be fully
  removed in a subsequent major version.  This RPC is only used for
  testing, but its implementation reached across multiple subsystems
  (wallet and mining), so it is being deprecated to simplify the
  wallet-node interface.  Projects that are using `generate` for testing
  purposes should transition to using the `generatetoaddress` RPC, which
  does not require or use the wallet component. Calling
  `generatetoaddress` with an address returned by the `getnewaddress`
  RPC gives the same functionality as the old `generate` RPC.  To
  continue using `generate` in this version, restart bitcoind with the
  `-deprecatedrpc=generate` configuration option.

New RPCs
--------

- A new `getnodeaddresses` RPC returns peer addresses known to this
  node. It may be used to find nodes to connect to without using a DNS
  seeder.

- A new `listwalletdir` RPC returns a list of wallets in the wallet
  directory (either the default wallet directory or the directory
  configured by the `-walletdir` parameter).

Updated RPCs
------------

Note: some low-level RPC changes mainly useful for testing are described
in the Low-level Changes section below.

- The `getpeerinfo` RPC now returns an additional "minfeefilter" field
  set to the peer's BIP133 fee filter.  You can use this to detect that
  you have peers that are willing to accept transactions below the
  default minimum relay fee.

- The mempool RPCs, such as `getrawmempool` with `verbose=true`, now
  return an additional "bip125-replaceable" value indicating whether the
  transaction (or its unconfirmed ancestors) opts-in to asking nodes and
  miners to replace it with a higher-feerate transaction spending any of
  the same inputs.

- The `settxfee` RPC previously silently ignored attempts to set the fee
  below the allowed minimums.  It now prints a warning.  The special
  value of "0" may still be used to request the minimum value.

- The `getaddressinfo` RPC now provides an `ischange` field indicating
  whether the wallet used the address in a change output.

- The `importmulti` RPC has been updated to support P2WSH, P2WPKH,
  P2SH-P2WPKH, and P2SH-P2WSH. Requests for P2WSH and P2SH-P2WSH accept
  an additional `witnessscript` parameter.

- See the [Mining](#mining) section for changes to `getblocktemplate`.

Low-level changes
=================

RPC
---

- The `submitblock` RPC previously returned the reason a rejected block
  was invalid the first time it processed that block but returned a
  generic "duplicate" rejection message on subsequent occasions it
  processed the same block.  It now always returns the fundamental
  reason for rejecting an invalid block and only returns "duplicate" for
  valid blocks it has already accepted.

- A new `submitheader` RPC allows submitting block headers independently
  from their block.  This is likely only useful for testing.

Configuration
-------------

- The `-usehd` configuration option was removed in version 0.16. From
  that version onwards, all new wallets created are hierarchical
  deterministic wallets. This release makes specifying `-usehd` an
  invalid configuration option.

Credits
=======

Thanks to everyone who directly contributed to this release:


As well as everyone that helped translating on [Transifex](https://www.transifex.com/projects/p/bitcoin/).
