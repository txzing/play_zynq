* * *
# memory access and msg access framework


* * *
## udp_logic

basic class open a udp socket and bind to port, then create a thread to recieve

* * *
## cmd

| msg_cmd | sub_msg | function |
|--------|--------|--------|
| 0x10 | NA | read mem of a given addr |
| 0x11 | NA | write mem of a given addr |
| 0x12 | NA | read mem bulk(max 200~ words, fixme) |
| 0x13 | NA | write mem bulk(max 200~ words, fixme) |
| 0x40 (var_ops) | 0x0001(0x01 0x11) | read firmware version (elf ver) |
| 0x20 (TODO) | NA | i2c read |
| 0x21 (TODO) | NA | i2c write |
| 0x22 (TODO) | NA | i2c status query |
| 0x30 (TODO) | NA | spi access |
|        |        |        |
|        |        |        |
|        |        |        |
|        |        |        |
|        |        |        |


* * *
## protocol

###1. 协议说明
####1.1 约定一条消息指一条完整的数据包，以消息代码区分；
####1.2 协议中用到的数据类型列表如下：

缩写 说明

Int8 带符号 8 位整型

Int16 带符号 16 位整型

Int32 带符号 32 位整型

Int64 带符号 64 位整型

uInt8 无符号 8 位整型

uInt16 无符号 16 位整型

uInt32 无符号 32 位整型

uInt64 无符号 64 位整型

Real32 单精度浮点 (32bit)

Real64 双精度浮点 (64bit)

Char 字符型

[n] 数组类型,n字节

协议中的数值型数据如无特别说明，均采用**小端模式**，即低字节在前；


####1.3 消息的组成

#####1.3.1 消息结构

消息长(uInt16)	全部长度，字节为单位

预留段(uInt16)	协议版本、设备 ID等, 先预留16bit

消息代码(uInt8)	即读(0x10)，写(0x11)，块读(0x12)，块写(0x13)等命令，按照特点格式组织消息体

消息体

数据段

校验和(uInt8)	除校验和外消息其它字节的累加取反

#####1.3.2 应答消息

消息长(uInt16)	全部长度，字节为单位

预留段(uInt16)	协议版本、设备 ID等, 先预留16bit

消息代码

消息体

数据段

校验和	

###2. 通信协议的基本功能定义
####2.1 单内存地址读写

#####例如 读 地址0x07200000

PC向服务器发出

|消息长|预留段|消息代码|消息体|校验和|
|-|-|-|-|-|
|0x0E 0x00|0x00 0x00|0x10|0x00 0x00 0x20 0x07 0x00 0x00 0x00 0x00|0xBA|

服务器应答

|消息长|预留段|消息代码|消息体|校验和|
|-|-|-|-|-|
|0x0E 0x00|0x00 0x00|0x10|0x00 0x00 0x20 0x07 0x75 0xba 0x26 0x11|0x54|

#####写 数据0x12345678 到 地址0x07200000

PC向服务器发出

|消息长|预留段|消息代码|消息体|校验和|
|-|-|-|-|-|
|0x0E 0x00|0x00 0x00|0x11|0x00 0x00 0x20 0x07 0x78 0x56 0x34 0x12|0xA5|
	
服务器应答

|消息长|预留段|消息代码|消息体(地址+实际读值)|校验和|
|-|-|-|-|-|
|0x0E 0x00|0x00 0x00|0x11|0x00 0x00 0x20 0x07 0x78 0x56 0x34 0x12|0xA5|

