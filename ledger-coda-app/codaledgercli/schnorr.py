import hashlib
import binascii
import sys
from random import getrandbits
from codaledgercli import poseidon

a = 11
b = 0x7DA285E70863C79D56446237CE2E1468D14AE9BB64B2BB01B10E60A5D5DFE0A25714B7985993F62F03B22A9A3C737A1A1E0FCF2C43D7BF847957C34CCA1E3585F9A80A95F401867C4E80F4747FDE5ABA7505BA6FCF2485540B13DFC8468A
a_coeff = a
b_coeff = b

p = 0x1C4C62D92C41110229022EEE2CDADB7F997505B8FAFED5EB7E8F96C97D87307FDB925E8A0ED8D99D124D9A15AF79DB26C5C28C859A99B3EEBCA9429212636B9DFF97634993AA4D6C381BC3F0057974EA099170FA13A4FD90776E240000001
n = 0x1C4C62D92C41110229022EEE2CDADB7F997505B8FAFED5EB7E8F96C97D87307FDB925E8A0ED8D99D124D9A15AF79DB117E776F218059DB80F0DA5CB537E38685ACCE9767254A4638810719AC425F0E39D54522CDD119F5E9063DE245E8001

G = (3458420969484235708806261200128850544017070333833944116801482064540723268149235477762870414664917360605949659630933184751526227993647030875167687492714052872195770088225183259051403087906158701786758441889742618916006546636728, 27460508402331965149626600224382137254502975979168371111640924721589127725376473514838234361114855175488242007431439074223827742813911899817930728112297763448010814764117701403540298764970469500339646563344680868495474127850569)

# MNT6753
assert n == 41898490967918953402344214791240637128170709919953949071783502921025352812571106773058893763790338921418070971888253786114353726529584385201591605722013126468931404347949840543007986327743462853720628051692141265303114721689601
assert p == 41898490967918953402344214791240637128170709919953949071783502921025352812571106773058893763790338921418070971888458477323173057491593855069696241854796396165721416325350064441470418137846398469611935719059908164220784476160001

N = 753

def next_bit():
    return getrandbits(1)

