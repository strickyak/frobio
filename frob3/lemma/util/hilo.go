package util

func Hi(x uint) byte {
	return 255 & byte(x>>8)
}

func Lo(x uint) byte {
	return 255 & byte(x)
}

func HiLo(a, b byte) uint {
	return (uint(a) << 8) | uint(b)
}

func HiLoBy(bb []byte) uint {
	return HiLo(bb[0], bb[1])
}
