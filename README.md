# P2PDownload
## 架构分析
&emsp;&emsp;分为客户端与服务端，服务端可以获取本地下载文件夹中的文件列表，并且根据所选文件获取文件内容然后通过http协议将数据返回客户端。客户端主要负责获取局域网内所有可以连接的主机列表，然后选择主机ip地址后获取主机下载文件夹中的文件列表然后继续发送请求请求下载指定文件中的内容。
### 服务端
&emsp;&emsp;1、实现主机配对的响应功能。</br>
&emsp;&emsp;2、实现文件列表的获取。</br>
&emsp;&emsp;3、实现文件下载的响应。</br>
### 客户端
&emsp;&emsp;1、获取局域网中的所有主机ip地址。</br>
&emsp;&emsp;2、获取在线主机列表（逐个发送配对请求判断响应，完成广播功能）。</br>
&emsp;&emsp;3、打印在线主机列表，并且用户选择想要查看的主机共享文件列表。</br>
&emsp;&emsp;4、向选择的主机发送文件列表请求，获取文件列表。</br>
&emsp;&emsp;5、打印文件列表，并且用户选择想要下载的文件。</br>
&emsp;&emsp;6、下载文件（向指定的主机发送指定的文件下载列表）</br>
