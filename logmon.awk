#!/usr/bin/awk -f
# echo -e NAS-TAKEOVER\nNAS-WAKE| ./logmon.awk /dev/stdin
BEGIN {
  ETH="br0:1"
  CONTROL_ETH="br0:0"
  IP="192.168.1.149"
  CONTROL_IP="192.168.1.254"
  MAC="4C:72:B9:42:EA:97"
  setup_iptables("NAS-WAKE", IP);
  setup_iptables("NAS-TAKEOVER", CONTROL_IP);
  mysystem("/sbin/ifconfig "CONTROL_ETH" "CONTROL_IP);
}

function mysystem(cmd) {
  print cmd
  system(cmd)
}

function setup_iptables(RULE, IP, EXTRA)
{
  IPT = "/usr/sbin/iptables -t nat" 
  mysystem(IPT" -N "RULE)
  mysystem(IPT" -A "RULE" -j LOG --log-prefix "RULE" --log-level 4")
  mysystem(IPT" -A "RULE" -j DROP")
  mysystem(IPT" -A PREROUTING -d "IP" -j "RULE" "EXTRA)
}

/NAS-TAKEOVER/ {
  mysystem("/sbin/ifconfig "ETH" "IP);
}

/NAS-WAKE/ {
  mysystem("/sbin/ifconfig "ETH" 0.0.0.0");
  mysystem("/usr/bin/ether-wake -b "MAC);
}
