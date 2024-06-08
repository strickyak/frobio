package lan

import (
	"flag"
	"fmt"
	"log"
	"net"

	. "github.com/strickyak/frobio/frob3/lemma/util"
)

var FlagConfigByDHCP = flag.Bool("config_by_dhcp", false, "Client gets IP and Gateway via dhcp")
var LAN = flag.String("lan", "", "Local IP address of interface for LAN Discovery Response.")

const LAN_CLIENT_PORT = 12113 // L=12 A=1 M=13
const LAN_SERVER_PORT = 12114 // L=12 A=1 N=14

var NextIP = byte(31)

func ListenForLan() {
	if *LAN == "" {
		return  // Lan listener not wanted.
	}

	localBind := fmt.Sprintf("%s:%d", *LAN, LAN_CLIENT_PORT)

	local, err := net.ResolveUDPAddr("udp4", localBind)
	if err != nil {
		log.Fatalf("cannot resolve UDP Broadcast local %q: %v", ":"+Str(LAN_SERVER_PORT), err)
	}
	local4 := local.AddrPort().Addr().As4()
	log.Printf("Lan discovery server local: %v port %v", local4, LAN_SERVER_PORT)

	remote, err := net.ResolveUDPAddr("udp4", "255.255.255.255:"+Str(LAN_CLIENT_PORT))
	if err != nil {
		log.Fatalf("cannot resolve UDP Broadcast remote %q: %v", "255.255.255.255:"+Str(LAN_CLIENT_PORT), err)
	}
	log.Printf("Lan discovery server remote: %v port %d", remote, LAN_CLIENT_PORT)

	conn, err := net.DialUDP("udp4", local, remote)
	if err != nil {
		log.Fatalf("cannot dial UDP Broadcast (%v, %v): %v", local, remote, err)
	}

	pc, err := net.ListenPacket("udp4", ":"+Str(LAN_SERVER_PORT))
	if err != nil {
		log.Fatalf("Cannot ListenPacket: %v", err)
	}
	log.Printf("Lan discovery server listening.")

	for {
		request := make([]byte, 512)

		n, addr, err := pc.ReadFrom(request)
		if err != nil {
			log.Fatalf("Cannot pc.ReadFrom: %v", err)
		}

		log.Printf("GOT LAN REQUEST FROM %v: % 3x", addr, request[:n])

		if request[0] == 'Q' && (request[1]|request[2]|request[3]) == 0 {

			reply := make([]byte, 96)
			reply[0] = 'R'
			copy(reply[4:8], request[4:8])

			var command string
			if *FlagConfigByDHCP {
				command = fmt.Sprintf("D ; W %s ; @", *LAN)
			} else {
				command = fmt.Sprintf("I %d.%d.%d.%d/24 ; W %s ; @", local4[0], local4[1], local4[2], NextIP, *LAN)
				NextIP++
				if NextIP > 250 {
					NextIP = 31 // start over
				}
			}
			copy(reply[16:], []byte(command))

			_, err := conn.Write(reply)
			if err != nil {
				log.Fatalf("cannot Write UDP Broadcast for LAN: %v", err)
			}
			log.Printf("SENT LAN REPLY: % 3x", reply)
		}
	}
}
