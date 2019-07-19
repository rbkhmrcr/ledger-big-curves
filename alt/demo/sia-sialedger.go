package main

import (
	"bytes"
	"encoding/base64"
	"encoding/binary"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"math"
	"os"
	"strconv"

	"github.com/karalabe/hid"
	"gitlab.com/NebulousLabs/Sia/types"
	"lukechampine.com/flagg"
)

var DEBUG bool

type hidFramer struct {
	rw  io.ReadWriter
	seq uint16
	buf [64]byte
	pos int
}

func (hf *hidFramer) Reset() {
	hf.seq = 0
}

func (hf *hidFramer) Write(p []byte) (int, error) {
	if DEBUG {
		fmt.Println("HID <=", hex.EncodeToString(p))
	}
	// split into 64-byte chunks
	chunk := make([]byte, 64)
	binary.BigEndian.PutUint16(chunk[:2], 0x0101)
	chunk[2] = 0x05
	var seq uint16
	buf := new(bytes.Buffer)
	binary.Write(buf, binary.BigEndian, uint16(len(p)))
	buf.Write(p)
	for buf.Len() > 0 {
		binary.BigEndian.PutUint16(chunk[3:5], seq)
		n, _ := buf.Read(chunk[5:])
		if n, err := hf.rw.Write(chunk[:5+n]); err != nil {
			return n, err
		}
		seq++
	}
	return len(p), nil
}

func (hf *hidFramer) Read(p []byte) (int, error) {
	if hf.seq > 0 && hf.pos != 64 {
		// drain buf
		n := copy(p, hf.buf[hf.pos:])
		hf.pos += n
		return n, nil
	}
	// read next 64-byte packet
	if n, err := hf.rw.Read(hf.buf[:]); err != nil {
		return 0, err
	} else if n != 64 {
		panic("read less than 64 bytes from HID")
	}
	// parse header
	channelID := binary.BigEndian.Uint16(hf.buf[:2])
	commandTag := hf.buf[2]
	seq := binary.BigEndian.Uint16(hf.buf[3:5])
	if channelID != 0x0101 {
		return 0, fmt.Errorf("bad channel ID 0x%x", channelID)
	} else if commandTag != 0x05 {
		return 0, fmt.Errorf("bad command tag 0x%x", commandTag)
	} else if seq != hf.seq {
		return 0, fmt.Errorf("bad sequence number %v (expected %v)", seq, hf.seq)
	}
	hf.seq++
	// start filling p
	n := copy(p, hf.buf[5:])
	hf.pos = 5 + n
	return n, nil
}

type APDU struct {
	CLA     byte
	INS     byte
	P1, P2  byte
	Payload []byte
}

type apduFramer struct {
	hf  *hidFramer
	buf [2]byte // to read APDU length prefix
}

func (af *apduFramer) Exchange(apdu APDU) ([]byte, error) {
	if len(apdu.Payload) > 255 {
		panic("APDU payload cannot exceed 255 bytes")
	}
	af.hf.Reset()
	data := append([]byte{
		apdu.CLA,
		apdu.INS,
		apdu.P1, apdu.P2,
		byte(len(apdu.Payload)),
	}, apdu.Payload...)
	if _, err := af.hf.Write(data); err != nil {
		return nil, err
	}

	// read APDU length
	if _, err := io.ReadFull(af.hf, af.buf[:]); err != nil {
		return nil, err
	}
	// read APDU payload
	respLen := binary.BigEndian.Uint16(af.buf[:2])
	resp := make([]byte, respLen)
	_, err := io.ReadFull(af.hf, resp)
	if DEBUG {
		fmt.Println("HID =>", hex.EncodeToString(resp))
	}
	return resp, err
}

type NanoS struct {
	device *apduFramer
}

type ErrCode uint16

func (c ErrCode) Error() string {
	return fmt.Sprintf("Error code 0x%x", uint16(c))
}

const codeSuccess = 0x9000
const codeUserRejected = 0x6985
const codeInvalidParam = 0x6b01

var errUserRejected = errors.New("user denied request")
var errInvalidParam = errors.New("invalid request parameters")

func (n *NanoS) Exchange(cmd byte, p1, p2 byte, data []byte) (resp []byte, err error) {
	resp, err = n.device.Exchange(APDU{
		CLA:     0xe0,
		INS:     cmd,
		P1:      p1,
		P2:      p2,
		Payload: data,
	})
	if err != nil {
		return nil, err
	} else if len(resp) < 2 {
		return nil, errors.New("APDU response missing status code")
	}
	code := binary.BigEndian.Uint16(resp[len(resp)-2:])
	resp = resp[:len(resp)-2]
	switch code {
	case codeSuccess:
		err = nil
	case codeUserRejected:
		err = errUserRejected
	case codeInvalidParam:
		err = errInvalidParam
	default:
		err = ErrCode(code)
	}
	return
}

const (
	cmdGetVersion   = 0x01
	cmdGetPublicKey = 0x02
	cmdSignHash     = 0x04
	cmdCalcTxnHash  = 0x08

	p1First = 0x00
	p1More  = 0x80

	p2DisplayAddress = 0x00
	p2DisplayPubkey  = 0x01
	p2DisplayHash    = 0x00
	p2SignHash       = 0x01
)

