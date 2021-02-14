#!/bin/bash

[[ !  -z  $1  ]] && echo Specify a folder to save the backup $1 || exit
cd $1
sudo cp *-5.1.0 /boot
sudo cp -r 5.1.0 /lib/modules/
sudo update-grub2
cd ..
