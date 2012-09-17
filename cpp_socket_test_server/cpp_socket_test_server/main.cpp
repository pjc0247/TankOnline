#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
 
#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "protocol.h"

#pragma comment (lib,"ws2_32")
 
using namespace std;
 
#define BUFSIZE (1024 * 20)
#define SERVER_PORT 9916 

typedef struct
{
	char id[32];
	char pw[128];
	char nick[32];

	int x,y,speed;

	char area[32];
} USER_DATA;
typedef struct
{
	SOCKET      hClntSock;
	SOCKADDR_IN clntAddr;
	int n;
	USER_DATA user;

	int fileReceived;
	char fileName[256];
	int fileLength;
	bool fileRecv;
	FILE *filePointer;
	int fileWritten;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct
{
	OVERLAPPED overlapped;
	char buffer[BUFSIZE];
	WSABUF wsaBuf;
} PER_IO_DATA, *LPPER_IO_DATA;

typedef struct
{
	void (*handler)(int ,char *);
} HANDLER;


DWORD WINAPI CompletionThread(LPVOID CompletionPortIO);
void RegistHandler(int code,void (*cb)(int,char *));
void ParsePacket(int w,char *msg,int msgLength);
void Send(int w,int p,char *msg);
void SendBinary(int w,int p,char *data,int len);
void SendFile(int w,char *file);

vector<PER_HANDLE_DATA*> clients;
map<int,HANDLER> handler;
map<string,int> session;

map<string,vector<PER_HANDLE_DATA*>> area;

CRITICAL_SECTION csClients;
CRITICAL_SECTION csSession;
CRITICAL_SECTION csArea;

//#define __SYNCRONIZE
#ifdef	__SYNCRONIZE
	#define __ENTER(cs) EnterCriticalSection(&cs)
	#define __LEAVE(cs) LeaveCriticalSection(&cs)
#else
	#define __ENTER(cs) // do nothing
	#define __LEAVE(cs) // do nothing
#endif//__SYNCRONIZE



bool isSessionOpened(char *id){
	map<string,int>::iterator itor;
	bool find = false;

	__ENTER(csSession);
	for(itor=session.begin();itor!=session.end();++itor){
		if(itor->first == string(id)){
			find = true;
			break;
		}
	}
	__LEAVE(csSession);
	return find;
}
void BroadcastArea(char *_area,int p,char *msg,int exclude=-1){
	vector<PER_HANDLE_DATA*> &Area = area[string(_area)];

	__ENTER(csArea);
	for(int i=0;i<Area.size();i++){
		if(Area[i]->n == exclude)
			continue;
		Send(Area[i]->n,p,msg);
	}
	__LEAVE(csArea);
}

/*
TODO : ������ �����ϴ� Ŭ���̾�Ʈ���Ը� �޼��� ������
*/
void onTankJoin(int w,char *msg){
	char *_area = msg;

	sprintf(clients[w]->user.area,_area);

	printf("%d joined to area %s\n",w, _area);

	__ENTER(csClients);
	clients[w]->user.x = 0;
	clients[w]->user.x = 0;
	clients[w]->user.speed = 8;

	__ENTER(csArea);
	//game.push_back(clients[w]);
	area[string(_area)].push_back(clients[w]);
	__LEAVE(csArea);

	char msg2[64];
	sprintf(msg2,"%d", w);
	BroadcastArea(_area,TANK_JOIN,msg2,w);
	/*for(int i=0;i<clients.size();i++){
		if(clients[i]->n == w) continue;
		Send(i,TANK_JOIN,msg2);
	}*/

	vector<PER_HANDLE_DATA*> &Area = area[string(_area)];
	for(int i=0;i<Area.size();i++){
		if(Area[i]->n == w) continue;
		sprintf(msg2,"%d", Area[i]->n);
		Send(w,TANK_JOIN,msg2);
		sprintf(msg2,"%d,%d,%d", Area[i]->n,Area[i]->user.x,Area[i]->user.y);
		Send(w,TANK_MOVE,msg2);
	}
	__LEAVE(csClients);
}
void onTankMove(int w,char *msg){
	char *x,*y;
	x = strtok(msg,",");
	y = strtok(NULL,",");

	__ENTER(csClients);
	clients[w]->user.x = atoi(x);
	clients[w]->user.y = atoi(y);
	__LEAVE(csClients);

	char msg2[16];
	sprintf(msg2,"%d,%s,%s", w,x,y);
	BroadcastArea(clients[w]->user.area,
				TANK_MOVE,msg2,w);
	/*for(int i=0;i<clients.size();i++){
		if(clients[i]->n == w) continue;
		Send(i,TANK_MOVE,msg2);
	}*/
	
}
void onTankLeave(int w,char *msg){
	char msg2[16];
	sprintf(msg2,"%d", w);

	printf("%d left area %s\n",w, clients[w]->user.area);

	vector<PER_HANDLE_DATA*> &Area =
			area[string(clients[w]->user.area)];

	__ENTER(csArea);
	for(int i=0;i<Area.size();i++){
		if(Area[i]->n == w){
			for(int j=i;j<Area.size()-1;j++)
				Area[j] = Area[j+1];
			Area.pop_back();
			break;
		}
	}
	__LEAVE(csArea);

	BroadcastArea(clients[w]->user.area,
				TANK_LEAVE,msg2,w);
	//__ENTER(csClients);
	/*for(int i=0;i<clients.size();i++){
		if(clients[i]->n == w) continue;
		Send(i,TANK_LEAVE,msg2);
	}*/
	//__LEAVE(csClients);
}

bool onConnect(int w,char *ip){

	return true;
}
void onDisconnect(int w,char *ip){
	char *id = clients[w]->user.id;
	printf("%s logged out\n", id);
	if(isSessionOpened(id)){
		printf("close session %d\n", session[string(id)]);

		map<string, int>::iterator  itor;

		__ENTER(csSession);
		itor = session.find(string(id));
		session.erase(itor);
		__LEAVE(csSession);
	}
}
void onLogin(int w,char *msg){
	char *id,*pw;
	id = strtok(msg,",");
	pw = strtok(NULL,",");
	
	// ������ �̹� ����
	if(isSessionOpened(id)){
		printf("%s already logged in..\n",id);
		Send(w,LOGIN_SESSION_EXIST,"already logged in");
		return;
	}

	char path[256];
	sprintf(path,"accounts\\%s", id);
	FILE *fp = fopen(path,"r");
	if(fp == NULL){
		Send(w,LOGIN_DENY,"out");
		return;
	}

	printf("login - %s\n", id);

	char rpw[128];
	fread(rpw,sizeof(char),128,fp);
	fread(
		clients[w]->user.nick,sizeof(char),32,fp);

	if(strcmp(rpw,pw)){
		Send(w,LOGIN_DENY,"wp");
		return;
	}

	sprintf(clients[w]->user.id,id);

	fclose(fp);

	__ENTER(csSession);
	session[string(id)] = w;
	__LEAVE(csSession);

	Send(w,LOGIN_ACCEPT,clients[w]->user.nick);
	sprintf(path,"accounts\\%s.png",id);
	SendFile(w,path);
}
void onSignup(int w,char *msg){
	char *id,*pw,*nick;
	id = strtok(msg,",");
	pw = strtok(NULL,",");
	nick = strtok(NULL,",");

	char path[256];
	sprintf(path,"accounts\\%s", id);
	FILE*fp = fopen(path,"r");
	if(fp != NULL){
		Send(w,SIGNUP_ERR_ID,"exist");
		fclose(fp);
		return;
	}
	fp = fopen(path,"w");
	if(fp == NULL){
		printf("failed to create account\n");
		Send(w,SIGNUP_ERR_ID,"err");
		return;
	}

	printf("new account : %s/%s\n", id,nick);

	// ���� ���� �Է�
	char wpw[128];
	char wnick[32];
	sprintf(wpw,pw);
	sprintf(wnick,nick);
	fwrite(wpw,sizeof(char),128,fp);
	fwrite(wnick,sizeof(char),32,fp);
	fclose(fp);

	// ������ ������ �����Ѵ�
	sprintf(path,"accounts\\%s.png",id);
	FILE *fpSrc = fopen("resource\\profile.png","rb");
	FILE *fpDst = fopen(path,"wb");
	char d;
	if(fpSrc == NULL || fpDst == NULL){
		if(fpSrc != NULL) fclose(fpSrc);
		if(fpDst != NULL) fclose(fpDst);

		printf("failed to copy profile image\n");
		Send(w,SIGNUP_ERR_UNKNOWN,"pi");
		return;
	}
	while(1){
		if(feof(fpSrc)){
			break;
		}
		char c = fgetc(fpSrc);
		fputc(c,fpDst);
	}
	fclose(fpSrc);
	fclose(fpDst);

	// �Ϸ��� �޼��� ����
	Send(w,SIGNUP_OK,"logged in");
}

void Initialize(){

	InitializeCriticalSection(&csSession);
	InitializeCriticalSection(&csClients);
	InitializeCriticalSection(&csArea);

	RegistHandler(LOGIN,onLogin);
	RegistHandler(SIGNUP,onSignup);

	RegistHandler(TANK_JOIN,onTankJoin);
	RegistHandler(TANK_MOVE,onTankMove);
	RegistHandler(TANK_LEAVE,onTankLeave);

	//area[string("main")].push_back(NULL);
}
void Quit(){
	DeleteCriticalSection(&csSession);
	DeleteCriticalSection(&csClients);
	DeleteCriticalSection(&csArea);
}


int main(int argc, char** argv)
{
        WSADATA wsaData;
        HANDLE hCompletionPort;      
        SYSTEM_INFO SystemInfo;
        SOCKADDR_IN servAddr;
        LPPER_IO_DATA PerIoData;
        LPPER_HANDLE_DATA PerHandleData;
 
        SOCKET hServSock;
        DWORD RecvBytes;
        int i;
        DWORD Flags;
 
        if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
               printf("startup error\n");
 
        DWORD dwProcessor;
       
        GetSystemInfo(&SystemInfo);
 
        dwProcessor = SystemInfo.dwNumberOfProcessors;
 
        hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, (dwProcessor * 2));
 
        for(i=0; i < (dwProcessor * 2); i++) {
               CreateThread(NULL, 0, CompletionThread, (LPVOID)hCompletionPort, 0, NULL);
               // _beginthreadex(NULL, 0, CompletionThread, (LPVOID)hCompletionPort, 0, NULL);
        }
 
        hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        servAddr.sin_family=AF_INET;
        servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
        servAddr.sin_port=htons(SERVER_PORT);
 
 
        if(bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
               printf("bind error\n");
			   return 1;
        }
 
        if(listen(hServSock, 5) == SOCKET_ERROR) {
               printf("listen error\n");
			   return 1;
        }
       
		printf("server ready\n");
 

		// ���α׷� �ʱ�ȭ
		Initialize();

		int N = 0;
        do
        {      
               SOCKET hClntSock;
               SOCKADDR_IN clntAddr;        
               int addrLen=sizeof(clntAddr);
			   bool ret;
              
			   // �� Ŭ���̾�Ʈ ������ �����Ѵ�.
               hClntSock=accept(hServSock, (SOCKADDR*)&clntAddr, &addrLen);  

			   printf("connected... %s - %d\n", inet_ntoa(clntAddr.sin_addr),N);

			   // onConnect �ڵ鷯�� ȣ���Ѵ�.
			   ret = onConnect(N,inet_ntoa(clntAddr.sin_addr));
			   if(ret == false){
				   // ���� �źε�
				   printf("connection denied\n");
				   continue;
			   }
              
               PerHandleData=(LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));          
               PerHandleData->hClntSock=hClntSock;
			   PerHandleData->n = N;
			   PerHandleData->fileRecv = false;
			   memset(&PerHandleData->user,0,sizeof(USER_DATA));
               memcpy(&(PerHandleData->clntAddr), &clntAddr, addrLen);
 
               CreateIoCompletionPort((HANDLE)hClntSock,hCompletionPort,(DWORD)PerHandleData,0);
              
               PerIoData = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
               memset(&(PerIoData->overlapped), 0, sizeof(OVERLAPPED));           
               PerIoData->wsaBuf.len = BUFSIZE;
               PerIoData->wsaBuf.buf = PerIoData->buffer;
               Flags=0;

			   clients.push_back(PerHandleData);
 
               WSARecv(PerHandleData->hClntSock,
                              &(PerIoData->wsaBuf),
                              1,                
                              &RecvBytes,                                 
                              &Flags,
                              &(PerIoData->overlapped),
                              NULL
                              );        
        }while(++N);

		WSACleanup();
		Quit();

        return 0;
}
 