func (n *NanoS) GetVersion() (version string, err error) {
	resp, err := n.Exchange(cmdGetVersion, 0, 0, nil)
	if err != nil {
		return "", err
	} else if len(resp) != 3 {
		return "", errors.New("version has wrong length")
	}
	return fmt.Sprintf("v%d.%d.%d", resp[0], resp[1], resp[2]), nil
}

func (n *NanoS) GetPublicKey(index uint32) (pubkey [32]byte, err error) {
	encIndex := make([]byte, 4)
	binary.LittleEndian.PutUint32(encIndex, index)

	resp, err := n.Exchange(cmdGetPublicKey, 0, p2DisplayPubkey, encIndex)
	if err != nil {
		return [32]byte{}, err
	}
	if copy(pubkey[:], resp) != len(pubkey) {
		return [32]byte{}, errors.New("pubkey has wrong length")
	}
	return
}

func (n *NanoS) GetAddress(index uint32) (addr types.UnlockHash, err error) {
	encIndex := make([]byte, 4)
	binary.LittleEndian.PutUint32(encIndex, index)

	resp, err := n.Exchange(cmdGetPublicKey, 0, p2DisplayAddress, encIndex)
	if err != nil {
		return types.UnlockHash{}, err
	}
	err = addr.LoadString(string(resp[32:]))
	return
}

func (n *NanoS) SignHash(hash [32]byte, keyIndex uint32) (sig [64]byte, err error) {
	encIndex := make([]byte, 4)
	binary.LittleEndian.PutUint32(encIndex, keyIndex)

	resp, err := n.Exchange(cmdSignHash, 0, 0, append(encIndex, hash[:]...))
	if err != nil {
		return [64]byte{}, err
	}
	if copy(sig[:], resp) != len(sig) {
		return [64]byte{}, errors.New("signature has wrong length")
	}
	return
}

func (n *NanoS) CalcTxnHash(txn types.Transaction, sigIndex uint16) (hash [32]byte, err error) {
	buf := new(bytes.Buffer)
	binary.Write(buf, binary.LittleEndian, uint32(0)) // keyIndex
	binary.Write(buf, binary.LittleEndian, sigIndex)
	txn.MarshalSia(buf)

	var resp []byte
	for buf.Len() > 0 {
		var p1 byte = p1More
		if resp == nil {
			p1 = p1First
		}
		resp, err = n.Exchange(cmdCalcTxnHash, p1, p2DisplayHash, buf.Next(255))
		if err != nil {
			return [32]byte{}, err
		}
	}
	if copy(hash[:], resp) != len(hash) {
		return [32]byte{}, errors.New("hash has wrong length")
	}
	return
}

func (n *NanoS) SignTxn(txn types.Transaction, sigIndex uint16, keyIndex uint32) (sig [64]byte, err error) {
	buf := new(bytes.Buffer)
	binary.Write(buf, binary.LittleEndian, keyIndex)
	binary.Write(buf, binary.LittleEndian, sigIndex)
	txn.MarshalSia(buf)

	var resp []byte
	for buf.Len() > 0 {
		var p1 byte = p1More
		if resp == nil {
			p1 = p1First
		}
		resp, err = n.Exchange(cmdCalcTxnHash, p1, p2SignHash, buf.Next(255))
		if err != nil {
			return [64]byte{}, err
		}
	}
	if copy(sig[:], resp) != len(sig) {
		return [64]byte{}, errors.New("signature has wrong length")
	}
	return
}

func OpenNanoS() (*NanoS, error) {
	const (
		ledgerVendorID       = 0x2c97
		ledgerNanoSProductID = 0x0001
		//ledgerUsageID        = 0xffa0
	)

	// search for Nano S
	devices := hid.Enumerate(ledgerVendorID, ledgerNanoSProductID)
	if len(devices) == 0 {
		return nil, errors.New("Nano S not detected")
	} else if len(devices) > 1 {
		return nil, errors.New("Unexpected error -- Is the Sia wallet app running?")
	}

	// open the device
	device, err := devices[0].Open()
	if err != nil {
		return nil, err
	}

	// wrap raw device I/O in HID+APDU protocols
	return &NanoS{
		device: &apduFramer{
			hf: &hidFramer{
				rw: device,
			},
		},
	}, nil
}

func parseIndex(s string) uint32 {
	index, err := strconv.ParseUint(s, 10, 32)
	if err != nil {
		log.Fatalln("Couldn't parse index:", err)
	} else if index > math.MaxUint32 {
		log.Fatalf("Index too large (max %v)", math.MaxUint32)
	}
	return uint32(index)
}

