package main

// TODO -- why is this broken?

func init() {
	Os9(&Os9ConfigForLemma{
		Name:        "EOU_M6809",
		Level:       "level2",
		Port:        "coco3",
		DefaultDisk: "../eou-m6809/68SDC.VHD",
		Boot1Base: "eou-m6809/68EMU.dsk",
		Boot1Mods: []string{
			"<rel",
			"./boot.lemma",
			"<krn",
		},
		Boot2Base: "eou-m6809/68EMU.dsk",
		Boot2Mods: []string{
			"<krnp2",
			"<krnp3",
			"<ioman",
			"<init",

			"<RBF",
			"<EmuDsk",
			"<Rammer",
			"<R0",
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

            "<Verm",
            //"<scbbp",
            //"<P",
            //"<VRN",
            //"<Nil",
            //"<VI",
            //"<FTDD",

            "<PipeMan",
            "<Piper",
            "<Pipe",

			"<clock",
			"<clock2",

			//"./fuseman",
			//"./fuser",
			//"./fuse",

			"./rblemma",
			"./dd.b0",
		},
	})
}
