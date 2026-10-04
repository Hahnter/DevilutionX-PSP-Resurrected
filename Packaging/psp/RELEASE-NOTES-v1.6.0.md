# DevilutionX PSP v1.6.0

This is the current-master PSP port from `upstream-psp`, using PSP workflow build #56 (`0643e7092`). It supersedes v1.5.1 as the recommended PSP download. v1.5.1 remains available as an older build from the separate `psp-enhanced` branch.

The PSP-3000 tests covered retail Diablo, Hellfire, and shareware gameplay. Shareware worked with `spawn.mpq` freshly extracted from Blizzard's original `diablosw.exe` (50,274,091 bytes; SHA-256 `ea7de65bd1f12f1d04561c62a68f80eb36341e224974e1d93931fe82dc814e75`). This original archive contains WAV audio.

The smaller `spawn.mpq` from the [official DevilutionX assets download](https://github.com/diasurgical/devilutionx-assets/releases/latest/download/spawn.mpq) contains MP3 audio and does not currently work with this PSP build on a PSP-3000. A controlled test of the same game data and MPQ layout worked with WAV audio but powered off the PSP after the audio was changed to MP3. This points to a compatibility issue in the PSP port's MP3 audio path, **not evidence that the official download is damaged**.

The build includes the Hellfire mod files and the fix for duplicate `HF`/`hf` entries.

Extract `devilutionx-psp-v1.6.0.zip` into `PSP/GAME/DevilutionX/`. The ZIP contains `EBOOT.PBP`, `assets/`, `mods/hf/`, and `README-PSP.txt`. Read the included instructions before adding your own game data. **No Diablo, Hellfire, or shareware MPQs are included.**

Graphics offers 640×480 (4:3) and 848×480 (Widescreen) logical resolutions on the PSP's 480×272 display. The wide option renders a wider game view without stretching a 4:3 frame.
