/*SMTP������*/

#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#pragma comment(lib,"wsock32.lib")
#include <stdlib.h>
#include <errno.h>
#include <time.h> 

#define BUFSIZE 4096//��������С
#define PORT 25//�˿ں� 
//��ʾ��	
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

//ȫ�ֱ��� 
char ehlo[BUFSIZE];	//��ͻ���ehlo 
char mailFrom[BUFSIZE];	//��ͻ������� 
char rcptTo[5][BUFSIZE];	//��Ŀ������
char clientIP[BUFSIZE];	//�ͻ��˷�����ip��ַ 
char ip[BUFSIZE];			//��ÿ���û������IP��ַ 
char data[BUFSIZE];		//��Data
char imf[BUFSIZE * 10];	//���ʼ���׼��ʽ
char recvData[BUFSIZE];	//���������
char rcptJudge[BUFSIZE];	//�����ж�Ŀ���������� 
int i = 0;	//��¼Ŀ���������� 
int j = 0; //��¼Ŀ���������� 
int error;//�жϵ��ÿͻ��˺����Ƿ�ɹ� 
int now[6];	//��¼ʱ��
char fname[256] = {0};//��¼ʱ�� 
FILE *fp;//���ļ� 
char nativeIP[BUFSIZE];//�汾��IP
char temp[3];//��״̬�� 
char username[BUFSIZE];//���½�û���
char password[BUFSIZE];//���½���� 


//�Ӻ��� 
int server(); //���÷������˺���
int client(); //���ÿͻ��˺���
int validEmail(char*);		//�շ��������ַ(addr)�Ϸ��Լ��
char* translateIP(char* dominname);	//��Դ�����������ַת��Ϊip��ַ 
char* getIP(); 	//��ȡ�ͻ���ip��ַ 
int time1();		//ʱ������� 
void Error();	//�жϴ������� 

