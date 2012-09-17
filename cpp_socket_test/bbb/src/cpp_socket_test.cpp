// cpp_socket_test.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#include <WinSock2.h>
#include <Windows.h>

#include <vector>
#include <map>
using namespace std;

#include <SDL.h>
#include <SDL_gdiplus.h>
#include <SDL_ttf.h>

#include "drawtext.h"
#include "sprite.h"

#include "protocol.h"
#include "event.h"
#include "timer.h"

#include "sha1.h"

#include "resource.h"

#pragma comment (lib,"ws2_32")
#pragma comment (lib,"sdl")
#pragma comment (lib,"sdl_gdiplus")
#pragma comment (lib,"sdl_ttf")

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 9916

SOCKET hSocket;

typedef struct
{
	void (*handler)(char *);
} HANDLER;
typedef struct
{
	char id[32];
	char pw[128];
	char nick[32];

	int x,y;
	int speed;
} USER_DATA;

DWORD WINAPI ReceiveThread(LPVOID arg);
void ParsePacket(char *msg,int msgLength);
void RegistHandler(int code,void (*cb)(char *));
void SendBinary(int p,char *data,int len);
void Send(int p,char *msg);
void SendFile(int w,char *file);
void PushEvent(int type,void *data1=NULL,void *data2=NULL);

map<int,HANDLER> handler;
SDL_Window *window;
SDL_Renderer *renderer;
bool quit = false;

int fileReceived = false;
char fileName[256];

char tempString[256];

USER_DATA player;
map<int,USER_DATA*> user;


void onTankJoin(char *msg){
	printf("new tank %d\n", atoi(msg));
	int id = atoi(msg);
	user[id] = new USER_DATA;
	PushEvent(EVENT_TANK_JOIN);
}
void onTankMove(char *msg){
	int id,x,y;
	char *a;
	a = strtok(msg,",");
	id = atoi(a);
	a = strtok(NULL,",");
	x = atoi(a);
	a = strtok(NULL,",");
	y = atoi(a);

	user[id]->x = x;
	user[id]->y = y;
	PushEvent(EVENT_TANK_MOVE);
}
void onTankLeave(char *msg){
	printf("leave tank %d\n", atoi(msg));

	int id = atoi(msg);
	map<int,USER_DATA*>::iterator itor;

	itor = user.find(id);
	delete user[id];
	user.erase(itor);

	PushEvent(EVENT_TANK_LEAVE);
}

void SceneGame(){
	SDL_Event event;

	Sprite *tank = new Sprite("resource\\t1.png",1,1);

	player.x = 0;
	player.y = 0;
	player.speed = 8;

	Send(TANK_JOIN,"");

	quit = false;
	while(!quit){
		while(SDL_PollEvent(&event)){
			switch(event.type){
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				{
					switch(event.key.keysym.sym){
					case SDLK_LEFT:
						player.x -= player.speed;
						break;
					case SDLK_RIGHT:
						player.x += player.speed;
						break;
					case SDLK_UP:
						player.y -= player.speed;
						break;
					case SDLK_DOWN:
						player.y += player.speed;
						break;
					}

					char msg[256];
					sprintf(msg,"%d,%d",player.x,player.y);
					Send(TANK_MOVE,msg);
					break;
				}
			case EVENT_TANK_JOIN:
				break;
			case EVENT_TANK_MOVE:
				break;
			case EVENT_TANK_LEAVE:
				break;
			}
		}
		{
			SDL_RenderClear(renderer);
			SDL_RenderFillRect(renderer,NULL);

			map<int,USER_DATA*>::iterator itor;
			for(itor=user.begin();itor!=user.end();++itor){
				tank->Draw(
						itor->second->x,
						itor->second->y,
						200,100);
			}
			tank->Draw(player.x,player.y,200,100);
			
			SDL_RenderPresent(renderer);
		}

		SDL_Delay(1);
	}

	Send(TANK_LEAVE,"");
}

