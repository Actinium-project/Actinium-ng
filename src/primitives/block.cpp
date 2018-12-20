// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <tinyformat.h>
#include <util/strencodings.h>
#include <crypto/common.h>
<<<<<<< HEAD
#include <crypto/scrypt.h>
#include <crypto/Lyra2Z/Lyra2Z.h>
#include <versionbits.h>
=======
>>>>>>> 86e0a33f5c382513d5179e3fdf158baf952d7e2f

uint256 CBlockHeader::GetHash() const
{
    return SerializeHash(*this);
}

<<<<<<< HEAD
uint256 CBlockHeader::GetPoWScryptHash() const
{
    uint256 thash;
    scrypt_1024_1_1_256(BEGIN(nVersion), BEGIN(thash));
    return thash;
}

uint256 CBlockHeader::GetPoWLyra2ZHash() const
{
    uint256 thash;
    lyra2z_hash(BEGIN(nVersion), BEGIN(thash));
    return thash;
}

uint256 CBlockHeader::GetPoWHash() const
{
	if (nVersion & VERSIONBITS_FORK_GPU_SUPPORT)
		return GetPoWLyra2ZHash();
	else
		return GetPoWScryptHash();
}

=======
>>>>>>> 86e0a33f5c382513d5179e3fdf158baf952d7e2f
std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
