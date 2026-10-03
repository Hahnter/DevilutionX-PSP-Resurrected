DevilutionX PSP v1.6.0
=====================

Extract this ZIP into ms0:/PSP/GAME/DevilutionX/ on your PSP Memory Stick
(or the equivalent PPSSPP game folder). The layout must be:

  PSP/GAME/DevilutionX/EBOOT.PBP
  PSP/GAME/DevilutionX/assets/
  PSP/GAME/DevilutionX/mods/hf/

Keep the subdirectories inside assets/ and mods/hf/. No renaming is needed.
Use the assets/ directory supplied with this EBOOT. A separate
devilutionx.mpq is not needed.

Supply your own game data in the same folder as EBOOT.PBP:

  Diablo:    DIABDAT.MPQ
  Hellfire:  DIABDAT.MPQ, hellfire.mpq, hfmonk.mpq, hfmusic.mpq, hfvoice.mpq
  Shareware: spawn.mpq extracted from Blizzard's original diablosw.exe

For Hellfire, enable the single Hellfire entry under Mods in the game.
For a shareware-only install, leave out DIABDAT.MPQ and the Hellfire MPQs.
The known-good original spawn.mpq tested on a PSP-3000 is 50,274,091 bytes
and has SHA-256:
ea7de65bd1f12f1d04561c62a68f80eb36341e224974e1d93931fe82dc814e75
The 25,448,219-byte public spawn.mpq linked in generic DevilutionX install
instructions repeatedly failed in this PSP port's hardware tests.

No Diablo, Hellfire, or shareware MPQ data is included in this download.

Graphics offers 640x480 (4:3) and 848x480 (Widescreen) logical resolutions.
The widescreen mode shows a wider view without stretching a 4:3 frame.

This release uses the normal PSP build #56 from upstream-psp commit 0643e7092.
The older v1.5.1 release was built from the separate psp-enhanced branch.
