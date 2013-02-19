ethtool -s eth0 wol g
sync
echo mem > /sys/power/state
