#!/bin/bash
[[ !  -z  $1  ]] || exit
[[ !  -z  $2  ]] || exit
# adduser username
# usermod -aG sudo username
echo "" | ssh-keygen -t rsa

ssh $1@$2 mkdir -p .ssh

cat ~/.ssh/id_rsa.pub | ssh $1@$2 'cat >> .ssh/authorized_keys'

echo "$1 ALL=(ALL) NOPASSWD: ALL" 

ssh -t $1@$2 'echo "$USER ALL=(ALL) NOPASSWD:ALL" | sudo tee -a /etc/sudoers'
