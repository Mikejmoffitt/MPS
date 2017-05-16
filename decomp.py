#!/bin/python
import zlib, sys
str_obj1 = open(sys.argv[1], 'rb').read()
str_obj2 = zlib.decompress(str_obj1)
f = open(sys.argv[2], 'wb')
f.write(str_obj2)
f.close()

