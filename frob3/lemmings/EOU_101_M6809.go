package main

func init() {
	Os9(&Os9ConfigForLemma{
		Name:        "EOU_101_M6809",
		Level:       "level2",
		Port:        "coco3",
		DefaultDisk: "../eou-101-m6809/68SDC.VHD",
		Boot1Base:   "eou-101-m6809/68EMU.dsk",
		Boot1Mods: []string{
			"_HEAD_",       // Special mark to keep the Base's head.
			"./boot.lemma", // Insert our own boot.lemma file.
			"_TAIL_",       // Special mark to keep the Base's tail.
		},
		Boot2Base: "eou-101-m6809/68EMU.dsk",
		Boot2Mods: []string{
			"<krnp2",
			"<krnp3",
			"<ioman",
			"<init",

			"<RBF",
			//"<EmuDsk",
			//"<Rammer",
			//"<R0",
			// "<MD",

			"<SCF",
			"<vtio",
			"<snddrv",
			"<joydrv",
			"<cowin",
			"<covdg",
			"<term",

			"<verm",
			"<vrn",
			"<nil",
			"<vi",
			"<ftdd",

			"<w",
			"<w1",
			"<w2",
			"<w3",
			"<w4",
			"<w5",
			"<w6",
			"<w7",
			"<w8",
			"<w9",
			"<w10",
			"<w11",
			"<w12",
			"<w13",
			"<w14",
			"<w15",

			//"<scbbp",
			//"<P",

			"<PipeMan",
			"<Piper",
			"<Pipe",

			"<clock",
			"<clock2",

			/*
			           "rbsuper.dr",
			           "llcocosdc.dr",
			           "sd0_cocosdc.dd",
			           "sd1_cocosdc.dd",
			   "./fuseman",
			   "./fuser",
			   "./fuse",
			*/

			"./rblemma",
			"./dd.b0",
		},
	})
}
