#!/bin/bash
#lab0 step3: read from xxx and copy to xxx.c at line 8 32 128 512 1024
rm $2
touch $2
sed -n 8p $1 >> $2
sed -n 32p $1 >> $2
sed -n 128p $1 >> $2
sed -n 512p $1 >> $2
sed -n 1024p $1 >> $2
