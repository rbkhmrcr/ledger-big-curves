#!/usr/bin/env python

"""
Implements the Poseidon permutation:
Starkad and Poseidon: New Hash Functions for Zero Knowledge Proof Systems
 - Lorenzo Grassi, Daniel Kales, Dmitry Khovratovich, Arnab Roy, Christian Rechberger, and Markus Schofnegger
 - https://eprint.iacr.org/2019/458.pdf
Other implementations:
 - https://github.com/shamatar/PoseidonTree/
 - https://github.com/iden3/circomlib/blob/master/src/poseidon.js
 - https://github.com/dusk-network/poseidon252
"""

from collections import namedtuple
import poseidon_params

p = 5543634365110765627805495722742127385843376434033820803590214255538854698464778703795540858859767700241957783601153

_PoseidonParams = namedtuple('_PoseidonParams', ('p', 't', 'nRoundsF', 'nRoundsP', 'e', 'constants_C', 'constants_M'))
DefaultParams = _PoseidonParams(p, 3, 8, 30, 11, poseidon_params.round_constants, poseidon_params.mds)

def poseidon_mix(state, M, p):
    return [ sum([M[i][j] * x for j, x in enumerate(state)]) % p
             for i in range(len(M)) ]

def poseidon(inputs, params=None, state=None):
    if params is None:
        params = DefaultParams
    assert isinstance(params, _PoseidonParams)
    assert len(inputs) > 0 and len(inputs) < params.t
    if state is None:
        state = [0] * params.t
    else:
        assert len(state) == params.t

    for i in range(len(inputs)):
        state[i] = inputs[i] + state[i]

    # half full rounds
    half = params.nRoundsF//2
    for i in range(half):
        for j in range(params.t):
            state[j] = state[j] + params.constants_C[i][j]          # ARK
        state = [pow(x, params.e, params.p) for x in state]         # x**a
        state = poseidon_mix(state, params.constants_M, params.p)   # MDS

    # partial rounds
    offset = half
    for i in range(params.nRoundsP):
        for j in range(params.t):
            state[j] = state[j] + params.constants_C[i + offset][j] # ARK
        state[0] = pow(state[0], params.e, params.p)                # x[0]**a
        state = poseidon_mix(state, params.constants_M, params.p)   # MDS

    # half full rounds
    offset = params.nRoundsP + half
    for i in range(half):
        for j in range(params.t):
            state[j] = state[j] + params.constants_C[i + offset][j] # ARK
        state = [pow(x, params.e, params.p) for x in state]         # x**a
        state = poseidon_mix(state, params.constants_M, params.p)   # MDS
    return state

def poseidon_digest(state):
    return state[0]

if __name__ == "__main__":
    msgs = [[240717916736854602989207148466022993262069182275],
       [0]]
    for i in range(len(msgs)):
        print(hex(poseidon(msgs[i])[0]))
        print(hex(poseidon(msgs[i])[1]))
        print(hex(poseidon(msgs[i])[2]))
    salted_state = [0x1cd65eabe7ba8054d22b27b48000ce60c6ee3519be7845cb2cfdae8a6ac784c335680ac91d55c045f3413c21324a1f79,
        0x2016b2acb898819099d8b1132300d5ebb5cf5b0fb6d73a79f0ec6ad71dc7c66f5aa4ffeba4f8f69d4ac3cb85647131dc,
        0x0d85b7e5fdce11a731ee504c23637803138dae7365d86934ddad174a06e71ff320232d8ba19882a1f437373387f7f0fa]
