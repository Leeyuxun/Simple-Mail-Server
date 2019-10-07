/*SMTP服务器*/

#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#pragma comment(lib,"wsock32.lib")
#include <stdlib.h>
#include <errno.h>
#include <time.h> 

#define BUFSIZE 4096//缓冲区大小
#define PORT 25//端口号 
//标示符	
#define r1  "220 Mysmtp Service Ready\r\n"
#define r2  "221 Bye\r\n"
#define r3  "250 AUTH LOGIN PLAINOK\r\n250-AUTH=LOGIN PLAIN\r\n250-STARTTLS\r\n250 8BITMIME\r\n"
#define r4  "250 OK\r\n"
#define r5  "354 Start mail input: end with <CRLF>.<CRLF>\r\n"
#define r6  "421 Service unavailable, closing tranmission channel\r\n"
#define r7  "450 Requested mail action not taken: mail box unavailable\r\n"
#define r8  "550 Requested mail action not taken: mail box unavailable\r\n"
#define r9  "503 Bad sequence of commands\r\n"
#define r10 "221 Service closing transmission channel\r\n"
#define r11 "251 User not local; will forward to\r\n"
#define r12 "451 Requested action aborted: local error in processing\r\n"
#define r13 "452 Requested action not taken: insufficient system storage\r\n"
#define r14 "500 Syntax error, command unrecognized\r\n"
#define r15 "501 Syntax error in parameters or arguments\r\n"
#define r16 "502 Command not implemented\r\n"
#define r17 "504 Command parameter not implemented\r\n"
#define r18 "551 User not local; please try\r\n"
#define r19 "552 Requested mail action aborted: exceeded storage allocation\r\n"
#define r20 "553 Requested action not taken: mailbox name not allowed\r\n"
#define r21 "554 Transaction failed\r\n"
#define r22 "DATA\r\n"
#define r23 "QUIT\r\n"
#define r24 "\r\n.\r\n"
#define r25 "AUTH LOGIN\r\n"
#define r26 "334 VXNlcm5hbWU6\r\n"
#define r27 "334 UGFzc3dvcmQ6\r\n"
#define r28 "235 Authentication successful\r\n"
#define r29 "503 Can't receive EHLO\r\n"
#define r30 "503 Can't receive AUTH LOGIN\r\n"
#define r31 "503 Can't receive DATA\r\n"
#define r32 "503 Can't receive ' . '\r\n"

//全局变量 
char ehlo[BUFSIZE];	//存客户端ehlo 
char mailFrom[BUFSIZE];	//存客户端邮箱 
char rcptTo[5][BUFSIZE];	//存目的邮箱
char clientIP[BUFSIZE];	//客户端服务器ip地址 
char ip[BUFSIZE];			//存每次用户输入的IP地址 
char data[BUFSIZE];		//存Data
char imf[BUFSIZE * 10];	//存邮件标准格式
char recvData[BUFSIZE];	//存接收数据
char rcptJudge[BUFSIZE];	//用于判断目的邮箱数量 
int i = 0;	//记录目的邮箱数量 
int j = 0; //记录目标邮箱数量 
int error;//判断调用客户端函数是否成功 
int now[6];	//记录时间
char fname[256] = {0};//记录时间 
FILE *fp;//打开文件 
char nativeIP[BUFSIZE];//存本机IP
char temp[3];//存状态码 
char username[BUFSIZE];//存登陆用户名
char password[BUFSIZE];//存登陆密码 


//子函数 
int server(); //调用服务器端函数
int client(); //调用客户端函数
int validEmail(char*);		//收发件邮箱地址(addr)合法性监测
char* translateIP(char* dominname);	//将源邮箱服务器地址转换为ip地址 
char* getIP(); 	//获取客户端ip地址 
int time1();		//时间戳函数 
void Error();	//判断错误类型 

