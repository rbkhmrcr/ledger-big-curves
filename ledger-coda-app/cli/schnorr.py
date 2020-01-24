import hashlib
import binascii
import sys
from random import getrandbits
import poseidon

a = 11
b = 0x7DA285E70863C79D56446237CE2E1468D14AE9BB64B2BB01B10E60A5D5DFE0A25714B7985993F62F03B22A9A3C737A1A1E0FCF2C43D7BF847957C34CCA1E3585F9A80A95F401867C4E80F4747FDE5ABA7505BA6FCF2485540B13DFC8468A
a_coeff = a
b_coeff = b

p = 0x1C4C62D92C41110229022EEE2CDADB7F997505B8FAFED5EB7E8F96C97D87307FDB925E8A0ED8D99D124D9A15AF79DB26C5C28C859A99B3EEBCA9429212636B9DFF97634993AA4D6C381BC3F0057974EA099170FA13A4FD90776E240000001
n = 0x1C4C62D92C41110229022EEE2CDADB7F997505B8FAFED5EB7E8F96C97D87307FDB925E8A0ED8D99D124D9A15AF79DB117E776F218059DB80F0DA5CB537E38685ACCE9767254A4638810719AC425F0E39D54522CDD119F5E9063DE245E8001

G = (3458420969484235708806261200128850544017070333833944116801482064540723268149235477762870414664917360605949659630933184751526227993647030875167687492714052872195770088225183259051403087906158701786758441889742618916006546636728, 27460508402331965149626600224382137254502975979168371111640924721589127725376473514838234361114855175488242007431439074223827742813911899817930728112297763448010814764117701403540298764970469500339646563344680868495474127850569)
# G =(0xB0D6E141836D261DBE17959758B33A19987126CB808DFA411854CF0A44C0F4962ECA2A213FFEAA770DAD44F59F260AC64C9FCB46DA65CBC9EEBE1CE9B83F91A64B685106D5F1E4A05DDFAE9B2E1A567E0E74C1B7FF94CC3F361FB1F064AA,
#    0x30BD0DCB53B85BD013043029438966FFEC9438150AD06F59B4CC8DDA8BFF0FE5D3F4F63E46AC91576D1B4A15076774FEB51BA730F83FC9EB56E9BCC9233E031577A744C336E1EDFF5513BF5C9A4D234BCC4AD6D9F1B3FDF00E16446A8268)

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

    (x, px, py, r) = msg
    state = poseidon.poseidon([int_from_bytes(x), int(px)], state=sign_state)
    state = poseidon.poseidon([int(py), int(r)], state=state)
    # state = poseidon.poseidon([m], state=state) XXX
    res = poseidon.poseidon_digest(state)
    # challenge length = 128
    return int_from_bytes(bytes_from_int(res)[:128])

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
    e = schnorr_hash((x, px, py, R[0]))
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
    e = schnorr_hash((x, px, py, r))
    if r == 0x000158edcd4c48a4045830e7e4228a0d824a3732ba6fb5424dfe313ddbf1cd4c53a4c527c7cc9767f5dcba62a1152ff677b7e5b0d93c408e8b39c66d368cd94682d168683f1492537b93fba8d137d007d812b52f92e456ebcb91177bba4b60a6:
        assert e == 0x00003934f5dae71985b53669059e67e9e9894f1aebb11b4cc896e3c8545674b26f6dfcdf6643b7cc782e3a3c56c1c632f0fe8e537ac7f56bbfc41aa85d616b0425d7a8889c9070732eb0557122e6653d38ab61128c41b362992ca8f13d564a2f
    (ex, ey) = point_mul(P, e)
    R = point_add(point_mul(G, s), (ex, p-ey))
    if R is None or (R[1] % 2 != 0) or R[0] != r:
        print(R[0])
        print(r)
        return False
    return True

if __name__ == '__main__':
    assert is_curve_point(G[0])
    MSG = (b'this is a test', b'an excellent test')
    KEY = random_field_elt()
    SIG = schnorr_sign(MSG, KEY)
    if schnorr_verify(MSG, bytes_from_point(point_mul(G, KEY)), SIG):
        print('Signature 1 verified')
    else:
        print('Signature 1 failed to verify')
        sys.exit(1)

    msg = [bytes_from_int(25615870705115042543646988269442600291065870610810566568351522267883229178288637645455574607750193516168494708212492106060991223252842215604841819346335592341288722448465861172621202305332949789691389509124500513443739624704490), bytes_from_int(165263992375525314283137939099243432440837431930670926230162855107585165655283990966064758496729853898257922318576908240849)]
    print(hex(25615870705115042543646988269442600291065870610810566568351522267883229178288637645455574607750193516168494708212492106060991223252842215604841819346335592341288722448465861172621202305332949789691389509124500513443739624704490))
    print(hex(165263992375525314283137939099243432440837431930670926230162855107585165655283990966064758496729853898257922318576908240849))
    keyx = 10551932552288404268471876784066179540075993514619627475810154681265731503848345236869614822262595684404683450023872748280053822651035682354773112233967924938626433271064352765366912532816572973581508364105657856637532501320032
    keyy = 41396185721063460085667958656928492292262244305644537076418786203838831474688775166371394317334447476021191354079249366422299414575425706623634945421230874239790174420259736648550892231502609508610889111707764651968852198331491
    key = b'\x03' + bytes_from_int(keyx)
    assert is_curve_point(keyx)
    sig = bytes_from_int(0x000158edcd4c48a4045830e7e4228a0d824a3732ba6fb5424dfe313ddbf1cd4c53a4c527c7cc9767f5dcba62a1152ff677b7e5b0d93c408e8b39c66d368cd94682d168683f1492537b93fba8d137d007d812b52f92e456ebcb91177bba4b60a6) + bytes_from_int(0x0001ac0d63b3e3bdd9b5ecda5c1e213b42b25cb9827f889940ea288a7974b083eac2c104a12abf52b89e7f0a0a0bf974c51fe3239db3fd4ef96f9a9207a3c7d1e49f09b3c8a4109ad82b63735e488215daa8a52aab67c344943e1ea50829ee56)
    if schnorr_verify(msg, key, sig):
        print('Signature 2 verified')
    else:
        print('Signature 2 failed to verify')
        sys.exit(1)
