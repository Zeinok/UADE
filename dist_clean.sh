#!/bin/sh
# Removes all generated files from src dir
# Generated using: find . -depth -type f -mmin -XXX
#
# by Airmann 8/2013
#
make clean
rm ./uade.pc
rm ./src/frontends/xmms/Makefile
rm ./src/frontends/uade123/Makefile
rm ./src/frontends/audacious/Makefile
rm ./src/frontends/uadesimple/Makefile
rm ./src/frontends/uadefs/Makefile
rm ./src/frontends/mod2ogg/mod2ogg2.sh
rm ./src/sd-sound.h
rm ./src/ossupport.c
rm ./src/sd-sound.c
rm ./src/include/ossupport.h
rm ./src/include/compilersupport.h
rm ./src/include/uadeconfig.h
rm ./src/include/sysincludes.h
rm ./src/Makefile
rm ./compat/stdinttest.c
rm ./compat/stdinttest
rm ./compat/memmemtest
rm ./compat/canontest
rm ./Makefile
