key：dff51e3cf58147c687884a86b88b72ea
topic：MyNB02
服务器地址：bemfa.com
普通端口：9501

可用指令：
AT+ECMTCFG="keepalive",0,60
AT+ECMTOPEN=0,"bemfa.com",9501
AT+ECMTCONN=0,"你的ClientId"
AT+ECMTSUB=0,1,"你的主题名",0
AT+ECMTPUB=0,1,0,0,"你的主题名","你要发送的消息"
经直连EC-01G板子测试指令可用 显示连接成功但是巴法云平台依旧离线无法收发信息

3.5 MQTT 命令
3.5.1 AT+ECMTCFG 创建客户端
该命令创建一个 MQTT 客户端。
AT+ECMTCFG
设置命令 响应
配置保活时间
OK
AT+ECMTCFG=”keepalive”,<tcpconnectID>[,<keep
-alive time>]
如果省略<keep-alive time>，请查
询keep-alive时间：
+ECMTCFG: "keepalive",<keep￾alive time>
OK
如果发生错误，响应:
ERROR
NB-IOT系列模组 AT指令集 V1.0
设置命令
配置会话类型
AT+ECMTCFG=”session”,<tcpconnectID>[,<clean_
session>]
响应
OK
如果省略<clean_session>，请查询
会话类型：
+ECMTCFG:
"session",<clean_session>
OK
如果发生错误，响应:
ERROR
设置命令
配置消息发送超时时间
AT+ECMTCFG=”timeout”,<tcpconnectID>[,<pkt_ti
meout>[,<retry_times>][,<timeout_notice>]]
响应
OK
如果省略<pkt_timeout>，
<retry_times>，
<timeout_notice>，请查询消息发
送的超时值：
+ECMTCFG:
"timeout",<pkt_timeout>,<retr
y_times>,<timeout_notice>
OK
如果发生错误，响应:
ERROR
设置命令
配置Will信息
AT+ECMTCFG=”will”,<tcpconnectID>[,<will_fg>[
,<will_qos>,<will_retain>,“<will_topic>”,“<w
ill_msg>”]]
响应
OK
如果省略<will_fg>，<will_qos>，
<will_retain>，<will_topic>和
<will_msg>，请查询Will信息：
+ECMTCFG:
"will",<will_fg>[,<will_qos>,
<will_retain>,<will_topic>,<w
ill_msg>]
NB-IOT系列模组 AT指令集 V1.0
设置命令
配置要使用的MQTT协议版本
AT+ECMTCFG=”version”,<tcpconnectID>[,<versio
n>]
设置命令
配置阿里云的设备信息
AT+ECMTCFG=”aliauth”,<tcpconnectID>[,“<produ
ct_key>”,“<device_name>”,“<device_secret>”]
设置命令
配置云类型和云发送数据的格式
AT+ECMTCFG=”cloud”,<tcpconnectID>,<cloud
type>,<data type>
OK
如果发生错误，响应:
ERROR
响应
OK
如果省略<version>，请查询MQTT
协议版本：
+ECMTCFG: "version",<version>
OK
如果发生错误，响应:
ERROR
响应
OK
如果省略“ <product_key>”，
“ <device_name>”，
“ <device_secret>”，请查询设
备信息：
+ECMTCFG:
"aliauth",<product_key>,<devi
ce_name>,<device_secret>
OK
如果发生错误，响应:
ERROR
响应
OK
如果发生错误，响应:
ERROR
测试命令
AT+ECMTCFG=?
响应
+ECMTCFG:
NB-IOT系列模组 AT指令集 V1.0
+ECMTCFG: "keepalive",(0),(0-
3600)
+ECMTCFG: "session",(0),(0,1)
+ECMTCFG: "timeout",(0),(1-
60),(1-10),(0,1)
+ECMTCFG:
"will",(0),(0,1),(0-
2),(0,1),"will_topic","will_m
sg"
+ECMTCFG: "version",(0),(3,4)
+ECMTCFG:
"aliauth",(0),"productkey","d
evicename","devicesecret"
+ECMTCFG: "cloud",(0-255),
(0-255) OK
最大响应时间 5秒
参数保存模式 不保存
参数
<tcpconnectID> 字符串类型
MQTT套接字标识符，值为0
<echo_mode> 整型。 是否在数据模式下将输入数据回显到UART（不支持）
0 不要将输入数据回显到UART
1 将输入数据回传到UART
<keep-alive
time>
整型。
范围是0-3600。 默认值为120。单位：秒。 它定义了从客户端收到的消息之
间的最大时间间隔。 如果服务器在保持活动时间段的1.5倍内未收到来自客户
端的消息，则它将断开客户端的连接，就好像客户端已发送DISCONNECT消
息一样。
<clean_session> 整型。 配置会话类型
0 断开连接后，服务器必须存储客户端的订阅。
1 服务器端丢弃之前为该客户端保留的任何信息，并将连接视为“clean（清除）” <pkt_timeout> 整型。
数据包传递超时时间。 范围是1-60。 默认值为10。单位：秒。
<retry_times> 整型. (不支持)
数据包传递超时的重试时间。 范围是0-10。 预设值为3。
<timeout_notice> 整型. (不支持)
0 传输数据包时不报告超时消息
1 传输数据包时报告超时消息
<will_fg> 整型。 配置Will标志
NB-IOT系列模组 AT指令集 V1.0
0 忽略Will标志配置
1 需要Will标志配置
<will_qos> 整型。 消息传递的服务质量
0 最多一次
1 最少一次
2 正好一次
<will_retain> 整型。 Will保留标志仅用于发布消息。
0 当客户端向服务器发送发布消息时，服务器在将消息传递给当前订户后将不
保留该消息
1 当客户端向服务器发送PUBLISH消息时，服务器在将消息传递给当前订户后
应保留该消息
<will_topic> 整型。
Will主题字符串，最大大小为255字节
<will_msg> 整型。
Will消息定义了客户端意外断开连接时发布到will主题的消息的内容。 它可以是
零长度的消息。 最大大小为255个字节
<version> 整型。 MQTT协议的版本，默认为MQTT v3.1.1
3 MQTT v3.1
4 MQTT v3.1.1
<product_key> 整型。
阿里云发布的产品密钥，最大大小为32个字节
<device_name> 整型. 阿里云发布的设备名称，最大为32个字节
<device_secret> 整型. 阿里云发布的设备密钥，最大为64个字节
<cloud type > 整型
0 mosquitto平台
1 OneNet平台
2 阿里云
3-255 客户自定义
<data type> 整型，范围是 0-255
OneNet 云平台定义如下
1 OneNet数据类型1
2 OneNet数据类型2
3 OneNet数据类型3
4 OneNet数据类型4
5 OneNet数据类型5
6 OneNet数据类型6
阿里云定义如下
1 Json 数据
2 字符串数据
其他平台，数据格式没有规定
NB-IOT系列模组 AT指令集 V1.0
3.5.2 AT+ECMTOPEN 打开客户端连接
该命令用于为 MQTT 客户端打开网络。
AT+ECMTOPE
设置命令 响应
AT+ECMTOPEN=<tcpconnectID>,“<host_name>”,
<port>
OK
+ECMTOPEN:
<tcpconnectID>,<result>
如果发生错误，响应:
ERROR
测试命令 响应
AT+ECMTOPEN=? +ECMTOPEN: (支持列表
<tcpconnectID>s),“<host_name>”,(支持列
表 <port>s)
OK
查询命令 响应
AT+ECMTOPEN? [+ECMTOPEN:
<tcpconnectID>,“<host_name>”,<po
rt>]
OK
最大响应时间 5秒
参数保存模式 不保存
参数
<tcpconnectID> 整型
MQTT套接字标识符。 值是0
<host_name> 字符串类型
服务器的地址。 它可以是IP地址或域名。 最大100个字节
<port> 整型
服务器的端口。 范围是1-65535
<result> 整型，命令执行结果
-1 打开网络失败
0 打开网络成功
NB-IOT系列模组 AT指令集 V1.0
3.5.3 AT+ECMTCLOSE 关闭客户端
该命令发送 MQTT 关闭数据包。
AT+ECMTCLOSE
设置命令
AT+ECMTCLOSE=<tcpconnectID>
响应
OK
+ECMTCLOSE: <tcpconnectID>,<result>
如果发生错误，响应：
ERROR
测试命令
AT+ECMTCLOSE=?
响应
+ECMTCLOSE: (支持列表 <tcpconnectID>s)
OK
最大响应时间 5秒
参数保存模式 不保存
参数
<tcpconnectID> 整型
MQTT套接字标识符。 值是0
<result> 整型
-1 关闭mqtt失败
0 关闭mqtt成功
3.5.4 AT+ECMTCONN 创建连接
连接客户端到 MQTT 服务器。
AT+ECMTCONN
设置命令
AT+ECMTCONN=<tcpconnectID>,“<clientID>”
[,“<username>”[,“<password>”]]
响应
OK
+ECMTCONN:
<tcpconnectID>,<result>[,<ret_code
>]
NB-IOT系列模组 AT指令集 V1.0
如果发生错误，响应:
ERROR
测试命令 响应
AT+ECMTCONN=? +ECMTCONN: (支持列表
<tcpconnectID>s),“<clientID>”[,“<u
sername>”[,“<password>”]]
OK
查询命令 响应
AT+ECMTCONN? [+ECMTCONN:
<tcpconnectID>,<state>]
OK
最大响应时间 5秒
参数保存模式 不保存
参数
<tcpconnectID> 整型
MQTT套接字标识符。 值是0
<clientID> 字符串类型
客户端标识符，最大48个字节
<username> 字符串类型
客户端的用户名。 可用于身份验证，最大48个字节
<password> 字符串类型
客户端用户名对应的密码。 它可以用于身份验证，最大96个字节
<result> 整型
0 发送数据成功，并收到服务器的回复
1 发送数据成功，并收到服务器的错误回复
2 发送失败
<ret_code> 整型，服务器返回的连接结果
0 连接服务器成功
1 连接服务器被拒绝 – 错误的协议版本
2 连接服务器被拒绝 – 错误的客户端ID
3 连接服务器被拒绝 – 找不到服务器
4 连接服务器被拒绝 – 用户名或者密码错误
5 连接服务器被拒绝 – 认证失败
6 连接服务器失败
NB-IOT系列模组 AT指令集 V1.0
3.5.5 AT+ECMTDISC 断开连接
断开客户端和 MQTT 服务器的连接。
AT+ECMTDISC
设置命令
AT+ECMTDISC=<tcpconnectID>
响应
OK
+ECMTDISC: <tcpconnectID>,<result>
如果发生错误，响应：
ERROR
测试命令
AT+ECMTDISC=?
响应
+ECMTDISC: (支持列表 <tcpconnectID>s)
OK
最大响应时间 5秒
参数保存模式 不保存
参数
<tcpconnectID> 整型
MQTT套接字标识符。 值是0
<result> 整型
-1 断开连接失败
0 断开连接成功
3.5.6 AT+ECMTSUB 发起订阅
该命令发送 MQTT 订阅数据包。
AT+ECMTSUB
设置命令
AT+ECMTSUB=<tcpconnectID>,<msgID>,“<topi
c>”,<qos>
响应
OK
+ECMTSUB:
<tcpconnectID>,<msgID>,<result>[,
<value>]
NB-IOT系列模组 AT指令集 V1.0
不保存 参数保存模式
5秒 最大响应时间
响应
+ECMTSUB: (支持列表
<tcpconnectID>s),(支持列表
<msgID>s),“<topic>”,(支持列表
<qos>s)
OK
测试命令
AT+ECMTSUB=?
如果发生错误，响应：
ERROR
参数
<tcpconnectID> 整型
MQTT套接字标识符。 值是0
<msgID> 整型
报文的报文标识。 范围是1-65535
<topic> 字符串类型
客户想要订阅或取消订阅的主题。 最大长度为255个字节
<qos> 整型
消息QoS，可以为0,1或2
<result> 整型
0 发送成功，并收到server回复
1 发送成功，但接收到的回复错误
2 发送失败
<value> 整型
服务器授予的qos等级
3.5.7 AT+ECMTUNS 取消订阅
该命令发送 MQTT 取消订阅数据包。
AT+ECMTUNS
设置命令
AT+ECMTUNS=<tcpconnectID>,<msgID>,“<topic>
” 响应
OK
+ECMTUNS:
<tcpconnectID>,<msgID>,<result
NB-IOT系列模组 AT指令集 V1.0
不保存 参数保存模式
5秒 最大响应时间
响应
+ECMTUNS: (支持列表
<tcpconnectID>s),(支持列表
<msgID>s),“<topic>”
OK
测试命令
AT+ECMTUNS=?
>
如果发生错误，响应：
ERROR
参数
<tcpconnectID> 整型
MQTT套接字标识符。 值是0
<msgID> 整型
报文的报文标识。 范围是1-65535
<topic> 字符串类型
客户想要订阅或取消订阅的主题。 最大长度为255个字节
<result> 整型
0 发送成功，并收到server回复
1 发送成功，但接收到的回复错误
2 发送失败
3.5.8 AT+ECMTPUB 发布数据
该命令发送 MQTT 发布数据包。
AT+ECMTPUB
设置命令
AT+ECMTPUB=<tcpconnectID>,<msgID>,<qos>,<retai
n>,“<topic>”,“<payload>" 响应
OK
+ECMTPUB:
<tcpconnectID>,<msgID>,<res
ult>[,<value>]
如果发生错误，响应：
ERROR
NB-IOT系列模组 AT指令集 V1.0
测试命令
AT+ECMTPUB=?
响应
+ECMTPUB: (支持列表
<tcpconnectID>s),(支持列表
<msgID>s),(支持列表
<qos>s),(支持列表
<retain>s),“<topic>”,“<msg>
”
OK
最大响应时间 5秒
参数保存模式 不保存
参数
<tcpconnectID> 整型
MQTT套接字标识符。 值是0
<msgID> 整型
报文的报文标识。 范围是0-65535。 仅当<qos> = 0时它将为0
<qos> 整型
消息QoS，可以为0,1或2
<retain> 整型
0 服务器不应保留该消息
1 服务器应保留该消息
<topic> 字符串类型
需要发布的主题。 最大长度为255个字节
<payload> 字符串类型 or Hex type
需要发布的消息。 最大长度为700个字节。 如果处于数据模式，则最大长度为
1024字节
<result> 整型
0 发送成功，并收到server回复
1 发送成功，但接收到的回复错误
2 发送失败
<value> 整型
服务器授予的qos等级
3.5.9 +ECMTSTAT URC 消息，报告链路层状态
当 MQTT 链路层状态发生变化时，将上报此URC。
+ECMTSTAT
NB-IOT系列模组 AT指令集 V1.0
+ECMTRECV
+ECMTRECV: <tcpconnectID>,<msgID>,<topic>,<data>
+ECMTSTAT: <tcpconnectID>,<err_code>
参数
<tcpconnectID> 整型
MQTT套接字标识符。 值是0
<err_code> 整型。错误代码
1 连接已关闭或由对等方重置
3.5.10 +ECMTRECV URC 消息，指示接收服务器数据
这是一条URC 消息，指示MQTT 客户端从 MQTT 服务器接收数据。
Parameter
<tcpconnectID> 整型
MQTT套接字标识符。 值是0
<msgID> 字符串类型
报文的报文标识。
<topic> 字符串类型
从MQTT服务器收到的主题。
<data> 字符串类型
从服务器接收数据。
3.6 Http 命令
3.6.1 AT+HTTPCREATE 创建实例
设置命令创建一个 http 或 https 客户端实例。 配置主机，服务器证书等。
查询命令返回受支持的值作为复合值。
注意：只有一个实例和 http 已完全验证。 https 和多个实例将在以后测试。
AT+HTTPCREATE
设置命令
AT+HTTPCREATE=<flag>,<host>
[,<authuser>,<authpasswd>]
响应
如果还有命令分段输入完成：
+HTTP CMD: CONTIUE ENTER CMD
NB-IOT系列模组 AT指令集 V1.0
AT+HTTPCREATE=0,”http://api.openweathermap.org:80”
+HTTPCREATE: 0
OK
不保存 参数保存模式
5s 最大响应时间
响应
+HTTPCREATE: ( 支 持 列 表 < flag >s), “<host>”,“<authuser>”,“<authpasswd>”
OK
查询命令
AT+HTTPCREATE=?
如果所有分段都输入完成：
+HTTPCREATE: <httpclientId>
如果发生错误，响应:
+HTTP ERROR: <err>
参数
<flag> 整型
1 不是命令的最后一部分
0 命令的最后一部分
<host> 字符串类型
http服务器的主机名
<authuser> 字符串类型
认证用户名
<authpasswd> 字符串类型
验证密码
< httpclientId > 整型
客户端序号，0
举例
3.6.2 AT+HTTPCON 连接服务器
设置命令创建一个套接字并与 http 服务器连接。 然后创建一个任务来接收来自 http 服务器的数据。
查询命令返回受支持的值作为复合值。
AT+HTTPCON
设置命令
AT+HTTPCON=<httpclientId>
响应
OK
如果发生错误，响应:
+HTTP ERROR: <err>
NB-IOT系列模组 AT指令集 V1.0
AT+HTTPCON=0
OK
AT+HTTPDESTROY=0
OK
AT+HTTPCON=? 响应
+HTTPCON: (支持列表< httpclientId >)
OK
最大响应时间 40秒
参数保存模式 不保存
参数
<httpclientId> 整型
+ HTTPCREATE指令返回的客户端序号
举例
3.6.3 AT+HTTPDESTROY 关闭连接
设置命令关闭套接字，停止从 http 服务器接收数据，并释放创建时客户端分配的内存。
查询命令返回受支持的值作为复合值。
AT+HTTPDESTROY
设置命令
AT+HTTPDESTROY=<httpclientId>
响应
OK
如果发生错误，响应:
+HTTP ERROR: <err>
AT+HTTPDESTROY=? 响应
+HTTPDESTROY: (支持列表< httpclientId >)
OK
最大响应时间 5秒
参数保存模式 不保存
参数
<httpclientId> 整型
+ HTTPCREATE指令返回的客户端序号
举例
NB-IOT系列模组 AT指令集 V1.0
3.6.4 AT+HTTPSEND 发送数据
设置命令将数据发送到 http 服务器。
NOTE: 需要等到上一次发送对应的接收完成后才能发起下一次发送。
举例：如果第一个发送对应的接收正在进行中，第二个发送命令返回 +HTTP ERROR: SEND FAILED. 测试命令返回支持的值作为返回值。
AT+HTTPSEND
设置命令
AT+HTTPSEND=<httpclientId>,<metho
d>,
<pathlen>,<path>[,<customheaderle
n>,
<customheader>,<contentTypelen>,
<contentType>,<contentlen>,<conte
nt>]
响应
OK
如果发生错误，响应:
+HTTP ERROR: <err>
AT+HTTPSEND=? 响应
+HTTPSEND: (支持列表< httpclientId>),(支持列
表 < method>), (range of supported< pathlen>), “<path>”, (range of supported<
customheaderlen>),“<
customheader>”,( range of supported<
contentTypelen>),“< contentType>”,( range
of supported< contentlen>),“<content>”
OK
最大响应时间 5秒
参数保存模式 不保存
参数
<httpclientId> 整型
+ HTTPCREATE指令返回的客户端序号
<method> 整型; http 模式
0 GET
1 POST
2 PUT
3 DELETE
4 HEAD
<pathlen> 整型
路径长度,范围0-260
<path> 字符串类型
路径
<customheaderlen> 整型
NB-IOT系列模组 AT指令集 V1.0
AT+HTTPSEND=0,0,89, "/data2.5/weather?q=shanghai&
appid=c592e14137c3471fa9627b44f6649db4&mode=xml&units=metric”
OK
+HTTPRESPH
+HTTPRESPH: <clientId>,<responseCode>,<headerlen>,<header>
自定义头部长度,0-255
<customheader> 字符串类型
自定义头部，16进制表示
<contentTypelen> 整型
内容类型的长度,0-64
<contentType> 字符串类型
内容类型
<contentlen> 整型,0-1024
内容长度
<content> 字符串类型
内容，16进制字符表示
举例
3.6.5 +HTTPRESPH 显示收到HTTP 服务器回复的消息头
主动上报消息，显示收到HTTP服务器回复的消息头。 参数
<clientId> 整型
+HTTPCREATE指令返回的客户端序号
<responseCode> 整型
HTTP状态码
<headerlen> 整型
头部长度
<header> 字符串类型
头部
3.6.6 +HTTPRESPC 指示收到服务器消息内容
主动上报消息，显示收到HTTP服务器回复的消息内容
NB-IOT系列模组 AT指令集 V1.0
+HTTPERR
+HTTPERR: <clientId>,<errorcode>,[<rspcode>]
+HTTPRESPC
+HTTPRESPC: <clientId>,<flag>,<contentlength>,<blockcontentlen>,<content>
参数
<clientId> 整型
+HTTPCREATE指令返回的客户端序号
<flag> 整型; 是否还有数据
0 没有更多的数据
1 有更多的数据
<contentlength> 整型
内容长度
<blockcontentlen> 整型
当前块长度
<content> 字符串类型
内容，长度为原始hex数据的2倍
3.6.7 +HTTPERR indicator of error message URC 消息，指示错误状
态
主动上报消息，当错误发生时，显示错误状态
参数
<clientId> 整型
+HTTPCREATE指令返回的客户端序号
<errorcode> 整型
2 URL 解析错误
3 DNS无法解析
4 HTTP 协议 错误
5 HTTP 404 错误, NOT FOUND
6 HTTP 403 错误, REFUSED
7 HTTP xxx 错误，后面跟着具体的response error code
8 连接超时
NB-IOT系列模组 AT指令集 V1.0
9 连接错误
10 连接时遇到fatal error
11 连接已关闭
13 缓冲区溢出错误