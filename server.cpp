#include <iostream>
#include <boost/filesystem.hpp>
#include "httplib.h"
#include <fstream>
#define SHARE_PATH "Download"
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
  }
  bool Start(uint16_t port)
  {
    _server.Get("/hostpair", GetHostPair);
    _server.Get("/list", GetFileList);
    _server.Get("/list/(.*)", GetFileData);
    _server.listen("0.0.0.0", port);
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
      std::string path = item_begin->path().string();
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
  //获取文件数据
  static void GetFileData(const Request& req, Response& rsp)
  {
    bf::path path(req.path);
    std::string name = std::string(SHARE_PATH) + "/" + path.filename().string();
    //打开文件
    std::ifstream file(name, std::ios::binary);
    if(!file.is_open())
    {
      std::cerr << "file open failed!" << std::endl;
      rsp.status = 404;
      return;
    }
    int64_t fsize = bf::file_size(name);
    rsp.body.resize(fsize);
    //读取数据到bdoy中
    file.read(&rsp.body[0], fsize);
    if(!file.good())
    {
      std::cerr << "read file" << name << " body error" << std::endl;
      rsp.status = 500;
      return;
    }
    file.close();
    //内容类型设置为二进制流
    rsp.set_header("Content-Type", "application/octet-stream");
    rsp.status = 200;
  }
};
int main()
{
  P2PServer srv;
  srv.Start(9000);
}
