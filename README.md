The router steals server ip while server sleeps, then any connections to the IP are logged which triggers an awk script

iptables rules:
```
iptables -t nat -N wol-nas # create new chain
iptables -t nat -A wol-nas -j LOG --log-prefix 'NAS-PING' --log-level 4
iptables -t nat -A wol-nas -j DROP
iptables -t nat -A PREROUTING -d 192.168.1.149 -j wol-nas
```

On the server the wakeup script should execute:
```
arping -U 192.168.1.149 -w 1
arping -A 192.168.1.149 -w 1
```
To flush the fake ip out of arp tables on the LAN

ifconfig br0:1 192.168.1.149;tail -n0 -f /var/log/messages  | awk '/NAS-PING/ {system("/sbin/ifconfig br0:1 0.0.0.0; /usr/bin/ether-wake -b 4C:72:B9:42:EA:97"); exit 0}'
