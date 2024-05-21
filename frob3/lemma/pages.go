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
	Add(801, 80, "1986-Radio-Shack-Color-C", &Card{
		Launch: ".1986-Radio-Shack-Color-Computer-2--A-family-pleaser--TV-Commercial-Kq72zmqsJ3M.mp4.lemma",
		Text:   "coco3 Video player:\n  1986-Radio-Shack-Color-Computer-2--A-family-pleaser--TV-Commercial-Kq72zmqsJ3M.mp4",
	})
	Add(802, 80, "2001---A-Space-Odyssey--", &Card{
		Launch: ".2001---A-Space-Odyssey--1968-----The-Blue-Danube---waltz--scene--1080p--0ZoSYsNADtY.mkv.lemma",
		Text:   "coco3 Video player:\n  2001---A-Space-Odyssey--1968-----The-Blue-Danube---waltz--scene--1080p--0ZoSYsNADtY.mkv",
	})
	Add(803, 80, "20-Games-That-Defined-th", &Card{
		Launch: ".20-Games-That-Defined-the-Tandy-TRS-80-Color-Computer--CoCo--H2QlXXU0waw.mkv.lemma",
		Text:   "coco3 Video player:\n  20-Games-That-Defined-the-Tandy-TRS-80-Color-Computer--CoCo--H2QlXXU0waw.mkv",
	})
	Add(804, 80, "a-ha---Take-On-Me---AKAI", &Card{
		Launch: ".a-ha---Take-On-Me---AKAI-MPK-MINI-MK3-COVER-K3yjSSj1w7w.mkv.lemma",
		Text:   "coco3 Video player:\n  a-ha---Take-On-Me---AKAI-MPK-MINI-MK3-COVER-K3yjSSj1w7w.mkv",
	})
	Add(805, 80, "Bach---Little-Fugue-in-G", &Card{
		Launch: ".Bach---Little-Fugue-in-G-Minor-wYqki8SbS-c.mkv.lemma",
		Text:   "coco3 Video player:\n  Bach---Little-Fugue-in-G-Minor-wYqki8SbS-c.mkv",
	})
	Add(806, 80, "Best-Movie-Scenes----RIS", &Card{
		Launch: ".Best-Movie-Scenes----RISKY-BUSINESS---Underwear-Dance-UuQZfwWyTWY.webm.lemma",
		Text:   "coco3 Video player:\n  Best-Movie-Scenes----RISKY-BUSINESS---Underwear-Dance-UuQZfwWyTWY.webm",
	})
	Add(807, 80, "CoCoTALK--episode-218---", &Card{
		Launch: ".CoCoTALK--episode-218---Test-Stream-Ema1w4IXC34.mp4.lemma",
		Text:   "coco3 Video player:\n  CoCoTALK--episode-218---Test-Stream-Ema1w4IXC34.mp4",
	})
	Add(808, 80, "Come-Sail-Away-with-Me--", &Card{
		Launch: ".Come-Sail-Away-with-Me---SOUTH-PARK-5Xm2ab3qfZY.mkv.lemma",
		Text:   "coco3 Video player:\n  Come-Sail-Away-with-Me---SOUTH-PARK-5Xm2ab3qfZY.mkv",
	})
	Add(809, 80, "Complete-History-Of-The-", &Card{
		Launch: ".Complete-History-Of-The-Soviet-Union--Arranged-To-The-Melody-Of-Tetris-hWTFG3J1CP8.mp4.lemma",
		Text:   "coco3 Video player:\n  Complete-History-Of-The-Soviet-Union--Arranged-To-The-Melody-Of-Tetris-hWTFG3J1CP8.mp4",
	})
	Add(81, 80, "MORE", &Card{})
	Add(811, 81, "C-W--McCall---Convoy-Sd5", &Card{
		Launch: ".C-W--McCall---Convoy-Sd5ZLJWQmss.mkv.lemma",
		Text:   "coco3 Video player:\n  C-W--McCall---Convoy-Sd5ZLJWQmss.mkv",
	})
	Add(812, 81, "Daft-Punk---Harder--Bett", &Card{
		Launch: ".Daft-Punk---Harder--Better--Faster--Stronger--Official-Video--gAjR4-CbPpQ.mkv.lemma",
		Text:   "coco3 Video player:\n  Daft-Punk---Harder--Better--Faster--Stronger--Official-Video--gAjR4-CbPpQ.mkv",
	})
	Add(813, 81, "Devo---Whip-It--Official", &Card{
		Launch: ".Devo---Whip-It--Official-Music-Video----Warner-Vault-j-QLzthSkfM.webm.lemma",
		Text:   "coco3 Video player:\n  Devo---Whip-It--Official-Music-Video----Warner-Vault-j-QLzthSkfM.webm",
	})
	Add(814, 81, "Dr--Strangelove--1964--T", &Card{
		Launch: ".Dr--Strangelove--1964--Trailer--1---Movieclips-Classic-Trailers-jPU1AYTxwg4.mp4.lemma",
		Text:   "coco3 Video player:\n  Dr--Strangelove--1964--Trailer--1---Movieclips-Classic-Trailers-jPU1AYTxwg4.mp4",
	})
	Add(815, 81, "Elmer-Fudd---Kill-the-Wa", &Card{
		Launch: ".Elmer-Fudd---Kill-the-Wabbit--Official-Video--ft--Bugs-Bunny-KZTE9MDoaLs.mp4.lemma",
		Text:   "coco3 Video player:\n  Elmer-Fudd---Kill-the-Wabbit--Official-Video--ft--Bugs-Bunny-KZTE9MDoaLs.mp4",
	})
	Add(816, 81, "Eric-Cartman-sings-Come-", &Card{
		Launch: ".Eric-Cartman-sings-Come-Sail-Away-gWMyo62MWoA.mp4.lemma",
		Text:   "coco3 Video player:\n  Eric-Cartman-sings-Come-Sail-Away-gWMyo62MWoA.mp4",
	})
	Add(817, 81, "Good-Morning-Sunshine---", &Card{
		Launch: ".Good-Morning-Sunshine---Original-Song-W7q1bHK8te0.mp4.lemma",
		Text:   "coco3 Video player:\n  Good-Morning-Sunshine---Original-Song-W7q1bHK8te0.mp4",
	})
	Add(818, 81, "HANDEL---ZADOK-THE-PRIES", &Card{
		Launch: ".HANDEL---ZADOK-THE-PRIEST--CORONATION-ANTHEM----PIPE-ORGAN-SOLO---JONATHAN-SCOTT-CPtFJ1-qZgQ.mkv.lemma",
		Text:   "coco3 Video player:\n  HANDEL---ZADOK-THE-PRIEST--CORONATION-ANTHEM----PIPE-ORGAN-SOLO---JONATHAN-SCOTT-CPtFJ1-qZgQ.mkv",
	})
	Add(819, 81, "Harry-Potter---Hedwig-s-", &Card{
		Launch: ".Harry-Potter---Hedwig-s-Theme--Piano-Version--jTPXwbDtIpA.mkv.lemma",
		Text:   "coco3 Video player:\n  Harry-Potter---Hedwig-s-Theme--Piano-Version--jTPXwbDtIpA.mkv",
	})
	Add(82, 81, "MORE", &Card{})
	Add(821, 82, "HEYYEYAAEYAAAEYAEYAA-ZZ5", &Card{
		Launch: ".HEYYEYAAEYAAAEYAEYAA-ZZ5LpwO-An4.mkv.lemma",
		Text:   "coco3 Video player:\n  HEYYEYAAEYAAAEYAEYAA-ZZ5LpwO-An4.mkv",
	})
	Add(822, 82, "J-S-Bach--Musette-in-D-m", &Card{
		Launch: ".J-S-Bach--Musette-in-D-major-detGsRQ7QW8.mkv.lemma",
		Text:   "coco3 Video player:\n  J-S-Bach--Musette-in-D-major-detGsRQ7QW8.mkv",
	})
	Add(823, 82, "Kraftwerk---The-Robots--", &Card{
		Launch: ".Kraftwerk---The-Robots--Official-Video--D-8Pma1vHmw.mkv.lemma",
		Text:   "coco3 Video player:\n  Kraftwerk---The-Robots--Official-Video--D-8Pma1vHmw.mkv",
	})
	Add(824, 82, "Lazy-Sunday---SNL-Digita", &Card{
		Launch: ".Lazy-Sunday---SNL-Digital-Short-sRhTeaa-B98.mp4.lemma",
		Text:   "coco3 Video player:\n  Lazy-Sunday---SNL-Digital-Short-sRhTeaa-B98.mp4",
	})
	Add(825, 82, "Metropolis--1927--Traile", &Card{
		Launch: ".Metropolis--1927--Trailer--1---Movieclips-Classic-Trailers-gdtZv3XROnc.mp4.lemma",
		Text:   "coco3 Video player:\n  Metropolis--1927--Trailer--1---Movieclips-Classic-Trailers-gdtZv3XROnc.mp4",
	})
	Add(826, 82, "Rush-E---Impossible-Pian", &Card{
		Launch: ".Rush-E---Impossible-Piano-Remix---Black-MIDI-uRZAQRhPHuE.mkv.lemma",
		Text:   "coco3 Video player:\n  Rush-E---Impossible-Piano-Remix---Black-MIDI-uRZAQRhPHuE.mkv",
	})
	Add(827, 82, "Sesame-Street---Number-9", &Card{
		Launch: ".Sesame-Street---Number-9-Martian-Beauty-AZ5Y3TizswY.mkv.lemma",
		Text:   "coco3 Video player:\n  Sesame-Street---Number-9-Martian-Beauty-AZ5Y3TizswY.mkv",
	})
	Add(828, 82, "Teletubbies-say--Eh-oh--", &Card{
		Launch: ".Teletubbies-say--Eh-oh--------Music-Video------DIDAUViRnAU.mkv.lemma",
		Text:   "coco3 Video player:\n  Teletubbies-say--Eh-oh--------Music-Video------DIDAUViRnAU.mkv",
	})
	Add(829, 82, "The-Beatles---Yellow-Sub", &Card{
		Launch: ".The-Beatles---Yellow-Submarine-m2uTFF-3MaA.mp4.lemma",
		Text:   "coco3 Video player:\n  The-Beatles---Yellow-Submarine-m2uTFF-3MaA.mp4",
	})
	Add(83, 82, "MORE", &Card{})
	Add(831, 83, "Tron-Legacy-Lightbike-Sc", &Card{
		Launch: ".Tron-Legacy-Lightbike-Scene--4K--SqSuRdkglxM.mkv.lemma",
		Text:   "coco3 Video player:\n  Tron-Legacy-Lightbike-Scene--4K--SqSuRdkglxM.mkv",
	})
	Add(832, 83, "Tron-Lightbike-Scene--3O", &Card{
		Launch: ".Tron-Lightbike-Scene--3ODe9mqoDE.mp4.lemma",
		Text:   "coco3 Video player:\n  Tron-Lightbike-Scene--3ODe9mqoDE.mp4",
	})
	Add(833, 83, "Wall-Of-Voodoo---Mexican", &Card{
		Launch: ".Wall-Of-Voodoo---Mexican-Radio--Official-Video--eyCEexG9xjw.mkv.lemma",
		Text:   "coco3 Video player:\n  Wall-Of-Voodoo---Mexican-Radio--Official-Video--eyCEexG9xjw.mkv",
	})
	Add(834, 83, "Willy-Wonka-1971-Oompa-L", &Card{
		Launch: ".Willy-Wonka-1971-Oompa-Loompa-Song-QkC8wPSmcPg.mkv.lemma",
		Text:   "coco3 Video player:\n  Willy-Wonka-1971-Oompa-Loompa-Song-QkC8wPSmcPg.mkv",
	})
	Add(888, 83, "BIG VIDEO", &Card{
		Launch: ".big.lemma",
		Text:   "coco3 Video player:\n  Big Video .mkv",
	})
}
