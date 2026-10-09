# DevilutionX PSP v1.6.2

This version updates the PSP EBOOT with the eight follow-up changes reviewed in [diasurgical/DevilutionX PR #8724](https://github.com/diasurgical/DevilutionX/pull/8724). It is built from this fork's `upstream-psp` branch.

The update removes temporary PSP debugging and the shareware-specific PCX conversion path, uses the regular UI background transition and sound effect loading, simplifies Hellfire mod handling, and tidies the PSP build and documentation. The PSP source and CMake files match the approved PR branch at commit `3cbb7a6ee`. This is a **new EBOOT**, unlike the documentation-only v1.6.1 release.

Extract `devilutionx-psp-v1.6.2.zip` into `PSP/GAME/DevilutionX/`. The ZIP contains `EBOOT.PBP`, `assets/`, `mods/hf/`, and `README-PSP.txt`. **No Diablo, Hellfire, or shareware MPQs are included.** Supply your own game data as described in the included README.

The original WAV-audio `spawn.mpq` extracted from Blizzard's `diablosw.exe` is the tested shareware archive for PSP-3000. The smaller MP3-audio `spawn.mpq` from the [DevilutionX assets download](https://github.com/diasurgical/devilutionx-assets/releases/latest/download/spawn.mpq) remains a known PSP compatibility issue and can power off the device. This does not mean the official archive is damaged or unsuitable for other platforms.

Graphics offers 640×480 (4:3) and 848×480 (Widescreen) logical resolutions on the PSP's 480×272 display. Widescreen shows more of the game world without stretching a 4:3 image. Earlier hardware testing observed about 30–33 FPS in standard-mode gameplay with per-pixel lighting off and about 20 FPS in a tested widescreen scene; those measurements are not guaranteed for every scene or this updated EBOOT.