void onSignupErrId(char *msg){
	PushEvent(EVENT_SIGNUP_ERR_ID);
}
void onSignupErrUnknown(char *msg){
	PushEvent(EVENT_SIGNUP_ERR_UNKNOWN);
}
void onSignupOk(char *msg){
	PushEvent(EVENT_SIGNUP_OK);
}
void onLoginOk(char *msg){
	sprintf(tempString,msg);
	PushEvent(EVENT_LOGIN_OK,tempString);
}
void onLoginDeny(char *msg){
	PushEvent(EVENT_LOGIN_DENY);
}
void onLoginSessionExist(char *msg){
	PushEvent(EVENT_LOGIN_SESSION_EXIST);
}
void onLogoutOk(char *msg){
	PushEvent(EVENT_LOGOUT_OK);
}
void onLogoutErr(char *msg){
	PushEvent(EVENT_LOGOUT_ERR);
}

void Initialize(){
	RegistHandler(LOGIN_ACCEPT,onLoginOk);
	RegistHandler(LOGIN_DENY,onLoginDeny);
	RegistHandler(LOGIN_SESSION_EXIST,onLoginSessionExist);

	RegistHandler(LOGOUT_OK,onLoginDeny);
	RegistHandler(LOGOUT_ERR,onLoginDeny);

	RegistHandler(SIGNUP_OK,onSignupOk);
	RegistHandler(SIGNUP_ERR_ID,onSignupErrId);
	RegistHandler(SIGNUP_ERR_UNKNOWN,onSignupErrUnknown);

	RegistHandler(TANK_JOIN,onTankJoin);
	RegistHandler(TANK_MOVE,onTankMove);
	RegistHandler(TANK_LEAVE,onTankLeave);
}
void SetupRC(){
	window = SDL_CreateWindow(
						"Tank Online",
						SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
						480,272,
						SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(
						window,
						-1,
						0);
	TTF_Init();
}




BOOL CALLBACK SignupCallback(HWND hDlg,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	switch(iMessage)
	{
	case WM_INITDIALOG:
		{
			int x,y;
			RECT rect;
			SDL_GetWindowPosition(window,&x,&y);
			GetWindowRect(hDlg,&rect);
			MoveWindow(hDlg,
						x,y,
						rect.right-rect.left,rect.bottom-rect.top,
						true);
		}
		return true;
	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			{
				char id[32];
				char pw[32];
				char pw2[32];
				char nick[32];

				GetDlgItemTextA(hDlg,IDC_ID,id,32);
				GetDlgItemTextA(hDlg,IDC_PW,pw,32);
				GetDlgItemTextA(hDlg,IDC_PW2,pw2,32);
				GetDlgItemTextA(hDlg,IDC_NICK,nick,32);
				
				

				if(strcmp(pw,pw2)){
					MessageBoxA(NULL,"암호 똑같이좀 쳐","ㄷㄱ객",MB_OK | MB_ICONERROR);
				}
				else{
					char msg[256];
				
					SHA1Context sha;
					uint8_t result[20];

					SHA1Reset(&sha);
					SHA1Input(&sha, (const unsigned char *) pw,strlen(pw));
					SHA1Result(&sha,result);

					printf("%s\n", id);
					sprintf(msg,"%s,", id);
					for(int i=0;i<20;i++){
						char s[16];
						sprintf(s,"%02X", result[i]);
						strcat(msg,s);
					}
					strcat(msg,",");
					strcat(msg,nick);

					//printf("%s\n", msg);

					Send(SIGNUP,msg);

					EndDialog(hDlg,true);
				}
			}	
			return true;
		case IDCANCEL:
			{
				EndDialog(hDlg,false);
			}
			return false;
		}
		break;
	}
	return false;
}