int main()
{
	int Client=0;
	int Server=0;
	//调用服务器端函数 
	Server = server();
	if(Server==-1)
	{
		return -1;
	}
	else
	{
		//调用客户端函数 
		
		Client = client();
		if(Client==-1)
		{
			Error();
			return -1;
		}
		else
		{
			printf("\n\n——————————————————————————————\n"); 
			fp = fopen(fname,"a");
			fprintf(fp,"\n\n——————————————————————————————\n"); 
			time1();
			fprintf(fp, "end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
			printf("end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
			return 0; 
		}
	}
}

//服务器端函数 
int server()
{
	int             Ret;
	WSADATA         wsaData;
	SOCKET          ListeningSocket;	//监听 
	SOCKET          socketConnection;	//连接 
	SOCKADDR_IN     ServerAddr;	//服务器地址 
	SOCKADDR_IN     ClientAddr;	//客户端地址 
	int             ClientAddrLen = sizeof(ClientAddr);	//客户端地址长度 
	int             flag = 1;	//判断是否可以连接

	//初始化WSAStartup
	Ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Ret != 0)//如果调用成功，WSAStartup函数返回0 
	{
		printf("WSASTARTUP_ERROR: %d\n", Ret);
		WSACleanup(); //释放分配资源
		return -1;
	}

	//创建一个套接字来监听客户机连接
	ListeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListeningSocket == INVALID_SOCKET)
	{
		printf("SOCKET_ERROR: %d\n", INVALID_SOCKET);
		WSACleanup(); //释放分配资源
		return -1;
	}

	//填充SOCKADDR_IN结构
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(PORT); //将端口变量从主机字节顺序转换位网络字节顺序
	ServerAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//只接本机发送的tcp连接


	//使用bind将这个地址信息和套接字绑定起来
	if (bind(ListeningSocket, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR)
	{
		printf("BIND_ERROR: %d\n", SOCKET_ERROR);
		WSACleanup(); //释放分配资源
		return -1;
	}

	//监听客户机连接。这里使用5个backlog
	if (listen(ListeningSocket, 5) == SOCKET_ERROR)
	{
		printf("LISTEN_ERROR: %d\n", SOCKET_ERROR);
		WSACleanup(); //释放分配资源
		return -1;
	}

	//连接到达时，接受连接
	printf("正在接受连接......");
	socketConnection = accept(ListeningSocket, (SOCKADDR *)&ClientAddr, &ClientAddrLen); 
	if (socketConnection == INVALID_SOCKET)
	{
		printf("\nACCPET_ERROR: %d\n", INVALID_SOCKET);
		closesocket(ListeningSocket);
		WSACleanup(); //释放分配资源
		return -1;
	}
	printf("\n检测到一个连接:\n\n"); 
	time1();
	printf("Time:%04d-%02d-%02d_%02d:%02d:%02d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
	printf("——————————————————————————————\n");
	strcpy(nativeIP, getIP());//获取客户端ip地址
	printf("客户端IP地址：%s  端口号:%d\n", nativeIP,ntohs(ClientAddr.sin_port));

	//监听 
	while (flag != 0)//等待客户请求 
	{
		time1();
		sprintf(fname, "E:/mail/Log-%04d-%02d-%02d_%02d-%02d-%02d.txt", now[0], now[1], now[2], now[3], now[4], now[5]);
		fp = fopen(fname,"a");
		time1();
		fprintf(fp, "start time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
		fprintf(fp,"——————————————————————————————\n"); 
		fprintf(fp,"客户端IP地址：%s  端口号:%d\n\n", nativeIP,ntohs(ClientAddr.sin_port));
		memset(rcptTo, 0, sizeof(rcptTo));	
		
		//向已经连接的套接字sockConnection发送连接建立信息
		{
			send(socketConnection, r1, strlen(r1), 0);  //发送连接建立信息：220
			printf("发送状态码：220 Mysmtp Service Ready.\n");
			fprintf(fp, "%s", r1);//将连接成功保存到文件里 
		}
		
		//接收ehlo 
		{
			recv(socketConnection, recvData, sizeof(recvData), 0); //接收数据 ehlo
			if(strncmp(recvData,"EHLO",4) != 0)//判断是否接收到EHLO 
			{
				printf("接收不到EHLO.");
				fprintf(fp,"接收不到EHLO，%s",r29); 
				send(socketConnection, r29, strlen(r29), 0);//发送503 
				WSACleanup(); //释放分配资源
				return -1;
			}
			printf("接收EHLO：%s",recvData); 
			fprintf(fp, "%s\n", recvData); //将ehlo数据写入文件
			memcpy(ehlo, recvData, sizeof(recvData));
			memset(recvData, 0, sizeof(recvData)); //将recvData前4096个字节用字符'0'替换
			send(socketConnection, r3, strlen(r3), 0); // 发送接收成功数据：250 AUTH LOGIN PLAINOK|250-AUTH=LOGIN PLAIN|250-STARTTLS|250 8BITMIME\r\n
			printf("发送状态码：250 AUTH LOGIN PLAINOK|250-AUTH=LOGIN PLAIN\r\n");
			fprintf(fp, "%s", r3);//将250写入文件 
		}
		
		//接收AUTH LOGIN
		{
			recv(socketConnection, recvData, sizeof(recvData), 0); 
			//判断接收是否正确 
			if(strncmp(recvData,"AUTH LOGIN",10) != 0)
			{
				printf("接收不到AUTH LOGIN.");
				fprintf(fp,"接收不到AUTH LOGIN，%s",r30); 
				send(socketConnection, r30, strlen(r30), 0);//发送550 time1(); 
				fprintf(fp, "end time: %d-%d-%d %d:%d:%d\n\n\n\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);  //关闭文件指针
				WSACleanup(); //释放分配资源
				return -1;
			} 
			printf("接收AUTH LOGIN\n");	
			fprintf(fp, "%s\n", recvData);	//将AUTH LOGIN四个字写进文件中 
			memset(recvData, 0, sizeof(recvData));	//将recvData前4096个字节用字符'0'替换
			send(socketConnection, r26, strlen(r26), 0);//发送334 
			printf("发送状态码334\n");
			fprintf(fp, "%s\n", r26);	
		} 
		
		//接收用户名
		{
			recv(socketConnection, recvData, sizeof(recvData), 0); 
			printf("接收用户名\n");	
			memcpy(username, recvData, sizeof(recvData));
			fprintf(fp, "%s\n", recvData);	
			memset(recvData, 0, sizeof(recvData));	//将recvData前4096个字节用字符'0'替换
			send(socketConnection, r27, strlen(r27), 0);//发送334 
			printf("发送状态码334\n");
			fprintf(fp, "%s\n", r27);	
		} 
		
		//接收密码 
		{
			recv(socketConnection, recvData, sizeof(recvData), 0);  
			printf("接收密码\n");	
			memcpy(password, recvData, sizeof(recvData));
			fprintf(fp, "%s\n", recvData);	
			memset(recvData, 0, sizeof(recvData));	//将recvData前4096个字节用字符'0'替换
			send(socketConnection, r28, strlen(r28), 0);//发送235 Authentication successful
			printf("发送状态码235\n");
			fprintf(fp, "%s\n", r28);	
		} 
		
		//接收源邮箱地址 
		{
			recv(socketConnection, recvData, sizeof(recvData), 0); //接收源邮箱：MAIL FROM:<...>
			//判断是否接收的是邮箱地址
			if(strncmp(recvData,"MAIL FROM",9) != 0)
			{
				printf("接收不到mail from.");
				fprintf(fp,"接收不到mail from，%s",r8); 
				send(socketConnection, r8, strlen(r8), 0);//发送550 
				WSACleanup(); //释放分配资源
				return -1;
			} 
			//判断源邮箱是否正确 
			if (validEmail(recvData)==-1)//发送错误代码550
			{
				send(socketConnection, r8, strlen(r8), 0);
				printf("接收客户端邮箱ERROR:%s",r8);
				fprintf(fp,"接收客户端邮箱ERROR:%s",r8); 
				closesocket(socketConnection);
				fclose(fp);
				WSACleanup(); //释放分配资源
				return -1;
			}
			memcpy(mailFrom, recvData, sizeof(recvData));//将源邮箱地址复制到mailFrom里面 
			printf("接收客户端邮箱：%s", recvData);	//打印源邮箱
			fprintf(fp, "%s", recvData);	//将源邮箱地址写入文件
			strcpy(ip, translateIP(mailFrom));//将源邮箱服务器地址转换为ip地址 
			printf("客户端邮箱服务器IP地址: %s  端口号：%d\n", ip , PORT);
			fprintf(fp,"客户端邮箱服务器IP地址: %s  端口号：%d\n\n", ip , PORT);//存入文件 
			memcpy(clientIP, ip, sizeof(ip));
			memset(recvData, 0, sizeof(recvData));	//将recvData前4096个字节用字符'0'替换
			send(socketConnection, r4, strlen(r4), 0); //发送接收成功代码：250 OK
			printf("发送状态码：250 OK \n");
			fprintf(fp, "%s", r4);	//将250 OK写入文件 
		}
		
		//接收目的邮箱地址 
		{
			recv(socketConnection, recvData, sizeof(recvData), 0);//接收邮件rcpt
			i = 0;//记录目的邮箱个数 
			strncpy(rcptJudge, recvData, 4);
			while ((strcmp(rcptJudge, "RCPT") == 0) && (i < 5))
			{
				//判断是否接收的是邮箱地址
				if(strncmp(recvData,"RCPT TO",7) != 0)
				{
					printf("接收不到rcpt to.");
					fprintf(fp,"接收不到rcpt to，%s",r8); 
					send(socketConnection, r8, strlen(r8), 0);//发送550 
					WSACleanup(); //释放分配资源
					return -1;
				} 
				if (validEmail(recvData)==-1)//发送550
				{
					send(socketConnection, r8, strlen(r8), 0);
					closesocket(socketConnection);
					printf("收目的邮箱ERROR:%s",r8);
					fprintf(fp,"接收目的邮箱ERROR:%s",r8); 
					fclose(fp);
					WSACleanup(); //释放分配资源
					return -1;
				}
				memcpy(rcptTo[i], recvData, sizeof(recvData));
				printf("接收目的邮箱%d: %s", i + 1, recvData);
				fprintf(fp, "%s\n", recvData);
				memset(recvData, 0, sizeof(recvData));
				memset(ip, 0, sizeof(ip));
				memset(rcptJudge, 0, sizeof(rcptJudge));
				send(socketConnection, r4, strlen(r4), 0); //发送成功250 OK状态吗 
				printf("发送状态码：250 OK \n");
				fprintf(fp, "%s", r4);//将250 OK存入文件 
				recv(socketConnection, recvData, sizeof(recvData), 0); //接收 RCPT TO:<....>
				strncpy(rcptJudge, recvData, 4);
				i++;
			}
		}
	
		//判断是否能够接收到 “DATA ”
		if(strcmp(recvData, r22) != 0)
		{
			printf("接收不到DATA\n");
			fprintf(fp,"接收不到DATA %s",r31);
			send(socketConnection, r31, strlen(r31), 0); //发送错误原因
			time1(); 
			fprintf(fp, "end time: %d-%d-%d %d:%d:%d\n\n\n\n", now[0], now[1], now[2], now[3], now[4], now[5]);
			fclose(fp);  //关闭文件指针
			WSACleanup(); //释放分配资源
			return -1;
		}
		printf("接收'DATA'\n");	
		fprintf(fp, "%s\n", recvData);		//将DATA四个字写进文件中 
		memset(recvData, 0, sizeof(recvData));	//将recvData前4096个字节用字符'0'替换

		//接收邮件内容
		{
			//发送邮件接收代码
			send(socketConnection, r5, strlen(r5), 0);//发送邮件接收代码：354 Start mail input;end with <CR><LF>.<CR><LF>\r\n
			printf("发送状态码：354 Start mail input;end with <CR><LF>.<CR><LF>\n");
			fprintf(fp, "%s", r5);//将354存入文件 
			
			//接收data 
			{
				recv(socketConnection, recvData, sizeof(recvData), 0); //接收邮件DATA fragment, ...bytes
				memcpy(data, recvData, sizeof(recvData));	//将接收到的邮件data拷贝到data数组中，方便下面的使用 
				printf("接收DATA fragment, ...bytes\n");
				fprintf(fp, "接收DATA fragment, ...bytes\n\n%s\n", recvData);	//将接收到的邮件内容存储到文件中 
				memset(recvData, 0, sizeof(recvData));	//将recvData前4096个字节用字符'0'替换
			}
		
			//接收IMF 
			{
				recv(socketConnection, recvData, sizeof(recvData), 0); //接收IMF
				memcpy(imf, recvData, sizeof(recvData));//将接收到的邮件IMF内容拷贝到imf数组中，方便下面的使用
				printf("接收imf\n");
				fprintf(fp, "接收imf\n\n%s\n", recvData);//将接收到的IMF内容存储到文件中
				memset(recvData, 0, sizeof(recvData));	//将recvData前4096个字节用字符'0'替换
			}

			//接收邮件末尾的 "." 
			{
				recv(socketConnection, recvData, sizeof(recvData), 0); //接收邮件末尾的 "." 
				//判断是否能够接收到 “. ”
				if(strcmp(recvData, " ") == 0)
				{
					printf("接收不到'.'\n");
					fprintf(fp,"接收不到'.' %s",r32);
					send(socketConnection, r32, strlen(r32), 0); //发送错误原因time1();
					time1(); 
					fprintf(fp, "end time: %d-%d-%d %d:%d:%d\n\n\n\n", now[0], now[1], now[2], now[3], now[4], now[5]);
					fclose(fp);  //关闭文件指针
					WSACleanup(); //释放分配资源
					return -1;
				}
				printf("接收'.'\n");
				fprintf(fp, "接收%s\n", recvData);	//将接收到的"."存储到文件中
				memset(recvData, 0, sizeof(recvData));	//将recvData前4096个字节用字符'0'替换
				send(socketConnection, r4, strlen(r4), 0); //发送接收成功代码：250 OK
				printf("发送状态码: 250 OK \n");
				fprintf(fp, "%s", r4);//将250 OK存入文件 
			}
		
			//接收断开请求 ，回复221 bye
			{
				recv(socketConnection, recvData, sizeof(recvData), 0); //接收邮件末尾的 "QIUT"
				fprintf(fp, "%s\n", recvData);	//将接收到的"QUIT"存储到文件中
				printf("接收状态码：QUIT \n");
				memset(recvData, 0, sizeof(recvData));	//将recvData前4096个字节用字符'0'替换
				send(socketConnection, r2, strlen(r2), 0); //发送TCP连接释放代码：221 bye
				fprintf(fp, "%s\n", r2);//将221 bye 存入文件 
				printf("发送状态码：221 bye \n");		
			} 
			
			printf("——————————————————————————————\n");
			fprintf(fp,"——————————————————————————————\n");
			//获得邮箱长度 
			{
				printf("\n邮件长度：%d字节\n\n",strlen(imf)) ;
				fprintf(fp,"\n邮件长度：%d字节\n\n",strlen(imf)) ; 
			}	
			
			printf("——————————————————————————————\n\n"); 
			printf("\n开始发送邮件\n\n");
			fprintf(fp,"——————————————————————————————\n\n"); 
			fprintf(fp,"\n开始发送邮件\n");
			
			fclose(fp);  //暂时关闭文件 	
		}

			
		flag = 0;
	}
	closesocket(socketConnection); //关闭套接字
	WSACleanup(); //释放分配资源
	return 0; 
}


//客户端函数
int client()
{
	
	int             Ret1;
	WSADATA         wsaData1;
	SOCKET          socketclient;	
	SOCKET          socketnative;
	SOCKADDR_IN     nativeAddr;	//本地ip地址 
	SOCKADDR_IN     clientAddr1;//客户端ip地址 
	
	//初始化WSAStartup
	Ret1 = WSAStartup(MAKEWORD(2, 2), &wsaData1);
	if (Ret1 != 0)//如果调用成功，WSAStartup函数返回0 
	{
		printf("WSASTARTUP_ERROR: %d\n", Ret1);
		WSACleanup(); //释放分配资源
		return -1;
	}
	
	
	while (j < i)
	{
		//建立连接 
		{
			socketclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			socketnative = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			
			//获取真正的客户端邮箱服务器地址 
			clientAddr1.sin_family = AF_INET;
			clientAddr1.sin_port = htons(PORT);
			clientAddr1.sin_addr.s_addr = inet_addr(clientIP);
			connect(socketclient, (SOCKADDR*)&clientAddr1, sizeof(SOCKADDR));
			
			//本机IP地址和端口 
			nativeAddr.sin_family = AF_INET;
			nativeAddr.sin_port = htons(PORT);
			nativeAddr.sin_addr.s_addr = inet_addr(nativeIP);
			connect(socketnative, (SOCKADDR*)&nativeAddr, sizeof(SOCKADDR));
			
		}

		//打开日志文件
		{
			fp = fopen(fname, "a");
			printf("——————————————————————————————"); 
			printf("\n目的邮箱%d\n连接成功\n",j+1);
			printf("本地IP地址：%s  SMTP发送端口:%d", inet_ntoa(nativeAddr.sin_addr),ntohs(nativeAddr.sin_port));
			fprintf(fp,"目的邮箱%d\n连接成功\n",j+1);
			fprintf(fp,"本地IP地址：%s  SMTP发送端口:%d", inet_ntoa(nativeAddr.sin_addr),ntohs(nativeAddr.sin_port));
			time1();
			fprintf(fp, "\nsend start time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
			printf("\nsend start time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
			printf("——————————————————————————————\n");
			fprintf(fp,"——————————————————————————————\n"); 
		}
		
		//接收220状态码并发送ehlo状态码 
		{
			recv(socketclient, recvData, sizeof(recvData), 0);  //接收状态吗220 OK
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "220") != 0) 
			{
				printf("接收状态码：%s, NO 220, 发送失败\n",temp);
				fprintf(fp,"接收状态码：%s, NO 220, 发送失败\n",temp);
				fclose(fp);
				WSACleanup(); //释放分配资源
				return -1;
			} 
			else
			{
				printf("接收状态码：%s",recvData);
				fprintf(fp, "接收状态码：%s", recvData);		//将状态码220 OK存入文件 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, ehlo, strlen(ehlo), 0);
				printf( "发送状态码：%s", ehlo);		
				fprintf(fp, "发送状态码：%s", ehlo);		//将发送的EHLO状态码存入文件
			}
		} 
		
		//接收ehlo返回状态码并发送AUTH LOGIN状态码
		{
			recv(socketclient, recvData, sizeof(recvData), 0);       //接收状态码
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "250") != 0) 
			{
				printf("接收状态码：%s, NO 250, 发送失败\n",temp);
				fprintf(fp,"接收状态码：%s, NO 250, 发送失败\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //释放分配资源
				return -1;
			} 
			else
			{
				printf("接收状态码：%s\n",temp);
				fprintf(fp, "接收状态码：%s\n", temp);		//将状态码250 OK存入文件 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, r25, strlen(r25), 0);   //发送:AUTH LOGIN
				printf("发送状态码：%s", r25);
				fprintf(fp, "发送状态码：%s", r25);		//将发送的AUTH LOGIN状态码存入文件
			}
		} 
		
		//接收状态码334并发送用户名
		{
			recv(socketclient, recvData, sizeof(recvData), 0);       //接收状态码 ——不一定是334 
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "334") != 0) 
			{
				printf("接收状态码：%s, NO 334, 发送失败\n",temp);
				fprintf(fp,"接收状态码：%s, NO 334, 发送失败\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //释放分配资源
				return -1;
			}
			else
			{
				printf("接收状态码：%s\n",temp);
				fprintf(fp, "接收状态码：%s\n", temp);		//将状态码250 OK存入文件 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, username, strlen(username), 0);   ////发送用户名
				printf("发送用户名：XXXXXXXX\n");
				fprintf(fp, "发送用户名：%s", username);		//将发送的用户名存入文件
			}
		}
		
		//接收状态码334并发送密码 
		{
			recv(socketclient, recvData, sizeof(recvData), 0);       //接收状态码 ——不一定是334 
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "334") != 0) 
			{
				printf("接收状态码：%s, NO 334, 发送失败\n",temp);
				fprintf(fp,"接收状态码：%s, NO 334, 发送失败\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //释放分配资源
				return -1;
			}
			else
			{
				printf("接收状态码：%s\n",temp);
				fprintf(fp, "接收状态码：%s\n", temp);		//将状态码250 OK存入文件 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, password, strlen(password), 0);   ////发送密码 
				printf("发送密码：********\n");
				fprintf(fp, "发送密码：%s", password);		//将发送的密码存入文件
			}
		}
		
		//接收状态码235并发送发件客户端邮箱名称 
		{
			recv(socketclient, recvData, sizeof(recvData), 0);         // 接收状态码,235 Authentication successful表示成功 
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "235") != 0) 
			{
				printf("接收状态码：%s, NO 235, 发送失败\n",temp);
				fprintf(fp,"接收状态码：%s, NO 235, 发送失败\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //释放分配资源
				return -1;
			}
			else
			{
				printf("接收状态码：%s\n",temp);
				fprintf(fp, "接收状态码：%s\n", temp);		//将状态码235存入文件 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, mailFrom, strlen(mailFrom), 0);     //发送客户端邮箱名称 
				printf("发送客户端邮箱：%s",mailFrom);
				fprintf(fp, "发送客户端邮箱：%s", mailFrom);			//存入客户端邮箱名称 
			}
		}
	
		//接收状态码250 OK，并发送目的邮箱名称
		{	
			recv(socketclient, recvData, sizeof(recvData), 0);             //接收状态码：250 mail ok
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "250") != 0) 
			{
				printf("接收状态码：%s, NO 250, 发送失败\n",temp);
				fprintf(fp,"接收状态码：%s, NO 250, 发送失败\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //释放分配资源
				return -1;
			}
			else
			{
				printf("接收状态码：%s\n",temp);
				fprintf(fp, "接收状态码：%s\n", temp);		//将状态码250存入文件 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, rcptTo[j], strlen(rcptTo[j]), 0);     //发送目的邮箱名称 
				printf("发送目的邮箱：%s", rcptTo[j]); 		
				fprintf(fp, "发送目的邮箱：%s", rcptTo[j]); 							//将目的邮箱存入文件 
			} 
		} 
		
		//接收状态码250 OK 并发送DATA
		{	
			recv(socketclient, recvData, sizeof(recvData), 0);         //接收状态码：250 mail ok
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "250") != 0) 
			{
				printf("接收状态码：%s, NO 250, 发送失败\n",temp);
				fprintf(fp,"接收状态码：%s, NO 250, 发送失败\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //释放分配资源
				return -1;
			}
			else
			{
				printf("接收状态码：%s\n",temp);
				fprintf(fp, "接收状态码：%s\n", temp);		//将状态码250存入文件 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, r22, strlen(r22), 0);     //发送DATA
				printf("发送:DATA\n");
				fprintf(fp,"发送:DATA\n");
			} 
		} 
		
		//接收状态码354 并发送data,imf，“. ” 
		{	
			recv(socketclient, recvData, sizeof(recvData), 0);        // 接收状态码：354 end data with <CR><LF>.<CR><LF>
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "354") != 0) 
			{
				printf("接收状态码：%s, NO 354, 发送失败\n",temp);
				fprintf(fp,"接收状态码：%s, NO 354, 发送失败\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //释放分配资源
				return -1;
			}
			else
			{
				printf("接收状态码：%s\n",temp);
				fprintf(fp, "接收状态码：%s\n", temp);		//将状态码354存入文件 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				
				send(socketclient, data, strlen(data), 0);		//发送data 
				printf("发送:data\n");
				fprintf(fp, "%s\n", data);	
		
				send(socketclient, imf, strlen(imf), 0);		//发送imf
				printf("发送:imf\n");
				fprintf(fp, "%s\n", imf);
		
				send(socketclient, r24, strlen(r24), 0);     //发送“ . ” 
				printf("发送:' . '\n");
				fprintf(fp, "%s", r24);
			} 
		} 
		
		//接收状态码 250 OK 并发送QUIT 
		{
			recv(socketclient, recvData, sizeof(recvData), 0);         //接收状态码：250
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "250") != 0) 
			{
				printf("接收状态码：%s, NO 250, 发送失败\n",temp);
				fprintf(fp,"接收状态码：%s, NO 250, 发送失败\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //释放分配资源
				return -1;
			}
			else
			{
				printf("接收状态码：%s\n",temp);
				fprintf(fp, "接收状态码：%s\n", temp);		//将状态码250存入文件 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, r23, strlen(r23),0);     //发送QUIT
				printf("发送:QUIT\n");
				fprintf(fp,"发送:QUIT\n");
			} 
		}
		
		//接收状态码221 bye
		{
			recv(socketclient, recvData, sizeof(recvData), 0);         //接收状态码：221 bye 
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "221") != 0) 
			{
				printf("接收状态码：%s, NO 221, 发送失败\n",temp);
				fprintf(fp,"接收状态码：%s, NO 221, 发送失败\n",temp);
				fclose(fp);
				WSACleanup(); //释放分配资源
				return -1;
			}
			else
			{
				printf("接收状态码：%s\n",temp);
				printf("邮件发送成功.\n");
				printf("——————————————————————————————\n"); 	
				fprintf(fp, "接收状态码：%s\n邮件发送成功\n", temp);		//将状态码221存入文件
				fprintf(fp,"——————————————————————————————\n"); 	
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n\n\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				printf("send end time: %d-%d-%d %d:%d:%d\n\n\n", now[0], now[1], now[2], now[3], now[4], now[5]);
			}
		}
		
		j++;
		fclose(fp);
		closesocket(socketclient);
		closesocket(socketnative);
	}
	WSACleanup();
	return 0; 
}

