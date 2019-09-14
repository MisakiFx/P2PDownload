#include "client.hpp"
#include "server.hpp"
void srv_start()
{
  P2PServer server;
  server.Start(9000);//默认9000端口
}
int main()
{
  //服务端
  std::thread thr(srv_start);
  thr.detach();
  //客户端
  P2PClient client(9000);
  client.Start();
}
