package main

import (
	"fmt"
	//"math"
)

func main() {
	// GenerateGravity1()  // Weak gravity replaced by pseudo-gravity.
	GenerateGravity2()
}

const W = 128
const H = 96

const Scale = 10000.0

func GenerateGravity1() {
	for y := 0; y < H/2; y += 2 {

		fmt.Printf("// [y=%3d]: ", y)
		for x := 0; x < W/2; x += 2 {
			sq := (x-W/2)*(x-W/2) + (y-H/2)*(y-H/2)

			var inv float64
			if sq != 0 {
				inv = Scale / float64(sq)
			}
			fmt.Printf("%10.3f ", inv)
		}
		fmt.Printf("\n\n")
		for x := 0; x < W/2; x += 2 {
			if x >= W/2-W/8 && y >= H/2-H/8 {
				fmt.Printf("  0, 0,\n")
				continue
			}

			sq := (x-W/2)*(x-W/2) + (y-H/2)*(y-H/2)
			var inv float64
			if sq != 0 {
				inv = Scale / float64(sq)
			}

			dx, dy := W/2 - x, H/2 - y
			var ratio float64
			var px, py int
			if dx==0 {
				py = int(inv)
			} else if dy == 0 {
				px = int(inv)
			} else {
				ratio = float64(W/2-x) / float64((W/2-x)+(H/2-y))
				px, py = int(ratio * inv), int((1.0-ratio) * inv)
			}
			fmt.Printf("  %3d, %3d, // %3d  (%.6f)\n", px, py, x, ratio)
		}
		fmt.Printf("\n\n")
	}
	fmt.Printf("\n\n")
}
func GenerateGravity2() {
	for y := H/2-H/8; y <= H/2; y += 1 {

		fmt.Printf("// [y=%3d]: [x=%3d]:", y, W/2-W/8)
		for x := W/2-W/8; x <= W/2; x += 1 {
			sq := (x-W/2)*(x-W/2) + (y-H/2)*(y-H/2)

			var inv float64
			if sq != 0 {
				inv = Scale / float64(sq)
			}

			fmt.Printf("%10.3f ", inv)
		}
		fmt.Printf("\n\n")
		for x := W/2-W/8; x <= W/2; x += 1 {
			sq := (x-W/2)*(x-W/2) + (y-H/2)*(y-H/2)

			var inv float64
			if sq != 0 {
				inv = Scale / float64(sq)
			}

			dx, dy := W/2 - x, H/2 - y
			var ratio float64
			var px, py int
			if dx==0 {
				py = int(inv)
			} else if dy == 0 {
				px = int(inv)
			} else {
				ratio = float64(W/2-x) / float64((W/2-x)+(H/2-y))
				px, py = int(ratio * inv), int((1.0-ratio) * inv)
			}
			fmt.Printf("  %3d, %3d, %3d, %3d, // %3d  (%.6f)\n", byte(px>>8), byte(px), byte(py>>8), byte(py), x, ratio)
		}
		fmt.Printf("\n\n")
	}
}