//收发件邮箱地址合法性监测
int validEmail(char *addr)
{
	int colonAddr = 0; //冒号 
	int atAddr = 0;	//@
	int pointAddr = 0;	//点号 
	int bracketAddr = 0;	//>
	int error1 = 0;
	unsigned int a = 0;

	//找出“：”的位置 
	for (a = 0; a < strlen(addr); a++)
	{
		if ((colonAddr == 0) && (addr[a] != ':'))
		{
			continue;
		}
		else
		{
			colonAddr = a;
			break;
		}
	}

	//找出“@ ”的位置
	for (a = 0; a < strlen(addr); a++)
	{
		if ((atAddr == 0) && (addr[a] != '@'))
		{
			continue;
		}
		else
		{
			atAddr = a;
			break;
		}
	}

	//找出“> ”的位置 
	for (a = 0; a < strlen(addr); a++)
	{
		if (addr[a] == '>')
		{
			bracketAddr = a;
		}
	}
	//确定两端字符 
	for (a = 0; a < strlen(addr); a++)
	{
		if ((addr[colonAddr + 3] != '.') && (addr[bracketAddr - 1] != '.'))
		{
			continue;
		}
		else
		{
			error1 = -1;
			return error1;
		}
	}
	//在“：”后与“@ ”前确定白名单 
	for (a = (unsigned int)(colonAddr + 3); a < (unsigned int)atAddr; a++)
	{
		if (((addr[a] < 58) && (addr[a] > 47)) || ((addr[a] < 91) && (addr[a] > 64)) || ((addr[a] < 123) && (addr[a] > 96)) || (addr[a] == 46))
		{
			if (addr[a] == 46)
			{
				if (a == (pointAddr + 1) || (a == (atAddr - 1)))
				{
					error1 = -1;
					return error1;
				}
				else
				{
					pointAddr = a;
					continue;
				}
			}
		}
	}

	//在“@ ”后面确定白名单
	for (a = (atAddr + 1); a < (strlen(addr) - 1); a++)
	{
		if (((addr[a] < 58) && (addr[a] > 47)) || ((addr[a] < 91) && (addr[a] > 64)) || ((addr[a] < 123) && (addr[a] > 96)) || (addr[a] == 46))
		{
			if (addr[a] == 46)
			{
				if ((a == (atAddr + 1)) || (a == (pointAddr + 1)))
				{
					error1 = -1;
					return error1;
				}
				else
				{
					pointAddr = a;
					continue;
				}
			}
		}
	}
	for (a = 0; a < strlen(addr); a++)
	{
		if (pointAddr < atAddr)
		{
			error1 = -1;
			return error1;
		}
	}
	return error1;
}

