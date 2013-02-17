ethtool -s eth0 wol g
ping -c 1 192.168.1.254 -w 1
sync
echo mem > /sys/power/state
ethtool -s eth0 wol d
arping -U 192.168.1.149 -w 1
arping -A 192.168.1.149 -w 1 
