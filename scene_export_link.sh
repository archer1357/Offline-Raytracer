#!/bin/bash 

b=~/.config/blender
v=`ls $b | tail --lines=1`
ln -sf scene_export.py $b/$v/scripts/addons/