/*
  Copies Rom Image from stdin
	to a DECB header on stout,
	saying to load at $3000.

	It should not be exectued there,
	so the execution address is bogus.
*/
package main

import (
	"os"
)

func Check(err error) {
	if err != nil {
		panic(err)
	}
}
func Assert(cond bool) {
	if !cond {
		panic("Assert")
	}
}

func Hi(x int) byte {
	return byte(x >> 8)
}
func Lo(x int) byte {
	return byte(x)
}

func main() {
	buf := make([]byte, 256*256)
	size, err := os.Stdin.Read(buf)
	Check(err)
	Assert(size > 0)
	Assert(size < 256*256)

	// Header to load size bytes at $3000.
	count, err := os.Stdout.Write(
		[]byte{0, Hi(size), Lo(size), 0x30, 0x00})
	Check(err)
	Assert(count == 5)

	// ROM image Payload.
	count, err = os.Stdout.Write(buf[:size])
	Check(err)
	Assert(count == size)

	// Bogus execution addr trailer.
	count, err = os.Stdout.Write(
		[]byte{255, 0, 0, 0, 0})
	Check(err)
	Assert(count == 5)
}
