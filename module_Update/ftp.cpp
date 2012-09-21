#include "stdafx.h"
#include "ftp.h"

#pragma comment (lib,"wininet.lib")



void FTP::SetParam(LPCTSTR Server,LPCTSTR User,LPCTSTR Pass){
	wsprintf(p_Server,Server);
	wsprintf(p_User,User);
	wsprintf(p_Pass,Pass);
}
BOOL FTP::Connect(){
	hInternet=InternetOpenA(PROG_NAME, INTERNET_OPEN_TYPE_PRECONFIG,
		NULL, NULL, 0);
	if (hInternet == NULL) {
		return FALSE;
	}
	
	hFtp=InternetConnect(hInternet,p_Server,INTERNET_DEFAULT_FTP_PORT,
		p_User,p_Pass,INTERNET_SERVICE_FTP,0,0);
	if (hFtp==NULL) {
		InternetCloseHandle(hInternet);
		return FALSE;
	}
	return TRUE;
}

void FTP::UnConnect(){
	InternetCloseHandle(hFtp);
	InternetCloseHandle(hInternet);
	hFtp=NULL;
	hInternet=NULL;
}
BOOL FTP::DownLoad(LPCTSTR remote,LPCTSTR local){
	return FtpGetFile(hFtp,remote,local,FALSE,FILE_ATTRIBUTE_NORMAL,
		FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD,0);
}

BOOL FTP::UpLoad(LPCTSTR remote,LPCTSTR local){
	return FtpPutFile(hFtp,local,remote,
		FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD,0);
}
BOOL FTP::Cd(LPCTSTR cd){
	return FtpSetCurrentDirectory(hFtp,cd);
}
BOOL FTP::CreateDirectory(LPCTSTR dn){
	return FtpCreateDirectory(hFtp,dn);
}
BOOL FTP::IsConnected(){
	return hFtp?true:false;
}