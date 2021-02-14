#!/bin/bash
sudo sysctl -w net.core.somaxconn=4096

sudo /etc/init.d/nginx start
