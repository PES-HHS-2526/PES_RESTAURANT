sudo nmcli con add type ethernet ifname eth0 con-name pi-link ip4 192.168.10.2/24
sudo nmcli con up pi-link