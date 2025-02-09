package main

import (
	"bytes"
	. "fmt"
	"io"
	"io/ioutil"
	"log"
)

func main() {
	PrintCalls()
}

// assumes all args are on stack.
func (c *Call) FormatArgsForCmoc() string {
	var bb bytes.Buffer
	off := 0
	if c.a != "" {
		Fprintf(&bb, "    /*%d*/ byte %s,\n", off, c.a)
		c.a_off = off
		off += 2
	}
	if c.b != "" {
		Fprintf(&bb, "    /*%d*/ byte %s,\n", off, c.b)
		c.b_off = off
		off += 2
	}
	if c.d != "" {
		Fprintf(&bb, "    /*%d*/ word %s,\n", off, c.d)
		c.d_off = off
		off += 2
	}
	if c.x != "" {
		Fprintf(&bb, "    /*%d*/ word %s,\n", off, c.x)
		c.x_off = off
		off += 2
	}
	if c.y != "" {
		Fprintf(&bb, "    /*%d*/ word %s,\n", off, c.y)
		c.y_off = off
		off += 2
	}
	if c.u != "" {
		Fprintf(&bb, "    /*%d*/ word %s,\n", off, c.u)
		c.u_off = off
		off += 2
	}
	if c.ra != "" {
		Fprintf(&bb, "    /*%d*/ byte* %s_out,\n", off, c.ra)
		c.ra_off = off
		off += 2
	}
	if c.rb != "" {
		Fprintf(&bb, "    /*%d*/ byte* %s_out,\n", off, c.rb)
		c.rb_off = off
		off += 2
	}
	if c.rd != "" {
		Fprintf(&bb, "    /*%d*/ word* %s_out,\n", off, c.rd)
		c.rd_off = off
		off += 2
	}
	if c.rx != "" {
		Fprintf(&bb, "    /*%d*/ word* %s_out,\n", off, c.rx)
		c.rx_off = off
		off += 2
	}
	if c.ry != "" {
		Fprintf(&bb, "    /*%d*/ word* %s_out,\n", off, c.ry)
		c.ry_off = off
		off += 2
	}
	if c.ru != "" {
		Fprintf(&bb, "    /*%d*/ word* %s_out,\n", off, c.ru)
		c.ru_off = off
		off += 2
	}
	s := bb.String()
	n := len(s)
	if n > 0 {
		s = s[:n-2] + "\n" // remove final comma
	}
	return s
}

func PrintAsmForCmoc(c *Call, w io.Writer) {
	P := func(format string, args ...any) {
		Fprintf(w, format+"\n", args...)
	}

	P("")
	P("*** %s *** %s", c.name, c.desc)
	P("")
	P("*** %#v", c)
	P("")
	P("   EXPORT _NewOs9%s", c.name[2:])
	P("_NewOs9%s", c.name[2:])

	// Always save and restore Y,U
	P("    pshs Y,U")
	base := 6

	if c.a != "" {
		P("    lda %d,s  ; %s", base+c.a_off+1, c.a)
	}
	if c.b != "" {
		P("    ldb %d,s  ; %s", base+c.b_off+1, c.b)
	}
	if c.d != "" {
		P("    ldd %d,s  ; %s", base+c.d_off, c.d)
	}
	if c.x != "" {
		P("    ldx %d,s  ; %s", base+c.x_off, c.x)
	}
	if c.y != "" {
		P("    ldy %d,s  ; %s", base+c.y_off, c.y)
	}
	if c.u != "" {
		P("    ldu %d,s  ; %s", base+c.u_off, c.u)
	}

	P("    swi2")
	P("    fcb $%02x ; %s", c.number, c.name)
	P("    bcc OK__%s", c.name)
	P("    clra ; keep errnum in b")
	P("    bra END__%s", c.name)

	P("OK__%s", c.name)
	if c.ra != "" {
		P("    sta [%d,s]  ; %s", base+c.ra_off, c.ra)
	}
	if c.rb != "" {
		P("    stb [%d,s]  ; %s", base+c.rb_off, c.rb)
	}
	if c.rd != "" {
		P("    std [%d,s]  ; %s", base+c.rd_off, c.rd)
	}
	if c.rx != "" {
		P("    stx [%d,s]  ; %s", base+c.rx_off, c.rx)
	}
	if c.ry != "" {
		P("    sty [%d,s]  ; %s", base+c.ry_off, c.ry)
	}
	if c.ru != "" {
		P("    stu [%d,s]  ; %s", base+c.ru_off, c.ru)
	}
	P("    clra")
	P("    clrb ; no error")

	P("END__%s", c.name)
	P("    puls Y,U,PC")
}

func PrintCalls() {
	var gen_hdr bytes.Buffer
	Fprintf(&gen_hdr, "#ifndef _GEN_HDR_FOR_CMOC_\n")
	Fprintf(&gen_hdr, "#define _GEN_HDR_FOR_CMOC_\n")
	Fprintf(&gen_hdr, "#include \"frob3/froblib.h\"\n")

	for _, c := range Calls {
		Fprintf(&gen_hdr, "\nextern errnum NewOs9%s(\n", c.name[2:])
		Fprintf(&gen_hdr, "%s);\n", c.FormatArgsForCmoc())
	}
	Fprintf(&gen_hdr, "#endif\n")

	const hdr_filename = "_generated_os9api_for_cmoc.h"
	err2 := ioutil.WriteFile(hdr_filename, gen_hdr.Bytes(), 0777)
	if err2 != nil {
		log.Fatalf("Cannot write %q: %v", hdr_filename, err2)
	}

	/////////////////////////////

	const asm_filename = "_generated_os9api_for_cmoc.asm"

	var gen_asm bytes.Buffer
	Fprintf(&gen_asm, "  SECTION code")
	for _, c := range Calls {
		PrintAsmForCmoc(c, &gen_asm)
	}
	Fprintf(&gen_asm, "  ENDSECTION")

	err := ioutil.WriteFile(asm_filename, gen_asm.Bytes(), 0777)
	if err != nil {
		log.Fatalf("Cannot write %q: %v", asm_filename, err)
	}
}
