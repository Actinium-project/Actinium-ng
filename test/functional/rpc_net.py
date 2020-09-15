#!/usr/bin/env python3
# Copyright (c) 2017-2019 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test RPC calls related to net.

Tests correspond to code in rpc/net.cpp.
"""

from decimal import Decimal
from itertools import product
import time

from test_framework.p2p import P2PInterface
import test_framework.messages
from test_framework.messages import (
    NODE_NETWORK,
    NODE_WITNESS,
)
from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import (
    assert_approx,
    assert_equal,
    assert_greater_than_or_equal,
    assert_greater_than,
    assert_raises_rpc_error,
    connect_nodes,
    p2p_port,
)


def assert_net_servicesnames(servicesflag, servicenames):
    """Utility that checks if all flags are correctly decoded in
    `getpeerinfo` and `getnetworkinfo`.

    :param servicesflag: The services as an integer.
    :param servicenames: The list of decoded services names, as strings.
    """
    servicesflag_generated = 0
    for servicename in servicenames:
        servicesflag_generated |= getattr(test_framework.messages, 'NODE_' + servicename)
    assert servicesflag_generated == servicesflag


class NetTest(BitcoinTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 2
        self.extra_args = [["-minrelaytxfee=0.00001000"], ["-minrelaytxfee=0.00000500"]]
        self.supports_cli = False

    def run_test(self):
        # Get out of IBD for the minfeefilter and getpeerinfo tests.
        self.nodes[0].generate(101)
        # Connect nodes both ways.
        connect_nodes(self.nodes[0], 1)
        connect_nodes(self.nodes[1], 0)

        self.test_connection_count()
        self.test_getpeerinfo()
        self.test_getnettotals()
        self.test_getnetworkinfo()
        self.test_getaddednodeinfo()
        self.test_service_flags()
        self.test_getnodeaddresses()

    def test_connection_count(self):
        self.log.info("Test getconnectioncount")
        # After using `connect_nodes` to connect nodes 0 and 1 to each other.
        assert_equal(self.nodes[0].getconnectioncount(), 2)

    def test_getnettotals(self):
        self.log.info("Test getnettotals")
        # getnettotals totalbytesrecv and totalbytessent should be
        # consistent with getpeerinfo. Since the RPC calls are not atomic,
        # and messages might have been recvd or sent between RPC calls, call
        # getnettotals before and after and verify that the returned values
        # from getpeerinfo are bounded by those values.
        net_totals_before = self.nodes[0].getnettotals()
        peer_info = self.nodes[0].getpeerinfo()
        net_totals_after = self.nodes[0].getnettotals()
        assert_equal(len(peer_info), 2)
        peers_recv = sum([peer['bytesrecv'] for peer in peer_info])
        peers_sent = sum([peer['bytessent'] for peer in peer_info])

        assert_greater_than_or_equal(peers_recv, net_totals_before['totalbytesrecv'])
        assert_greater_than_or_equal(net_totals_after['totalbytesrecv'], peers_recv)
        assert_greater_than_or_equal(peers_sent, net_totals_before['totalbytessent'])
        assert_greater_than_or_equal(net_totals_after['totalbytessent'], peers_sent)

        # test getnettotals and getpeerinfo by doing a ping
        # the bytes sent/received should change
        # note ping and pong are 32 bytes each
        self.nodes[0].ping()
        self.wait_until(lambda: (self.nodes[0].getnettotals()['totalbytessent'] >= net_totals_after['totalbytessent'] + 32 * 2), timeout=1)
        self.wait_until(lambda: (self.nodes[0].getnettotals()['totalbytesrecv'] >= net_totals_after['totalbytesrecv'] + 32 * 2), timeout=1)

        peer_info_after_ping = self.nodes[0].getpeerinfo()
        for before, after in zip(peer_info, peer_info_after_ping):
            assert_greater_than_or_equal(after['bytesrecv_per_msg'].get('pong', 0), before['bytesrecv_per_msg'].get('pong', 0) + 32)
            assert_greater_than_or_equal(after['bytessent_per_msg'].get('ping', 0), before['bytessent_per_msg'].get('ping', 0) + 32)

    def test_getnetworkinfo(self):
        self.log.info("Test getnetworkinfo")
        info = self.nodes[0].getnetworkinfo()
        assert_equal(info['networkactive'], True)
        assert_equal(info['connections'], 2)
        assert_equal(info['connections_in'], 1)
        assert_equal(info['connections_out'], 1)

        with self.nodes[0].assert_debug_log(expected_msgs=['SetNetworkActive: false\n']):
            self.nodes[0].setnetworkactive(state=False)
        assert_equal(self.nodes[0].getnetworkinfo()['networkactive'], False)
        # Wait a bit for all sockets to close
        self.wait_until(lambda: self.nodes[0].getnetworkinfo()['connections'] == 0, timeout=3)

        with self.nodes[0].assert_debug_log(expected_msgs=['SetNetworkActive: true\n']):
            self.nodes[0].setnetworkactive(state=True)
        # Connect nodes both ways.
        connect_nodes(self.nodes[0], 1)
        connect_nodes(self.nodes[1], 0)

        info = self.nodes[0].getnetworkinfo()
        assert_equal(info['networkactive'], True)
        assert_equal(info['connections'], 2)
        assert_equal(info['connections_in'], 1)
        assert_equal(info['connections_out'], 1)

        # check the `servicesnames` field
        network_info = [node.getnetworkinfo() for node in self.nodes]
        for info in network_info:
            assert_net_servicesnames(int(info["localservices"], 0x10), info["localservicesnames"])

    def test_getaddednodeinfo(self):
        self.log.info("Test getaddednodeinfo")
        assert_equal(self.nodes[0].getaddednodeinfo(), [])
        # add a node (node2) to node0
        ip_port = "127.0.0.1:{}".format(p2p_port(2))
        self.nodes[0].addnode(node=ip_port, command='add')
        # check that the node has indeed been added
        added_nodes = self.nodes[0].getaddednodeinfo(ip_port)
        assert_equal(len(added_nodes), 1)
        assert_equal(added_nodes[0]['addednode'], ip_port)
        # check that node cannot be added again
        assert_raises_rpc_error(-23, "Node already added", self.nodes[0].addnode, node=ip_port, command='add')
        # check that node can be removed
        self.nodes[0].addnode(node=ip_port, command='remove')
        assert_equal(self.nodes[0].getaddednodeinfo(), [])
        # check that trying to remove the node again returns an error
        assert_raises_rpc_error(-24, "Node could not be removed", self.nodes[0].addnode, node=ip_port, command='remove')
        # check that a non-existent node returns an error
        assert_raises_rpc_error(-24, "Node has not been added", self.nodes[0].getaddednodeinfo, '1.1.1.1')

    def test_getpeerinfo(self):
        self.log.info("Test getpeerinfo")
        # Create a few getpeerinfo last_block/last_transaction values.
        if self.is_wallet_compiled():
            self.nodes[0].sendtoaddress(self.nodes[1].getnewaddress(), 1)
        self.nodes[1].generate(1)
        self.sync_all()
        time_now = int(time.time())
        peer_info = [x.getpeerinfo() for x in self.nodes]
        # Verify last_block and last_transaction keys/values.
        for node, peer, field in product(range(self.num_nodes), range(2), ['last_block', 'last_transaction']):
            assert field in peer_info[node][peer].keys()
            if peer_info[node][peer][field] != 0:
                assert_approx(peer_info[node][peer][field], time_now, vspan=60)
        # check both sides of bidirectional connection between nodes
        # the address bound to on one side will be the source address for the other node
        assert_equal(peer_info[0][0]['addrbind'], peer_info[1][0]['addr'])
        assert_equal(peer_info[1][0]['addrbind'], peer_info[0][0]['addr'])
        assert_equal(peer_info[0][0]['minfeefilter'], Decimal("0.00000500"))
        assert_equal(peer_info[1][0]['minfeefilter'], Decimal("0.00001000"))
        # check the `servicesnames` field
        for info in peer_info:
            assert_net_servicesnames(int(info[0]["services"], 0x10), info[0]["servicesnames"])

    def test_service_flags(self):
        self.log.info("Test service flags")
        self.nodes[0].add_p2p_connection(P2PInterface(), services=(1 << 4) | (1 << 63))
        assert_equal(['UNKNOWN[2^4]', 'UNKNOWN[2^63]'], self.nodes[0].getpeerinfo()[-1]['servicesnames'])
        self.nodes[0].disconnect_p2ps()

    def test_getnodeaddresses(self):
        self.log.info("Test getnodeaddresses")
        self.nodes[0].add_p2p_connection(P2PInterface())

        # Add some addresses to the Address Manager over RPC. Due to the way
        # bucket and bucket position are calculated, some of these addresses
        # will collide.
        imported_addrs = []
        for i in range(10000):
            first_octet = i >> 8
            second_octet = i % 256
            a = "{}.{}.1.1".format(first_octet, second_octet)
            imported_addrs.append(a)
            self.nodes[0].addpeeraddress(a, 8333)

        # Obtain addresses via rpc call and check they were ones sent in before.
        #
        # Maximum possible addresses in addrman is 10000, although actual
        # number will usually be less due to bucket and bucket position
        # collisions.
        node_addresses = self.nodes[0].getnodeaddresses(0)
        assert_greater_than(len(node_addresses), 5000)
        assert_greater_than(10000, len(node_addresses))
        for a in node_addresses:
            assert_greater_than(a["time"], 1527811200)  # 1st June 2018
            assert_equal(a["services"], NODE_NETWORK | NODE_WITNESS)
            assert a["address"] in imported_addrs
            assert_equal(a["port"], 8333)

        node_addresses = self.nodes[0].getnodeaddresses(1)
        assert_equal(len(node_addresses), 1)

        assert_raises_rpc_error(-8, "Address count out of range", self.nodes[0].getnodeaddresses, -1)

        # addrman's size cannot be known reliably after insertion, as hash collisions may occur
        # so only test that requesting a large number of addresses returns less than that
        LARGE_REQUEST_COUNT = 10000
        node_addresses = self.nodes[0].getnodeaddresses(LARGE_REQUEST_COUNT)
        assert_greater_than(LARGE_REQUEST_COUNT, len(node_addresses))


if __name__ == '__main__':
    NetTest().main()
