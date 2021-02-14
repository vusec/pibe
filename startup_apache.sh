#!/bin/bash
source /etc/apache2/envvars
sudo sysctl -w net.core.somaxconn=4096

sudo /etc/init.d/apache2 start
