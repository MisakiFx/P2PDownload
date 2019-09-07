/* *
 * 使用httplib实现一个最简单的http服务器
 * 了解httplib最基本的使用
 * */
#include "httplib.h"
using namespace httplib;
void HelloWorld(const Request& req, Response& rsp)
{
  //重定向
  rsp.status = 302;
  //设置头部信息
  rsp.set_header("Location", "http://www.baidu.com");
  //正文
  rsp.body = "<html><h1>HelloWorld</h1></html>";
}
int main()
{
  Server server;
  server.Get("/", HelloWorld);
  server.listen("0.0.0.0", 9000);
}
