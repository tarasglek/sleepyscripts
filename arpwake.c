#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <net/ethernet.h>

#include <arpa/inet.h>

#include <linux/if_arp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PCKT_SIZE 60
#define IP_LEN 16
#define EA_LEN 24

int getifindex(const char *ifname) {
      int s;
      struct ifreq ifdev;

      if((s=socket(AF_INET,SOCK_DGRAM,0))<0) {
            return -1;
      }
      memset(&ifdev,0,sizeof(struct ifreq));
      strncpy(ifdev.ifr_name,ifname,IFNAMSIZ);
      if(ioctl(s,SIOCGIFINDEX,&ifdev)<0) {
            return -1;
      }
      close(s);
      return ifdev.ifr_ifindex;
}

struct arp_hdr {
      unsigned short int ar_hrd;          /* Format of hardware address.  */
      unsigned short int ar_pro;          /* Format of protocol address.  */
      unsigned char ar_hln;               /* Length of hardware address.  */
      unsigned char ar_pln;               /* Length of protocol address.  */
      unsigned short int ar_op;           /* ARP opcode (command).  */

      unsigned char ar_sha[ETH_ALEN];   /* Sender hardware address.  */
      unsigned char ar_sip[4];          /* Sender IP address.  */
      unsigned char ar_tha[ETH_ALEN];   /* Target hardware address.  */
      unsigned char ar_tip[4];          /* Target IP address.  */
};

void wol(const char *ip, unsigned char *mac) {
  unsigned char tosend[102];
  
  /** first 6 bytes of 255 **/
  int i;
  for( i = 0; i < 6; i++) {
    tosend[i] = 0xFF;
  }
  /** append it 16 times to packet **/
  for(i = 1; i <= 16; i++) {
    memcpy(&tosend[i * 6], mac, 6 * sizeof(mac[0]));
  }
  int udpSocket;
  struct sockaddr_in udpClient, udpServer;
  int broadcast = 1;

  udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

  /** you need to set this so you can broadcast **/
  if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
    perror("setsockopt (SO_BROADCAST)");
    exit(1);
  }
  udpClient.sin_family = AF_INET;
  udpClient.sin_addr.s_addr = INADDR_ANY;
  udpClient.sin_port = 0;

  bind(udpSocket, (struct sockaddr*)&udpClient, sizeof(udpClient));

  /** make the packet as shown above **/

  /** set server end point (the broadcast addres)**/
  udpServer.sin_family = AF_INET;
  udpServer.sin_addr.s_addr = inet_addr(ip);
  udpServer.sin_port = htons(9);

  /** send the packet **/
  sendto(udpSocket, &tosend, sizeof(unsigned char) * 102, 0, (struct sockaddr*)&udpServer, sizeof(udpServer));
}

int main(int argc, char **argv) {
      struct ether_header *ether; /* uint8 ether_dhost, ether_shost; uint16 ether_type */
      struct arp_hdr *arp;

      char *ifname;


      int s_raw;
      struct sockaddr_ll sall;

      int buflen;
      unsigned char buffer[PCKT_SIZE]={0};

      char ip_recvr[IP_LEN];
      char ip_sender[IP_LEN];
      char eth_sender[EA_LEN];
      char eth_recvr[EA_LEN];
      char *targetip;
      char *targetmac_str;
      unsigned char targetmac[6];
      char broadcast_ip[16];

      if (argc != 4) {
        printf("Usage: %s iface targetip targetmac\n", argv[0]);
        exit(1);
      }
      ifname=argv[1];
      targetip=argv[2];
      targetmac_str=argv[3];
      strcpy(broadcast_ip, targetip);
      {
        char *dot = strrchr(broadcast_ip, '.');
        if (!dot) {
          printf("Invalid ip %s\n", broadcast_ip);
          exit(1);
        }
        strcpy(dot + 1, "255");
        dot[4]=0;
      }
      {
        char *from = targetmac_str;
        int i;
        for(i = 0;i<6;i++) {
          targetmac[i] = strtol(from, &from, 16);
          from++;
        }
      }

      if((s_raw=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ARP)))<0) {
            perror("socket");
            exit(1);
      }
      memset(&sall,0,sizeof(sall));
      sall.sll_family=AF_PACKET;
      if((sall.sll_ifindex=getifindex(ifname))<0) {
            perror("getifindex");
            exit(1);
      }
      sall.sll_protocol=htons(ETH_P_ARP);
      if(bind(s_raw,(struct sockaddr *)&sall,sizeof(sall))<0) {
            perror("bind");
            exit(1);
      }
      ether=(struct ether_header *)buffer;
      arp=(struct arp_hdr *)((void *)ether+sizeof(struct ether_header));


      while(1) {
        /* cekaj pakete */
        if((buflen=recvfrom(s_raw,buffer,PCKT_SIZE,0,NULL,NULL))<0) {
          perror("recvfrom");
          exit(1);
        }
        /* imamo paket */
        if(ether->ether_type==htons(ETH_P_ARP)) {
          inet_ntop(AF_INET,(struct in_addr *)&arp->ar_tip,ip_recvr,IP_LEN);
          inet_ntop(AF_INET,(struct in_addr *)&arp->ar_sip,ip_sender,IP_LEN);
          sprintf(eth_sender,"%02x:%02x:%02x:%02x:%02x:%02x",
                  arp->ar_sha[0],arp->ar_sha[1],
                  arp->ar_sha[2],arp->ar_sha[3],
                  arp->ar_sha[4],arp->ar_sha[5]);
          sprintf(eth_recvr,"%02x:%02x:%02x:%02x:%02x:%02x",
                  arp->ar_tha[0],arp->ar_tha[1],
                  arp->ar_tha[2],arp->ar_tha[3],
                  arp->ar_tha[4],arp->ar_tha[5]);
          if(arp->ar_op==htons(ARPOP_REQUEST)) {
            printf("who-has %s tell %s at %s\n",
                   ip_recvr,
                   ip_sender,
                   eth_sender);
            if (!strcmp(ip_recvr, targetip)) {
              printf("WOL %s@%s\n", ip_recvr, targetmac_str);
              wol(broadcast_ip, targetmac);
            }

          } else if(arp->ar_op==htons(ARPOP_REPLY)) {
            printf("%s is-at %s\n",ip_sender,eth_sender);
          }
        } /* if paket ARP */
      } /* while(1) */
      return 0;
}
