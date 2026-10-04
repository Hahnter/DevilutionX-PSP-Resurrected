# DevilutionX PSP v1.6.1

This release gives the corrected PSP installation guidance its own version. It uses the **same EBOOT.PBP from PSP build #56** as v1.6.0; there are no gameplay or performance changes.

Shareware works on the tested PSP-3000 with `spawn.mpq` extracted from Blizzard's original `diablosw.exe` (50,274,091 bytes; SHA-256 `ea7de65bd1f12f1d04561c62a68f80eb36341e224974e1d93931fe82dc814e75`). That archive contains WAV audio. The smaller `spawn.mpq` from the [official DevilutionX assets download](https://github.com/diasurgical/devilutionx-assets/releases/latest/download/spawn.mpq) contains MP3 audio and currently powers off the PSP-3000 with this port. A controlled PSP test ran with a repacked WAV archive and crashed with the corresponding MP3 archive. This points to a **PSP-port MP3 audio compatibility issue**, not a damaged official download. MP3 shareware support is **not fixed** in v1.6.1.

Extract `devilutionx-psp-v1.6.1.zip` into `PSP/GAME/DevilutionX/`. The ZIP contains `EBOOT.PBP`, `assets/`, `mods/hf/`, and the updated `README-PSP.txt`. **No Diablo, Hellfire, or shareware MPQs are included.** Supply your own game data as described in the included README.

Graphics offers 640×480 (4:3) and 848×480 (Widescreen) logical resolutions on the PSP's 480×272 display. The wide option renders a wider game view without stretching a 4:3 frame.
