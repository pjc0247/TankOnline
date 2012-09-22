#include "ibindcallback.h"

CBindTransferStatus::CBindTransferStatus()
{
}

CBindTransferStatus::~CBindTransferStatus()
{
}

STDMETHODIMP_(ULONG) CBindTransferStatus::AddRef()
{
        return 1;
}


STDMETHODIMP_(ULONG) CBindTransferStatus::Release()
{
        return 0;
}

STDMETHODIMP CBindTransferStatus::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
        if( iid == __uuidof(IBindStatusCallback) ) {
                *ppvObj = (IBindStatusCallback*)this;
                return S_OK;
        }
        else if( iid == __uuidof(IUnknown) ) {
                *ppvObj = (IUnknown*)this;
                return S_OK;
        }

        return E_NOINTERFACE;
}

// Obtains the priority for the bind operation when called by an asynchronous moniker.
STDMETHODIMP CBindTransferStatus::GetPriority(LONG* pnPriority)
{
        return E_NOTIMPL;
}


// Provides data to the client as it becomes available during asynchronous bind operations
STDMETHODIMP CBindTransferStatus::OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pfmtetc, STGMEDIUM* pstgmed)
{
        return E_NOTIMPL;
}


//  Not currently implemented
STDMETHODIMP CBindTransferStatus::OnLowResource(DWORD dwReserved)
{
        return E_NOTIMPL;
}


//      Passes the requested object interface pointer to the client.
STDMETHODIMP CBindTransferStatus::OnObjectAvailable(REFIID riid, IUnknown* punk)
{
        return E_NOTIMPL;
}


STDMETHODIMP CBindTransferStatus::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusCode)
{
       return pCallBack(ulProgress,ulProgressMax,idx);
}




STDMETHODIMP CBindTransferStatus::OnStartBinding(DWORD dwReserved, IBinding* pbinding)
{
        return E_NOTIMPL;
}


STDMETHODIMP CBindTransferStatus::OnStopBinding(HRESULT hrStatus, LPCWSTR szStatusText)
{
        return E_NOTIMPL;
}
STDMETHODIMP CBindTransferStatus::GetBindInfo(DWORD *pgrfBINDF, BINDINFO *pbindinfo){
	return E_NOTIMPL;
}

void CBindTransferStatus::SetIndex(int i){
	idx = i;
}
void CBindTransferStatus::RegistCallBack(BOOL (*CallBack)(int now, int total,int index)){
	pCallBack = CallBack;

}