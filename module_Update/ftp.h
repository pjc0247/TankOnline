#include <windows.h>
#include <wininet.h>

#define T(str) TEXT(str)
#define PROG_NAME ("Aurelia FTP")
#define UP_LEVEL T("..")

class FTP{
public:
	void SetParam(LPCTSTR Server,LPCTSTR User,LPCTSTR Pass);		//접속할 정보 설정
	BOOL Connect();													// 접속
	void UnConnect();												// 접속 끊기

	BOOL DownLoad(LPCTSTR remote,LPCTSTR local);					// 파일 다운로드
	BOOL UpLoad(LPCTSTR remote,LPCTSTR local);						// 업로드
	BOOL Cd(LPCTSTR cd);											// 작업 디덱토리 변경
	BOOL CreateDirectory(LPCTSTR dn);								// 디덱토리 생성

	BOOL IsConnected();												// 연결되있는지 알아옴
private:
	TCHAR p_Server[50]; 
	TCHAR p_User[20];  
	TCHAR p_Pass[20]; 

	HINTERNET hInternet, hFtp;	
};