DWORD WINAPI CompletionThread(LPVOID pComPort)
{
        HANDLE hCompletionPort =(HANDLE)pComPort;
        DWORD BytesTransferred;
 
        LPPER_HANDLE_DATA      PerHandleData;
        LPPER_IO_DATA          PerIoData;
       
        DWORD flags;
       
        while(1)
        {
               GetQueuedCompletionStatus(hCompletionPort,
										&BytesTransferred,
										(LPDWORD)&PerHandleData,
										(LPOVERLAPPED*)&PerIoData,
										INFINITE);
 
               if(BytesTransferred == 0)
               {
				   printf("closed %d\n",PerHandleData->n);

				   onDisconnect(
					   PerHandleData->n,
					   inet_ntoa(PerHandleData->clntAddr.sin_addr)
					   );

				   __ENTER(csClients);
				   for(int i=0;i<clients.size();i++){
					   if(clients[i] == PerHandleData){
						   for(int j=i;j<clients.size()-1;j++)
							   clients[j] = clients[j+1];
					   }
				   }
				   __LEAVE(csClients);

				   closesocket(PerHandleData->hClntSock);
				   free(PerHandleData);
				   free(PerIoData);
				   continue;             
               }             
               PerIoData->wsaBuf.len = BytesTransferred;
			   PerIoData->wsaBuf.buf[BytesTransferred] = '\0';

			   ParsePacket(PerHandleData->n,
							PerIoData->wsaBuf.buf,
							PerIoData->wsaBuf.len);

			   memset(&(PerIoData->overlapped), 0, sizeof(OVERLAPPED));
               PerIoData->wsaBuf.len=BUFSIZE;
               PerIoData->wsaBuf.buf=PerIoData->buffer;
 
               flags=0;
               WSARecv(PerHandleData->hClntSock,
                              &(PerIoData->wsaBuf),
                              1,
                              NULL,
                              &flags,
                              &(PerIoData->overlapped),
                              NULL
                              );      
               
        }
        return 0;
}
 
