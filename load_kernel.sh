#!/bin/bash

./clean_kernel.sh

cd playground/images && ./add_grub_entry.sh $1 && cd ../..

sudo reboot
