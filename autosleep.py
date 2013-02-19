#!/usr/bin/python
import inotifyx, re, syslog, os
"""
TODO: Deal with log rotation, 9-5 no sleep schedule, xbmc/vlc(via psutil?) no sleep
"""
LOG =  "/var/log/auth.log"
ifd = inotifyx.init()
log = inotifyx.add_watch(ifd, LOG, inotifyx.IN_MODIFY|inotifyx.IN_ATTRIB|inotifyx.IN_DELETE_SELF|inotifyx.IN_MOVE_SELF)
f = open(LOG)
sessions = set()
while True:
    line = f.readline()
    if line == '':
        # we've reached end of log, inotify avoids polling for growth
        # suspend if after 60seconds after activity in auth log stopped, there are still 0 ssh sessions
        ls = []
        while len(ls) == 0:
            ls = inotifyx.get_events(ifd, 60)
            if len(ls) + len(sessions) == 0:
                syslog.syslog("No more ssh sessions, going to sleep");
                os.system("/home/taras/work/sleepyscripts/suspend.sh")
        for e in ls:
            print e.get_mask_description()
        continue
    m = re.search('sshd\[([0-9]+)\]:.*session (opened|closed)', line)
    if m != None:
        [pid, reason] = m.groups()
        if reason == "opened":
            sessions.add(pid)
        else:
            sessions.remove(pid)
    print [sessions,line.strip()]    