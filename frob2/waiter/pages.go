package waiter

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

	Add(30, 0, "Nitros-9", &Card{})

	Add(31, 30, "L1 3.3.0 for M6809", &Card{
		Launch: ".L1-330-M.ax",
		Block0: ".L1-330-M.dsk",
	})

	Add(32, 30, "Nitros9 Level2", &Card{})
	Add(323, 32, "L2 3.3.0 for H6309", &Card{
		Launch: ".L2-330-H.ax",
		Block0: ".L2-330-H.dsk",
	})
	Add(328, 32, "L2 3.3.0 for M6809", &Card{
		Launch: ".L2-330-M.ax",
		Block0: ".L2-330-M.dsk",
	})

	Add(33, 30, "Nitros9 EOU", &Card{})
	Add(333, 33, "Nitros9 EOU 1.0.0 for H6309", &Card{
		Launch: ".EOU-100-H.ax",
		Block0: ".EOU-100-H.dsk",
	})
	Add(338, 33, "Nitros9 EOU 1.0.0 for M6809", &Card{
		Launch: ".EOU-100-M.ax",
		Block0: ".EOU-100-M.dsk",
	})
}
