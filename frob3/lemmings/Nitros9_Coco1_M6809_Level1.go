package main

func init() {
	Os9(&Os9ConfigForLemma{
		Name:  "Nitros9_Coco1_M6809_Level1",
		Level: "level1",
		Port:  "coco1",
		Boot1Mods: []string{
			"rel", "krn", "krnp2", "init",
			"./boot.lemma",
		},
		Boot2Mods: []string{
			"ioman",
			"rbf.mn",
			"scf.mn",
			"vtio.dr",

			"covdg.io",
			"term_vdg.dt",

			"clock_60hz",
			"clock2_soft",
			"sysgo_dd",

			"./rblemma",
			"./dd.b0",
		},
	})
}
