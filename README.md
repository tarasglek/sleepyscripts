# Automatically Sleeping

To put the server to sleep run `autosleep.py` as root. It will monitor auth log for ssh activity and suspend a minute after last auth log activity

# Wake up on directed TCP activity
Run `arpwake.mips br0 192.168.1.149 4C:72:B9:42:EA:97` on the router. arpwake waits for arp lookups of 192.168.1.149 and sends a WOL packet to 4C:72:B9:42:EA:97

## Future improvements
* Allow to specify a list of comma-separated IPs. This is useful for hosts with virtual machines where there may be many possible ips.
* Run a command on arp. This would be useful to start virtual machines ondemand
* Some clients(eg android) have a short ARP timeout. arpwake should spoof an arp reply to keep the client from timing out ~2-3 seconds in. Once TCP kicks in, the timeouts are huge. ATM my server takes ~7-10seconds to wake up, that's too long for android.

# Wake up via ssh for outside connections
An easier way to wake up for outside ssh connections is to use ssh ProxyCommand, where the first part of the command sends an etherwake packet. This isn't as general as above(obviously) as it's done on the clientside.