//获取客户端ip地址
char* getIP()
{
	char* hostIP; 
	char hostName[256];
	struct hostent *hostEntry;
	hostEntry = gethostbyname(hostName);

	if (hostEntry != NULL && hostEntry->h_addr_list[2] != NULL)
	{
		 hostIP = inet_ntoa(*(struct in_addr*)hostEntry->h_addr_list[2]);
	}
	return hostIP;
}

//将源邮箱服务器地址转换为ip地址 
char* translateIP(char*mail)
{
	WSADATA wsa;
	char ip[100] = { "smtp." };
	char* IP;
	int atAddr = 0;	//@
	int bracketAddr = 0;	//>
	unsigned int x = 0;
	int y = 5;
	struct hostent *hostEntry;
	struct in_addr **addr_list;

	for (x = 0; x < strlen(mail); x++)
	{
		if (mail[x] == '@')
		{
			atAddr = x;
		}
		if (mail[x] == '>')
		{
			bracketAddr = x;
		}
	}
	for (x = (unsigned int)(atAddr + 1); x < (unsigned int)bracketAddr; x++)
	{
		ip[y] = mail[x];
		y++;
	}
	
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(-1);
	}
	if ((hostEntry = gethostbyname(ip)) == NULL)
	{
		printf("gethostbyname failed : %d", WSAGetLastError());
		exit(-1);
	}
	//将h_addr_list转换为in_addr，h_addr_list只有长格式的ip地址
	addr_list = (struct in_addr **)hostEntry->h_addr_list;
	for (x = 0; addr_list[x] != NULL; x++)
	{
		IP = inet_ntoa(*addr_list[x]);
	}
	return IP;
}

