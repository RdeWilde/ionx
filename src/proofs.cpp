// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2015-2017 The Silk Network developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "proofs.h"

#include "chainparams.h"
#include "main.h"
#include "uint256.h"
#include "util.h"
#include "amount.h"

#include <math.h>
#include <stdint.h> 

// miner's coin base reward
int64_t GetCoinbaseValue(int nHeight, CAmount nFees)
{
    CAmount nSubsidy = 0;

        if(nHeight == 1)
        {
            nSubsidy = 16030000 * COIN;
        }

    return nSubsidy + nFees;
}

// miner's coin stake reward based on coin age spent (coin-days)
int64_t GetCoinstakeValue(int64_t nCoinAge, CAmount nFees, int nHeight)
{
    CAmount nSubsidy = 0.2 * COIN;

      if(nHeight < 525600)
        {
            nSubsidy = 23 * COIN;
        }
        else if(nHeight < 1051200)
        {
            nSubsidy = 17 * COIN;
        }
        else if(nHeight < 1576800)
        {
            nSubsidy = 11.5 * COIN;
        }
        else if(nHeight < 2102400)
        {
            nSubsidy = 5.75 * COIN;
        }
        else if(nHeight < 2628000)
        {
            nSubsidy = 1.85 * COIN;
        }
        else
        {
            nSubsidy = 0.2 * COIN;
        }

    return nSubsidy + nFees;
}

unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    uint256 bnTargetLimit = fProofOfStake ? Params().ProofOfStakeLimit() : Params().ProofOfWorkLimit();

    if (pindexLast == NULL)
        return bnTargetLimit.GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // second block

    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

    if (nActualSpacing < 0) {
        nActualSpacing = nTargetSpacing;
    }
    else if (fProofOfStake && nActualSpacing > nTargetSpacing * 10) {
         nActualSpacing = nTargetSpacing * 10;
    }

    // target change every block
    // retarget with exponential moving toward target spacing
    // Includes fix for wrong retargeting difficulty by Mammix2
    uint256 bnNew;
    bnNew.SetCompact(pindexPrev->nBits);

    int64_t nInterval = fProofOfStake ? 10 : 10;
    bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * nTargetSpacing);

    if (bnNew <= 0 || bnNew > bnTargetLimit)
        bnNew = bnTargetLimit;

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits)
{
    uint256 bnTarget;
    bnTarget.SetCompact(nBits);

    // Check range
    if (bnTarget <= 0 || bnTarget > Params().ProofOfWorkLimit())
        return error("CheckProofOfWork() : nBits below minimum work");

    // Check proof of work matches claimed amount
    if (hash > bnTarget)
        return error("CheckProofOfWork() : hash doesn't match nBits");

    return true;
}
