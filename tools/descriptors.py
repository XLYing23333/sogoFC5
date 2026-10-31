"""Extract the two embedded proto2 descriptors without executing vendor code."""

import hashlib


def varint(data, pos):
    value = 0
    for shift in range(0, 70, 7):
        if pos >= len(data):
            raise ValueError("truncated varint")
        byte = data[pos]
        pos += 1
        if shift == 63 and byte > 1:
            raise ValueError("varint overflow")
        value |= (byte & 127) << shift
        if byte < 128:
            return value, pos
    raise ValueError("varint overflow")


def fields(data):
    pos = 0
    result = []
    while pos < len(data):
        tag, pos = varint(data, pos)
        number, wire = tag >> 3, tag & 7
        if number == 0 or number >= 1 << 29:
            raise ValueError("invalid field number")
        if wire == 0:
            value, pos = varint(data, pos)
        elif wire in (1, 5):
            size = 8 if wire == 1 else 4
            if pos + size > len(data):
                raise ValueError("truncated fixed field")
            value, pos = data[pos:pos + size], pos + size
        elif wire == 2:
            size, pos = varint(data, pos)
            if size > len(data) - pos:
                raise ValueError("truncated length field")
            value, pos = data[pos:pos + size], pos + size
        else:
            raise ValueError("unsupported wire type")
        result.append((number, wire, value))
    return result


def parse_descriptor(raw):
    top = fields(raw)
    names = [v.decode() for n, w, v in top if n == 1 and w == 2]
    if len(names) != 1 or any(n not in (1, 4) for n, w, v in top):
        raise ValueError("unsupported FileDescriptorProto schema")
    result = {"name": names[0], "messages": []}
    for n, w, v in top:
        if n != 4:
            continue
        message_fields = fields(v)
        if any(k not in (1, 2) for k, _, _ in message_fields):
            raise ValueError("unsupported DescriptorProto schema")
        message = {"name": next(x.decode() for k, _, x in message_fields if k == 1), "fields": []}
        for k, _, x in message_fields:
            if k != 2:
                continue
            definition = fields(x)
            if any(a not in (1, 3, 4, 5) for a, _, _ in definition):
                raise ValueError("unsupported FieldDescriptorProto schema")
            table = {a: b for a, _, b in definition}
            if set(table) != {1, 3, 4, 5} or len(definition) != 4:
                raise ValueError("incomplete field definition")
            message["fields"].append(dict(name=table[1].decode(), number=table[3],
                                          label=table[4], type=table[5]))
        result["messages"].append(message)
    return result


def extract(data, name):
    needle = bytes([10, len(name)]) + name.encode()
    begin = data.find(needle)
    if begin < 0 or data.find(needle, begin + 1) >= 0:
        raise ValueError(f"expected exactly one descriptor for {name}")
    end = data.find(b"\0", begin, begin + 32768)
    if end < 0:
        raise ValueError("descriptor terminator missing")
    raw = data[begin:end]
    parsed = parse_descriptor(raw)
    if parsed["name"] != name:
        raise ValueError("descriptor name mismatch")
    return parsed, dict(file_offset=begin, size=len(raw), sha256=hashlib.sha256(raw).hexdigest()), raw


def proto_source(parsed):
    types = {5: "int32", 8: "bool", 9: "string"}
    labels = {1: "optional", 2: "required", 3: "repeated"}
    lines = ['syntax = "proto2";', ""]
    for message in parsed["messages"]:
        lines.append("message " + message["name"] + " {")
        numbers = set()
        for field in message["fields"]:
            if field["number"] in numbers:
                raise ValueError("duplicate field number")
            numbers.add(field["number"])
            lines.append(f'  {labels[field["label"]]} {types[field["type"]]} '
                         f'{field["name"]} = {field["number"]};')
        lines.extend(["}", ""])
    return "\n".join(lines)
