#!/bin/bash
/OSLAB/gxemul $1 -E testmips -C R3000 -M 64 -d gxemul/fs.img gxemul/vmlinux
