package main

import (
	"bytes"
	. "fmt"
	"io"
	"io/ioutil"
	"log"
)

// assumes all args are on stack.
func (c *Call) FormatArgsForGcc() string {
	var bb bytes.Buffer
	if c.a != "" {
		Fprintf(&bb, "    /* A */ byte %s,\n", c.a)
	}
	if c.b != "" {
		Fprintf(&bb, "    /* B */ byte %s,\n", c.b)
	}
	if c.d != "" {
		Fprintf(&bb, "    /* D */ word %s,\n", c.d)
	}
	if c.x != "" {
		Fprintf(&bb, "    /* X */ word %s,\n", c.x)
	}
	if c.y != "" {
		Fprintf(&bb, "    /* Y */ word %s,\n", c.y)
	}
	if c.u != "" {
		Fprintf(&bb, "    /* U */ word %s,\n", c.u)
	}
	if c.ra != "" {
		Fprintf(&bb, "    /* RA */ byte* %s_out,\n", c.ra)
	}
	if c.rb != "" {
		Fprintf(&bb, "    /* RB */ byte* %s_out,\n", c.rb)
	}
	if c.rd != "" {
		Fprintf(&bb, "    /* RD */ word* %s_out,\n", c.rd)
	}
	if c.rx != "" {
		Fprintf(&bb, "    /* RX */ word* %s_out,\n", c.rx)
	}
	if c.ry != "" {
		Fprintf(&bb, "    /* RY */ word* %s_out,\n", c.ry)
	}
	if c.ru != "" {
		Fprintf(&bb, "    /* RU */ word* %s_out,\n", c.ru)
	}
	s := bb.String()
	n := len(s)
	if n > 0 {
		s = s[:n-2] + "\n" // remove final comma
	}
	return s
}

