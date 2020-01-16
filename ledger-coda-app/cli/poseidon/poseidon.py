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
from poseidon_params import mds, round_constants

p = 0x1C4C62D92C41110229022EEE2CDADB7F997505B8FAFED5EB7E8F96C97D87307FDB925E8A0ED8D99D124D9A15AF79DB26C5C28C859A99B3EEBCA9429212636B9DFF97634993AA4D6C381BC3F0057974EA099170FA13A4FD90776E240000001

 _PoseidonParams = namedtuple('_PoseidonParams', ('p', 't', 'nRoundsF', 'nRoundsP', 'e', 'constants_C', 'constants_M'))
DefaultParams = _PoseidonParams(p, 3, 8, 33,  11, round_constants, mds)


def poseidon_sbox(state, i, params):
    """
    iacr.org/2019/458 § 2.2 The Hades Strategy (pg 6)
    In more details, assume R_F = 2 · R_f is an even number. Then
     - the first R_f rounds have a full S-Box layer,
     - the middle R_P rounds have a partial S-Box layer (i.e., 1 S-Box layer),
     - the last R_f rounds have a full S-Box layer
    """
    half_F, nRoundsF = params.nRoundsF // 2, params.nRoundsP
    e, p = params.e, params.p
    if i < half_F or i >= (half_F + params.nRoundsP):
        for j, _ in enumerate(state):
            state[j] = pow(_, e, p)
    else:
        state[0] = pow(state[0], e, p)


def poseidon_mix(state, M, p):
    """
    The mixing layer is a matrix vector product of the state with the mixing matrix
     - https://mathinsight.org/matrix_vector_multiplication
    """
    return [ sum([M[i][j] * _ for j, _ in enumerate(state)]) % p
             for i in range(len(M)) ]

def poseidon(inputs, params=None):
    if params is None:
        params = DefaultParams
    assert isinstance(params, _PoseidonParams)
    assert len(inputs) > 0 and len(inputs) < params.t
    state = [0] * params.t
    state[:len(inputs)] = inputs
    for i, C_i in enumerate(params.constants_C):
        state = [_ + C_i for _ in state]  # ARK(.)
        poseidon_sbox(state, i, params)
        state = poseidon_mix(state, params.constants_M, params.p)
    return state[0]
