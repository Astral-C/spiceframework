import sys, os
from bStream import *

adir = sys.argv[1]
arc = sys.argv[2]

files = os.listdir(adir)

header = bStream()
filesData = bStream()

header.writeUInt32(0x414B5250) #write PRKA
header.writeUInt32(len(files))

data_offset = header.tell()

header.writeUInt32(0)

header.writeUInt32(0) #padding, can be a compression flag later

for file in files:

    data = bStream(path=os.path.join(adir, file)).readAll()

    namelen = len(file)
    if(namelen > 0xFF):
        header.writeUInt8(0xFF)
        header.writeString(file[0:0xFF])
    else:
        header.writeUInt8(namelen)
        header.writeString(file)

    header.writeUInt32(filesData.tell())
    header.writeUInt32(len(data))

    filesData.write(data)

header_size = header.tell()
header.seek(data_offset)
header.writeUInt32(header_size)

header.seek(0)
filesData.seek(0)

archive_file = open(arc, 'wb')
archive_file.write(header.readAll())
archive_file.write(filesData.readAll())

    