//时间戳函数
int time1()
{   
	time_t t;
	struct tm * lt;
	time(&t);//获取时间戳。 
	lt = localtime(&t);//转为时间结构。
	now[0] = lt->tm_year + 1900;
	now[1] = lt->tm_mon + 1;
	now[2] = lt->tm_mday;
	now[3] = lt->tm_hour;
	now[4] = lt->tm_min;
	now[5] = lt->tm_sec;
} 

//判断错误类型函数 
void  Error()
{
	fp = fopen(fname,"a");//打开文件 
	if((strcmp(temp, "250") == 0)||(strcmp(temp, "334") == 0)||(strcmp(temp, "220") == 0)||(strcmp(temp, "235") == 0))
	{
		printf("客户端邮箱服务器接收数据顺序错误，请重新发送"); 
		fprintf(fp, "接收真实服务器状态码%s，接收数据顺序错误，发送失败\n", temp);//将temp存入文件
	}
	else if((strcmp(temp, "221") == 0))
	{
		printf("接收真实服务器状态码%s\n", r10); 
		fprintf(fp, "接收真实服务器状态码%s\n", r10);//将错误代码存入文件
	}
	else if((strcmp(temp, "251") == 0))
	{
		printf("接收真实服务器状态码%s\n", r11); 
		fprintf(fp, "接收真实服务器状态码%s\n", r11);//将错误代码存入文件
	}
	else if((strcmp(temp, "421") == 0))
	{
		printf("接收真实服务器状态码%s\n", r6); 
		fprintf(fp, "接收真实服务器状态码%s\n", r6);//将错误代码存入文件
	}
	else if((strcmp(temp, "450") == 0))
	{
		printf("接收真实服务器状态码%s\n", r7); 
		fprintf(fp, "接收真实服务器状态码%s\n", r7);//将错误代码存入文件
	}
	else if((strcmp(temp, "451") == 0))
	{
		printf("接收真实服务器状态码%s\n", r12); 
		fprintf(fp, "接收真实服务器状态码%s\n", r12);//将错误代码存入文件
	}
	else if((strcmp(temp, "452") == 0))
	{
		printf("接收真实服务器状态码%s\n", r13); 
		fprintf(fp, "接收真实服务器状态码%s\n", r13);//将错误代码存入文件
	}
	else if((strcmp(temp, "500") == 0))
	{
		printf("接收真实服务器状态码%s\n", r14); 
		fprintf(fp, "接收真实服务器状态码%s\n", r14);//将错误代码存入文件
	}
	else if((strcmp(temp, "501") == 0))
	{
		printf("接收真实服务器状态码%s\n", r15); 
		fprintf(fp, "接收真实服务器状态码%s\n", r15);//将错误代码存入文件
	}
	else if((strcmp(temp, "502") == 0))
	{
		printf("接收真实服务器状态码%s\n", r16); 
		fprintf(fp, "接收真实服务器状态码%s\n", r16);//将错误代码存入文件
	}
	else if((strcmp(temp, "503") == 0))
	{
		printf("接收真实服务器状态码%s\n", r9); 
		fprintf(fp, "接收真实服务器状态码%s\n", r9);//将错误代码存入文件
	}
	else if((strcmp(temp, "504") == 0))
	{
		printf("接收真实服务器状态码%s\n", r17); 
		fprintf(fp, "接收真实服务器状态码%s\n", r17);//将错误代码存入文件
	
	}
	else if((strcmp(temp, "550") == 0))
	{
		printf("接收真实服务器状态码%s\n", r8); 
		fprintf(fp, "接收真实服务器状态码%s\n", r8);//将错误代码存入文件
	
	}
	else if((strcmp(temp, "551") == 0))
	{
		printf("接收真实服务器状态码%s\n", r18); 
		fprintf(fp, "接收真实服务器状态码%s\n", r18);//将错误代码存入文件
	}
	else if((strcmp(temp, "552") == 0))
	{
		printf("接收真实服务器状态码%s\n", r19); 
		fprintf(fp, "接收真实服务器状态码%s\n", r19);//将错误代码存入文件
	}
	else if((strcmp(temp, "553") == 0))
	{
		printf("接收真实服务器状态码%s\n", r20); 
		fprintf(fp, "接收真实服务器状态码%s\n", r20);//将错误代码存入文件
	}
	else if((strcmp(temp, "554") == 0))
	{
		printf("接收真实服务器状态码%s\n", r21); 
		fprintf(fp, "接收真实服务器状态码%s\n", r21);//将错误代码存入文件
	}
	else
	{
		printf("客户端邮箱服务器接收数据顺序错误，请重新发送"); 
		fprintf(fp, "接收真实服务器状态码%s，接收数据顺序错误，发送失败\n", temp);//将错误代码存入文件
	}	
	time1();
	fprintf(fp, "end time: %d-%d-%d %d:%d:%d\n\n\n\n", now[0], now[1], now[2], now[3], now[4], now[5]);
	fclose(fp);  //关闭文件指针
 } 
