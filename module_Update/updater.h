#include <windows.h>
#include <urlmon.h>
#include <stdio.h>

struct DownloadList{
	char URL[100];									// 파일을 다운로드할 url
	char Local[100];								// 파일을 저장할 경로
	int Version;									// 이 파일의 버전 
};

class UPDATER{
public:
	
	void SetParam(LPCTSTR url,int Ver);				// 업데이트 정보를 설정한다

	BOOL Check();									// 업데이트를 체크한다

	int GetNewestVersion();							// 최신 버전을 구해온다 
	DownloadList *GetDownloadList();				// 업데이트 받을 항목들이 저장되있는 배열의 포인터를 얻어온다
	int GetDownloadListMax();						// 업데이트 받을 항목의 갯수를 알아온다

	BOOL Update(BOOL (*CallBack)(int, int, int));	// 업데이트한다
	
private:
	TCHAR URL[100];									// 업데이트 정보를 받아올 주소
	int Version;									// 현재 프로그램의 버전
	int NewVersion;									// 최신 버전
	DownloadList *d_list;						
	int d_list_max;
};