void Send(int w,int p,char *msg){
	char packet[1024];
	sprintf(packet,"%d:%s\r\n",
							p,msg);
	send(clients[w]->hClntSock,packet,strlen(packet),0);
}
void SendBinary(int w,int p,char *data,int len){
	char *packet;
	packet = (char *)malloc(sizeof(char) * len + 7);
	sprintf(packet,"%d:%s\r\n",
							p,data);
	send(clients[w]->hClntSock,packet,len + 6,0);
}
void SendFile(int w,char *file){
	//FILE *fp = fopen(file,"rb");
	HANDLE fp = CreateFileA(
						file,
						GENERIC_READ,
						FILE_SHARE_READ,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);
	char msg[32];

	if(fp == INVALID_HANDLE_VALUE){
		printf("failed to send file\n");
		Send(w,FILE_ERR,"error");
		return;
	}
	sprintf(msg,"%d", GetFileSize(fp,NULL));
	Send(w,FILE_LENGTH,msg);
	for(int i=strlen(file);i>=0;i--){
		if(file[i] == '\\'){
			sprintf(msg,"%s", file+i+1);
			printf("send file %s\n", msg);
			Send(w,FILE_NAME,msg);
			break;
		}
	}
	while(1){
		char buffer[128];
		DWORD dwRead;

		ReadFile(fp,buffer,128,&dwRead,NULL);
		send(clients[w]->hClntSock,buffer,dwRead,0);

		if(dwRead != 128)
			break;
	}

	CloseHandle(fp);
}

