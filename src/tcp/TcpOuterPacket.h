#pragma once
//------------------------------------------------------------------------------
/**
@class CTcpOuterPacket
Net IO message handler

(C) 2016 n.lee
*/
#include "ITcpConn.h"
#include "IPacket.h"

//------------------------------------------------------------------------------
/**
@brief CTcpOuterPacket
*/
class CTcpOuterPacket : public IPacket {
public:
	CTcpOuterPacket(ITcpConn *pConn);
	virtual ~CTcpOuterPacket();

	virtual void				Clear();

	virtual void				EncodeStream(uint64_t uInnerUuid, uint8_t uSerialNo, const char *sTypeName, size_t szTypeNameLen, size_t szBodyLen, int& nOutBodyOffset);
	virtual int					DecodeStream(bip_buf_t& bb);

	virtual void				SetGotPacketCb(got_packet_cb_t cb) {
		_gotPacketCb = cb;
	}

	virtual size_t				Post(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody);

private:
	unsigned int				ConsumeFramePage(const uint8_t *pBuf, int nLen, int nCapacity);
	int							ConsumePacket(const uint8_t *pBuf, int nLen);

private:
	ITcpConn					*_refConn;
	got_packet_cb_t				_gotPacketCb;

	FRAME_PAGE_STATE			_eRecvPageState = FRAME_PAGE_STATE_RECEIVE_LEADING;
	tcp_frame_page_leading_t	_recv_page_leading;
	packet_buffer_t				*_recv_buf = nullptr;
	packet_buffer_t				*_send_buf = nullptr;

public:
	static void					InitPacketBuffer();
	static void					DestroyPacketBuffer();

};

/*EOF*/