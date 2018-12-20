#!/usr/bin/env bash

export LC_ALL=C
TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
BUILDDIR=${BUILDDIR:-$TOPDIR}

BINDIR=${BINDIR:-$BUILDDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

BITCOIND=${BITCOIND:-$BINDIR/bitcoind}
<<<<<<< HEAD
BITCOINCLI=${BITCOINCLI:-$BINDIR/Actinium-cli}
BITCOINTX=${BITCOINTX:-$BINDIR/Actinium-tx}
BITCOINQT=${BITCOINQT:-$BINDIR/qt/Actinium-qt}
=======
BITCOINCLI=${BITCOINCLI:-$BINDIR/bitcoin-cli}
BITCOINTX=${BITCOINTX:-$BINDIR/bitcoin-tx}
BITCOINQT=${BITCOINQT:-$BINDIR/qt/bitcoin-qt}
>>>>>>> 86e0a33f5c382513d5179e3fdf158baf952d7e2f

[ ! -x $BITCOIND ] && echo "$BITCOIND not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
BTCVER=($($BITCOINCLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }'))

# Create a footer file with copyright content.
# This gets autodetected fine for bitcoind if --version-string is not set,
<<<<<<< HEAD
# but has different outcomes for Actinium-qt and Actinium-cli.
=======
# but has different outcomes for bitcoin-qt and bitcoin-cli.
>>>>>>> 86e0a33f5c382513d5179e3fdf158baf952d7e2f
echo "[COPYRIGHT]" > footer.h2m
$BITCOIND --version | sed -n '1!p' >> footer.h2m

for cmd in $BITCOIND $BITCOINCLI $BITCOINTX $BITCOINQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${BTCVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${BTCVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m
