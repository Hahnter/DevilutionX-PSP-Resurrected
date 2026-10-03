# DevilutionX PSP v1.6.0

This is the current-master PSP port from `upstream-psp`, using PSP workflow build #56 (`0643e7092`). It supersedes v1.5.1 as the recommended PSP download. v1.5.1 remains available as an older build from the separate `psp-enhanced` branch.

The PSP-3000 tests covered retail Diablo, Hellfire, and shareware gameplay. Shareware worked with `spawn.mpq` freshly extracted from Blizzard's original `diablosw.exe` (50,274,091 bytes; SHA-256 `ea7de65bd1f12f1d04561c62a68f80eb36341e224974e1d93931fe82dc814e75`). The smaller public `spawn.mpq` used in earlier tests repeatedly failed on PSP hardware. The build includes the Hellfire mod files and the fix for duplicate `HF`/`hf` entries.

Extract `devilutionx-psp-v1.6.0.zip` into `PSP/GAME/DevilutionX/`. The ZIP contains `EBOOT.PBP`, `assets/`, `mods/hf/`, and `README-PSP.txt`. Read the included instructions before adding your own game data. **No Diablo, Hellfire, or shareware MPQs are included.**

Graphics offers 640×480 (4:3) and 848×480 (Widescreen) logical resolutions on the PSP's 480×272 display. The wide option renders a wider game view without stretching a 4:3 frame.
