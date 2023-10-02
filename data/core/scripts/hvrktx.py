#!/usr/bin/env python3

# SPEC : https://registry.khronos.org/KTX/specs/2.0/ktxspec.v2.html

# TODO
#   CHECK IF KVD OFFSET == LENGTH == 0
#   SORT KEYS :https://registry.khronos.org/KTX/specs/2.0/ktxspec.v2.html#_keyvalue_data


def check_ktx_installed():
    import shutil
    return shutil.which("ktx") is not None

# tex_shape doesn't include datadim unlike numpy.shape
def store_numpy_array(arr, tex_shape, datadim, filename_out):
    if not check_ktx_installed():
        raise BaseException("ktx command is not installed, see https://github.com/KhronosGroup/KTX-Software/releases")

    import os
    import numpy as np
    import subprocess, tempfile

    command = ['ktx', 'create', '--raw', '--format']

    if datadim < 1 or datadim > 4:
        raise BaseException("store_numpy_array: datadim param is out of 1:4 range")
    if datadim == 1:
        command.append('R32_SFLOAT')
    elif datadim == 2:
        command.append('R32G32_SFLOAT')
    elif datadim == 3:
        command.append('R32G32B32_SFLOAT')
    elif datadim == 4:
        command.append('R32G32B32A32_SFLOAT')

    dims = len(tex_shape)
    depth = 1
    if dims < 1 or dims > 3:
        raise BaseException("store_numpy_array: tex_shape param is below 1D or above 3D")
    if dims >= 1:
        command.append("--width")
        command.append(str(tex_shape[0]))
    if dims >= 2:
        command.append("--height")
        command.append(str(tex_shape[1]))
    if dims >= 3:
        command.append("--depth")
        command.append(str(tex_shape[2]))
        depth = tex_shape[2]
    if dims == 1:
        command.append("--1d")


    tempdir = tempfile.TemporaryDirectory()
    for i in range(depth):
        temp = tempdir.name + "/" + str(i) + ".raw"
        tempf = open(temp, "wb")
        if dims == 3:
            arr[i,:,:].tofile(tempf)
        else:
            arr.tofile(tempf)
        command.append(temp)

    command.append(filename_out)
    print(command)
    subprocess.run(command)


def insert_metadata(filename_in, filename_out, metadata_dict):
    import io
    import struct
    with open(filename_in, 'rb') as f:
        in_memory_file = io.BytesIO(f.read())

    # read metadata block pos/size
    in_memory_file.seek(56); # byte pos for kvdByteOffset;kvdByteLength
    kvdByteOffset = struct.unpack('<I', in_memory_file.read(4))[0]
    kvdByteLength = struct.unpack('<I', in_memory_file.read(4))[0]

    # divide in 2
    in_memory_file.seek(0)
    first_block = io.BytesIO(in_memory_file.read(kvdByteOffset + kvdByteLength))
    second_block = io.BytesIO(in_memory_file.read())

    # create metadata block
    metadata_io = io.BytesIO()
    current_pos = 0
    for it in metadata_dict.items():
        key = it[0]
        value = it[1]
        value_bytes = ""
        if isinstance(value, str):
            value_bytes = bytes(value + '\x00', 'utf8')
        elif isinstance(value, int):
            key = 'i32_' + key
            value_bytes = struct.pack('<i', value)
        elif isinstance(value, float):
            key = 'f32_' + key
            value_bytes = struct.pack('<f', value)
        else:
            raise BaseException("Error: metadata value is neither str, int or float.")
        keyAndValueByteLength = len(key) + 1 + len(value_bytes)

        # write keyAndValue
        metadata_io.write(struct.pack('<I', keyAndValueByteLength))
        metadata_io.write(bytes(key, 'utf8'))
        metadata_io.write(bytes('\00', 'utf8'))
        metadata_io.write(value_bytes)
        for i in range((4 - keyAndValueByteLength % 4) % 4):
            metadata_io.write(bytes('\00', 'utf8'))

    # update file indices kvdByteLength, sgdByteOffset, and level indices
    metadata_io.seek(0)
    metadata_added_size = len(metadata_io.read())

    first_block.seek(40) # seek levelCount
    levelCount = struct.unpack('<I', first_block.read(4))[0]

    first_block.seek(60) # seek kvdByteLength
    first_block.write(struct.pack('<I', kvdByteLength + metadata_added_size))
    sgdByteOffset = struct.unpack('<Q', first_block.read(8))[0]
    if sgdByteOffset > 0:
        first_block.seek(64)
        first_block.write(struct.pack('<Q', sgdByteOffset + metadata_added_size))

    levelByteOffset = 80 # first level byteOffset
    for i in range(max(1, levelCount)): # for each level
        first_block.seek(levelByteOffset)
        byteOffset = struct.unpack('<Q', first_block.read(8))[0] # read byteOffset
        if byteOffset > 0:
            byteOffset += metadata_added_size # add added size
        first_block.seek(levelByteOffset)
        first_block.write(struct.pack('<Q', byteOffset)) # override
        levelByteOffset += 3*8 # skip over byteOffset, byteLength and uncompressedByteLength


    # Write file
    with open(filename_out, 'wb') as f:
        first_block.seek(0)
        metadata_io.seek(0)
        second_block.seek(0)
        f.write(first_block.read())
        f.write(metadata_io.read())
        f.write(second_block.read())


def store_numpy_array_metadata(arr, tex_shape, datadim, metadata_dict, filename_out):
    import tempfile
    glFormat = ["", "R", "RG", "RGB", "RGBA"]
    metadata_dict["glinternalformat"] = "GL_" + glFormat[datadim] + "32F"
    temp = tempfile.NamedTemporaryFile()
    store_numpy_array(arr, tex_shape, datadim, temp.name)
    insert_metadata(temp.name, filename_out, metadata_dict)

if __name__ == "__main__":
    import numpy as np
    import os, tempfile

    width=32
    height=32
    depth=32

    data=[]
    for d in range(depth):
        data.append([])
        for h in range(height):
            data[d].append([])
            for w in range(width):
                dist = ((d-16)**2 + (h-16)**2 + (w-16)**2)**0.5
                dist /= 16*2.0**0.5
                dist = 1.0 - dist
                data[d][h].append([dist**10, (1.0-dist)**10 / 100.0])

    store_numpy_array_metadata(np.array(data, dtype=np.float32), (width, height, depth), 2, {"minx": -1.0, "maxx": 1.0, "miny": -1.0, "maxy": 1.0, "minz": -1.0, "maxz": 1.0}, "volume.ktx")
