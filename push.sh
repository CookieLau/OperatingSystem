#!/bin/bash
#auto-git-push
git add ./pageReplace.c
git commit -m $1
git push origin racing:racing
