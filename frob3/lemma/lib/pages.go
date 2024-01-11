package lib

func init() {
	Add(0, 0, "Home", &Card{
		Text: `Welcome to Lemma.
This is the home card.
You can return here by typing 0.
`,
	})

	Add(10, 0, "CocoIO Stress Tests", &Card{
		Text: `These are some tests you can try
to stress your CocoIO Card.
`,
	})
	Add(11, 10, "Life Viewer", &Card{
		Launch: "@life",
	})
	Add(12, 10, "Verifying Life Viewer (slower)", &Card{
		Launch: "@test",
	})

	Add(20, 0, "Demos", &Card{})
	Add(21, 20, "Nyan Cat (coco3, coco2?)", &Card{
		Launch: ".demo-nyancat.coco3.loadm",
	})
	Add(22, 20, "DGNPEPPR (coco1, coco2?)", &Card{
		Launch: ".demo-dgnpeppr.coco1.loadm",
	})

	Add(28, 20, "screen splash burner demo", &Card{
		Launch: ".burn-splash.lem",
	})
	Add(2929, 20, "burn axiom4 rom code into cocoior eeprom", &Card{
		Launch: ".burn-rom.lem",
                Text: `
Danger!
Launch only if you are sure!
Does not ask for confirmation.
Do not turn your computer off until it says OK.
`
	})

	Add(30, 0, "Nitros-9", &Card{})

	Add(31, 30, "L1 3.3.0 for M6809", &Card{
		Launch: ".Nitros9_Coco1_M6809_Level1.lem",
		Block0: ".Nitros9_Coco1_M6809_Level1.dsk",
	})

	Add(32, 30, "Nitros9 Level2", &Card{})
	Add(323, 32, "L2 3.3.0 for HITACHI 6309", &Card{
		Launch: ".Nitros9_Coco3_H6309_Level2.lem",
		Block0: ".Nitros9_Coco3_H6309_Level2.dsk",
	})

	Add(325, 32, "L2 3.3.0 for M6809 (VDG SMALL)", &Card{
		Launch: ".Nitros9_Coco3_M6809_Level2vdg_small.lem",
		Block0: ".Nitros9_Coco3_M6809_Level2.dsk",
	})
	Add(326, 32, "L2 3.3.0 for M6809 (VDG)", &Card{
		Launch: ".Nitros9_Coco3_M6809_Level2vdg.lem",
		Block0: ".Nitros9_Coco3_M6809_Level2.dsk",
	})
	Add(326, 32, "L2 3.3.0 for M6809 (VDG) frob", &Card{
		Launch: ".Nitros9_Coco3_M6809_Level2vdg_frob.lem",
		Block0: ".NOS9_6809_L2_v030300_coco3_80d.dsk",
	})
	Add(327, 32, "L2 3.3.0 for M6809 (40 col)", &Card{
		Launch: ".Nitros9_Coco3_M6809_Level2_40col.lem",
		Block0: ".Nitros9_Coco3_M6809_Level2.dsk",
	})
	Add(328, 32, "L2 3.3.0 for M6809 (80 col)", &Card{
		Launch: ".Nitros9_Coco3_M6809_Level2.lem",
		Block0: ".Nitros9_Coco3_M6809_Level2.dsk",
	})
	Add(328001, 32, "(untested) L2 3.3.0 for M6809 with LEMMAN", &Card{
		Launch: ".Nitros9_Coco3_M6809_Level2_L.lem",
		Block0: ".Nitros9_Coco3_M6809_Level2.dsk",
	})

	Add(33, 30, "(Broken) Nitros9 EOU", &Card{})
	Add(333, 33, "(Broken) Nitros9 EOU 1.0.0 for H6309", &Card{
		Launch: ".EOU_H6309.lem",
		Block0: ".EOU_H6309.dsk",
	})
	Add(338, 33, "(Broken) Nitros9 EOU 1.0.0 for M6809", &Card{
		Launch: ".EOU_M6809.lem",
		Block0: ".EOU_M6809.dsk",
	})
}
