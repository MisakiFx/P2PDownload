#pragma once
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include "httplib.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#define RANGE_SIZE (100 << 20)
using namespace httplib;
namespace bf = boost::filesystem;
//class Download
//{
//  public:
//    bool _res;
//    Download()
//      :_res(false)
//    {
//
//    }
//    void SetData(std::string host, std::string& name, int64_t& start, int64_t& end, int64_t& len)
//    {
//      _host = host;
//      _name = name;
//      _start = start;
//      _end = end;
//      _len = len;
//    }
//    bool Start()
//    {
//      return true;
//    }
//  private:
//    std::string _name;
//    std::string _host;
//    int64_t _start;
//    int64_t _end;
//    int64_t _len;
//};
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
            std::cout << "input error" << std::endl;
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
        //网络号
        net = ntohl(ip->sin_addr.s_addr & mask->sin_addr.s_addr);
        //最大主机号
        host = ntohl(~mask->sin_addr.s_addr);
        for(int i = 1; i <= host; i++)
        {
          struct in_addr ip;
          ip.s_addr = htonl(net + i);
          list.push_back(inet_ntoa(ip));
        }
      }
      freeifaddrs(addrs);
      return true;
    }
    //线程配对入口函数
    //注意这里的参数要和下面线程创建中的入口函数参数类型严格匹配，如果是引用下面也得传引用
    void HostPair(std::string& i)
    {
      Client client(i.c_str(),_srv_port);
      auto rsp = client.Get("/hostpair");
      if(rsp && rsp->status == 200)
      {
        _online_list.push_back(i);
      }
      printf("%s\n", i.c_str());
      return;
    }
    //获取在线主机列表
    bool GetOnlineHost(std::vector<std::string>& list)
    {
      _online_list.clear();
      std::vector<std::thread> thr_list(list.size());
      //广播配对
      for(int i = 0; i < list.size(); i++)
      {
        //一个个发送连接请求过慢，使用多线程并行完成
        //参数与入口函数参数类型严格匹配
        thr_list[i] = std::move(std::thread(&P2PClient::HostPair, this, std::ref(list[i])));
      }
      for(auto& i : thr_list)
      {
        i.join();
      }
      return true;
    }
    //打印在线主机列表
    bool ShowOnlineHost()
    {
      if(_online_list.empty())
      {
        std::cerr << "no online host!" << std::endl;
        return false;
      }
      for(int i = 0; i < _online_list.size(); i++)
      {
        std::cout << i << ". " << _online_list[i] << std::endl;
      }
      std::cout << _online_list.size() << ". " << "return" << std::endl;
      std::cout << "please choose:";
      fflush(stdout);
      std::cin >> _host_idx;
      if(_host_idx == _online_list.size())
      {
        _host_idx = -1;
        return false;
      }
      if(_host_idx < 0 || _host_idx > _online_list.size())
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
      if(_file_list.empty())
      {
        std::cerr << "no file!" << std::endl;
        return false;
      }
      for(int i = 0; i < _file_list.size(); i++)
      {
        std::cout << i << ". " << _file_list[i] << std::endl;
      }
      std::cout << _file_list.size() << ". " << "return" << std::endl;
      std::cout << "please choose:";
      fflush(stdout);
      int file_idx;
      std::cin >> file_idx;
      if(file_idx == _file_list.size())
      {
        return false;
      }
      if(file_idx < 0 || file_idx > _file_list.size())
      {
        std::cerr << "choose error" << std::endl;
        return false;
      }
      name = _file_list[file_idx];
      return true;
    }
    //分块下载，线程入口函数
    void RangeDownload(std::string host, std::string name, int64_t start, int64_t end, int* res)
    {
      *res = 0;
      std::string uri = "/list/" + name;
      std::string realpath = "Download/" + name;
      std::stringstream range_val;
      range_val << "bytes=" << start << "-" << end;
      Client client(host.c_str(), _srv_port);
      Headers header;
      header.insert(std::make_pair("Range", range_val.str().c_str()));
      auto rsp = client.Get(uri.c_str(), header);
      //收到的回复正常
      if(rsp && rsp->status == 206)
      {
        int fd = open(realpath.c_str(), O_CREAT | O_WRONLY, 0664);
        //文件未成功打开
        if(fd < 0)
        {
          std::cerr << "file " << realpath << " open error" << std::endl;
          return;
        }
        lseek(fd, start, SEEK_SET);
        int ret = write(fd, &rsp->body[0], rsp->body.size());
        //文件传输失败
        if(ret < 0)
        {
          std::cerr << "file " << realpath << " write error" << std::endl;
          close(fd);
          return;
        }
        close(fd);
        *res = 1;
        std::cerr << "file " << realpath << " download range: " << range_val.str() << " success" << std::endl;
        return;
      }
    }
    //获取文件长度
    int64_t GetFileSize(std::string host, std::string& name)
    {
      int64_t fsize = -1;
      Client client(host.c_str(), _srv_port);
      std::string path = "/list/" + name;
      auto rsp = client.Head(path.c_str());
      if(rsp && rsp->status == 200)
      {
        if(!rsp->has_header("Content-Length"))
        {
          return -1;
        }
        std::string len = rsp->get_header_value("Content-Length");
        std::stringstream tmp;
        tmp << len;
        tmp >> fsize;
      }
      return fsize;
    }
    //下载文件
    bool DownloadFIle(std::string& name)
    {
      std::string host = _online_list[_host_idx];
      int64_t fsize = GetFileSize(host.c_str(), name);
      if(fsize < 0)
      {
        std::cerr << "download file " << name << " failed" << std::endl;
        return false;
      }
      int count = fsize / RANGE_SIZE;
      std::vector<boost::thread> thr_list(count + 1);
      std::vector<int> res_list(count + 1);
      int ret = true;
      int64_t rangeSize = ((int64_t)2 << 30);
      for(int64_t i = 0; i <= count; i++)
      {
        int64_t start, end, rlen;
        start = i * RANGE_SIZE;
        end = (i + 1) * RANGE_SIZE - 1;;
        if(i == count)
        {
          if(fsize % RANGE_SIZE == 0)
          {
            break;
          }
          end = fsize - 1;
        }
        rlen = end - start + 1;
        int* res = &res_list[i];
        boost::thread thr(&P2PClient::RangeDownload, this, host, name, start, end, res); 
        if(fsize >= rangeSize)
        {
          thr.join();
          if(*res == 0)
          {
            ret = false;
          }
        }
        else 
        {
          thr_list[i] = std::move(thr);
        }
      }
      if(fsize < rangeSize)
      {
        for(int i = 0; i <= count; i++)
        {
          if(i == count  && (fsize % RANGE_SIZE) == 0)
          {
            break;
          }
          thr_list[i].join();
          if(res_list[i] == 0)
          {
            ret = false;
          }
        }
      }
      if(ret == true)
      {
        std::cout << "download file " << name << " success" << std::endl;
        return true;
      }
      else
      {
        std::cout << "download file " << name << " false" << std::endl;
        return false;
      }
   //   Client client(_online_list[_host_idx].c_str(), _srv_port);
   //   std::string uri = "/list/" + name;
   //   auto rsp = client.Head(uri.c_str());
   //   if(rsp && rsp->status == 200)
   //   {
   //     //std::string realpath = "Shared/" + name;
   //     //std::ofstream file(realpath, std::ios::binary);
   //     ////打开失败
   //     //if(!file.is_open())
   //     //{
   //     //  std::cerr << "file " << realpath << " open failed" << std::endl;
   //     //  return false;
   //     //}
   //     ////全部写入文件中
   //     //file.write(&rsp->body[0], rsp->body.size());
   //     ////写入失败
   //     //if(!file.good())
   //     //{
   //     //  std::cerr << "file " << realpath << " write body error" << std::endl;
   //     //  return false;
   //     //}
   //     //file.close();
   //     //std::cout << "file " << realpath << " download success" << std::endl;
   //     if(rsp->has_header("Content-Length"))
   //     {
   //       //表示可以获取文件长度进行分块下载
   //       std::string len = rsp->get_header_value("Content-Length");
   //       int64_t content_len;
   //       std::stringstream tmp;
   //       tmp << len;
   //       tmp >> content_len;
   //       int count = content_len / RANGE_SIZE;
   //       std::vector<std::thread> thr_list(count + 1);
   //       std::vector<Download> res_list(count + 1);
   //       for(int i = 0; i <= count; i++)
   //       {
   //         int64_t start = i * RANGE_SIZE;
   //         int64_t end = (i + 1) * RANGE_SIZE;
   //         if(i == count)
   //         {
   //           if(content_len % RANGE_SIZE == 0)
   //           {
   //             break;
   //           }
   //           end = content_len - 1;
   //         }
   //         int64_t len = end - start + 1;
   //         res_list[i].SetData(host, name, start, end, len);
   //         std::thread thr(&P2PClient::RangeDownload, this, &res_list[i]); 
   //         thr_list[i] = std::move(thr);
   //       }
   //       bool ret = true;
   //       for(int i = 0; i <= count; i++)
   //       {
   //         thr_list[i].join();
   //         if(res_list[i]._res == true)
   //         {
   //           continue;
   //         }
   //         ret = false;
   //       }
   //       if(ret == false)
   //       {
   //         std::cerr << "download file " << name << " failed" << std::endl;;
   //         return false;
   //       }
   //     }
   //     else 
   //     {
   //       //无法进行分块下载
   //       //1.正常下载（有风险）
   //       //2.报错不支持
   //       std::cerr << "download file " << name << " failed" << std::endl;;
   //       return false;
   //     }
   //   }
   //   else
   //   {
   //     std::cerr << "file " << name << " download failed" << std::endl;
   //     return false;
   //   }
   //   std::cout << "download file " << name << " success" << std::endl;
   //   return true;
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
      scanf("%d", &choose);
      return choose;
    }
};
//int main()
//{
//  P2PClient client(9000);
//  client.Start();
//}

