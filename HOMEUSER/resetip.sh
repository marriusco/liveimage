#!/bin/bash
sudo iptables -F
sudo iptables -X
sudo iptables-save
sudo iptables -F
sudo iptables -X
sudo sh -c "iptables-save > /etc/iptables_default-no_rules.txt"