```
2.2 内存块读写

例如 从 地址0x07200000 读长度1k
PC向服务器发出
消息长		预留段		消息代码	消息体							校验和
0x0c 0x00	0x00 0x00	0x12 		0x00 0x00 0x20 0x07 0x00 0x04 0x00 0x00			0x??
服务器应答
消息长		预留段		消息代码	消息体							数据		校验和
0x0c 0x04	0x00 0x00	0x12 		0x00 0x00 0x20 0x07 0x00 0x04 0x00 0x00			......		0x??


向 地址0x07200000 写长度1k
PC向服务器发出
消息长		预留段		消息代码	消息体							数据		校验和
0x0c 0x00	0x00 0x00	0x13 		0x00 0x00 0x20 0x07 0x00 0x04 0x00 0x00			......		0x??
服务器应答
消息长		预留段		消息代码	消息体							校验和
0x0c 0x04	0x00 0x00	0x13 		0x00 0x00 0x20 0x07 0x00 0x04 0x00 0x00			0x??



2.3 i2c总线消息
消息代码
0x20	读i2c
0x21	写i2c
0x22	查询是否占用

例如 写 i2c0: 0xaabbccddee, slave addr:0x69
1. PC向服务器发出查询
消息长		预留段		消息代码	消息体(ch)	校验和
0x07 0x00	0x00 0x00	0x22		0x00		0x??
服务器应答
消息长		预留段		消息代码	消息体(status,ch)	校验和
0x08 0x00	0x00 0x00	0x22		0x00 0x00		0x??
2. PC向服务器发出数据
消息长		预留段		消息代码	消息体(ch,slave_addr,datalen,data)		校验和
0x0e 0x00	0x00 0x00	0x21 		0x00 0x69 0x05 0xaa 0xbb 0xcc 0xdd 0xee 	0x??
服务器应答
消息长		预留段		消息代码	消息体(status,ch,slave_addr,datalen,data)	校验和
0x0f 0x00	0x00 0x00	0x21 		0x00 0x00 0x69 0x05 0xaa 0xbb 0xcc 0xdd 0xee 	0x??

例如 读 i2c0: slave addr:0x69
1. PC向服务器发出查询
消息长		预留段		消息代码	消息体(ch)	校验和
0x07 0x00	0x00 0x00	0x22		0x00		0x??
服务器应答
消息长		预留段		消息代码	消息体(status,ch)	校验和
0x08 0x00	0x00 0x00	0x22		0x00 0x00		0x??
2. PC向服务器发出数据
消息长		预留段		消息代码	消息体(ch,slave_addr,datalen,data)		校验和
0x0e 0x00	0x00 0x00	0x20 		0x00 0x69 0x05 0x00 0x00 0x00 0x00 0x00 	0x??
服务器应答
消息长		预留段		消息代码	消息体(status,ch,slave_addr,datalen,data)	校验和
0x0f 0x00	0x00 0x00	0x20		0x00 0x00 0x69 0x05 0xaa 0xbb 0xcc 0xdd 0xee 	0x??


例如 读 i2c0:从8bit subaddr 0xaa读取长度2, slave addr:0x69
PC向服务器发出数据
消息长		预留段		消息代码	消息体[len salvaddr mode subaddr]		校验和
0x09 0x00	0x00 0x00	0x20 		0x02 0x69 0x00 0xaa				0x??
服务器应答
消息长		预留段		消息代码	消息体						校验和
0x0a 0x00	0x00 0x00	0x20 		0x02 0x69 0x00 0xaa				0x??
PC向服务器发出查询
消息长		预留段		消息代码	校验和
0x07 0x00	0x00 0x00	0x24		0x??
服务器应答
消息长		预留段		消息代码	消息体(0|1)	校验和
0x07 0x00	0x00 0x00	0x24		0x00		0x??
PC向服务器发出获取数据命令
消息长		预留段		消息代码	校验和
0x07 0x00	0x00 0x00	0x25		0x??
服务器应答
消息长		预留段		消息代码	数据		校验和
0x09 0x00	0x00 0x00	0x25		0x??		0x??



2.3 spi总线消息
消息代码
0x30	spi_send_recv


例如 读节写 spi1: cs 0x00
PC向服务器发出数据
消息长		预留段		消息代码	消息体(ch,cs,datalen,data)		校验和
0x0e 0x00	0x00 0x00	0x30 		0x01 0x00 0x02 0x00 0x03		0x??
服务器应答
消息长		预留段		消息代码	消息体(status,ch,cs,datalen,data)	校验和
0x0e 0x00	0x00 0x00	0x30 		0x00 0x01 0x00 0x02 0x00 0x03		0x??



2.4 读取全局变量，如软件版本等
消息长		预留段		消息代码	消息体(index)				校验和
0x0e 0x00	0x00 0x00	0x40		0x01 0x00				0x??
服务器应答
消息长		预留段		消息代码	消息体(index,data)			校验和
0x0e 0x00	0x00 0x00	0x30 		0x01 0x00 0x00 0x02 0x00 0x03		0x??
```

