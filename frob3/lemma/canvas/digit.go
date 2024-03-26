package canvas

import (
	"fmt"
)

var Digits = [10][5]string{
	{
		" 0 ",
		"0 0",
		"0 0",
		"0 0",
		" 0 ",
	},
	{
		" 1 ",
		" 1 ",
		" 1 ",
		" 1 ",
		" 1 ",
	},
	{
		"22 ",
		"  2",
		" 2 ",
		"2  ",
		"222",
	},
	{
		"33 ",
		"  3",
		"33 ",
		"  3",
		"33 ",
	},
	{
		"4 4",
		"4 4",
		"444",
		"  4",
		"  4",
	},
	{
		"555",
		"5  ",
		"55 ",
		"  5",
		"55 ",
	},
	{
		"666",
		"6  ",
		"666",
		"6 6",
		"666",
	},
	{
		"777",
		"  7",
		"  7",
		"  7",
		"  7",
	},
	{
		"888",
		"8 8",
		"888",
		"8 8",
		"888",
	},
	{
		"999",
		"9 9",
		"999",
		"  9",
		"  9",
	},
}

func Print(z Color, c *Canvas, x int, y int, n byte) {
	x = 96 + x*5 + 2
	y = y * 8
	d := Digits[n]
	for i := 0; i < 5; i++ {
		s := d[i]
		for j := 0; j < 3; j++ {
			if s[j] != ' ' {
				c.Set(x+j, y+i, z, W)
			}
		}
	}
}

func PrintInt(z Color, c *Canvas, y int, n uint) {
	s := fmt.Sprintf("%6d", n)
	for i := 0; i < 6; i++ {
		if s[i] != ' ' {
			Print(z, c, i, y, s[i]-'0')
		}
	}
}
