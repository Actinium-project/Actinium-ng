#!/usr/bin/env bash
#
# Copyright (c) 2020 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

export LC_ALL=C.UTF-8

export CONTAINER_NAME=ci_native_multiprocess
export PACKAGES="cmake python3"
export DEP_OPTS="MULTIPROCESS=1"
export GOAL="install"
export BITCOIN_CONFIG=""
export TEST_RUNNER_ENV="BITCOIND=bitcoin-node"
