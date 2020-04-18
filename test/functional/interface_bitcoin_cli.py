#!/usr/bin/env python3
# Copyright (c) 2017-2020 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test Actinium-cli"""
from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import assert_equal, assert_raises_process_error, get_auth_cookie

# The block reward of coinbaseoutput.nValue (50) BTC/block matures after
# COINBASE_MATURITY (100) blocks. Therefore, after mining 101 blocks we expect
# node 0 to have a balance of (BLOCKS - COINBASE_MATURITY) * 50 BTC/block.
BLOCKS = 101
BALANCE = (BLOCKS - 100) * 50

class TestBitcoinCli(BitcoinTestFramework):

    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 1

    def skip_test_if_missing_module(self):
        self.skip_if_no_cli()

    def run_test(self):
        """Main test logic"""
        self.nodes[0].generate(BLOCKS)

        self.log.info("Compare responses from getblockchaininfo RPC and `Actinium-cli getblockchaininfo`")
        cli_response = self.nodes[0].cli.getblockchaininfo()
        rpc_response = self.nodes[0].getblockchaininfo()
        assert_equal(cli_response, rpc_response)

        user, password = get_auth_cookie(self.nodes[0].datadir, self.chain)

        self.log.info("Test -stdinrpcpass option")
        assert_equal(BLOCKS, self.nodes[0].cli('-rpcuser=%s' % user, '-stdinrpcpass', input=password).getblockcount())
        assert_raises_process_error(1, "Incorrect rpcuser or rpcpassword", self.nodes[0].cli('-rpcuser=%s' % user, '-stdinrpcpass', input="foo").echo)

        self.log.info("Test -stdin and -stdinrpcpass")
        assert_equal(["foo", "bar"], self.nodes[0].cli('-rpcuser=%s' % user, '-stdin', '-stdinrpcpass', input=password + "\nfoo\nbar").echo())
        assert_raises_process_error(1, "Incorrect rpcuser or rpcpassword", self.nodes[0].cli('-rpcuser=%s' % user, '-stdin', '-stdinrpcpass', input="foo").echo)

        self.log.info("Test connecting to a non-existing server")
        assert_raises_process_error(1, "Could not connect to the server", self.nodes[0].cli('-rpcport=1').echo)

        self.log.info("Test connecting with non-existing RPC cookie file")
        assert_raises_process_error(1, "Could not locate RPC credentials", self.nodes[0].cli('-rpccookiefile=does-not-exist', '-rpcpassword=').echo)

        self.log.info("Test -getinfo with arguments fails")
        assert_raises_process_error(1, "-getinfo takes no arguments", self.nodes[0].cli('-getinfo').help)

        self.log.info("Test that -getinfo returns the expected network and blockchain info")
        if self.is_wallet_compiled():
            self.nodes[0].encryptwallet(password)
        cli_get_info = self.nodes[0].cli('-getinfo').send_cli()
        network_info = self.nodes[0].getnetworkinfo()
        blockchain_info = self.nodes[0].getblockchaininfo()
        assert_equal(cli_get_info['version'], network_info['version'])
        assert_equal(cli_get_info['blocks'], blockchain_info['blocks'])
        assert_equal(cli_get_info['headers'], blockchain_info['headers'])
        assert_equal(cli_get_info['timeoffset'], network_info['timeoffset'])
        assert_equal(cli_get_info['connections'], network_info['connections'])
        assert_equal(cli_get_info['proxy'], network_info['networks'][0]['proxy'])
        assert_equal(cli_get_info['difficulty'], blockchain_info['difficulty'])
        assert_equal(cli_get_info['chain'], blockchain_info['chain'])

        if self.is_wallet_compiled():
            self.log.info("Test -getinfo and bitcoin-cli getwalletinfo return expected wallet info")
            assert_equal(cli_get_info['balance'], BALANCE)
            wallet_info = self.nodes[0].getwalletinfo()
            assert_equal(cli_get_info['keypoolsize'], wallet_info['keypoolsize'])
            assert_equal(cli_get_info['unlocked_until'], wallet_info['unlocked_until'])
            assert_equal(cli_get_info['paytxfee'], wallet_info['paytxfee'])
            assert_equal(cli_get_info['relayfee'], network_info['relayfee'])
            assert_equal(self.nodes[0].cli.getwalletinfo(), wallet_info)
        else:
            self.log.info("*** Wallet not compiled; cli getwalletinfo and -getinfo wallet tests skipped")

        self.stop_node(0)

        self.log.info("Test -version with node stopped")
        cli_response = self.nodes[0].cli("-version").send_cli()
        assert "{} RPC client version".format(self.config['environment']['PACKAGE_NAME']) in cli_response

        self.log.info("Test -rpcwait option waits for RPC connection instead of failing")
        # Start node without RPC connection.
        self.nodes[0].start()
        # Verify failure without -rpcwait.
        assert_raises_process_error(1, "Could not connect to the server", self.nodes[0].cli('getblockcount').echo)
        # Verify success using -rpcwait.
        assert_equal(BLOCKS, self.nodes[0].cli('-rpcwait', 'getblockcount').send_cli())
        self.nodes[0].wait_for_rpc_connection()


if __name__ == '__main__':
    TestBitcoinCli().main()
