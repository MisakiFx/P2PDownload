#pragma  once
#include <iostream>
#include <boost/filesystem.hpp>
#include "httplib.h"
#include <fstream>
#define SHARE_PATH "Shared"
#define DOWNLOAD_PATH "Download"
using namespace httplib;
namespace bf = boost::filesystem;
class P2PServer
{
public:
  P2PServer()
  {
    if(!bf::exists(SHARE_PATH))
    {
      bf::create_directory(SHARE_PATH);
    }
    if(!bf::exists(DOWNLOAD_PATH))
    {
      bf::create_directory(DOWNLOAD_PATH);
    }
  }
  bool Start(uint16_t port)
  {
    _server.Get("/hostpair", GetHostPair);
    _server.Get("/list", GetFileList);
    _server.Get("/list/(.*)", GetFileData);
    _server.listen("0.0.0.0", port);
    return true;
  }
private:
  Server _server;
  //配对检查
  static void GetHostPair(const Request& req, Response& rsp)
  {
    rsp.status = 200;
  }
  //获取文件列表
  static void GetFileList(const Request& req, Response& rsp)
  {
    bf::directory_iterator item_begin(SHARE_PATH);
    bf::directory_iterator item_end;
    //std::stringstream body;
    //body << "<html><body>";
    //std::string body;
    for(; item_begin != item_end; ++item_begin)
    {
      //越过目录
      if(bf::is_directory(item_begin->status()))
      {
        continue;
      }
      //获取路径
      //std::string path = item_begin->path().string();
      //获取文件名
      std::string name = item_begin->path().filename().string();
      //std::cout << name << std::endl;
      rsp.body += name + "\n";
      //body << "<h4><a href='/list/" << name <<  "'>";
      //body << name;
      //body << "</a></h4>";
    }
    //body << "</body></html>";
    rsp.set_header("Content-Type", "text/html");
    //rsp.body = body.str();
    rsp.status = 200;
    //rsp.set_content(&body[0], body.size(), "text");
  }
  static bool RangeParse(std::string& range_val, int64_t& start, int64_t& len)
  {
      size_t pos1 = range_val.find('=');
      size_t pos2 = range_val.find('-');
      //没找到
      if(pos1 == std::string::npos || pos2 == std::string::npos)
      {
        std::cerr << "range " << range_val << " format error" << std::endl;
        return false;
      }
      int64_t end;
      std::string rstart, rend;
      rstart = range_val.substr(pos1 + 1, pos2 - pos1 - 1);
      rend = range_val.substr(pos2 + 1, std::string::npos);
      std::stringstream tmp;
      tmp << rstart;
      tmp >> start;
      tmp.clear();
      tmp << rend;
      tmp >> end;
      len = end - start + 1;
      return true;
  }
  //获取文件数据
  static void GetFileData(const Request& req, Response& rsp)
  {
    bf::path path(req.path);
    std::string name = std::string(SHARE_PATH) + "/" + path.filename().string();
    if(!bf::exists(name))
    {
      rsp.status = 404;
      return;
    }
    if(bf::is_directory(name))
    {
      rsp.status = 403;
      return;
    }
    //条件符合正式开始传输文件
    //获取文件大小
    int64_t fsize = bf::file_size(name);
    //如果发送的是HEAD请求，客户端此时只想要文件长度，则只用包装头部即可
    if(req.method == "HEAD")
    {
      rsp.status = 200;
      std::string len = std::to_string(fsize);
      rsp.set_header("Content-Length", len.c_str());
      return;
    }
    //GET方法，开始分块传输
    else
    {
      //没有Range信息则直接报错
      if(!req.has_header("Range"))
      {
        rsp.status = 400;
        return;
      }
      std::string range_val = req.get_header_value("Range");
      int64_t start, end, rlen;
      //开始解析range_val:bytes=start-end
      bool ret = RangeParse(range_val, start, rlen);
      if(ret == false)
      {
        rsp.status = 400;
        return;
      }
      //打开文件
      std::ifstream file(name, std::ios::binary);
      if(!file.is_open())
      {
        std::cerr << "file open failed!" << std::endl;
        rsp.status = 404;
        return;
      }
      file.seekg(start, std::ios::beg);
      rsp.body.resize(rlen);
      //读取数据到bdoy中
      file.read(&rsp.body[0], rlen);
      if(!file.good())
      {
        std::cerr << "read file" << name << " body error" << std::endl;
        rsp.status = 500;
        return;
      }
      file.close();
      //内容类型设置为二进制流
      rsp.set_header("Content-Type", "application/octet-stream");
      rsp.status = 206;
      std::cout << "file range " << range_val << " download success" << std::endl;
    }
  }
};
//int main()
//{
//  P2PServer srv;
//  srv.Start(9000);
//}
