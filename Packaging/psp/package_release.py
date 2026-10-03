"""Package a downloaded PSP Actions artifact as an installable release ZIP."""

import argparse
import hashlib
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile


EXPECTED_EBOOT_SHA256 = "50b3b12ddd2e69da4ecbba7bf116074116cdd388becea0e87268bacb79ed3130"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("artifact_dir", type=Path)
    parser.add_argument("output_zip", type=Path)
    arguments = parser.parse_args()

    root = arguments.artifact_dir
    files = sorted(path for path in root.rglob("*") if path.is_file())
    names = [path.relative_to(root).as_posix() for path in files]
    if names.count("EBOOT.PBP") != 1:
        raise SystemExit("Expected one EBOOT.PBP at the artifact root")
    if not any(name.startswith("assets/") for name in names):
        raise SystemExit("PSP assets are missing")
    if "mods/hf/manifest.ini" not in names:
        raise SystemExit("Hellfire mod files are missing")
    if any(name.lower().endswith(".mpq") for name in names):
        raise SystemExit("Refusing to package game MPQs")

    eboot = (root / "EBOOT.PBP").read_bytes()
    if eboot[:4] != bytes((0, 80, 66, 80)):
        raise SystemExit("Invalid EBOOT.PBP header")
    digest = hashlib.sha256(eboot).hexdigest()
    if digest != EXPECTED_EBOOT_SHA256:
        raise SystemExit(f"EBOOT hash differs from uploaded build #56: {digest}")

    instructions = Path(__file__).with_name("README-PSP.txt")
    with ZipFile(arguments.output_zip, "w", compression=ZIP_DEFLATED, compresslevel=6) as archive:
        for name, path in zip(names, files):
            archive.write(path, name)
        archive.write(instructions, "README-PSP.txt")

    with ZipFile(arguments.output_zip) as archive:
        if archive.testzip() is not None:
            raise SystemExit("Release ZIP failed CRC check")
    print(f"Packaged {len(names) + 1} files; EBOOT SHA-256 {digest}")


if __name__ == "__main__":
    main()
