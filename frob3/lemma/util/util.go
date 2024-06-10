package util

import (
	"errors"
	"fmt"
	"log"
	// "reflect"
	// "unsafe"
)

/*
func ID(obj any) string {
	tn := reflect.TypeOf(obj).Name()
	hash := Hash(obj)
	return Format("%s~%d", tn, hash)
}

func Hash(obj any) uint {
	return uint(Addr(obj) % 9999)
}

func Addr(obj any) uintptr {
	return uintptr(reflect.ValueOf(obj).Elem().Elem().UnsafePointer())
	// return uintptr(unsafe.Pointer(obj))
}
*/

func Value[T any](value T, err error) T {
	if err != nil {
		log.Panicf("Error: %v", err)
	}
	return value
}

func Cond[T any](a bool, b T, c T) T {
	if a {
		return b
	}
	return c
}

func Min[T Number](b T, c T) T {
	if b < c {
		return b
	}
	return c
}

func Max[T Number](b T, c T) T {
	if b > c {
		return b
	}
	return c
}

func CatSlices[T any](args ...[]T) (result []T) {
	for _, a := range args {
		result = append(result, a...)
	}
	return
}

func Errorf(f string, args ...any) error {
	return errors.New(fmt.Sprintf(f, args...))
}

type Number interface {
	~int8 | ~int16 | ~int32 | ~uint8 | ~uint16 | ~uint32 | ~int | ~uint | ~int64 | ~uint64 | ~uintptr
}

func Assert(b bool, args ...any) {
	if !b {
		s := "Assert Fails"
		for _, x := range args {
			s += fmt.Sprintf(" ; %v", x)
		}
		log.Panic(s)
	}
}

func AssertEQ[N Number](a, b N, args ...any) {
	if a != b {
		s := fmt.Sprintf("AssertEQ Fails: (%v .EQ. %v)", a, b)
		for _, x := range args {
			s += fmt.Sprintf(" ; %v", x)
		}
		log.Panic(s)
	}
}

func AssertGT[N Number](a, b N, args ...any) {
	if a <= b {
		s := fmt.Sprintf("AssertGT Fails: (%v .GT. %v)", a, b)
		for _, x := range args {
			s += fmt.Sprintf(" ; %v", x)
		}
		log.Panic(s)
	}
}

func AssertGE[N Number](a, b N, args ...any) {
	if a < b {
		s := fmt.Sprintf("AssertGE Fails: (%v .GE. %v)", a, b)
		for _, x := range args {
			s += fmt.Sprintf(" ; %v", x)
		}
		log.Panic(s)
	}
}

func AssertLT[N Number](a, b N, args ...any) {
	if a >= b {
		s := fmt.Sprintf("AssertLT Fails: (%v .LT. %v)", a, b)
		for _, x := range args {
			s += fmt.Sprintf(" ; %v", x)
		}
		log.Panic(s)
	}
}

func AssertLE[N Number](a, b N, args ...any) {
	if a > b {
		s := fmt.Sprintf("AssertLE Fails: (%v .LE. %v)", a, b)
		for _, x := range args {
			s += fmt.Sprintf(" ; %v", x)
		}
		log.Panic(s)
	}
}

func Check(err error, args ...any) {
	if err != nil {
		s := fmt.Sprintf("Check Fails: %v", err)
		for _, x := range args {
			s += fmt.Sprintf(" ; %v", x)
		}
		log.Panic(s)
	}
}

func Str[T any](a T) string {
	return fmt.Sprintf("%v", a)
}

func Repr[T any](a T) string {
	return fmt.Sprintf("%#v", a)
}

var Format = fmt.Sprintf
var Log = log.Printf
var Panic = log.Panicf
var Fatal = log.Fatalf

func BytesFormat(format string, args ...any) []byte {
	return []byte(Format(format, args...))
}

// LenSlice is for slices; it returns uint.
func LenSlice[T any](x []T) uint {
	return uint(len(x))
}

// LenStr is for strings; it returns uint.
func LenStr(s string) uint { // returns unsigned
	return uint(len(s))
}

// InSlice tells whether x is in slice.
func InSlice[T comparable_](x T, slice []T) bool {
	for _, e := range slice {
		if e == x {
			return true
		}
	}
	return false
}

// Until Go 1.20:
type comparable_ interface {
	~int8 | ~uint8 | ~int16 | ~uint16 | ~int32 | ~uint32 | ~int | ~uint | ~int64 | ~uint64 | ~string
}

func NonBlockingReadChan[T any](c <-chan *T) (*T, bool) {
	select {
	case p, ok := <-c:
		if !ok {
			log.Panicf("Read on bad channel of type %T", p)
		}
		return p, true
	default:
		return nil, false
	}
}

func NonBlockingWriteChan[T any](c chan<- *T, p *T) (ok bool) {
	select {
	case c <- p:
		return true
	default:
		return false
	}
}
