"""Read-only ELF64/x86-64 inspection; never loads the inspected image."""

import struct
from pathlib import Path


class Elf:
    def __init__(self, path):
        self.data = Path(path).read_bytes()
        if self.data[:7] != b"\x7fELF\x02\x01\x01":
            raise ValueError("expected little-endian ELF64")
        if self.unpack("H", 18)[0] != 62:
            raise ValueError("expected x86-64")
        phoff, shoff = self.unpack("QQ", 32)
        phsize, phnum, shsize, shnum, shnames = self.unpack("HHHHH", 54)
        if phsize != 56 or shsize != 64 or not shnum:
            raise ValueError("unsupported ELF table layout")
        self.segments = [self.unpack("IIQQQQQQ", phoff + i * phsize)
                         for i in range(phnum)]
        self.sections = [self.unpack("IIQQQQIIQQ", shoff + i * shsize)
                         for i in range(shnum)]
        names = self.section_data(self.sections[shnames])
        self.named = {self.string(names, s[0]): s for s in self.sections}
        self.relocations = {}
        for s in self.sections:
            if s[1] == 4:
                if s[9] != 24 or s[5] % 24:
                    raise ValueError("invalid RELA entry size")
                for i in range(s[5] // 24):
                    address, info, addend = self.unpack("QQq", s[4] + i * 24)
                    self.relocations[address] = (info >> 32, info & 0xffffffff, addend)

    def unpack(self, fmt, offset):
        return struct.unpack_from("<" + fmt, self.data, offset)

    @staticmethod
    def string(data, offset):
        if not 0 <= offset < len(data):
            raise ValueError("string outside table")
        end = data.find(b"\0", offset)
        if end < 0:
            raise ValueError("unterminated ELF string")
        return data[offset:end].decode("utf-8")

    def section_data(self, section):
        begin, size = section[4:6]
        if begin + size > len(self.data):
            raise ValueError("section outside file")
        return self.data[begin:begin + size]

    def read(self, address, size):
        for typ, flags, off, va, pa, filesz, memsz, align in self.segments:
            if typ == 1 and va <= address and address + size <= va + filesz:
                begin = off + address - va
                return self.data[begin:begin + size]
        raise ValueError(f"unmapped file address {address:#x}")

    def pointer(self, address):
        relocation = self.relocations.get(address)
        if relocation:
            symbol, kind, addend = relocation
            if symbol or kind != 8:
                raise ValueError(f"not a relative pointer: {address:#x}")
            return addend
        return struct.unpack("<Q", self.read(address, 8))[0]

    def executable(self, address):
        return any(typ == 1 and flags & 1 and va <= address < va + filesz
                   for typ, flags, off, va, pa, filesz, memsz, align in self.segments)

    def symbols(self):
        s = self.named[".dynsym"]
        strings = self.section_data(self.sections[s[6]])
        if s[9] != 24 or s[5] % 24:
            raise ValueError("invalid dynamic symbol size")
        result = []
        for i in range(s[5] // 24):
            name, info, other, section, value, size = self.unpack("IBBHQQ", s[4] + i * 24)
            if name:
                result.append(dict(name=self.string(strings, name), address=value,
                                   size=size, defined=section != 0, type=info & 15))
        return result

    def dynamic(self):
        s = self.named[".dynamic"]
        strings = self.section_data(self.named[".dynstr"])
        values = {"needed": [], "rpath": [], "runpath": []}
        keys = {1: "needed", 15: "rpath", 29: "runpath"}
        for i in range(s[5] // 16):
            tag, value = self.unpack("qQ", s[4] + i * 16)
            if tag in keys:
                values[keys[tag]].append(self.string(strings, value))
        return values
