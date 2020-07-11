// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <banman.h>
#include <chainparams.h>
#include <consensus/consensus.h>
#include <net.h>
#include <net_processing.h>
#include <protocol.h>
#include <scheduler.h>
#include <script/script.h>
#include <streams.h>
#include <test/fuzz/FuzzedDataProvider.h>
#include <test/fuzz/fuzz.h>
#include <test/util/mining.h>
#include <test/util/net.h>
#include <test/util/setup_common.h>
#include <util/memory.h>
#include <validationinterface.h>
#include <version.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iosfwd>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

void ProcessMessage(
    CNode& pfrom,
    const std::string& msg_type,
    CDataStream& vRecv,
    const std::chrono::microseconds time_received,
    const CChainParams& chainparams,
    ChainstateManager& chainman,
    CTxMemPool& mempool,
    CConnman* connman,
    BanMan* banman,
    const std::atomic<bool>& interruptMsgProc);

namespace {

#ifdef MESSAGE_TYPE
#define TO_STRING_(s) #s
#define TO_STRING(s) TO_STRING_(s)
const std::string LIMIT_TO_MESSAGE_TYPE{TO_STRING(MESSAGE_TYPE)};
#else
const std::string LIMIT_TO_MESSAGE_TYPE;
#endif

const TestingSetup* g_setup;
} // namespace

void initialize()
{
    static TestingSetup setup{
        CBaseChainParams::REGTEST,
        {
            "-nodebuglogfile",
        },
    };
    g_setup = &setup;

    for (int i = 0; i < 2 * COINBASE_MATURITY; i++) {
        MineBlock(g_setup->m_node, CScript() << OP_TRUE);
    }
    SyncWithValidationInterfaceQueue();
}

void test_one_input(const std::vector<uint8_t>& buffer)
{
    FuzzedDataProvider fuzzed_data_provider(buffer.data(), buffer.size());
    ConnmanTestMsg& connman = *(ConnmanTestMsg*)g_setup->m_node.connman.get();
    const std::string random_message_type{fuzzed_data_provider.ConsumeBytesAsString(CMessageHeader::COMMAND_SIZE).c_str()};
    if (!LIMIT_TO_MESSAGE_TYPE.empty() && random_message_type != LIMIT_TO_MESSAGE_TYPE) {
        return;
    }
    CDataStream random_bytes_data_stream{fuzzed_data_provider.ConsumeRemainingBytes<unsigned char>(), SER_NETWORK, PROTOCOL_VERSION};
    CNode& p2p_node = *MakeUnique<CNode>(0, ServiceFlags(NODE_NETWORK | NODE_WITNESS | NODE_BLOOM), 0, INVALID_SOCKET, CAddress{CService{in_addr{0x0100007f}, 7777}, NODE_NETWORK}, 0, 0, CAddress{}, std::string{}, false).release();
    p2p_node.fSuccessfullyConnected = true;
    p2p_node.nVersion = PROTOCOL_VERSION;
    p2p_node.SetSendVersion(PROTOCOL_VERSION);
    connman.AddTestNode(p2p_node);
    g_setup->m_node.peer_logic->InitializeNode(&p2p_node);
    try {
        ProcessMessage(p2p_node, random_message_type, random_bytes_data_stream, GetTime<std::chrono::microseconds>(),
            Params(), *g_setup->m_node.chainman, *g_setup->m_node.mempool,
            g_setup->m_node.connman.get(), g_setup->m_node.banman.get(),
            std::atomic<bool>{false});
    } catch (const std::ios_base::failure&) {
    }
    SyncWithValidationInterfaceQueue();
    LOCK2(::cs_main, g_cs_orphans); // See init.cpp for rationale for implicit locking order requirement
    g_setup->m_node.connman->StopNodes();
}
