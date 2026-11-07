#!/usr/bin/env python3
"""Register a header in an existing OpenRenderBoxCxx submodule."""

import argparse
import os
from pathlib import Path
import re
import stat
import tempfile


MODULE = re.compile(
    r"^module (?P<name>OpenRenderBoxCxx(?:\.[A-Za-z_][A-Za-z0-9_]*)+) \{\n"
    r"(?P<body>[^{}]*)^\}",
    re.MULTILINE,
)
HEADER = re.compile(r'^\s*(?:textual )?header "(?P<path>[^"\n]+)"', re.MULTILINE)


def write_module_map(path, contents):
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="w", encoding="utf-8", dir=path.parent, prefix=".modulemap-", delete=False
        ) as output:
            temporary = Path(output.name)
            output.write(contents)
        temporary.chmod(stat.S_IMODE(path.stat().st_mode))
        temporary.replace(path)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def add_header(root, header_path, module_name):
    source = root / "Sources/OpenRenderBoxCxx"
    include = source / "include"
    public = include / "OpenRenderBox"
    module_map = include / "module.modulemap"
    header = (root / header_path).resolve(strict=True)
    if not header.is_file() or not header.is_relative_to(source) or header.is_relative_to(include):
        raise ValueError("Use a source header under Sources/OpenRenderBoxCxx, outside include/.")
    relative = header.relative_to(source).as_posix()
    # These paths also become quoted shell arguments in the Tuist header copy script.
    if not re.fullmatch(r"[A-Za-z0-9_./ -]+", relative):
        raise ValueError("Header paths can contain only letters, digits, spaces, _, -, ., and /.")

    contents = module_map.read_text(encoding="utf-8")
    modules = {match["name"]: match for match in MODULE.finditer(contents)}
    name = module_name if module_name.startswith("OpenRenderBoxCxx.") else f"OpenRenderBoxCxx.{module_name}"
    if name not in modules:
        available = ", ".join(key.removeprefix("OpenRenderBoxCxx.") for key in modules)
        raise ValueError(f"Unknown module {module_name}. Use one of: {available}.")
    module = modules[name]
    cplusplus = re.search(r"^\s*requires cplusplus\s*$", module["body"], re.MULTILINE)
    extensions = (".h", ".hpp", ".tpp") if cplusplus else (".h",)
    if header.suffix not in extensions:
        raise ValueError(f"Module {name} accepts these header extensions: {', '.join(extensions)}.")

    link = None if cplusplus else public / header.name
    entry = f"../{relative}" if cplusplus else f"OpenRenderBox/{header.name}"
    create_link = link is not None and not os.path.lexists(link)
    if link is not None and not create_link:
        if not link.is_symlink() or link.resolve() != header:
            raise ValueError(f"Public header path already exists: {link.relative_to(root)}.")

    registered = False
    for owner in modules.values():
        for declaration in HEADER.finditer(owner["body"]):
            path = declaration["path"]
            if path != entry and (include / path).resolve() != header:
                continue
            if owner["name"] != name or path != entry:
                raise ValueError(f"Header already registered in {owner['name']} as {path}.")
            registered = True

    updated = contents
    if not registered:
        export = re.search(r"^([ \t]+)export \*[ \t]*$", module["body"], re.MULTILINE)
        if export is None:
            raise ValueError(f"Module {name} must contain an 'export *' line.")
        kind = "textual header" if header.suffix == ".tpp" else "header"
        position = module.start("body") + export.start()
        updated = contents[:position] + f'{export[1]}{kind} "{entry}"\n' + contents[position:]

    if create_link:
        public.mkdir(parents=True, exist_ok=True)
        link.symlink_to(os.path.relpath(header, public))
    try:
        if updated != contents:
            write_module_map(module_map, updated)
    except OSError:
        if create_link:
            link.unlink()
        raise

    if create_link or updated != contents:
        print(f"Registered {relative} in {name}.")
        print("Regenerate the Tuist project before the next framework build.")
    else:
        print(f"Already registered: {relative} in {name}.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("header", type=Path, help="Source header path, absolute or relative to the repository root.")
    parser.add_argument("module", help="Existing submodule, such as C, ObjC, or Animation.")
    arguments = parser.parse_args()
    try:
        add_header(Path(__file__).resolve().parents[1], arguments.header, arguments.module)
    except (OSError, ValueError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    main()
