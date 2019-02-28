#pragma once
//------------------------------------------------------------------------------
/**
	@class IURLRequest

	(C) 2016 n.lee
*/
typedef size_t(*ssl_write_callback)(
	char *buffer,
	size_t size,
	size_t nitems,
	void *outstream);

//------------------------------------------------------------------------------
/**

*/
class IURLSSLRequest
{
public:
	virtual ~IURLSSLRequest() { }

	virtual int					OnInit() = 0;
	virtual void				OnDelete() = 0;
	virtual void				OnUpdate() = 0;

	virtual void *				AddRequest(
		const char *sURL,
		ssl_write_callback pCallback,
		void *pArg,
		const char *sPemCert,
		const char *sPemKey,
		const char *sPemPass,
		const char *sPostData,
		unsigned int nPostDataSize) = 0;

	virtual void *				AddRequest(
		const char *sURL,
		ssl_write_callback pCallback,
		void *pArg,
		const char *sPostData,
		unsigned int nPostDataSize) = 0;

	virtual void *				AddRequest(
		const char *sURL,
		ssl_write_callback pCallback,
		void *pArg) = 0;

	virtual void				RemoveRequest(void *pHandle) = 0;
};

/*EOF*/