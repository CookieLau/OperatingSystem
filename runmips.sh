#!/bin/bash
#run emulated mips !
make clean
make
/OSLAB/gxemul -E testmips -C R3000 -M 64 gxemul/vmlinux
make clean
