package lib

import (
	"errors"
	"fmt"
	"log"
)

func Cond[T any](a bool, b T, c T) T {
	if a {
		return b
	}
	return c
}

func Errorf(f string, args ...any) error {
	return errors.New(fmt.Sprintf(f, args...))
}

type Number interface {
	~byte | ~rune | ~int | ~uint | ~int64
}

func Assert(b bool) {
	if b {
		log.Panic("Assert Fails")
	}
}

func AssertLT[N Number](a, b N) {
	if a >= b {
		log.Panicf("AssertLT Fails: %v < %v", a, b)
	}
}

func Check(err error) {
	if err != nil {
		log.Panicf("Check Fail: %v", err)
	}
}
