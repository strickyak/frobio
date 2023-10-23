package lib

import (
	"fmt"
	"log"
	"net"
)

const LAN_CLIENT_PORT = 12113 // L=12 A=1 M=13
const LAN_SERVER_PORT = 12114 // L=12 A=1 N=14

func ListenForLan() {

	local, err := net.ResolveUDPAddr("udp4", ":"+Str(LAN_SERVER_PORT))
	if err != nil {
		log.Fatalf("cannot resolve UDP Broadcast local %q: %v", ":"+Str(LAN_SERVER_PORT), err)
	}
	println("Lan 1")
	remote, err := net.ResolveUDPAddr("udp4", "255.255.255.255:"+Str(LAN_CLIENT_PORT))
	if err != nil {
		log.Fatalf("cannot resolve UDP Broadcast remote %q: %v", "255.255.255.255:"+Str(LAN_CLIENT_PORT), err)
	}
	println("Lan 2")

	conn, err := net.DialUDP("udp4", local, remote)
	if err != nil {
		log.Fatal("cannot dial UDP Broadcast (%v, %v): %v", err)
	}
	println("Lan 3")

	for {
		request := make([]byte, 512)
		n, addr, err := conn.ReadFromUDP(request)
		if err != nil {
			log.Fatalf("cannot Write UDP Broadcast for LAN: %v", err)
		}
		log.Printf("GOT LAN REQUEST FROM %v: %v", addr, request[:n])

		if request[0] == 'Q' && (request[1]|request[2]|request[3]) == 0 {

			reply := make([]byte, 96)
			reply[0] = 'R'
			copy(reply[4:8], request[4:8])

			command := fmt.Sprintf("I 192.168.23.%d/24 ; W 192.168.23.23 ; @", Next)
			copy(reply[16:], []byte(command))
			Next++
			if Next > 250 {
				Next = 32 // start over
			}

			_, err := conn.Write(reply)
			if err != nil {
				log.Fatalf("cannot Write UDP Broadcast for LAN: %v", err)
			}
			log.Printf("SENT LAN REPLY %v", reply)
		}
	}
}

var Next = byte(32)
