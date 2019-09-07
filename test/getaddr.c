#include <stdlib.h>
#include <stdio.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
int main()
{
  struct ifaddrs *addrs;
  getifaddrs(&addrs);
  while(addrs != NULL)
  {
    //ip地址获取
    struct sockaddr_in *ip = (struct sockaddr_in*)addrs->ifa_addr;
    //子网掩码
    struct sockaddr_in *mask = (struct sockaddr_in*)addrs->ifa_netmask;
    //跳过不是ipv4的网卡
    if(ip->sin_family != AF_INET)
    {
      addrs = addrs->ifa_next;
      continue;
    }
    //inet_addr可以将点分十进制的ip地址转换为网络字节序的ip地址
    //跳过本地回环网卡
    if(ip->sin_addr.s_addr == inet_addr("127.0.0.1"))
    {
      addrs = addrs->ifa_next;
      continue;
    }
    printf("name:%s\n", addrs->ifa_name);
    printf("ip:%s\n", inet_ntoa(ip->sin_addr));
    printf("mask:%s\n", inet_ntoa(mask->sin_addr));
    //网络字节序转换主机字节序
    uint32_t net = ntohl(mask->sin_addr.s_addr & ip->sin_addr.s_addr);
    uint32_t host = ntohl(~mask->sin_addr.s_addr);
    int i;
    for(i = 1; i < host; i++)
    {
      struct in_addr ip;
      ip.s_addr = htonl(net + i);
      printf("ip:%s\n", inet_ntoa(ip));
    }
    addrs = addrs->ifa_next;
  }
}
