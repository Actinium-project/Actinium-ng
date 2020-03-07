// Copyright (c) 2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <compressor.h>
#include <core_io.h>
#include <core_memusage.h>
#include <policy/policy.h>
#include <pubkey.h>
#include <script/descriptor.h>
#include <script/script.h>
#include <script/sign.h>
#include <script/signingprovider.h>
#include <script/standard.h>
#include <streams.h>
#include <test/fuzz/fuzz.h>
#include <univalue.h>
#include <util/memory.h>

void initialize()
{
    // Fuzzers using pubkey must hold an ECCVerifyHandle.
    static const auto verify_handle = MakeUnique<ECCVerifyHandle>();

    SelectParams(CBaseChainParams::REGTEST);
}

void test_one_input(const std::vector<uint8_t>& buffer)
{
    const CScript script(buffer.begin(), buffer.end());

    std::vector<unsigned char> compressed;
    if (CompressScript(script, compressed)) {
        const unsigned int size = compressed[0];
        assert(size >= 0 && size <= 5);
        CScript decompressed_script;
        const bool ok = DecompressScript(decompressed_script, size, compressed);
        assert(ok);
    }

    for (unsigned int size = 0; size < 6; ++size) {
        std::vector<unsigned char> vch(GetSpecialScriptSize(size), 0x00);
        vch.insert(vch.end(), buffer.begin(), buffer.end());
        CScript decompressed_script;
        (void)DecompressScript(decompressed_script, size, vch);
    }

    CTxDestination address;
    (void)ExtractDestination(script, address);

    txnouttype type_ret;
    std::vector<CTxDestination> addresses;
    int required_ret;
    (void)ExtractDestinations(script, type_ret, addresses, required_ret);

    (void)GetScriptForWitness(script);

    const FlatSigningProvider signing_provider;
    (void)InferDescriptor(script, signing_provider);

    (void)IsSegWitOutput(signing_provider, script);

    (void)IsSolvable(signing_provider, script);

    txnouttype which_type;
    (void)IsStandard(script, which_type);

    (void)RecursiveDynamicUsage(script);

    std::vector<std::vector<unsigned char>> solutions;
    (void)Solver(script, solutions);

    (void)script.HasValidOps();
    (void)script.IsPayToScriptHash();
    (void)script.IsPayToWitnessScriptHash();
    (void)script.IsPushOnly();
    (void)script.IsUnspendable();
    (void)script.GetSigOpCount(/* fAccurate= */ false);

    (void)FormatScript(script);
    (void)ScriptToAsmStr(script, false);
    (void)ScriptToAsmStr(script, true);

    UniValue o1(UniValue::VOBJ);
    ScriptPubKeyToUniv(script, o1, true);
    UniValue o2(UniValue::VOBJ);
    ScriptPubKeyToUniv(script, o2, false);
    UniValue o3(UniValue::VOBJ);
    ScriptToUniv(script, o3, true);
    UniValue o4(UniValue::VOBJ);
    ScriptToUniv(script, o4, false);
}
