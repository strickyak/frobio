package lib

import (
	"log"
)

type Number interface {
	~byte | ~rune | ~int | ~uint | ~int64
}

func Assert(b bool) {
	if b {
		log.Panic("Assert Fail")
	}
}

func AssertLT[N Number](a, b N) {
	if a >= b {
		log.Panicf("AssertLT Fail: %v < %v", a, b)
	}
}

func Check(err error) {
	if err != nil {
		log.Panicf("Check Fail: %v", err)
	}
}
