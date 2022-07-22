package main

import (
    "flag"
    //"fmt"
    "net"
)

var LOCAL = flag.String("listen", "10.8.0.0:6969", "UDP network and port to listen on")
var DIR = flag.String("dir", "/tmp/tftpd", "Top directory to serve")

func main() {
	localAddr, err := net.ResolveUDPAddr("udp", *LOCAL)
    if err != nil {
        panic(err)
    }
	conn, err := net.ListenUDP("udp", localAddr)
    if err != nil {
        panic(err)
    }
    print(conn)

    buf := make([]byte, 600)
    n, addr, err := conn.ReadFromUDP(buf);

    n, err = conn.WriteToUDP(buf, addr)
    print(n)
}
