//go:build main
// +build main

package main

import (
	"bufio"
	"bytes"
	"fmt"
	"log"
	"os"
)

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	// optionally, resize scanner's capacity for lines over 64K, see next example
	for scanner.Scan() {
		Do(scanner.Text())
	}

	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}
}

func Do(a string) {
	var z bytes.Buffer
	var state int
	for _, r := range a {
		switch state {
		case 0:
			if r == '$' {
				state = 1
				z.WriteString("flip---")
			} else {
				z.WriteRune(r)
			}
		case 1:
			if r == '"' {
				state = 2
			}
			z.WriteString(Flip13(string([]byte{byte(r)})))
		case 2:
			z.WriteRune(r)
		}
	}
	fmt.Println(z.String())
}

// tr A-Za-z ZYXWVUTSRQPONMLKJIHGFEDCBAzyxwvutsrqponmlkjihgfedcba
func Flip13(a string) string {
	var bb bytes.Buffer
	for _, c := range a {
		if 'a' <= c && c <= 'z' {
			letter := c - 'a'        // 0 to 25
			newLetter := 25 - letter // 25 to 0
			bb.WriteRune(newLetter + 'a')
		} else if 'A' <= c && c <= 'Z' {
			letter := c - 'A'        // 0 to 25
			newLetter := 25 - letter // 25 to 0
			bb.WriteRune(newLetter + 'A')
		} else {
			bb.WriteRune(c)
		}
	}
	return bb.String()
}