void SceneLogin_Signup(){
	
	DialogBoxA(
		GetModuleHandle(NULL),
		MAKEINTRESOURCEA(IDD_SIGNUP),
		NULL,SignupCallback);
}
void SceneLogin_DoLogin(char *id,char *pw){
	char msg[256];
	SHA1Context sha;
	uint8_t result[20];

	SHA1Reset(&sha);
	SHA1Input(&sha, (const unsigned char *) pw,strlen(pw));
	SHA1Result(&sha,result);

	sprintf(msg,"%s,",id);
	for(int i=0;i<20;i++){
		char s[16];
		sprintf(s,"%02X", result[i]);
		strcat(msg,s);
	}

	Send(LOGIN,msg);
}
void SceneLogin(){
	SDL_Event event;
		
	Sprite *bgi = new Sprite("resource\\bgi_login.jpg",1,1);
	Sprite *profile = NULL;

	FONT gulim;
	
	Timer t_login;
	Timer t_result;
	Timer t_cursor;

	int login_animation = 0;
	int result_animation = 0;

	char id[32] = "\0";
	char pw[32] = "\0";
	char *input;
	int id_cur = 0,pw_cur = 0;
	int cursor = 0;
	int cursor_animation = 0;

	int logined = 0;

	int nextScene = 0;

	gulim.InitFont("c:\\windows\\fonts\\gulim.ttc",20);
	gulim.SetColor(0,0,0);
	
	t_login.SetInterval(30);
	t_login.start();
	t_cursor.SetInterval(30);
	t_result.SetInterval(30);

	input = id;

	quit = false;
	while(!quit){
		while(SDL_PollEvent(&event)){
			switch(event.type){
			case SDL_QUIT:
				quit = true;
				break;
	
			case SDL_MOUSEBUTTONDOWN:
				{
					SceneLogin_Signup();
				}
				break;
			// 입력 처리
			case SDL_KEYDOWN:
				if(cursor < 2){
					if(event.key.keysym.sym == SDLK_RETURN){
						cursor ++;
						input = pw;
					}
					else if(event.key.keysym.sym == SDLK_BACKSPACE
							&&
							strlen(input) > 0){
						int len = strlen(input);
						input[len-1] = '\0';
					}
					else{
						if((event.key.keysym.sym >= 'a' && event.key.keysym.sym <= 'z')
							||
							(event.key.keysym.sym >= 'A' && event.key.keysym.sym <= 'Z')
							||
							(event.key.keysym.sym >= '0' && event.key.keysym.sym <= '9')){
							int len = strlen(input);
							input[len] = event.key.keysym.sym;
							input[len+1] = '\0';
						}
					}
				}
				else{
					if(event.key.keysym.sym == SDLK_SPACE){
						quit = true;
						nextScene = 1;
					}
				}
				break;

			case EVENT_LOGIN_OK:
				{
					printf("login ok\n");

					t_result.start();

					// 프로필 이미지 수신이 완료될때까지 대기
					printf("1fileRecv %d\n", fileReceived);
					while(fileReceived == false);
					printf("2fileRecv %d\n", fileReceived);
					fileReceived = false;
					// 로그인 상태를 변경 -> 성공
					logined = 1;

					// 프로필 이미지를 로드
					char path[256];
					sprintf(path,fileName);
					profile = new Sprite(path,1,1,1,1,1);

					sprintf(player.id,id);
					sprintf(player.nick,(char*)event.user.data1);

					printf("%s\n", event.user.data1);

					break;
				}
			case EVENT_LOGIN_DENY:
				printf("login denied\n");
				t_result.start();
				logined = -1;
				break;
			case EVENT_LOGIN_SESSION_EXIST:
				printf("already logged in\n");
				t_result.start();
				logined = -2;
				break;
			case EVENT_LOGOUT_OK:
				printf("logout ok\n");
				break;
			case EVENT_LOGOUT_ERR:
				printf("logout err\n");
				break;
			case EVENT_SIGNUP_OK:
				printf("signup ok\n");
				break;
			case EVENT_SIGNUP_ERR_ID:
				printf("signup err\n");
				break;
			case EVENT_SIGNUP_ERR_UNKNOWN:
				printf("signup err\n");
				break;
			}
		}
		if(t_login.done()){
			if(cursor < 2){
				if(login_animation <= 70)
					login_animation += 2;
			}
			else{
				if(login_animation != 0)
					login_animation -= 2;
				else{
					SceneLogin_DoLogin(id,pw);
					t_login.stop();
				}
			}
			gulim.SetColor(
					255-login_animation*3 - 40,
					255-login_animation*3 - 40,
					255-login_animation*3 - 40);


		}
		if(t_result.done()){
			if(result_animation != 200)
				result_animation += 5;
		}
		if(t_cursor.done()){

		}
		{
			bgi->Draw(0,0,480,272);

			if(logined == 0){
				SDL_Rect rt = {250,170,200,login_animation};
				SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
				SDL_SetRenderDrawColor(renderer,32,64,255,128);
				SDL_RenderFillRect(renderer,&rt);

				SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);

				gulim.SetStyle(TTF_STYLE_BOLD);
				gulim.Draw(renderer,"ID : ",260,180);
				gulim.Draw(renderer,"PW: ",260,210);

				gulim.SetStyle(TTF_STYLE_NORMAL);
				gulim.Draw(renderer,id,310,180);
				for(int i=0;i<strlen(pw);i++){
					gulim.Draw(renderer,"*",310 + i*10,210);
				}
			}
			else{
				SDL_Rect rt = {150,30,200,result_animation};
				SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
				SDL_SetRenderDrawColor(renderer,32,64,255,128);
				SDL_RenderFillRect(renderer,&rt);

				SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);

				if(profile != NULL)
					profile->Draw(185,50,130,100);

				gulim.SetStyle(TTF_STYLE_BOLD);
				if(logined == 1){
					gulim.Draw(renderer,player.nick,235,150);
					gulim.Draw(renderer,"입장",230,200);
				}
				else if(logined == -1){
					gulim.Draw(renderer,"로그인실패",195,130);
					gulim.Draw(renderer,"확인",230,200);
				}
				else {
					gulim.Draw(renderer,"이미 접속중",195,130);
					gulim.Draw(renderer,"확인",230,200);
				}
				
			}

			SDL_RenderPresent(renderer);
		}
		

		SDL_Delay(1);
	}
	
	delete bgi;
	if(profile != NULL)
		delete profile;

	if(nextScene == 1)
		SceneGame();
}


