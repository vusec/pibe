#!/bin/bash

sudo apt purge apache2 apache2-utils apache2-bin apache2-data

sudo apt install apache2 apache2-utils

echo "ListenBackLog 1024" | sudo tee -a /etc/apache2/apache2.conf

dd bs=1 count=4 </dev/urandom | sudo tee /var/www/html/index.html 

./startup_apache.sh

./stop_apache.sh

sudo update-rc.d apache2 disable


