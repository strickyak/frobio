package lemma

func init() {
	Add(0, 0, "LEMMA(41C)", &Card{
		Text: `Welcome to Lemma(41C), "{{ .Hostname }}".
This is LEMMA's starting point.
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
	Add(12, 10, "Verifying Life Viewer", &Card{
		Launch: "@life-check",
	})
	Add(13, 10, "Life Viewer with stats", &Card{
		Launch: "@life-stats",
	})
	Add(14, 10, "Verifying Life Viewer with stats", &Card{
		Launch: "@life-stats-check",
	})

	Add(20, 0, "Demos", &Card{})
	Add(21, 20, "Nyan Cat (coco3, coco2?)", &Card{
		Launch: ".demo-nyancat.coco3.loadm",
	})
	Add(22, 20, "DGNPEPPR (coco1, coco2?)", &Card{
		Launch: ".demo-dgnpeppr.coco1.loadm",
	})

	Add(25, 0, "Featured Games", &Card{})
	Add(251, 25, "spacewario for COCOIO", &Card{
		Text: `
Uses broadcast UDP/IP on a LAN.
You must be on the same LAN to play multi-user.
`,
		Launch: ".spacewario.bin",
	})
	/*
			Add(252, 25, "Rogue for Coco3/512K (Experimental)", &Card{
				Text: `
		(Experimental)
		`,
				Launch: ".Rogue512.lem",
				Block0: ".Rogue512.dsk",
			})
	*/

	Add(30, 0, "OS: Nitros-9", &Card{})

	Add(31, 30, "L1 3.3.0 for M6809", &Card{
		Launch: ".Nitros9_Coco1_M6809_Level1.lem",
		Block0: ".Nitros9_Coco1_M6809_Level1.dsk",
	})
	Add(317, 30, "L1 3.3.0 for M6809 for Copico", &Card{
		Launch: ".Nitros9_Coco1_M6809_Level1_Copico.lem",
		Block0: ".Nitros9_Coco1_M6809_Level1_Copico.dsk",
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
	Add(327, 32, "L2 3.3.0 for M6809 (40 col)", &Card{
		Launch: ".Nitros9_Coco3_M6809_Level2_40col.lem",
		Block0: ".Nitros9_Coco3_M6809_Level2.dsk",
	})
	Add(3271, 32, "L2 3.3.0 for M6809 (40 + fuse)", &Card{
		Launch: ".Nitros9_Coco3_M6809_Level2_40col_fuse.lem",
		Block0: ".Nitros9_Coco3_M6809_Level2_40col_fuse.dsk",
	})
	Add(3272, 32, "L2 3.3.0 for M6809 (40 + lem)", &Card{
		Launch: ".Nitros9_Coco3_M6809_Level2_40col_lem.lem",
		Block0: ".Nitros9_Coco3_M6809_Level2_40col_lem.dsk",
	})
	Add(3277, 32, "L2 3.3.0 for M6809 (40 col)", &Card{
		Launch: ".Nitros9_Coco3_M6809_Level2_40col_Copico.lem",
		Block0: ".Nitros9_Coco3_M6809_Level2.dsk",
	})
	Add(328, 32, "L2 3.3.0 for M6809 (80 col)", &Card{
		Launch: ".Nitros9_Coco3_M6809_Level2.lem",
		Block0: ".Nitros9_Coco3_M6809_Level2.dsk",
	})
	Add(328001, 328, "(untested) L2 3.3.0 for M6809 with LEMMAN", &Card{
		Launch: ".Nitros9_Coco3_M6809_Level2_L.lem",
		Block0: ".Nitros9_Coco3_M6809_Level2.dsk",
	})

	Add(33, 30, "Nitros9 EOU 1.0.0 (old)", &Card{})
	Add(333, 33, "Nitros9 EOU 1.0.0 for H6309", &Card{
		Launch: ".EOU_H6309.lem",
		Block0: ".EOU_H6309.dsk",
	})
	Add(338, 33, "Nitros9 EOU 1.0.0 for M6809", &Card{
		Launch: ".EOU_M6809.lem",
		Block0: ".EOU_M6809.dsk",
	})

	Add(34, 30, "Nitros9 EOU 1.0.1 (new)", &Card{})
	Add(343, 34, "Nitros9 EOU 1.0.1 for H6309", &Card{
		Launch: ".EOU_101_H6309.lem",
		Block0: ".EOU_101_H6309.dsk",
	})
	Add(348, 34, "Nitros9 EOU 1.0.1 for M6809", &Card{
		Launch: ".EOU_101_M6809.lem",
		Block0: ".EOU_101_M6809.dsk",
	})

	Add(35, 0, "OS: HDB-DOS (Disk Basic)", &Card{})
	Add(351, 35, "HDB-DOS with Remote Filesystems", &Card{
		Launch: ".hdbdos.lem",
		Text: `
For the disk chooser, hit
<CLEAR> to enter, and <CLEAR>
to leave. Arrow keys navigate.
Hit 0 to mount drive 0,
1 for 1, ... through 9.
`,
	})

	Add(39, 0, "NekotOS Gaming OS", &Card{
		Launch: ".nekotos-cocoior.lem",
        Text: `
Experimental.
See the #nekot-game-os
channel under Networking
on the Coco Discord.
`,
    })

	Add(40, 0, "Utilities", &Card{})

	Add(4241, 40, "burn prime-sieve (ROM DEMO) into eeprom", &Card{
		Launch: ".burn-primes-fast.lem",
		Text: `
Burns a silly demo into your eeprom.
Proves that burning actually works.

Danger!  re-burns your eeprom!
Launch only if you are sure!
`,
	})
	Add(4242, 40, "burn OLD axiom rom code into cocoior eeprom", &Card{
		Launch: ".burn-rom.lem",
		Text: `
You should be using page [4243] instead, unless you have a good reason to run old code.

Burns the OLD ROM code (Axiom41C) into your eeprom.

Danger!  re-burns your eeprom!
Launch only if you are sure!
`,
	})
	Add(4243, 40, "burn production axiom(41c) rom code into cocoior eeprom", &Card{
		Launch: ".burn-rom-fast.lem",
		Text: `
Burns the production ROM code (Axiom41C) into your eeprom.  (After this, you will want to set your hostname, page 43.)

Danger!  re-burns your eeprom!
Launch only if you are sure!
`,
	})

	Add(43, 40, "burn hostname into eeprom", &Card{
		Launch: ".burn-hostname.bin",
		Text: `
This is how you set the name
for you computer.  You burn the
hostname into the tail end of
the eeprom.  Lemma will use
this name.
`,
	})
	Add(49, 40, "show rom config & secrets", &Card{
		Launch: ".show-secrets.bin",
	})

	Add(70, 0, "Lemma Versions", &Card{
		Text: `
Here are alternate servers you may be able to connect to.
`,
	})
	Add(71, 70, "Stable Fallback", &Card{ReconnectPort: 2371})
	Add(72, 70, "Beta Server", &Card{ReconnectPort: 2372})
	Add(73, 70, "Alpha Server", &Card{ReconnectPort: 2373})
	Add(74, 70, "Experimental Server", &Card{ReconnectPort: 2374})
	Add(75, 70, "Experimental Server", &Card{ReconnectPort: 2375})
	Add(79, 70, "Default Server", &Card{ReconnectPort: 2321})

	Add(90, 0, "Empty test slots", &Card{
		Text: `
These probably do nothing, unless the 'LEMMINGS' directory has files test9[1-9].lem added to it for testing.
`,
	})
	Add(91, 90, "(test 91)", &Card{
		Launch: ".test91.lem",
	})
	Add(92, 90, "(test 92)", &Card{
		Launch: ".test92.lem",
	})
	Add(93, 90, "(test 93)", &Card{
		Launch: ".test93.lem",
	})
	Add(94, 90, "(test 94)", &Card{
		Launch: ".test94.lem",
	})
	Add(95, 90, "(test 95)", &Card{
		Launch: ".test95.lem",
	})
	Add(96, 90, "(test 96)", &Card{
		Launch: ".test96.lem",
	})
	Add(97, 90, "(test 97)", &Card{
		Launch: ".test97.lem",
	})
	Add(98, 90, "(test 98)", &Card{
		Launch: ".test98.lem",
	})
	Add(99, 90, "(test coypu-for-copico)", &Card{
		Launch: ".coypu-for-copico.bin",
	})

	Add(80, 0, "Video Player for coco3", &Card{})
	Add(801, 80, "flip---1986-Izwrl-Hszxp-Xloli-X", &Card{
		Launch: "%1.1986-Izwrl-Hszxp-Xloli-Xlnkfgvi-2--Z-uznrob-kovzhvi--GE-Xlnnvixrzo-Pj72anjhQ3N.nk4.ovnnz",
		Text:   "coco3 Video player:\n  flip---1986-Izwrl-Hszxp-Xloli-Xlnkfgvi-2--Z-uznrob-kovzhvi--GE-Xlnnvixrzo-Pj72anjhQ3N.nk4",
	})
	Add(802, 80, "flip---2001---Z-Hkzxv-Lwbhhvb--", &Card{
		Launch: "%1.2001---Z-Hkzxv-Lwbhhvb--1968-----Gsv-Yofv-Wzmfyv---dzoga--hxvmv--1080k--0AlHBhMZWgB.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---2001---Z-Hkzxv-Lwbhhvb--1968-----Gsv-Yofv-Wzmfyv---dzoga--hxvmv--1080k--0AlHBhMZWgB.npe",
	})
	Add(803, 80, "flip---20-Tznvh-Gszg-Wvurmvw-gs", &Card{
		Launch: "%1.20-Tznvh-Gszg-Wvurmvw-gsv-Gzmwb-GIH-80-Xloli-Xlnkfgvi--XlXl--S2JoCCF0dzd.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---20-Tznvh-Gszg-Wvurmvw-gsv-Gzmwb-GIH-80-Xloli-Xlnkfgvi--XlXl--S2JoCCF0dzd.npe",
	})
	Add(804, 80, "flip---z-sz---Gzpv-Lm-Nv---ZPZR", &Card{
		Launch: "%1.z-sz---Gzpv-Lm-Nv---ZPZR-NKP-NRMR-NP3-XLEVI-P3bqHHq1d7d.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---z-sz---Gzpv-Lm-Nv---ZPZR-NKP-NRMR-NP3-XLEVI-P3bqHHq1d7d.npe",
	})
	Add(805, 80, "flip---Yzxs---Orggov-Uftfv-rm-T", &Card{
		Launch: "%1.Yzxs---Orggov-Uftfv-rm-T-Nrmli-dBjpr8HyH-x.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---Yzxs---Orggov-Uftfv-rm-T-Nrmli-dBjpr8HyH-x.npe",
	})
	Add(806, 80, "flip---Yvhg-Nlerv-Hxvmvh----IRH", &Card{
		Launch: "%1.Yvhg-Nlerv-Hxvmvh----IRHPB-YFHRMVHH---Fmwvidvzi-Wzmxv-FfJAudDbGDB.dvyn.ovnnz",
		Text:   "coco3 Video player:\n  flip---Yvhg-Nlerv-Hxvmvh----IRHPB-YFHRMVHH---Fmwvidvzi-Wzmxv-FfJAudDbGDB.dvyn",
	})
	Add(807, 80, "flip---XlXlGZOP--vkrhlwv-218---", &Card{
		Launch: "%1.XlXlGZOP--vkrhlwv-218---Gvhg-Hgivzn-Vnz1d4RCX34.nk4.ovnnz",
		Text:   "coco3 Video player:\n  flip---XlXlGZOP--vkrhlwv-218---Gvhg-Hgivzn-Vnz1d4RCX34.nk4",
	})
	Add(808, 80, "flip---Xlnv-Hzro-Zdzb-drgs-Nv--", &Card{
		Launch: "%1.Xlnv-Hzro-Zdzb-drgs-Nv---HLFGS-KZIP-5Cn2zy3juAB.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---Xlnv-Hzro-Zdzb-drgs-Nv---HLFGS-KZIP-5Cn2zy3juAB.npe",
	})
	Add(809, 80, "flip---Xlnkovgv-Srhglib-Lu-Gsv-", &Card{
		Launch: "%1.Xlnkovgv-Srhglib-Lu-Gsv-Hlervg-Fmrlm--Ziizmtvw-Gl-Gsv-Nvolwb-Lu-Gvgirh-sDGUT3Q1XK8.nk4.ovnnz",
		Text:   "coco3 Video player:\n  flip---Xlnkovgv-Srhglib-Lu-Gsv-Hlervg-Fmrlm--Ziizmtvw-Gl-Gsv-Nvolwb-Lu-Gvgirh-sDGUT3Q1XK8.nk4",
	})
	Add(81, 80, "MORE", &Card{})
	Add(811, 81, "C-W--McCall---Convoy-Sd5", &Card{
		Launch: "%1.X-D--NxXzoo---Xlmelb-Hw5AOQDJnhh.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---X-D--NxXzoo---Xlmelb-Hw5AOQDJnhh.npe",
	})
	Add(812, 81, "flip---Wzug-Kfmp---Sziwvi--Yvgg", &Card{
		Launch: "%1.Wzug-Kfmp---Sziwvi--Yvggvi--Uzhgvi--Hgilmtvi--Luurxrzo-Erwvl--tZqI4-XyKkJ.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---Wzug-Kfmp---Sziwvi--Yvggvi--Uzhgvi--Hgilmtvi--Luurxrzo-Erwvl--tZqI4-XyKkJ.npe",
	})
	Add(813, 81, "flip---Wvel---Dsrk-Rg--Luurxrzo", &Card{
		Launch: "%1.Wvel---Dsrk-Rg--Luurxrzo-Nfhrx-Erwvl----Dzimvi-Ezfog-q-JOagsHpuN.dvyn.ovnnz",
		Text:   "coco3 Video player:\n  flip---Wvel---Dsrk-Rg--Luurxrzo-Nfhrx-Erwvl----Dzimvi-Ezfog-q-JOagsHpuN.dvyn",
	})
	Add(814, 81, "flip---Wi--Hgizmtvolev--1964--G", &Card{
		Launch: "%1.Wi--Hgizmtvolev--1964--Gizrovi--1---Nlervxorkh-Xozhhrx-Gizrovih-qKF1ZBGcdt4.nk4.ovnnz",
		Text:   "coco3 Video player:\n  flip---Wi--Hgizmtvolev--1964--Gizrovi--1---Nlervxorkh-Xozhhrx-Gizrovih-qKF1ZBGcdt4.nk4",
	})
	Add(815, 81, "flip---Vonvi-Ufww---Proo-gsv-Dz", &Card{
		Launch: "%1.Vonvi-Ufww---Proo-gsv-Dzyyrg--Luurxrzo-Erwvl--ug--Yfth-Yfmmb-PAGV9NWlzOh.nk4.ovnnz",
		Text:   "coco3 Video player:\n  flip---Vonvi-Ufww---Proo-gsv-Dzyyrg--Luurxrzo-Erwvl--ug--Yfth-Yfmmb-PAGV9NWlzOh.nk4",
	})
	Add(816, 81, "flip---Virx-Xzignzm-hrmth-Xlnv-", &Card{
		Launch: "%1.Virx-Xzignzm-hrmth-Xlnv-Hzro-Zdzb-tDNbl62NDlZ.nk4.ovnnz",
		Text:   "coco3 Video player:\n  flip---Virx-Xzignzm-hrmth-Xlnv-Hzro-Zdzb-tDNbl62NDlZ.nk4",
	})
	Add(817, 81, "flip---Tllw-Nlimrmt-Hfmhsrmv---", &Card{
		Launch: "%1.Tllw-Nlimrmt-Hfmhsrmv---Lirtrmzo-Hlmt-D7j1ySP8gv0.nk4.ovnnz",
		Text:   "coco3 Video player:\n  flip---Tllw-Nlimrmt-Hfmhsrmv---Lirtrmzo-Hlmt-D7j1ySP8gv0.nk4",
	})
	Add(818, 81, "flip---SZMWVO---AZWLP-GSV-KIRVH", &Card{
		Launch: "%1.SZMWVO---AZWLP-GSV-KIRVHG--XLILMZGRLM-ZMGSVN----KRKV-LITZM-HLOL---QLMZGSZM-HXLGG-XKgUQ1-jAtJ.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---SZMWVO---AZWLP-GSV-KIRVHG--XLILMZGRLM-ZMGSVN----KRKV-LITZM-HLOL---QLMZGSZM-HXLGG-XKgUQ1-jAtJ.npe",
	})
	Add(819, 81, "flip---Sziib-Klggvi---Svwdrt-h-", &Card{
		Launch: "%1.Sziib-Klggvi---Svwdrt-h-Gsvnv--Krzml-Evihrlm--qGKCdyWgRkZ.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---Sziib-Klggvi---Svwdrt-h-Gsvnv--Krzml-Evihrlm--qGKCdyWgRkZ.npe",
	})
	Add(82, 81, "MORE", &Card{})
	Add(821, 82, "flip---SVBBVBZZVBZZZVBZVBZZ-AA5", &Card{
		Launch: "%1.SVBBVBZZVBZZZVBZVBZZ-AA5OkdL-Zm4.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---SVBBVBZZVBZZZVBZVBZZ-AA5OkdL-Zm4.npe",
	})
	Add(822, 82, "flip---Q-H-Yzxs--Nfhvggv-rm-W-n", &Card{
		Launch: "%1.Q-H-Yzxs--Nfhvggv-rm-W-nzqli-wvgThIJ7JD8.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---Q-H-Yzxs--Nfhvggv-rm-W-nzqli-wvgThIJ7JD8.npe",
	})
	Add(823, 82, "flip---Pizugdvip---Gsv-Ilylgh--", &Card{
		Launch: "%1.Pizugdvip---Gsv-Ilylgh--Luurxrzo-Erwvl--W-8Knz1eSnd.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---Pizugdvip---Gsv-Ilylgh--Luurxrzo-Erwvl--W-8Knz1eSnd.npe",
	})
	Add(824, 82, "flip---Ozab-Hfmwzb---HMO-Wrtrgz", &Card{
		Launch: "%1.Ozab-Hfmwzb---HMO-Wrtrgzo-Hslig-hIsGvzz-Y98.nk4.ovnnz",
		Text:   "coco3 Video player:\n  flip---Ozab-Hfmwzb---HMO-Wrtrgzo-Hslig-hIsGvzz-Y98.nk4",
	})
	Add(825, 82, "flip---Nvgilklorh--1927--Gizrov", &Card{
		Launch: "%1.Nvgilklorh--1927--Gizrovi--1---Nlervxorkh-Xozhhrx-Gizrovih-twgAe3CILmx.nk4.ovnnz",
		Text:   "coco3 Video player:\n  flip---Nvgilklorh--1927--Gizrovi--1---Nlervxorkh-Xozhhrx-Gizrovih-twgAe3CILmx.nk4",
	})
	Add(826, 82, "flip---Ifhs-V---Rnklhhryov-Krzm", &Card{
		Launch: "%1.Ifhs-V---Rnklhhryov-Krzml-Ivnrc---Yozxp-NRWR-fIAZJIsKSfV.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---Ifhs-V---Rnklhhryov-Krzml-Ivnrc---Yozxp-NRWR-fIAZJIsKSfV.npe",
	})
	Add(827, 82, "flip---Hvhznv-Hgivvg---Mfnyvi-9", &Card{
		Launch: "%1.Hvhznv-Hgivvg---Mfnyvi-9-Nzigrzm-Yvzfgb-ZA5B3GrahdB.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---Hvhznv-Hgivvg---Mfnyvi-9-Nzigrzm-Yvzfgb-ZA5B3GrahdB.npe",
	})
	Add(828, 82, "flip---Gvovgfyyrvh-hzb--Vs-ls--", &Card{
		Launch: "%1.Gvovgfyyrvh-hzb--Vs-ls--------Nfhrx-Erwvl------WRWZFErImZF.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---Gvovgfyyrvh-hzb--Vs-ls--------Nfhrx-Erwvl------WRWZFErImZF.npe",
	})
	Add(829, 82, "flip---Gsv-Yvzgovh---Bvoold-Hfy", &Card{
		Launch: "%1.Gsv-Yvzgovh---Bvoold-Hfynzirmv-n2fGUU-3NzZ.nk4.ovnnz",
		Text:   "coco3 Video player:\n  flip---Gsv-Yvzgovh---Bvoold-Hfynzirmv-n2fGUU-3NzZ.nk4",
	})
	Add(83, 82, "MORE", &Card{})
	Add(831, 83, "flip---Gilm-Ovtzxb-Ortsgyrpv-Hx", &Card{
		Launch: "%1.Gilm-Ovtzxb-Ortsgyrpv-Hxvmv--4P--HjHfIwptocN.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---Gilm-Ovtzxb-Ortsgyrpv-Hxvmv--4P--HjHfIwptocN.npe",
	})
	Add(832, 83, "flip---Gilm-Ortsgyrpv-Hxvmv--3L", &Card{
		Launch: "%1.Gilm-Ortsgyrpv-Hxvmv--3LWv9njlWV.nk4.ovnnz",
		Text:   "coco3 Video player:\n  flip---Gilm-Ortsgyrpv-Hxvmv--3LWv9njlWV.nk4",
	})
	Add(833, 83, "flip---Dzoo-Lu-Ellwll---Nvcrxzm", &Card{
		Launch: "%1.Dzoo-Lu-Ellwll---Nvcrxzm-Izwrl--Luurxrzo-Erwvl--vbXVvcT9cqd.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---Dzoo-Lu-Ellwll---Nvcrxzm-Izwrl--Luurxrzo-Erwvl--vbXVvcT9cqd.npe",
	})
	Add(834, 83, "flip---Droob-Dlmpz-1971-Llnkz-O", &Card{
		Launch: "%1.Droob-Dlmpz-1971-Llnkz-Ollnkz-Hlmt-JpX8dKHnxKt.npe.ovnnz",
		Text:   "coco3 Video player:\n  flip---Droob-Dlmpz-1971-Llnkz-Ollnkz-Hlmt-JpX8dKHnxKt.npe",
	})
}