int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	SOCKADDR_IN servAddr;
	HANDLE hThread;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		fputs("WSAStartup() error", stderr);

	hSocket = socket(PF_INET, SOCK_STREAM, 0);
	if(hSocket == INVALID_SOCKET)
		printf("cannot create socket\n");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(
										SERVER_ADDR);
	servAddr.sin_port = htons(
										SERVER_PORT);

	if(connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR){
		printf("cannot connect to server\n");
		MessageBoxA(NULL,"서버와에 연결할 수 없습니다..","error",
				MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
		return 0;
	}

	hThread = CreateThread(NULL,NULL,ReceiveThread,NULL,NULL,NULL);

	Initialize();
	SetupRC();

	SceneLogin();

	closesocket(hSocket);
	WSACleanup();



	return 0;
}


void PushEvent(int type,void *data1,void *data2){
	SDL_Event event;

	event.type = type;
	event.user.data1 = data1;
	event.user.data2 = data2;

	SDL_PushEvent(&event);
}
void Send(int p,char *msg){
	char packet[1024];
	sprintf(packet,"%d:%s\r\n",
							p,msg);
	send(hSocket,packet,strlen(packet),0);
}
void SendBinary(int p,char *data,int len){
	char *packet;
	packet = (char *)malloc(sizeof(char) * len + 7);
	sprintf(packet,"%d:%s\r\n",
							p,data);
	send(hSocket,packet,len + 6,0);
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
		Send(FILE_ERR,"error");
		return;
	}
	sprintf(msg,"%d", GetFileSize(fp,NULL));
	Send(FILE_LENGTH,msg);
	for(int i=strlen(file);i>=0;i--){
		if(file[i] == '\\'){
			sprintf(msg,"%s", file+i+1);
			printf("send file %s\n", msg);
			Send(FILE_NAME,msg);
			break;
		}
	}
	while(1){
		char buffer[128];
		DWORD dwRead;

		ReadFile(fp,buffer,128,&dwRead,NULL);
		send(hSocket,buffer,dwRead,0);

		if(dwRead != 128)
			break;
	}

	CloseHandle(fp);
}
DWORD WINAPI ReceiveThread(LPVOID arg){
	while(1){
		char buffer[1025];
		int len;
		len = recv(hSocket,buffer,1024,0);

		if(len == -1){
			MessageBoxA(NULL,"서버와의 연결이 끊겼습니다.","error",
				MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
			break;
		}
		
		buffer[len] = '\0';

		ParsePacket(buffer,len);
	}
	return 0;
}

void ParsePacket(char *msg,int msgLength){
	static int fileLength = 0;
	//static char fileName[256] = "\0";
	static bool fileRecv = false;
	static FILE *filePointer = NULL;
	static int fileWritten = 0;

	int read;
	vector<char *> token;

	if(fileRecv == true){
		// 남은 써야 될 양이 현재 받은 패킷보다 클 때
		if((fileLength - fileWritten) > msgLength){
		//	printf("a1 %d , %d\n",(fileLength - fileWritten),msgLength);
			fwrite(msg,sizeof(char),msgLength,filePointer);
			fileWritten += msgLength;
			return;
		}
		// 남은 써야 될 양이 현재 받은 패킷보다 작음 -> 남은만큼 쓰고 나머지 파싱
		else{
	//		printf("a2 \n");
			fwrite(msg,sizeof(char),
					fileLength - fileWritten,filePointer);
			fclose(filePointer);
			msg = msg + fileLength - fileWritten;
			fileRecv = false;
			fileReceived = true;

			printf("file received %s\n", fileName);

			if((fileLength - fileWritten) == msgLength){
				//printf("ASDF\n");
				return;
			}
		}
	}

	// 1차 파싱
	//   \r\n을 기준으로 패킷을 나눈다.
	char msg2[1024];
	memcpy(msg2,msg,sizeof(char) * msgLength);
	token.push_back(strtok(msg,"\r\n"));
	while(1){
		char *tok;
		tok = strtok(NULL,"\r\n");
		if(tok != NULL){
			token.push_back(tok);
		}
		else break;
	}

	printf("msg : %s\n", msg);

	// 2차 파싱
	//   코드번호와 메세지를 나눈다.
	read = 0;
	for(int i=0;i<token.size();i++){
		int code;
		char *msg = NULL;

		//printf("%d |%s|\n",i, token[i]);
		for(int j=0;j<strlen(token[i]);j++){
			if(token[i][j] == ':'){
				token[i][j] = '\n';
				code = atoi(token[i]);
				msg = token[i] + j+1;	
				break;
			}
		}
		read += strlen(token[i]) + 2;

		// 파일 길이를 전송받았을 때
		if(code == FILE_LENGTH){
			fileLength = atoi(msg);
			printf("file length : %d\n", fileLength);
		}
		// 파일 이름을 전송받았을 때 -> 파일 수신을 시작한다.
		else if(code == FILE_NAME){
			sprintf(fileName,msg);
			printf("file name : %s\n", fileName);

			filePointer = fopen(fileName,"wb");
			if(filePointer == NULL){
				printf("failed to create file\n");
			}
			fileWritten = 0;
			fileRecv = true;

			if(i != token.size()-1){
				ParsePacket(msg2+read,msgLength-read);
				/*fwrite(msg2+read,sizeof(char),msgLength-read,filePointer);
				fileWritten = msgLength - read;
				if(fileWritten >= fileLength){
					fclose(filePointer);

					fileRecv = false;
					fileReceived = true;
					printf("file received %s\n", fileName);

				}*/
				
			/*	int j;
				for(j=i;j<token.size();j++){
					int len = strlen(msg);
					fwrite(msg,sizeof(char),len,filePointer);
					fileWritten += len;
					if(fileWritten >= fileLength){
						break;
					}
				}
				i += (j-i);
				continue*/
				break;
			}
			break;
		}

		if(handler[code].handler != NULL){
			//printf("%s\n", msg);
			handler[code].handler(msg);
		}
	}
}
void RegistHandler(int code,void (*cb)(char *)){
	handler[code].handler = cb;
}