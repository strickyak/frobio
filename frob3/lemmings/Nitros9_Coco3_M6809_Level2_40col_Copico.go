package main

func init() {
	Os9(&Os9ConfigForLemma{
		Name:  "Nitros9_Coco3_M6809_Level2_40col_Copico",
		Level: "level2",
		Port:  "coco3",
		// DefaultDisk: "nitros9/level2/coco3/NOS9_6809_L2_cocosdc.dsk",
		DefaultDisk: "NOS9_6809_L2_coco3_80d.dsk",
		Boot1Mods: []string{
			"rel_80", "./boot.lemma_copico", "krn",
		},
		Boot2Mods: []string{
			"krnp2",
			"ioman",
			"init",
			"rbf.mn",
			"scf.mn",
			"vtio.dr",
			// "keydrv_cc3.sb",
			"snddrv_cc3.sb",
			"joydrv_joy.sb",
			"cogrf.io",
			"term_win40.dt",

			"w.dw",
			"w1.dw",
			"w2.dw",
			"w3.dw",
			// "w4.dw",
			// "w5.dw",
			// "w6.dw",
			// "w7.dw",

			"clock_60hz",
			"clock2_soft",
			"sysgo_dd",

			// "pipeman.mn",
			// "piper.dr",
			// "pipe.dd",

			// "rbsuper.dr",
			// "llcocosdc.dr",
			// "sd0_cocosdc.dd",
			// "sd1_cocosdc.dd",

			// "./fuseman",
			// "./fuser",
			// "./fuse",

			// "./rblemma",
			// "./dd.b0",
		},
	})
}
