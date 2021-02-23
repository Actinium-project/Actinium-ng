// Copyright (c) 2018-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <core_io.h>
#include <psbt.h>
#include <util/strencodings.h>
#include <util/system.h>
#include <wallet/external_signer.h>

ExternalSigner::ExternalSigner(const std::string& command, const std::string& fingerprint, std::string chain, std::string name): m_command(command), m_fingerprint(fingerprint), m_chain(chain), m_name(name) {}

const std::string ExternalSigner::NetworkArg() const
{
    return " --chain " + m_chain;
}

#ifdef ENABLE_EXTERNAL_SIGNER

bool ExternalSigner::Enumerate(const std::string& command, std::vector<ExternalSigner>& signers, std::string chain, bool ignore_errors)
{
    // Call <command> enumerate
    const UniValue result = RunCommandParseJSON(command + " enumerate");
    if (!result.isArray()) {
        if (ignore_errors) return false;
        throw ExternalSignerException(strprintf("'%s' received invalid response, expected array of signers", command));
    }
    for (UniValue signer : result.getValues()) {
        // Check for error
        const UniValue& error = find_value(signer, "error");
        if (!error.isNull()) {
            if (ignore_errors) return false;
            if (!error.isStr()) {
                throw ExternalSignerException(strprintf("'%s' error", command));
            }
            throw ExternalSignerException(strprintf("'%s' error: %s", command, error.getValStr()));
        }
        // Check if fingerprint is present
        const UniValue& fingerprint = find_value(signer, "fingerprint");
        if (fingerprint.isNull()) {
            if (ignore_errors) return false;
            throw ExternalSignerException(strprintf("'%s' received invalid response, missing signer fingerprint", command));
        }
        std::string fingerprintStr = fingerprint.get_str();
        // Skip duplicate signer
        bool duplicate = false;
        for (ExternalSigner signer : signers) {
            if (signer.m_fingerprint.compare(fingerprintStr) == 0) duplicate = true;
        }
        if (duplicate) break;
        std::string name = "";
        const UniValue& model_field = find_value(signer, "model");
        if (model_field.isStr() && model_field.getValStr() != "") {
            name += model_field.getValStr();
        }
        signers.push_back(ExternalSigner(command, fingerprintStr, chain, name));
    }
    return true;
}

UniValue ExternalSigner::DisplayAddress(const std::string& descriptor) const
{
    return RunCommandParseJSON(m_command + " --fingerprint \"" + m_fingerprint + "\"" + NetworkArg() + " displayaddress --desc \"" + descriptor + "\"");
}

UniValue ExternalSigner::GetDescriptors(int account)
{
    return RunCommandParseJSON(m_command + " --fingerprint \"" + m_fingerprint + "\"" + NetworkArg() + " getdescriptors --account " + strprintf("%d", account));
}

bool ExternalSigner::SignTransaction(PartiallySignedTransaction& psbtx, std::string& error)
{
    // Serialize the PSBT
    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << psbtx;

    // Check if signer fingerprint matches any input master key fingerprint
    bool match = false;
    for (unsigned int i = 0; i < psbtx.inputs.size(); ++i) {
        const PSBTInput& input = psbtx.inputs[i];
        for (auto entry : input.hd_keypaths) {
            if (m_fingerprint == strprintf("%08x", ReadBE32(entry.second.fingerprint))) match = true;
        }
    }

    if (!match) {
        error = "Signer fingerprint " + m_fingerprint + " does not match any of the inputs:\n" + EncodeBase64(ssTx.str());
        return false;
    }

    std::string command = m_command + " --stdin --fingerprint \"" + m_fingerprint + "\"" + NetworkArg();
    std::string stdinStr = "signtx \"" + EncodeBase64(ssTx.str()) + "\"";

    const UniValue signer_result = RunCommandParseJSON(command, stdinStr);

    if (find_value(signer_result, "error").isStr()) {
        error = find_value(signer_result, "error").get_str();
        return false;
    }

    if (!find_value(signer_result, "psbt").isStr()) {
        error = "Unexpected result from signer";
        return false;
    }

    PartiallySignedTransaction signer_psbtx;
    std::string signer_psbt_error;
    if (!DecodeBase64PSBT(signer_psbtx, find_value(signer_result, "psbt").get_str(), signer_psbt_error)) {
        error = strprintf("TX decode failed %s", signer_psbt_error);
        return false;
    }

    psbtx = signer_psbtx;

    return true;
}

#endif