func PrintCForGcc(c *Call, w io.Writer) {
	P := func(format string, args ...any) {
		Fprintf(w, format+"\n", args...)
	}

    P("")
    P("/////////// %s // %s", c.name, c.desc)
    P("")
	P("errnum GccOs9%s(\n", c.name[2:])
	P("%s) {\n", c.FormatArgsForGcc())
    P("  errnum err = 0;\n");

	if c.a != "" {
		P("    volatile byte vol_a = (byte)%s;", c.a)
	}
	if c.b != "" {
		P("    volatile byte vol_b = (byte)%s;", c.b)
	}
	if c.d != "" {
		P("    volatile word vol_d = (word)%s;", c.d)
	}
	if c.x != "" {
		P("    volatile word vol_x = (word)%s;", c.x)
	}
	if c.y != "" {
		P("    volatile word vol_y = (word)%s;", c.y)
	}
	if c.u != "" {
		P("    volatile word vol_u = (word)%s;", c.u)
	}

	if c.ra != "" {
		P("    volatile byte vol_ra = 0;")
	}
	if c.rb != "" {
		P("    volatile byte vol_rb = 0;")
	}
	if c.rd != "" {
		P("    volatile word vol_rd = 0;")
	}
	if c.rx != "" {
		P("    volatile word vol_rx = 0;")
	}
	if c.ry != "" {
		P("    volatile word vol_ry = 0;")
	}
	if c.ru != "" {
		P("    volatile word vol_ru = 0;")
	}

    P(`    asm volatile ("\n"`);
	if c.a != "" {
		P(`    "  lda %%[vol_a]\n"`)
	}
	if c.b != "" {
		P(`    "  ldb %%[vol_b]\n"`)
	}
	if c.d != "" {
		P(`    "  ldd %%[vol_d]\n"`)
	}
	if c.x != "" {
		P(`    "  ldx %%[vol_x]\n"`)
	}
	if c.y != "" {
		P(`    "  ldy %%[vol_y]\n"`)
	}
	if c.u != "" {
		P(`    "  ldu %%[vol_u]\n"`)
	}

	P(`    "  os9 %d ; number $%02x. \n"`, c.number, c.number)
    P(`    "  bcc OK__%s \n"`, c.name)
    P(`    "  stb %%[err]\n"`)
	P(`    "  bra END__%s \n"`, c.name)
	P(`    "OK__%s:\n"`, c.name)

	if c.ra != "" {
		P(`    "  sta %%[vol_ra]\n"`)
	}
	if c.rb != "" {
		P(`    "  stb %%[vol_rb]\n"`)
	}
	if c.rd != "" {
		P(`    "  std %%[vol_rd]\n"`)
	}
	if c.rx != "" {
		P(`    "  stx %%[vol_rx]\n"`)
	}
	if c.ry != "" {
		P(`    "  sty %%[vol_ry]\n"`)
	}
	if c.ru != "" {
		P(`    "  stu %%[vol_ru]\n"`)
	}

    ////////// nando ////////////



	P(`    "END__%s:\n"`, c.name)

    P("  : // outputs")

	if c.ra != "" {
		P(`    [vol_ra] "=m" (vol_ra),`)
	}
	if c.rb != "" {
		P(`    [vol_rb] "=m" (vol_rb),`)
	}
	if c.rd != "" {
		P(`    [vol_rd] "=m" (vol_rd),`)
	}
	if c.rx != "" {
		P(`    [vol_rx] "=m" (vol_rx),`)
	}
	if c.ry != "" {
		P(`    [vol_ry] "=m" (vol_ry),`)
	}
	if c.ru != "" {
		P(`    [vol_ru] "=m" (vol_ru),`)
	}
	P(`    [err] "=m" (err)`)


    P("  : // inputs")

    comma := " "
	if c.a != "" {
		P(`    %s [vol_a] "m" (vol_a)`, comma)
        comma = ","
	}
	if c.b != "" {
		P(`    %s [vol_b] "m" (vol_b)`, comma)
        comma = ","
	}
	if c.d != "" {
		P(`    %s [vol_d] "m" (vol_d)`, comma)
        comma = ","
	}
	if c.x != "" {
		P(`    %s [vol_x] "m" (vol_x)`, comma)
        comma = ","
	}
	if c.y != "" {
		P(`    %s [vol_y] "m" (vol_y)`, comma)
        comma = ","
	}
	if c.u != "" {
		P(`    %s [vol_u] "m" (vol_u)`, comma)
        comma = ","
	}



    P(`  : "d", "x", "y", "u" // clobbers`)
    P(`  );`)
    P(`   return err;`)
    P(`};`)
}

func PrintCallsForGcc() {
	var gen_hdr bytes.Buffer
	Fprintf(&gen_hdr, "#ifndef _GEN_HDR_FOR_GCC_\n")
	Fprintf(&gen_hdr, "#define _GEN_HDR_FOR_GCC_\n")
	Fprintf(&gen_hdr, "#include \"types_gcc6809.h\"\n")

	for _, c := range Calls {
		Fprintf(&gen_hdr, "\nextern errnum GccOs9%s(\n", c.name[2:])
		Fprintf(&gen_hdr, "%s);\n", c.FormatArgsForGcc())
	}
	Fprintf(&gen_hdr, "#endif\n")

	const hdr_filename = "_generated_os9api_for_gcc.h"
	err2 := ioutil.WriteFile(hdr_filename, gen_hdr.Bytes(), 0777)
	if err2 != nil {
		log.Fatalf("Cannot write %q: %v", hdr_filename, err2)
	}

	/////////////////////////////

	const c_filename = "_generated_os9api_for_gcc.c"

	var gen_asm bytes.Buffer

    Fprintln(&gen_asm, `#include "types_gcc6809.h"`)
    Fprintln(&gen_asm, `#include "_generated_os9api_for_gcc.h"`)
    Fprintln(&gen_asm, ``)
	for _, c := range Calls {
		PrintCForGcc(c, &gen_asm)
	}

	err := ioutil.WriteFile(c_filename, gen_asm.Bytes(), 0777)
	if err != nil {
		log.Fatalf("Cannot write %q: %v", c_filename, err)
	}
}
