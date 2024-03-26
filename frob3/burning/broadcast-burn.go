/*
  broadcast-burn

  Send Broadcast UDP packets, repeatedly, for a rescue burn.
  Each packet must be exactly 128 bytes.
  They will be sent at a rate of 10 per second.

  The first 59 bytes are for a description, in VDG's version
  of upper-case ASCII.  The -d option sets this description.

  Starting at offset 59 is this 5-byte struct:
		// struct tiny_burn {
		//         byte ith_record;   // 59
		//         byte num_records;  // 60
		//         byte payload_len;  // 61
		//         word payload_addr; // 62
		// };

  The final 64 bytes are up to 64 bytes of payload.
  That payload has the given length and destination address,
  which should be in the range $C300-$DFFF.

  This command needs to know which address it binds to,
  so it knows which interface to broadcast on.
  The default is the Class A Lemma Server default, 10.23.23.23.
  Change it with the -b option.
*/
package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"net"
	"os"
	"strings"
	"time"
)

const BURN_PORT = 12323

var Conn *net.UDPConn

var Description = flag.String("d", "UNKNOWN DESCRIPTION", "describe the data")
var Bind = flag.String("b", "10.23.23.23", "bind to interface at this local address")

func main() {
	flag.Parse()

	bb, err := ioutil.ReadAll(os.Stdin)
	if err != nil {
		log.Fatalf("Error reading from Stdin: %v", err)
	}

	localBind := fmt.Sprintf("%s:%d", *Bind, BURN_PORT)

	local, err := net.ResolveUDPAddr("udp4", localBind)
	if err != nil {
		log.Fatalf("cannot resolve UDP Broadcast local %q: %v", ":"+Str(BURN_PORT), err)
	}
	println("Burn 1")

	remote, err := net.ResolveUDPAddr("udp4", "255.255.255.255:"+Str(BURN_PORT))
	if err != nil {
		log.Fatalf("cannot resolve UDP Broadcast remote %q: %v", "255.255.255.255:"+Str(BURN_PORT), err)
	}
	println("Burn 2", Str(remote))

	Conn, err = net.DialUDP("udp4", local, remote)
	if err != nil {
		log.Fatalf("cannot dial UDP Broadcast (%v, %v): %v", local, remote, err)
	}
	println("Burn 3")

	TransmitAll(bb)
}

func Str(a any) string {
	return fmt.Sprintf("%v", a)
}

func TransmitAll(bb []byte) {
LOOP:
	for len(bb) > 0 {
		cmd := bb[0]
		n := (uint(bb[1]) << 8) | uint(bb[2])
		p := (uint(bb[3]) << 8) | uint(bb[4])
		switch cmd {
		case 0:
			TransmitGroup(bb[5:5+n], p)
			bb = bb[5+n:]
		case 255:
			if len(bb) != 5 || n != 0 {
				log.Fatalf("Bad remainder of %d bytes: %v", len(bb), bb)
			}
			break LOOP
		default:
			panic(cmd)
		}
	}
	for {
		TransmitSegments()
	}
}

var Segments []*Segment
var Data []byte
var Base uint
var Offset uint

type Segment struct {
	Addr uint
	Data []byte
}

func Keep() {
	if len(Data) == 0 {
		return
	}
	Segments = append(Segments, &Segment{
		Addr: Base + Offset,
		Data: Data,
	})
	Data = nil
	Base = 0
	Offset = 0
}

func TransmitGroup(bb []byte, p uint) {
	Data = nil
	Base = 0
	Offset = 0

	for i := uint(0); i < uint(len(bb)); i++ {
		addr := p + i
		base := addr &^ 63
		if base == Base {
			// In the same segment of 64 bytes
			Data = append(Data, bb[i])
		} else {
			// In different segment of 64 bytes
			Keep()
			Data = nil
			Data = append(Data, bb[i])
			Base = base
			Offset = 63 & addr
		}
	}
	Keep()
}

func TransmitSegments() {
	const HUNDRED_MILLI_SECONDS = "100ms"
	hundredMilliSesconds, err := time.ParseDuration(HUNDRED_MILLI_SECONDS)
	if err != nil {
		panic(HUNDRED_MILLI_SECONDS)
	}
	desc := strings.ToUpper(*Description)
	for ith, seg := range Segments {
		// struct tiny_burn {
		//         byte ith_record;   // 59
		//         byte num_records;  // 60
		//         byte payload_len;  // 61
		//         word payload_addr; // 62
		// };
		var packet [128]byte
		copy(packet[:], []byte(fmt.Sprintf("%q (%d/%d)     ", desc, ith, len(Segments))))
		packet[59] = byte(ith)
		packet[60] = byte(len(Segments))
		packet[61] = byte(len(seg.Data))
		packet[62] = byte(seg.Addr >> 8)
		packet[63] = byte(seg.Addr)
		copy(packet[64:], seg.Data)

		TransmitPacket(packet)
		time.Sleep(hundredMilliSesconds)
	}
}

func TransmitPacket(packet [128]byte) {
	_, err := Conn.Write(packet[:])
	if err != nil {
		log.Fatalf("cannot Send UDP Broadcast: %v", err)
	}
	log.Printf("SENT %d of %d, %d at $%02x%02x: %q", packet[59], packet[60], packet[61], packet[62], packet[63], packet)
}
