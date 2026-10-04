DevilutionX PSP v1.6.1
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

Original Blizzard installer source: http://ftp.blizzard.com/pub/demos/diablosw.exe

For Hellfire, enable the single Hellfire entry under Mods in the game.
For a shareware-only install, leave out DIABDAT.MPQ and the Hellfire MPQs.
The known-good original spawn.mpq tested on a PSP-3000 is 50,274,091 bytes
and has SHA-256:
ea7de65bd1f12f1d04561c62a68f80eb36341e224974e1d93931fe82dc814e75
The 25,448,219-byte spawn.mpq from the DevilutionX assets download at
https://github.com/diasurgical/devilutionx-assets/releases/latest/download/spawn.mpq
has SHA-256 64427cd7c1ba904eaa2e0031c16a6b136d0ecef9abc888c5ff8344b459356e38.
It contains MP3 audio and does not currently work with this PSP build on a
PSP-3000. In a controlled PSP test, the same game data and MPQ layout worked
with WAV audio but powered off the PSP when the audio was changed to MP3.
This points to a PSP-port MP3 audio compatibility issue. There is no evidence
that the official DevilutionX download is damaged or unusable elsewhere.

No Diablo, Hellfire, or shareware MPQ data is included in this download.

Graphics offers 640x480 (4:3) and 848x480 (Widescreen) logical resolutions.
The widescreen mode shows a wider view without stretching a 4:3 frame.

This documentation update uses the same normal PSP build #56 EBOOT from
upstream-psp commit 0643e7092 as v1.6.0. No game code changed in v1.6.1.
The older v1.5.1 release was built from the separate psp-enhanced branch.