const (
	rootUsage = `Usage:
    sialedger [flags] [action]

Actions:
    addr            generate an address
    pubkey          generate a pubkey
    hash            sign a trusted hash
    txn             sign a transaction
`
	debugUsage = `print raw APDU exchanges`

	versionUsage = `Usage:
	sialedger version

Prints the version of the sialedger binary, as well as the version reported by
the Sia Ledger Nano S app (if available).
`
	addrUsage = `Usage:
	sialedger addr [key index]

Generates an address using the public key with the specified index.
`
	pubkeyUsage = `Usage:
	sialedger pubkey [key index]

Generates the public key with the specified index.
`
	hashUsage = `Usage:
	sialedger hash [hex-encoded hash] [key index]

Signs a 256-bit hash using the private key with the specified index. The hash
must be hex-encoded.

Only sign hashes you trust. In practice, it is very difficult
to calculate a hash in a trusted manner.
`
	txnUsage = `Usage:
	sialedger txn [flags] [txn.json] [sig index] [key index]

Calculates and signs the hash of a transaction using the private key with the
specified key index. The CoveredFields of the specified TransactionSignature
must set WholeTransaction = true.
`
	txnHashUsage = `calculate the transaction hash, but do not sign it`
)

func main() {
	log.SetFlags(0)
	rootCmd := flagg.Root
	rootCmd.Usage = flagg.SimpleUsage(rootCmd, rootUsage)
	rootCmd.BoolVar(&DEBUG, "apdu", false, debugUsage)

	versionCmd := flagg.New("version", versionUsage)
	addrCmd := flagg.New("addr", addrUsage)
	pubkeyCmd := flagg.New("pubkey", pubkeyUsage)
	hashCmd := flagg.New("hash", hashUsage)
	txnCmd := flagg.New("txn", txnUsage)
	txnHash := txnCmd.Bool("sighash", false, txnHashUsage)

	cmd := flagg.Parse(flagg.Tree{
		Cmd: rootCmd,
		Sub: []flagg.Tree{
			{Cmd: versionCmd},
			{Cmd: addrCmd},
			{Cmd: pubkeyCmd},
			{Cmd: hashCmd},
			{Cmd: txnCmd},
		},
	})
	args := cmd.Args()

	var nanos *NanoS
	if cmd != rootCmd && cmd != versionCmd {
		var err error
		nanos, err = OpenNanoS()
		if err != nil {
			log.Fatalln("Couldn't open device:", err)
		}
	}

	switch cmd {
	case rootCmd:
		if len(args) != 0 {
			rootCmd.Usage()
			return
		}
		fallthrough
	case versionCmd:
		// try to get Nano S app version
		var appVersion string
		nanos, err := OpenNanoS()
		if err != nil {
			appVersion = "(could not connect to Nano S)"
		} else if appVersion, err = nanos.GetVersion(); err != nil {
			appVersion = "(could not read version from Nano S: " + err.Error() + ")"
		}

		fmt.Printf("%s v0.1.0\n", os.Args[0])
		fmt.Println("Nano S app version:", appVersion)

	case addrCmd:
		if len(args) != 1 {
			addrCmd.Usage()
			return
		}
		addr, err := nanos.GetAddress(parseIndex(args[0]))
		if err != nil {
			log.Fatalln("Couldn't get address:", err)
		}
		fmt.Println(addr)

	case pubkeyCmd:
		if len(args) != 1 {
			pubkeyCmd.Usage()
			return
		}
		pubkey, err := nanos.GetPublicKey(parseIndex(args[0]))
		if err != nil {
			log.Fatalln("Couldn't get public key:", err)
		}
		pk := types.Ed25519PublicKey(pubkey)
		fmt.Println(pk.String())

	case hashCmd:
		if len(args) != 2 {
			hashCmd.Usage()
			return
		}
		var hash [32]byte
		hashBytes, err := hex.DecodeString(args[0])
		if err != nil {
			log.Fatalln("Couldn't read hash:", err)
		} else if len(hashBytes) != 32 {
			log.Fatalf("Wrong hex hash length (%v, wanted 32)", len(hashBytes))
		}
		copy(hash[:], hashBytes)

		sig, err := nanos.SignHash(hash, parseIndex(args[1]))
		if err != nil {
			log.Fatalln("Couldn't get signature:", err)
		}
		fmt.Println(base64.StdEncoding.EncodeToString(sig[:]))

	case txnCmd:
		if (*txnHash && len(args) != 2) || (!*txnHash && len(args) != 3) {
			txnCmd.Usage()
			return
		}
		txnBytes, err := ioutil.ReadFile(args[0])
		if err != nil {
			log.Fatalln("Couldn't read transaction:", err)
		}
		var txn types.Transaction
		if err := json.Unmarshal(txnBytes, &txn); err != nil {
			log.Fatalln("Couldn't decode transaction:", err)
		}
		sigIndex := uint16(parseIndex(args[1]))

		if *txnHash {
			sighash, err := nanos.CalcTxnHash(txn, sigIndex)
			if err != nil {
				log.Fatalln("Couldn't get hash:", err)
			}
			fmt.Println(hex.EncodeToString(sighash[:]))
		} else {
			sig, err := nanos.SignTxn(txn, sigIndex, parseIndex(args[2]))
			if err != nil {
				log.Fatalln("Couldn't get signature:", err)
			}
			fmt.Println(base64.StdEncoding.EncodeToString(sig[:]))
		}
	}
}