int main()
{
	int Client=0;
	int Server=0;
	//���÷������˺��� 
	Server = server();
	if(Server==-1)
	{
		return -1;
	}
	else
	{
		//���ÿͻ��˺��� 
		
		Client = client();
		if(Client==-1)
		{
			Error();
			return -1;
		}
		else
		{
			printf("\n\n������������������������������������������������������������\n"); 
			fp = fopen(fname,"a");
			fprintf(fp,"\n\n������������������������������������������������������������\n"); 
			time1();
			fprintf(fp, "end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
			printf("end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
			return 0; 
		}
	}
}

//�������˺��� 
int server()
{
	int             Ret;
	WSADATA         wsaData;
	SOCKET          ListeningSocket;	//���� 
	SOCKET          socketConnection;	//���� 
	SOCKADDR_IN     ServerAddr;	//��������ַ 
	SOCKADDR_IN     ClientAddr;	//�ͻ��˵�ַ 
	int             ClientAddrLen = sizeof(ClientAddr);	//�ͻ��˵�ַ���� 
	int             flag = 1;	//�ж��Ƿ��������

	//��ʼ��WSAStartup
	Ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Ret != 0)//������óɹ���WSAStartup��������0 
	{
		printf("WSASTARTUP_ERROR: %d\n", Ret);
		WSACleanup(); //�ͷŷ�����Դ
		return -1;
	}

	//����һ���׽����������ͻ�������
	ListeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListeningSocket == INVALID_SOCKET)
	{
		printf("SOCKET_ERROR: %d\n", INVALID_SOCKET);
		WSACleanup(); //�ͷŷ�����Դ
		return -1;
	}

	//���SOCKADDR_IN�ṹ
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(PORT); //���˿ڱ����������ֽ�˳��ת��λ�����ֽ�˳��
	ServerAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//ֻ�ӱ������͵�tcp����


	//ʹ��bind�������ַ��Ϣ���׽��ְ�����
	if (bind(ListeningSocket, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR)
	{
		printf("BIND_ERROR: %d\n", SOCKET_ERROR);
		WSACleanup(); //�ͷŷ�����Դ
		return -1;
	}

	//�����ͻ������ӡ�����ʹ��5��backlog
	if (listen(ListeningSocket, 5) == SOCKET_ERROR)
	{
		printf("LISTEN_ERROR: %d\n", SOCKET_ERROR);
		WSACleanup(); //�ͷŷ�����Դ
		return -1;
	}

	//���ӵ���ʱ����������
	printf("���ڽ�������......");
	socketConnection = accept(ListeningSocket, (SOCKADDR *)&ClientAddr, &ClientAddrLen); 
	if (socketConnection == INVALID_SOCKET)
	{
		printf("\nACCPET_ERROR: %d\n", INVALID_SOCKET);
		closesocket(ListeningSocket);
		WSACleanup(); //�ͷŷ�����Դ
		return -1;
	}
	printf("\n��⵽һ������:\n\n"); 
	time1();
	printf("Time:%04d-%02d-%02d_%02d:%02d:%02d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
	printf("������������������������������������������������������������\n");
	strcpy(nativeIP, getIP());//��ȡ�ͻ���ip��ַ
	printf("�ͻ���IP��ַ��%s  �˿ں�:%d\n", nativeIP,ntohs(ClientAddr.sin_port));

	//���� 
	while (flag != 0)//�ȴ��ͻ����� 
	{
		time1();
		sprintf(fname, "E:/mail/Log-%04d-%02d-%02d_%02d-%02d-%02d.txt", now[0], now[1], now[2], now[3], now[4], now[5]);
		fp = fopen(fname,"a");
		time1();
		fprintf(fp, "start time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
		fprintf(fp,"������������������������������������������������������������\n"); 
		fprintf(fp,"�ͻ���IP��ַ��%s  �˿ں�:%d\n\n", nativeIP,ntohs(ClientAddr.sin_port));
		memset(rcptTo, 0, sizeof(rcptTo));	
		
		//���Ѿ����ӵ��׽���sockConnection�������ӽ�����Ϣ
		{
			send(socketConnection, r1, strlen(r1), 0);  //�������ӽ�����Ϣ��220
			printf("����״̬�룺220 Mysmtp Service Ready.\n");
			fprintf(fp, "%s", r1);//�����ӳɹ����浽�ļ��� 
		}
		
		//����ehlo 
		{
			recv(socketConnection, recvData, sizeof(recvData), 0); //�������� ehlo
			if(strncmp(recvData,"EHLO",4) != 0)//�ж��Ƿ���յ�EHLO 
			{
				printf("���ղ���EHLO.");
				fprintf(fp,"���ղ���EHLO��%s",r29); 
				send(socketConnection, r29, strlen(r29), 0);//����503 
				WSACleanup(); //�ͷŷ�����Դ
				return -1;
			}
			printf("����EHLO��%s",recvData); 
			fprintf(fp, "%s\n", recvData); //��ehlo����д���ļ�
			memcpy(ehlo, recvData, sizeof(recvData));
			memset(recvData, 0, sizeof(recvData)); //��recvDataǰ4096���ֽ����ַ�'0'�滻
			send(socketConnection, r3, strlen(r3), 0); // ���ͽ��ճɹ����ݣ�250 AUTH LOGIN PLAINOK|250-AUTH=LOGIN PLAIN|250-STARTTLS|250 8BITMIME\r\n
			printf("����״̬�룺250 AUTH LOGIN PLAINOK|250-AUTH=LOGIN PLAIN\r\n");
			fprintf(fp, "%s", r3);//��250д���ļ� 
		}
		
		//����AUTH LOGIN
		{
			recv(socketConnection, recvData, sizeof(recvData), 0); 
			//�жϽ����Ƿ���ȷ 
			if(strncmp(recvData,"AUTH LOGIN",10) != 0)
			{
				printf("���ղ���AUTH LOGIN.");
				fprintf(fp,"���ղ���AUTH LOGIN��%s",r30); 
				send(socketConnection, r30, strlen(r30), 0);//����550 time1(); 
				fprintf(fp, "end time: %d-%d-%d %d:%d:%d\n\n\n\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);  //�ر��ļ�ָ��
				WSACleanup(); //�ͷŷ�����Դ
				return -1;
			} 
			printf("����AUTH LOGIN\n");	
			fprintf(fp, "%s\n", recvData);	//��AUTH LOGIN�ĸ���д���ļ��� 
			memset(recvData, 0, sizeof(recvData));	//��recvDataǰ4096���ֽ����ַ�'0'�滻
			send(socketConnection, r26, strlen(r26), 0);//����334 
			printf("����״̬��334\n");
			fprintf(fp, "%s\n", r26);	
		} 
		
		//�����û���
		{
			recv(socketConnection, recvData, sizeof(recvData), 0); 
			printf("�����û���\n");	
			memcpy(username, recvData, sizeof(recvData));
			fprintf(fp, "%s\n", recvData);	
			memset(recvData, 0, sizeof(recvData));	//��recvDataǰ4096���ֽ����ַ�'0'�滻
			send(socketConnection, r27, strlen(r27), 0);//����334 
			printf("����״̬��334\n");
			fprintf(fp, "%s\n", r27);	
		} 
		
		//�������� 
		{
			recv(socketConnection, recvData, sizeof(recvData), 0);  
			printf("��������\n");	
			memcpy(password, recvData, sizeof(recvData));
			fprintf(fp, "%s\n", recvData);	
			memset(recvData, 0, sizeof(recvData));	//��recvDataǰ4096���ֽ����ַ�'0'�滻
			send(socketConnection, r28, strlen(r28), 0);//����235 Authentication successful
			printf("����״̬��235\n");
			fprintf(fp, "%s\n", r28);	
		} 
		
		//����Դ�����ַ 
		{
			recv(socketConnection, recvData, sizeof(recvData), 0); //����Դ���䣺MAIL FROM:<...>
			//�ж��Ƿ���յ��������ַ
			if(strncmp(recvData,"MAIL FROM",9) != 0)
			{
				printf("���ղ���mail from.");
				fprintf(fp,"���ղ���mail from��%s",r8); 
				send(socketConnection, r8, strlen(r8), 0);//����550 
				WSACleanup(); //�ͷŷ�����Դ
				return -1;
			} 
			//�ж�Դ�����Ƿ���ȷ 
			if (validEmail(recvData)==-1)//���ʹ������550
			{
				send(socketConnection, r8, strlen(r8), 0);
				printf("���տͻ�������ERROR:%s",r8);
				fprintf(fp,"���տͻ�������ERROR:%s",r8); 
				closesocket(socketConnection);
				fclose(fp);
				WSACleanup(); //�ͷŷ�����Դ
				return -1;
			}
			memcpy(mailFrom, recvData, sizeof(recvData));//��Դ�����ַ���Ƶ�mailFrom���� 
			printf("���տͻ������䣺%s", recvData);	//��ӡԴ����
			fprintf(fp, "%s", recvData);	//��Դ�����ַд���ļ�
			strcpy(ip, translateIP(mailFrom));//��Դ�����������ַת��Ϊip��ַ 
			printf("�ͻ������������IP��ַ: %s  �˿ںţ�%d\n", ip , PORT);
			fprintf(fp,"�ͻ������������IP��ַ: %s  �˿ںţ�%d\n\n", ip , PORT);//�����ļ� 
			memcpy(clientIP, ip, sizeof(ip));
			memset(recvData, 0, sizeof(recvData));	//��recvDataǰ4096���ֽ����ַ�'0'�滻
			send(socketConnection, r4, strlen(r4), 0); //���ͽ��ճɹ����룺250 OK
			printf("����״̬�룺250 OK \n");
			fprintf(fp, "%s", r4);	//��250 OKд���ļ� 
		}
		
		//����Ŀ�������ַ 
		{
			recv(socketConnection, recvData, sizeof(recvData), 0);//�����ʼ�rcpt
			i = 0;//��¼Ŀ��������� 
			strncpy(rcptJudge, recvData, 4);
			while ((strcmp(rcptJudge, "RCPT") == 0) && (i < 5))
			{
				//�ж��Ƿ���յ��������ַ
				if(strncmp(recvData,"RCPT TO",7) != 0)
				{
					printf("���ղ���rcpt to.");
					fprintf(fp,"���ղ���rcpt to��%s",r8); 
					send(socketConnection, r8, strlen(r8), 0);//����550 
					WSACleanup(); //�ͷŷ�����Դ
					return -1;
				} 
				if (validEmail(recvData)==-1)//����550
				{
					send(socketConnection, r8, strlen(r8), 0);
					closesocket(socketConnection);
					printf("��Ŀ������ERROR:%s",r8);
					fprintf(fp,"����Ŀ������ERROR:%s",r8); 
					fclose(fp);
					WSACleanup(); //�ͷŷ�����Դ
					return -1;
				}
				memcpy(rcptTo[i], recvData, sizeof(recvData));
				printf("����Ŀ������%d: %s", i + 1, recvData);
				fprintf(fp, "%s\n", recvData);
				memset(recvData, 0, sizeof(recvData));
				memset(ip, 0, sizeof(ip));
				memset(rcptJudge, 0, sizeof(rcptJudge));
				send(socketConnection, r4, strlen(r4), 0); //���ͳɹ�250 OK״̬�� 
				printf("����״̬�룺250 OK \n");
				fprintf(fp, "%s", r4);//��250 OK�����ļ� 
				recv(socketConnection, recvData, sizeof(recvData), 0); //���� RCPT TO:<....>
				strncpy(rcptJudge, recvData, 4);
				i++;
			}
		}
	
		//�ж��Ƿ��ܹ����յ� ��DATA ��
		if(strcmp(recvData, r22) != 0)
		{
			printf("���ղ���DATA\n");
			fprintf(fp,"���ղ���DATA %s",r31);
			send(socketConnection, r31, strlen(r31), 0); //���ʹ���ԭ��
			time1(); 
			fprintf(fp, "end time: %d-%d-%d %d:%d:%d\n\n\n\n", now[0], now[1], now[2], now[3], now[4], now[5]);
			fclose(fp);  //�ر��ļ�ָ��
			WSACleanup(); //�ͷŷ�����Դ
			return -1;
		}
		printf("����'DATA'\n");	
		fprintf(fp, "%s\n", recvData);		//��DATA�ĸ���д���ļ��� 
		memset(recvData, 0, sizeof(recvData));	//��recvDataǰ4096���ֽ����ַ�'0'�滻

		//�����ʼ�����
		{
			//�����ʼ����մ���
			send(socketConnection, r5, strlen(r5), 0);//�����ʼ����մ��룺354 Start mail input;end with <CR><LF>.<CR><LF>\r\n
			printf("����״̬�룺354 Start mail input;end with <CR><LF>.<CR><LF>\n");
			fprintf(fp, "%s", r5);//��354�����ļ� 
			
			//����data 
			{
				recv(socketConnection, recvData, sizeof(recvData), 0); //�����ʼ�DATA fragment, ...bytes
				memcpy(data, recvData, sizeof(recvData));	//�����յ����ʼ�data������data�����У����������ʹ�� 
				printf("����DATA fragment, ...bytes\n");
				fprintf(fp, "����DATA fragment, ...bytes\n\n%s\n", recvData);	//�����յ����ʼ����ݴ洢���ļ��� 
				memset(recvData, 0, sizeof(recvData));	//��recvDataǰ4096���ֽ����ַ�'0'�滻
			}
		
			//����IMF 
			{
				recv(socketConnection, recvData, sizeof(recvData), 0); //����IMF
				memcpy(imf, recvData, sizeof(recvData));//�����յ����ʼ�IMF���ݿ�����imf�����У����������ʹ��
				printf("����imf\n");
				fprintf(fp, "����imf\n\n%s\n", recvData);//�����յ���IMF���ݴ洢���ļ���
				memset(recvData, 0, sizeof(recvData));	//��recvDataǰ4096���ֽ����ַ�'0'�滻
			}

			//�����ʼ�ĩβ�� "." 
			{
				recv(socketConnection, recvData, sizeof(recvData), 0); //�����ʼ�ĩβ�� "." 
				//�ж��Ƿ��ܹ����յ� ��. ��
				if(strcmp(recvData, " ") == 0)
				{
					printf("���ղ���'.'\n");
					fprintf(fp,"���ղ���'.' %s",r32);
					send(socketConnection, r32, strlen(r32), 0); //���ʹ���ԭ��time1();
					time1(); 
					fprintf(fp, "end time: %d-%d-%d %d:%d:%d\n\n\n\n", now[0], now[1], now[2], now[3], now[4], now[5]);
					fclose(fp);  //�ر��ļ�ָ��
					WSACleanup(); //�ͷŷ�����Դ
					return -1;
				}
				printf("����'.'\n");
				fprintf(fp, "����%s\n", recvData);	//�����յ���"."�洢���ļ���
				memset(recvData, 0, sizeof(recvData));	//��recvDataǰ4096���ֽ����ַ�'0'�滻
				send(socketConnection, r4, strlen(r4), 0); //���ͽ��ճɹ����룺250 OK
				printf("����״̬��: 250 OK \n");
				fprintf(fp, "%s", r4);//��250 OK�����ļ� 
			}
		
			//���նϿ����� ���ظ�221 bye
			{
				recv(socketConnection, recvData, sizeof(recvData), 0); //�����ʼ�ĩβ�� "QIUT"
				fprintf(fp, "%s\n", recvData);	//�����յ���"QUIT"�洢���ļ���
				printf("����״̬�룺QUIT \n");
				memset(recvData, 0, sizeof(recvData));	//��recvDataǰ4096���ֽ����ַ�'0'�滻
				send(socketConnection, r2, strlen(r2), 0); //����TCP�����ͷŴ��룺221 bye
				fprintf(fp, "%s\n", r2);//��221 bye �����ļ� 
				printf("����״̬�룺221 bye \n");		
			} 
			
			printf("������������������������������������������������������������\n");
			fprintf(fp,"������������������������������������������������������������\n");
			//������䳤�� 
			{
				printf("\n�ʼ����ȣ�%d�ֽ�\n\n",strlen(imf)) ;
				fprintf(fp,"\n�ʼ����ȣ�%d�ֽ�\n\n",strlen(imf)) ; 
			}	
			
			printf("������������������������������������������������������������\n\n"); 
			printf("\n��ʼ�����ʼ�\n\n");
			fprintf(fp,"������������������������������������������������������������\n\n"); 
			fprintf(fp,"\n��ʼ�����ʼ�\n");
			
			fclose(fp);  //��ʱ�ر��ļ� 	
		}

			
		flag = 0;
	}
	closesocket(socketConnection); //�ر��׽���
	WSACleanup(); //�ͷŷ�����Դ
	return 0; 
}


//�ͻ��˺���
int client()
{
	
	int             Ret1;
	WSADATA         wsaData1;
	SOCKET          socketclient;	
	SOCKET          socketnative;
	SOCKADDR_IN     nativeAddr;	//����ip��ַ 
	SOCKADDR_IN     clientAddr1;//�ͻ���ip��ַ 
	
	//��ʼ��WSAStartup
	Ret1 = WSAStartup(MAKEWORD(2, 2), &wsaData1);
	if (Ret1 != 0)//������óɹ���WSAStartup��������0 
	{
		printf("WSASTARTUP_ERROR: %d\n", Ret1);
		WSACleanup(); //�ͷŷ�����Դ
		return -1;
	}
	
	
	while (j < i)
	{
		//�������� 
		{
			socketclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			socketnative = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			
			//��ȡ�����Ŀͻ��������������ַ 
			clientAddr1.sin_family = AF_INET;
			clientAddr1.sin_port = htons(PORT);
			clientAddr1.sin_addr.s_addr = inet_addr(clientIP);
			connect(socketclient, (SOCKADDR*)&clientAddr1, sizeof(SOCKADDR));
			
			//����IP��ַ�Ͷ˿� 
			nativeAddr.sin_family = AF_INET;
			nativeAddr.sin_port = htons(PORT);
			nativeAddr.sin_addr.s_addr = inet_addr(nativeIP);
			connect(socketnative, (SOCKADDR*)&nativeAddr, sizeof(SOCKADDR));
			
		}

		//����־�ļ�
		{
			fp = fopen(fname, "a");
			printf("������������������������������������������������������������"); 
			printf("\nĿ������%d\n���ӳɹ�\n",j+1);
			printf("����IP��ַ��%s  SMTP���Ͷ˿�:%d", inet_ntoa(nativeAddr.sin_addr),ntohs(nativeAddr.sin_port));
			fprintf(fp,"Ŀ������%d\n���ӳɹ�\n",j+1);
			fprintf(fp,"����IP��ַ��%s  SMTP���Ͷ˿�:%d", inet_ntoa(nativeAddr.sin_addr),ntohs(nativeAddr.sin_port));
			time1();
			fprintf(fp, "\nsend start time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
			printf("\nsend start time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
			printf("������������������������������������������������������������\n");
			fprintf(fp,"������������������������������������������������������������\n"); 
		}
		
		//����220״̬�벢����ehlo״̬�� 
		{
			recv(socketclient, recvData, sizeof(recvData), 0);  //����״̬��220 OK
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "220") != 0) 
			{
				printf("����״̬�룺%s, NO 220, ����ʧ��\n",temp);
				fprintf(fp,"����״̬�룺%s, NO 220, ����ʧ��\n",temp);
				fclose(fp);
				WSACleanup(); //�ͷŷ�����Դ
				return -1;
			} 
			else
			{
				printf("����״̬�룺%s",recvData);
				fprintf(fp, "����״̬�룺%s", recvData);		//��״̬��220 OK�����ļ� 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, ehlo, strlen(ehlo), 0);
				printf( "����״̬�룺%s", ehlo);		
				fprintf(fp, "����״̬�룺%s", ehlo);		//�����͵�EHLO״̬������ļ�
			}
		} 
		
		//����ehlo����״̬�벢����AUTH LOGIN״̬��
		{
			recv(socketclient, recvData, sizeof(recvData), 0);       //����״̬��
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "250") != 0) 
			{
				printf("����״̬�룺%s, NO 250, ����ʧ��\n",temp);
				fprintf(fp,"����״̬�룺%s, NO 250, ����ʧ��\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //�ͷŷ�����Դ
				return -1;
			} 
			else
			{
				printf("����״̬�룺%s\n",temp);
				fprintf(fp, "����״̬�룺%s\n", temp);		//��״̬��250 OK�����ļ� 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, r25, strlen(r25), 0);   //����:AUTH LOGIN
				printf("����״̬�룺%s", r25);
				fprintf(fp, "����״̬�룺%s", r25);		//�����͵�AUTH LOGIN״̬������ļ�
			}
		} 
		
		//����״̬��334�������û���
		{
			recv(socketclient, recvData, sizeof(recvData), 0);       //����״̬�� ������һ����334 
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "334") != 0) 
			{
				printf("����״̬�룺%s, NO 334, ����ʧ��\n",temp);
				fprintf(fp,"����״̬�룺%s, NO 334, ����ʧ��\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //�ͷŷ�����Դ
				return -1;
			}
			else
			{
				printf("����״̬�룺%s\n",temp);
				fprintf(fp, "����״̬�룺%s\n", temp);		//��״̬��250 OK�����ļ� 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, username, strlen(username), 0);   ////�����û���
				printf("�����û�����XXXXXXXX\n");
				fprintf(fp, "�����û�����%s", username);		//�����͵��û��������ļ�
			}
		}
		
		//����״̬��334���������� 
		{
			recv(socketclient, recvData, sizeof(recvData), 0);       //����״̬�� ������һ����334 
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "334") != 0) 
			{
				printf("����״̬�룺%s, NO 334, ����ʧ��\n",temp);
				fprintf(fp,"����״̬�룺%s, NO 334, ����ʧ��\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //�ͷŷ�����Դ
				return -1;
			}
			else
			{
				printf("����״̬�룺%s\n",temp);
				fprintf(fp, "����״̬�룺%s\n", temp);		//��״̬��250 OK�����ļ� 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, password, strlen(password), 0);   ////�������� 
				printf("�������룺********\n");
				fprintf(fp, "�������룺%s", password);		//�����͵���������ļ�
			}
		}
		
		//����״̬��235�����ͷ����ͻ����������� 
		{
			recv(socketclient, recvData, sizeof(recvData), 0);         // ����״̬��,235 Authentication successful��ʾ�ɹ� 
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "235") != 0) 
			{
				printf("����״̬�룺%s, NO 235, ����ʧ��\n",temp);
				fprintf(fp,"����״̬�룺%s, NO 235, ����ʧ��\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //�ͷŷ�����Դ
				return -1;
			}
			else
			{
				printf("����״̬�룺%s\n",temp);
				fprintf(fp, "����״̬�룺%s\n", temp);		//��״̬��235�����ļ� 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, mailFrom, strlen(mailFrom), 0);     //���Ϳͻ����������� 
				printf("���Ϳͻ������䣺%s",mailFrom);
				fprintf(fp, "���Ϳͻ������䣺%s", mailFrom);			//����ͻ����������� 
			}
		}
	
		//����״̬��250 OK��������Ŀ����������
		{	
			recv(socketclient, recvData, sizeof(recvData), 0);             //����״̬�룺250 mail ok
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "250") != 0) 
			{
				printf("����״̬�룺%s, NO 250, ����ʧ��\n",temp);
				fprintf(fp,"����״̬�룺%s, NO 250, ����ʧ��\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //�ͷŷ�����Դ
				return -1;
			}
			else
			{
				printf("����״̬�룺%s\n",temp);
				fprintf(fp, "����״̬�룺%s\n", temp);		//��״̬��250�����ļ� 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, rcptTo[j], strlen(rcptTo[j]), 0);     //����Ŀ���������� 
				printf("����Ŀ�����䣺%s", rcptTo[j]); 		
				fprintf(fp, "����Ŀ�����䣺%s", rcptTo[j]); 							//��Ŀ����������ļ� 
			} 
		} 
		
		//����״̬��250 OK ������DATA
		{	
			recv(socketclient, recvData, sizeof(recvData), 0);         //����״̬�룺250 mail ok
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "250") != 0) 
			{
				printf("����״̬�룺%s, NO 250, ����ʧ��\n",temp);
				fprintf(fp,"����״̬�룺%s, NO 250, ����ʧ��\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //�ͷŷ�����Դ
				return -1;
			}
			else
			{
				printf("����״̬�룺%s\n",temp);
				fprintf(fp, "����״̬�룺%s\n", temp);		//��״̬��250�����ļ� 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, r22, strlen(r22), 0);     //����DATA
				printf("����:DATA\n");
				fprintf(fp,"����:DATA\n");
			} 
		} 
		
		//����״̬��354 ������data,imf����. �� 
		{	
			recv(socketclient, recvData, sizeof(recvData), 0);        // ����״̬�룺354 end data with <CR><LF>.<CR><LF>
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "354") != 0) 
			{
				printf("����״̬�룺%s, NO 354, ����ʧ��\n",temp);
				fprintf(fp,"����״̬�룺%s, NO 354, ����ʧ��\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //�ͷŷ�����Դ
				return -1;
			}
			else
			{
				printf("����״̬�룺%s\n",temp);
				fprintf(fp, "����״̬�룺%s\n", temp);		//��״̬��354�����ļ� 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				
				send(socketclient, data, strlen(data), 0);		//����data 
				printf("����:data\n");
				fprintf(fp, "%s\n", data);	
		
				send(socketclient, imf, strlen(imf), 0);		//����imf
				printf("����:imf\n");
				fprintf(fp, "%s\n", imf);
		
				send(socketclient, r24, strlen(r24), 0);     //���͡� . �� 
				printf("����:' . '\n");
				fprintf(fp, "%s", r24);
			} 
		} 
		
		//����״̬�� 250 OK ������QUIT 
		{
			recv(socketclient, recvData, sizeof(recvData), 0);         //����״̬�룺250
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "250") != 0) 
			{
				printf("����״̬�룺%s, NO 250, ����ʧ��\n",temp);
				fprintf(fp,"����״̬�룺%s, NO 250, ����ʧ��\n",temp);
				time1();
				fprintf(fp, "send end time: %d-%d-%d %d:%d:%d\n", now[0], now[1], now[2], now[3], now[4], now[5]);
				fclose(fp);
				WSACleanup(); //�ͷŷ�����Դ
				return -1;
			}
			else
			{
				printf("����״̬�룺%s\n",temp);
				fprintf(fp, "����״̬�룺%s\n", temp);		//��״̬��250�����ļ� 
				memset(recvData, 0, sizeof(recvData));
				memset(temp, 0, sizeof(temp));
				send(socketclient, r23, strlen(r23),0);     //����QUIT
				printf("����:QUIT\n");
				fprintf(fp,"����:QUIT\n");
			} 
		}
		
		//����״̬��221 bye
		{
			recv(socketclient, recvData, sizeof(recvData), 0);         //����״̬�룺221 bye 
			strncpy(temp, recvData, 3);
			if (strcmp(temp, "221") != 0) 
			{
				printf("����״̬�룺%s, NO 221, ����ʧ��\n",temp);
				fprintf(fp,"����״̬�룺%s, NO 221, ����ʧ��\n",temp);
				fclose(fp);
				WSACleanup(); //�ͷŷ�����Դ
				return -1;
			}
			else
			{
				printf("����״̬�룺%s\n",temp);
				printf("�ʼ����ͳɹ�.\n");
				printf("������������������������������������������������������������\n"); 	
				fprintf(fp, "����״̬�룺%s\n�ʼ����ͳɹ�\n", temp);		//��״̬��221�����ļ�
				fprintf(fp,"������������������������������������������������������������\n"); 	
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

//�շ��������ַ�Ϸ��Լ��
int validEmail(char *addr)
{
	int colonAddr = 0; //ð�� 
	int atAddr = 0;	//@
	int pointAddr = 0;	//��� 
	int bracketAddr = 0;	//>
	int error1 = 0;
	unsigned int a = 0;

	//�ҳ���������λ�� 
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

	//�ҳ���@ ����λ��
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

	//�ҳ���> ����λ�� 
	for (a = 0; a < strlen(addr); a++)
	{
		if (addr[a] == '>')
		{
			bracketAddr = a;
		}
	}
	//ȷ�������ַ� 
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
	//�ڡ��������롰@ ��ǰȷ�������� 
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

	//�ڡ�@ ������ȷ��������
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

//��ȡ�ͻ���ip��ַ
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

//��Դ�����������ַת��Ϊip��ַ 
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
	//��h_addr_listת��Ϊin_addr��h_addr_listֻ�г���ʽ��ip��ַ
	addr_list = (struct in_addr **)hostEntry->h_addr_list;
	for (x = 0; addr_list[x] != NULL; x++)
	{
		IP = inet_ntoa(*addr_list[x]);
	}
	return IP;
}

//ʱ�������
int time1()
{   
	time_t t;
	struct tm * lt;
	time(&t);//��ȡʱ����� 
	lt = localtime(&t);//תΪʱ��ṹ��
	now[0] = lt->tm_year + 1900;
	now[1] = lt->tm_mon + 1;
	now[2] = lt->tm_mday;
	now[3] = lt->tm_hour;
	now[4] = lt->tm_min;
	now[5] = lt->tm_sec;
} 

//�жϴ������ͺ��� 
void  Error()
{
	fp = fopen(fname,"a");//���ļ� 
	if((strcmp(temp, "250") == 0)||(strcmp(temp, "334") == 0)||(strcmp(temp, "220") == 0)||(strcmp(temp, "235") == 0))
	{
		printf("�ͻ��������������������˳����������·���"); 
		fprintf(fp, "������ʵ������״̬��%s����������˳����󣬷���ʧ��\n", temp);//��temp�����ļ�
	}
	else if((strcmp(temp, "221") == 0))
	{
		printf("������ʵ������״̬��%s\n", r10); 
		fprintf(fp, "������ʵ������״̬��%s\n", r10);//�������������ļ�
	}
	else if((strcmp(temp, "251") == 0))
	{
		printf("������ʵ������״̬��%s\n", r11); 
		fprintf(fp, "������ʵ������״̬��%s\n", r11);//�������������ļ�
	}
	else if((strcmp(temp, "421") == 0))
	{
		printf("������ʵ������״̬��%s\n", r6); 
		fprintf(fp, "������ʵ������״̬��%s\n", r6);//�������������ļ�
	}
	else if((strcmp(temp, "450") == 0))
	{
		printf("������ʵ������״̬��%s\n", r7); 
		fprintf(fp, "������ʵ������״̬��%s\n", r7);//�������������ļ�
	}
	else if((strcmp(temp, "451") == 0))
	{
		printf("������ʵ������״̬��%s\n", r12); 
		fprintf(fp, "������ʵ������״̬��%s\n", r12);//�������������ļ�
	}
	else if((strcmp(temp, "452") == 0))
	{
		printf("������ʵ������״̬��%s\n", r13); 
		fprintf(fp, "������ʵ������״̬��%s\n", r13);//�������������ļ�
	}
	else if((strcmp(temp, "500") == 0))
	{
		printf("������ʵ������״̬��%s\n", r14); 
		fprintf(fp, "������ʵ������״̬��%s\n", r14);//�������������ļ�
	}
	else if((strcmp(temp, "501") == 0))
	{
		printf("������ʵ������״̬��%s\n", r15); 
		fprintf(fp, "������ʵ������״̬��%s\n", r15);//�������������ļ�
	}
	else if((strcmp(temp, "502") == 0))
	{
		printf("������ʵ������״̬��%s\n", r16); 
		fprintf(fp, "������ʵ������״̬��%s\n", r16);//�������������ļ�
	}
	else if((strcmp(temp, "503") == 0))
	{
		printf("������ʵ������״̬��%s\n", r9); 
		fprintf(fp, "������ʵ������״̬��%s\n", r9);//�������������ļ�
	}
	else if((strcmp(temp, "504") == 0))
	{
		printf("������ʵ������״̬��%s\n", r17); 
		fprintf(fp, "������ʵ������״̬��%s\n", r17);//�������������ļ�
	
	}
	else if((strcmp(temp, "550") == 0))
	{
		printf("������ʵ������״̬��%s\n", r8); 
		fprintf(fp, "������ʵ������״̬��%s\n", r8);//�������������ļ�
	
	}
	else if((strcmp(temp, "551") == 0))
	{
		printf("������ʵ������״̬��%s\n", r18); 
		fprintf(fp, "������ʵ������״̬��%s\n", r18);//�������������ļ�
	}
	else if((strcmp(temp, "552") == 0))
	{
		printf("������ʵ������״̬��%s\n", r19); 
		fprintf(fp, "������ʵ������״̬��%s\n", r19);//�������������ļ�
	}
	else if((strcmp(temp, "553") == 0))
	{
		printf("������ʵ������״̬��%s\n", r20); 
		fprintf(fp, "������ʵ������״̬��%s\n", r20);//�������������ļ�
	}
	else if((strcmp(temp, "554") == 0))
	{
		printf("������ʵ������״̬��%s\n", r21); 
		fprintf(fp, "������ʵ������״̬��%s\n", r21);//�������������ļ�
	}
	else
	{
		printf("�ͻ��������������������˳����������·���"); 
		fprintf(fp, "������ʵ������״̬��%s����������˳����󣬷���ʧ��\n", temp);//�������������ļ�
	}	
	time1();
	fprintf(fp, "end time: %d-%d-%d %d:%d:%d\n\n\n\n", now[0], now[1], now[2], now[3], now[4], now[5]);
	fclose(fp);  //�ر��ļ�ָ��
 } 
