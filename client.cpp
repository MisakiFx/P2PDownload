#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "httplib.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
using namespace httplib;
namespace bf = boost::filesystem;
class P2PClient
{
  public:
    P2PClient(uint16_t port)
      :_srv_port(port)
    {

    }
    bool Start()
    {
      while(1)
      {
        int choose = DoFace();
        std::vector<std::string> list;
        std::string filename;
        switch(choose)
        {
          case 1:
            GetAllHost(list);
            GetOnlineHost(list);
            break;
          case 2:
            if(ShowOnlineHost() == false)
            {
              break;
            }
            GetFileList();
            break;
          case 3:
            if(ShowFileList(filename) == false)
            {
              break;
            }
            DownloadFIle(filename);
            break;
          case 0:
            exit(0);
            break;
          default:
            break;
        }
      }
    }
  private:
    int _host_idx;//选择的主机下标
    int _srv_port;//目标服务器的端口
    std::vector<std::string> _online_list;//在线主机列表
    std::vector<std::string> _file_list;//文件列表
    //获取所有主机
    bool GetAllHost(std::vector<std::string>& list)
    {
      struct ifaddrs* addrs = NULL;
      getifaddrs(&addrs);
      struct sockaddr_in* ip = NULL;
      struct sockaddr_in* mask = NULL;
      for(; addrs != NULL; addrs = addrs->ifa_next)
      {
        ip = (struct sockaddr_in*)addrs->ifa_addr;
        mask = (struct sockaddr_in*)addrs->ifa_netmask;
        //不是ipv4的跳过
        if(ip->sin_family != AF_INET)
        {
          continue;
        }
        //跳过本地回环网卡
        if(ip->sin_addr.s_addr == inet_addr("127.0.0.1"))
        {
          continue;
        }
        uint32_t net, host;
        net = ntohl(ip->sin_addr.s_addr & mask->sin_addr.s_addr);
        host = ntohl(~mask->sin_addr.s_addr);
        for(int i = 128; i < host; i++)
        {
          struct in_addr ip;
          ip.s_addr = htonl(net + i);
          list.push_back(inet_ntoa(ip));
        }
      }
      freeifaddrs(addrs);
      return true;
    }
    //获取在线主机列表
    bool GetOnlineHost(std::vector<std::string>& list)
    {
      //广播配对
      for(auto& i : list)
      {
        Client client(&i[0],_srv_port);
        auto rsp = client.Get("/hostpair");
        if(rsp && rsp->status == 200)
        {
          std::cout << "host" << i << " pair success" << std::endl;
          _online_list.push_back(i);
          break;
          continue;
        }
        std::cout << "host" << i << " pair failed" << std::endl;
      }
      return true;
    }
    //打印在线主机列表
    bool ShowOnlineHost()
    {
      for(int i = 0; i < _online_list.size(); i++)
      {
        std::cout << i << ". " << _online_list[i] << std::endl;
      }
      std::cout << "please choose:";
      fflush(stdout);
      std::cin >> _host_idx;
      if(_host_idx < 0 || _host_idx >= _online_list.size())
      {
        _host_idx = -1;
        std::cerr << "choose error" << std::endl;
        return false;
      }
      return true;
    }
    //获取文件列表
    bool GetFileList()
    {

      Client client(_online_list[_host_idx].c_str(), _srv_port);
      auto rsp = client.Get("/list");
      if(rsp && rsp->status == 200)
      {
        std::vector<std::string> list;
        boost::split(list, rsp->body, boost::is_any_of("\n"));
        list.pop_back();
        _file_list = list;
        return true;
      }
      else 
      {
        std::cerr << "get file list error" << std::endl;
        return false;
      }
    }
    //打印文件列表
    bool ShowFileList(std::string& name)
    {
      for(int i = 0; i < _file_list.size(); i++)
      {
        std::cout << i << ". " << _file_list[i] << std::endl;
      }
      std::cout << "please choose:";
      fflush(stdout);
      int file_idx;
      std::cin >> file_idx;
      if(file_idx < 0 || file_idx >= _file_list.size())
      {
        std::cerr << "choose error" << std::endl;
        return false;
      }
      name = _file_list[file_idx];
      return true;
    }
    //下载文件
    bool DownloadFIle(std::string& name)
    {
      Client client(_online_list[_host_idx].c_str(), _srv_port);
      std::string uri = "/list/" + name;
      auto rsp = client.Get(uri.c_str());
      if(rsp && rsp->status == 200)
      {
        std::string realpath = "Shared/" + name;
        std::ofstream file(realpath, std::ios::binary);
        //打开失败
        if(!file.is_open())
        {
          std::cerr << "file " << realpath << " open failed" << std::endl;
          return false;
        }
        //全部写入文件中
        file.write(&rsp->body[0], rsp->body.size());
        //写入失败
        if(!file.good())
        {
          std::cerr << "file " << realpath << " write body error" << std::endl;
          return false;
        }
        file.close();
        std::cout << "file " << realpath << " download success" << std::endl;
      }
      else
      {
        std::cerr << "file " << name << " download failed" << std::endl;
      }
      return true;
    }
    //界面
    int DoFace()
    {
      std::cout << "1.Serch Host nearby" << std::endl;
      std::cout << "2.Show Online Host" << std::endl;
      std::cout << "3.Show File List" << std::endl;
      std::cout << "0.Exit" << std::endl;
      int choose;
      std::cout << "please choose:";
      fflush(stdout);
      std::cin >> choose;
      return choose;
    }
};
int main()
{
  P2PClient client(9000);
  client.Start();
}

