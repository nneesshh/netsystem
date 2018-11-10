#pragma once
//------------------------------------------------------------------------------
/**
@class CCURLHttp

(C) 2016 n.lee
*/
#include <vector>

#include "../IURLHttpRequest.h"
#include "../UsingHttp.h"

//------------------------------------------------------------------------------
/**
@brief CCURLHttp
*/
class CCURLHttp : public IURLHttpRequest
{
public:
	CCURLHttp();
	virtual ~CCURLHttp();

	virtual int					OnInit();
	virtual void				OnDelete();

	virtual void				OnUpdate();

public:
	virtual void *				AddRequest(const char *sURL, http_write_callback pCallback, void *pArg);

	virtual void *				AddRequest(
		const char *sURL,
		http_write_callback pCallback,
		void *pArg,
		const char *sPostData,
		unsigned int nPostDataSize);

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