def legendre_symbol(a):
    ls = pow(a, (p - 1)//2, p)
    if ls == p - 1:
        return -1
    return ls

def prime_mod_sqrt(a):
    """
    Square root modulo prime number
    Solve the equation
        x^2 = a mod p
    and return list of x solution
    http://en.wikipedia.org/wiki/Tonelli-Shanks_algorithm
    """
    a %= p

    # Simple case
    if a == 0:
        return [0]
    if p == 2:
        return [a]

    # Check solution existence on odd prime
    if legendre_symbol(a) != 1:
        return []

    # Simple case
    if p % 4 == 3:
        x = pow(a, (p + 1)//4, p)
        return [x, p-x]

    # Factor p-1 on the form q * 2^s (with Q odd)
    q, s = p - 1, 0
    while q % 2 == 0:
        s += 1
        q //= 2

    # Select a z which is a quadratic non resudue modulo p
    z = 1
    while legendre_symbol(z) != -1:
        z += 1
    c = pow(z, q, p)

    # Search for a solution
    x = pow(a, (q + 1)//2, p)
    t = pow(a, q, p)
    m = s
    while t != 1:
        # Find the lowest i such that t^(2^i) = 1
        i, e = 0, 2
        for i in range(1, m):
            if pow(t, e, p) == 1:
                break
            e *= 2

        # Update next value to iterate
        b = pow(c, 2**(m - i - 1), p)
        x = (x * b) % p
        t = (t * b * b) % p
        c = (b * b) % p
        m = i

    return [x, p-x]

def random_field_elt():
    res = 0
    for i in range(N):
        res += next_bit() * (2 ** i)
    if res < p:
        return res
    else:
        return random_field_elt()

def both_sqrt(y2):
    [y, negy] = prime_mod_sqrt(y2)
    return (y, negy) if y < negy else (negy, y)

def is_square(x):
    return legendre_symbol(x) == 1

def is_curve_point(x):
    y2 = (x*x*x + a*x + b) % p
    return is_square(y2)

def is_on_curve(x, y):
    return (y*y) % p == (x*x*x + a*x + b) % p

def random_curve_point():
    x = random_field_elt()
    y2 = (x*x*x + a*x + b) % p

    if not is_square(y2):
        return random_curve_point()

    (y1, y2) = both_sqrt(y2)
    y = y1 if next_bit() else y2
    return (x, y)

def point_add(P1, P2):
    if (P1 is None):
        return P2
    if (P2 is None):
        return P1
    if (P1[0] == P2[0] and P1[1] != P2[1]):
        return None
    if (P1 == P2):
        lam = ((3 * P1[0] * P1[0] + a) * pow(2 * P1[1], p - 2, p)) % p
    else:
        lam = ((P2[1] - P1[1]) * pow(P2[0] - P1[0], p - 2, p)) % p
    x3 = (lam * lam - P1[0] - P2[0]) % p
    return (x3, (lam * (P1[0] - x3) - P1[1]) % p)

def point_neg(P):
    return (P[0], p - P[1])

def point_mul(P, n):
    R = None
    for i in range(N):
        if ((n >> i) & 1):
            R = point_add(R, P)
        P = point_add(P, P)
    return R

def bytes_from_int(x):
    return x.to_bytes(95, byteorder="little")

def bytes_from_point(P):
    return (b'\x03' if P[1] & 1 else b'\x02') + bytes_from_int(P[0])

def point_from_bytes(b):
    if b[0:1] in [b'\x02', b'\x03']:
        odd = b[0] - 0x02
    else:
        return None
    x = int_from_bytes(b[1:96])
    y_sq = (pow(x, 3, p) + a_coeff * x + b_coeff) % p
    if not is_square(y_sq):
        return None
    y0 = prime_mod_sqrt(y_sq)[0]
    y = p - y0 if y0 & 1 != odd else y0
    return [x, y]

def int_from_bytes(b):
    return int.from_bytes(b, byteorder="little")

def bits_from_int(x):
    return [ (x >> i) & 1 for i in range(N) ]

def hash_blake2s(x):
    return hashlib.blake2s(x).digest()

def bits_from_bytes(bs):
    def bits_from_byte(b):
        return [ (b >> i) & 1 for i in range(8) ]

    return [b for by in bs for b in bits_from_byte(by)]

def schnorr_hash(msg):
    sign_state = [11755705189390821252061646515347634803721008034042221776076198442214047097736416191977544342102890624152325311676405596068127350375523649920335519154711182264164630561278473895587364446094244598242170557714936573775626048265230,
        27515311459815300529244367822740863112445008780714100624704075938494921938258491804705259071021760016622352448625351667277308772230882264292787686108079884001293592223592113739068864847707009580257641842924278675467231562803771,
        2396632434414439310737449031743778257385962871664374090342438175577792963806884089307026050137579268946498687720760431246070701978535951122430182545476853420233411791434797499475546364343179083506203526451182750041179531584522]

    (x, px, py, r, m) = msg
    state = poseidon.poseidon([int_from_bytes(x), int(px)], state=sign_state)
    state = poseidon.poseidon([int(py), int(r)], state=state)
    state = poseidon.poseidon([int_from_bytes(m)], state=state)
    res = poseidon.poseidon_digest(state)
    # challenge length = 128 bits
    return int_from_bytes(bytes_from_int(res)[:17])

def schnorr_sign(msg, seckey):
    (x, m) = msg
    if not (1 <= seckey <= n - 1):
        raise ValueError('The secret key must be an integer in the range 1..n-1.')
    k0 = int_from_bytes(
            hash_blake2s(bytes_from_int(seckey) + x + m))
    if k0 == 0:
        raise RuntimeError('Failure. This happens only with negligible probability.')
    R = point_mul(G, k0)
    k = n - k0 if (R[1] % 2 != 0) else k0
    (px, py) = point_mul(G, seckey)
    e = schnorr_hash((x, px, py, R[0], m))
    return bytes_from_int(R[0]) + bytes_from_int((k + e * seckey) % n)

def schnorr_verify(msg, pubkey, sig):
    if len(pubkey) != 96:
        raise ValueError('The public key must be a 96-byte array.')
    if len(sig) != 190:
        raise ValueError('The signature must be a 190-byte array.')
    P = point_from_bytes(pubkey)
    if (P is None):
        return False
    r = int_from_bytes(sig[0:95])
    s = int_from_bytes(sig[95:190])
    if (r >= p or s >= n):
        return False
    # field elts = [x, px, py, r], bitstrings = m
    (x, m) = msg
    (px, py) = P
    e = schnorr_hash((x, px, py, r, m))
    (ex, ey) = point_mul(P, e)
    R = point_add(point_mul(G, s), (ex, p-ey))
    (rx, ry) = R
    if R is None or (R[1] % 2 != 0) or R[0] != r:
        return False
    return True

if __name__ == '__main__':
    assert is_on_curve(G[0], G[1])

    msg = [bytes_from_int(25615870705115042543646988269442600291065870610810566568351522267883229178288637645455574607750193516168494708212492106060991223252842215604841819346335592341288722448465861172621202305332949789691389509124500513443739624704490), bytes_from_int(165263992375525314283137939099243432440837431930670926230162855107585165655283990966064758496729853898257922318576908240849)]
    xx = 0x000072076b1ced72c972633bcb6d7789de299887c8cdb4834fca1f71379277fd506f997fc96665ae7cdb78f60a355bf79b0972eddbbb54d1f11779672e70ff9270123a9c6c1d781a6754bd6b3a91c5682a0288e360a044044cba50a785462160
    ss = bytes_from_int(0x000151ddb1ecda8596755da7a42b71a5a580a95ef451d460ae5f9ad4c5b983b544a5082be1fad09bcfee3b7af86646ba0997c6e218c10c2faa1917b99d44e7af6daea04e5fcf117cc2ab52aaa825a1fcae62262303154b613d3c89f8fb8b5d16) + bytes_from_int(0x000176a0ec71ed9a5bd86a4d4a864c018f2b3b81acbbbe8652c307d176be08379d89154c3afd3d3c3434b9cd738b5e953dc376e5e806fa7596f78f784b86799d74063e1e6f67f43bc24b9a247a420290fca81e988dcf5168382caa287c6ce93d)

    MSG = (b'this is a test', b'an excellent test')
    KEY = random_field_elt()
    SIG = schnorr_sign(MSG, KEY)
    if schnorr_verify(MSG, bytes_from_point(point_mul(G, KEY)), SIG):
        print('Signature 1 verified')
    else:
        print('Signature 1 failed to verify')
        sys.exit(1)

    pkpk = b'\x03' + bytes_from_int(xx)
    if schnorr_verify(msg, pkpk, ss):
        print("Signature 2 verified")
    else:
        print('Signature 2 failed to verify')
        sys.exit(1)
