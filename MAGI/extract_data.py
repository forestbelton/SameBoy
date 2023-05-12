import argparse
import dataclasses
import csv
import enum
import hashlib
import os.path
import sys
from typing import NoReturn, TypeVar

BANK_SIZE = 0x4000
BANK_ADDR_START = 0x4000
CHUNK_SIZE = 2048
MAGI_GBC_MD5 = "1624f857098ca278b15629914f48352b"
MAGI_GBC_SIZE = BANK_SIZE * 128


class ExtractType(enum.Enum):
    STRING = "STRING"
    TILEDATA = "TILEDATA"
    TILEMAP = "TILEMAP"
    BYTES = "BYTES"


@dataclasses.dataclass
class ExtractFieldInfo:
    bank: int
    addr: int
    length: int
    type: ExtractType
    name: str



@dataclasses.dataclass
class ExtractField:
    type: ExtractType
    value: bytes


@dataclasses.dataclass
class DataSpecification:
    fields: list[ExtractFieldInfo]


@dataclasses.dataclass
class ExtractRom:
    rom_data: bytes
    used: bytearray = dataclasses.field(default_factory=bytearray)
    fields: dict[str, ExtractField] = dataclasses.field(default_factory=dict)

    @property
    def total_extracted_bytes(self) -> int:
        return sum(1 for b in self.used if b != 0)


A = TypeVar("A")


def abort(msg: str) -> NoReturn:
    print(f"error: {msg}", file=sys.stderr)
    sys.exit(1)


def assert_equal(field: str, expected: A, actual: A) -> None:
    if actual == expected:
        return
    abort(f"unexpected value for {field} (wanted={expected}, got={actual})")


def load_rom_data(rom_file_path: str) -> bytes:
    rom = bytearray()
    digest = hashlib.md5()
    assert_equal("rom file exists", True, os.path.exists(rom_file_path))
    with open(rom_file_path, "rb") as f:
        chunk = f.read(CHUNK_SIZE)
        rom.extend(chunk)
        digest.update(chunk)
        while len(chunk) == CHUNK_SIZE:
            chunk = f.read(CHUNK_SIZE)
            rom.extend(chunk)
            digest.update(chunk)
    assert_equal("rom md5", MAGI_GBC_MD5, digest.hexdigest())
    assert_equal("rom size", MAGI_GBC_SIZE, len(rom))
    return bytes(rom)


def load_spec(spec_file_path: str) -> DataSpecification:
    assert_equal("spec file exists", True, os.path.exists(spec_file_path))
    fields = []
    with open(spec_file_path, "r") as f:
        r = csv.DictReader(f)
        for i, raw_field in enumerate(r):
            try:
                info = ExtractFieldInfo(
                    bank=int(raw_field["bank"], base=0),
                    addr=int(raw_field["addr"], base=0),
                    length=int(raw_field["length"], base=0),
                    type=ExtractType[raw_field["type"]],
                    name=raw_field["name"],
                )
                fields.append(info)
            except TypeError as exc:
                abort(f"could not parse spec field {i+1}: {exc}")
    return DataSpecification(fields=fields)


def extract_rom(rom_data: bytes, spec: DataSpecification) -> ExtractRom:
    rom = ExtractRom(rom_data=rom_data)
    for _ in range(len(rom_data)):
        rom.used.append(0)
    for field_idx, field in enumerate(spec.fields):
        value = bytearray()
        base_addr = field.bank * BANK_SIZE
        base_addr += field.addr - (0 if field.bank == 0 else BANK_ADDR_START)
        for i in range(field.length):
            addr = base_addr + i
            other_field_idx = rom.used[addr]
            if other_field_idx != 0:
                other_field = spec.fields[other_field_idx - 1]
                abort(f"address {hex(addr)} for field '{field.name}' already used by field '{other_field.name}'")
            value.append(rom_data[addr])
            rom.used[addr] = field_idx + 1
        rom.fields[field.name] = ExtractField(field.type, value)
        if field.type == ExtractType.TILEDATA:
            with open(f"{field.name}.tiles.2bpp", "wb") as f:
                f.write(value)
    return rom

def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("spec_file_path", metavar="<spec>")
    parser.add_argument("rom_file_path", metavar="<rom>")
    args = parser.parse_args()
    rom = extract_rom(
        load_rom_data(args.rom_file_path),
        load_spec(args.spec_file_path)
    )
    total_extract_bytes = rom.total_extracted_bytes
    total_extract_ratio = round((100.0 * total_extract_bytes) / len(rom.rom_data), 2)
    print(f"fields: {list(rom.fields.keys())}")
    print(f"total bytes: {total_extract_bytes} ({total_extract_ratio}%)")
    


if __name__ == "__main__":
    main()
