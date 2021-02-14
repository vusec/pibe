#!/bin/bash

sudo /etc/init.d/nginx stop

sudo rm -f /tools/log/nginx/access.log
sudo rm -f /tools/log/nginx/error.log
