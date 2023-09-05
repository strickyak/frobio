package main

func init() {
	Os9(&Os9ConfigForLemma{
		Name:  "Nitros9_Coco3_H6309_Level2",
		Level: "level2",
		Port:  "coco3_6309",
		Boot1Mods: []string{
			"rel_80", "./boot.lemma", "krn",
		},
		Boot2Mods: []string{
			"krnp2",
			"ioman",
			"init",
			"rbf.mn",
			"scf.mn",
			"vtio.dr",
			"keydrv_cc3.sb",
			"snddrv_cc3.sb",
			"joydrv_joy.sb",
			"cowin.io",
			"term_win80.dt",

			"w.dw",
			"w1.dw",
			"w2.dw",
			"w3.dw",
			"w4.dw",
			"w5.dw",
			"w6.dw",
			"w7.dw",

			"clock_60hz",
			"clock2_soft",
			"sysgo_dd",

			"pipeman.mn",
			"piper.dr",
			"pipe.dd",

			"rbsuper.dr",
			"llcocosdc.dr",
			"sd0_cocosdc.dd",
			"sd1_cocosdc.dd",

			"./fuseman",
			"./fuser",
			"./fuse",

			"./rblemma",
			"./dd.b0",
		},
	})
}
