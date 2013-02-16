The router steals server ip while server sleeps, then any connections to the IP are logged which triggers an awk script

In the tomate firmware, copy the awk script to /jffs partion, then in the firewall script(or anywhere else in startup path) add

```
/usr/bin/tail -f /var/log/messages |/jffs/logmon.awk &
```


On the server the sleep script should execute:
```
ping -c 1 -w 1 192.168.1.254
```
to notify that the server is about to go to sleep.

Then after wake up the server should do
```
arping -U 192.168.1.149 -w 1
arping -A 192.168.1.149 -w 1
```
to flush the fake ip out of arp tables on the LAN
