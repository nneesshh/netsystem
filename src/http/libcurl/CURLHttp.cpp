//------------------------------------------------------------------------------
//  BankHandler.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "CURLHttp.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CCURLHttp::CCURLHttp() {
	/* init a multi stack */
	_multi_handle = curl_multi_init();

	_share_handle = curl_share_init();
	curl_share_setopt(_share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);

	_x_www_form_urlencoded_headers = nullptr;
	_x_www_form_urlencoded_headers = curl_slist_append(_x_www_form_urlencoded_headers, "Content-Type:application/x-www-form-urlencoded;charset:utf-8");
	_x_www_form_urlencoded_headers = curl_slist_append(_x_www_form_urlencoded_headers, "Accept:*/*");
	_x_www_form_urlencoded_headers = curl_slist_append(_x_www_form_urlencoded_headers, "Expect:");
}

//------------------------------------------------------------------------------
/**

*/
CCURLHttp::~CCURLHttp() {
	curl_multi_cleanup(_multi_handle);
	curl_share_cleanup(_share_handle);

	curl_slist_free_all(_x_www_form_urlencoded_headers);
}

//------------------------------------------------------------------------------
/**

*/
int
CCURLHttp::OnInit() {
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
void
CCURLHttp::OnDelete() {
	MultiClear();
}

//------------------------------------------------------------------------------
/**

*/
void
CCURLHttp::OnUpdate() {
	MultiExec(true);
}

//------------------------------------------------------------------------------
/**

*/
void *
CCURLHttp::AddRequest(const char *sURL, http_write_callback pCallback, void *pArg) {
	CURL *easy_handle = curl_easy_init();
	if (easy_handle) {
		curl_easy_setopt(easy_handle, CURLOPT_ERRORBUFFER, _curl_errbuf);
		curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy_handle, CURLOPT_FILETIME, 0L);
		curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 0L);

		curl_easy_setopt(easy_handle, CURLOPT_NOSIGNAL, 1L);

		curl_easy_setopt(easy_handle, CURLOPT_CONNECTTIMEOUT_MS, 0L);
		curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT_MS, 0L);

		curl_easy_setopt(easy_handle, CURLOPT_URL, sURL);
		curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, (http_write_callback)pCallback);
		curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, pArg);

		//
		SetShareHandle(easy_handle);

		//
		curl_easy_setopt(easy_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_NONE);
		curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, _x_www_form_urlencoded_headers);
	}

	//
	CURLMcode code = curl_multi_add_handle(_multi_handle, easy_handle);
	if (CURLM_OK == code) {
		return easy_handle;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
/**

*/
void *
CCURLHttp::AddRequest(
	const char *sURL,
	http_write_callback pCallback,
	void *pArg,
	const char *sPostData,
	unsigned int nPostDataSize) {

	CURL *easy_handle = curl_easy_init();
	if (easy_handle) {
		curl_easy_setopt(easy_handle, CURLOPT_ERRORBUFFER, _curl_errbuf);
		curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy_handle, CURLOPT_FILETIME, 0L);
		curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 0L);

		curl_easy_setopt(easy_handle, CURLOPT_NOSIGNAL, 1L);

		curl_easy_setopt(easy_handle, CURLOPT_CONNECTTIMEOUT_MS, 0L);
		curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT_MS, 0L);

		curl_easy_setopt(easy_handle, CURLOPT_URL, sURL);
		curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, (http_write_callback)pCallback);
		curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, pArg);

		//
		SetShareHandle(easy_handle);

		//
		curl_easy_setopt(easy_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_NONE);
		curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, _x_www_form_urlencoded_headers);

		curl_easy_setopt(easy_handle, CURLOPT_POST, 1L);
		curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, sPostData);
		curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDSIZE, nPostDataSize);
		// 		curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
	}

	//
	CURLMcode code = curl_multi_add_handle(_multi_handle, easy_handle);
	if (CURLM_OK == code) {
		return easy_handle;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
/**

*/
void
CCURLHttp::RemoveRequest(void *pHandle) {
	CURL *easy_handle = (CURL *)pHandle;
	curl_multi_remove_handle(_multi_handle, easy_handle);
	curl_easy_cleanup(easy_handle);
}
//------------------------------------------------------------------------------
/**

*/
void
CCURLHttp::SetShareHandle(CURL *easy_handle) {
	curl_easy_setopt(easy_handle, CURLOPT_SHARE, _share_handle);
	curl_easy_setopt(easy_handle, CURLOPT_DNS_CACHE_TIMEOUT, 60 * 5);
}

//------------------------------------------------------------------------------
/**

*/
int
CCURLHttp::MultiExec(bool bRetry) {
	/* we start some action by calling perform right away */
	int running_handle_count;	/* keep number of running handles */
	CURLMcode code = curl_multi_perform(_multi_handle, &running_handle_count);
	if (CURLM_OK != code) {
		printf("errcode=%d", code);
	}

	//
	_vRetryList.clear();

	/* See how the transfers went */
	CURLMsg *message; /* for picking up messages with the transfer status */
	int pending; /* how many messages are left */
	char *done_url;
	while ((message = curl_multi_info_read(_multi_handle, &pending))) {
		switch (message->msg) {
		case CURLMSG_DONE: {
			curl_easy_getinfo(message->easy_handle, CURLINFO_EFFECTIVE_URL, &done_url);

			switch (message->data.result) {
			case CURLE_OK: {
				RemoveRequest(message->easy_handle);
				break;
			}

			case CURLE_COULDNT_CONNECT: {
				// do nothing
				break;
			}

			case CURLE_SEND_ERROR:
			case CURLE_RECV_ERROR:
			case CURLE_OPERATION_TIMEDOUT: {
				// check retry
				if (bRetry) {
					curl_multi_remove_handle(_multi_handle, message->easy_handle);
					_vRetryList.push_back(message->easy_handle);	// save in retry list
				}
				else {
					RemoveRequest(message->easy_handle);
				}
				break;
			}

			default: {
				printf("[CCURLHttp::MultiExec()] !!!! failed !!!! -- !!!! result=%d, url=%s !!!!\n", message->data.result, done_url);
				break;
			}
			}

			break;
		}

		default: {
			fprintf(stderr, "CURLMSG default\n");
			abort();
		}
		}
	}

	if (bRetry) {
		for (auto& it : _vRetryList) {
			// try again
			curl_multi_add_handle(_multi_handle, it);
		}
	}
	return running_handle_count + _vRetryList.size();
}

//------------------------------------------------------------------------------
/**

*/
void
CCURLHttp::MultiClear() {
	int nMaxWaitLoops = 1000 * 1000;
	int nStillRunning = MultiExec(false);

	fprintf(stderr, "\n[CCURLHttp::MultiClear()] still_running=%d...\n", nStillRunning);

	while (nStillRunning && nMaxWaitLoops) {
		nStillRunning = MultiExec(false);
		--nMaxWaitLoops;
		Sleep(1);
	};
}
/* -- EOF -- */