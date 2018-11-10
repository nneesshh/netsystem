#pragma once
//------------------------------------------------------------------------------
/**
@class CCURLSSL

(C) 2016 n.lee
*/
#include <vector>

#include "../IURLSSLRequest.h"
#include "../UsingHttp.h"

//------------------------------------------------------------------------------
/**
@brief CCURLSSL
*/
class CCURLSSL : public IURLSSLRequest
{
public:
	CCURLSSL();
	virtual ~CCURLSSL();

	virtual int					OnInit();
	virtual void				OnDelete();

	virtual void				OnUpdate();

public:
	virtual void *				AddRequest(const char *sURL
		, ssl_write_callback pCallback
		, void *pArg
		, const char *sPemCert
		, const char *sPemKey
		, const char *sPemPass
		, const char *sPostData
		, unsigned int nPostDataSize);

	virtual void *				AddRequest(const char *sURL
		, ssl_write_callback pCallback
		, void *pArg
		, const char *sPostData
		, unsigned int nPostDataSize);

	virtual void *				AddRequest(const char *sURL
		, ssl_write_callback pCallback
		, void *pArg);

	void						RemoveRequest(void *pHandle);

private:
	void						SetShareHandle(CURL *easy_handle);

	int							MultiExec(bool bRetry);
	void						MultiClear();

private:
	CURLM						*_multi_handle;
	char						_curl_errbuf[CURL_ERROR_SIZE];

	CURLSH						*_share_handle;

	std::vector<CURL *>			_vRetryList;
	struct curl_slist			*_x_www_form_urlencoded_headers;
};

/*EOF*/