void ParsePacket(int w,char *msg,int msgLength){
	int read;
	vector<char *> token;
	token.push_back(strtok(msg,"\r\n"));

	// ���� �������̸�
	if(clients[w]->fileRecv == true){
		// ���� ��� �� ���� ���� ���� ��Ŷ���� Ŭ ��
		if((clients[w]->fileLength - clients[w]->fileWritten) > msgLength){
			fwrite(msg,sizeof(char),msgLength,clients[w]->filePointer);
			clients[w]->fileWritten += msgLength;
			return;
		}
		// ���� ��� �� ���� ���� ���� ��Ŷ���� ���� -> ������ŭ ���� ������ �Ľ�
		else{
			fwrite(msg,sizeof(char),
					clients[w]->fileLength - clients[w]->fileWritten,
					clients[w]->filePointer);
			fclose(clients[w]->filePointer);
			msg = msg + clients[w]->fileLength - clients[w]->fileWritten;
			clients[w]->fileRecv = false;
			clients[w]->fileReceived = true;

			if(clients[w]->fileLength - clients[w]->fileWritten == msgLength)
				return;
		}
	}

	// 1�� �Ľ�
	//   \r\n�� �������� ��Ŷ�� ������.
	char msg2[1024];
	memcpy(msg2,msg,sizeof(char) * msgLength);
	while(1){
		char *tok;
		tok = strtok(NULL,"\r\n");
		if(tok != NULL){
			token.push_back(tok);
		}
		else break;
	}

	// 2�� �Ľ�
	//   �ڵ��ȣ�� �޼����� ������.
	read = 0;
	for(int i=0;i<token.size();i++){
		int code;
		char *msg = NULL;
		for(int j=0;j<strlen(token[i]);j++){
			if(token[i][j] == ':'){
				token[i][j] = '\n';
				code = atoi(token[i]);
				msg = token[i] + j+1;
				break;
			}
		}
		read += strlen(token[i]) + 2;

		// ���� ���̸� ���۹޾��� ��
		if(code == FILE_LENGTH){
			clients[w]->fileLength = atoi(msg);
			printf("file length : %d\n", clients[w]->fileLength);
		}
		// ���� �̸��� ���۹޾��� �� -> ���� ������ �����Ѵ�.
		else if(code == FILE_NAME){
			sprintf(clients[w]->fileName,msg);
			printf("file name : %s\n", clients[w]->fileName);

			clients[w]->filePointer = fopen(clients[w]->fileName,"wb");
			if(clients[w]->filePointer == NULL){
				printf("failed to create file\n");
			}
			clients[w]->fileWritten = 0;
			clients[w]->fileRecv = true;

			if(i != token.size()-1){
				ParsePacket(w,msg2+read,msgLength-read);
				break;
			}
			break;
		}

		if(handler[code].handler != NULL){
			handler[code].handler(w,msg);
		}
	}
}
void RegistHandler(int code,void (*cb)(int,char *)){
	handler[code].handler = cb;
}