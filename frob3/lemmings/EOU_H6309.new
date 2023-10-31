package main

// TODO -- fix this, after fixing EOU_M6809.
// TODO -- copy same modules as M6809
// TODO -- untested but known to be broken.

func init() {
	Os9(&Os9ConfigForLemma{
		Name:        "EOU_H6309",
		Level:       "level2",
		Port:        "coco3",
		DefaultDisk: "../eou-h6309/63SDC.VHD",
		Boot1Base: "eou-h6309/63EMU.dsk",
		Boot1Mods: []string{
			"<rel",
			"./boot.lemma",
			"<krn",
		},
		Boot2Base: "eou-h6309/63EMU.dsk",
		Boot2Mods: []string{
			"<krnp2",
			"<krnp3",
			"<ioman",
			"<init",

			"<RBF",
			"<SCF",
			"<vtio",
			//"<keydrv",
			"<snddrv",
			"<joydrv",
			"<cowin",
			"<covdg",
			"<term",

			"<verm",
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

			"<clock",
			"<clock2",
			// "<sysgo",

				"<pipeman",
				"<piper",
				"<pipe",

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
