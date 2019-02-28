#pragma once
//------------------------------------------------------------------------------
/**
	@class CTcpOuterPacket
	Net IO message handler

	(C) 2016 n.lee
*/
#include <deque>

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

	virtual void				Clear() override;

	virtual void				EncodeStream(uint64_t uInnerUuid, uint8_t uSerialNo, const char *sTypeName, size_t szTypeNameLen, size_t szBodyLen, int& nOutBodyOffset) override;
	virtual int					DecodeStream(bip_buf_t& bb) override;

	virtual void				SetGotPacketCb(got_packet_cb_t cb) override {
		_gotPacketCb = cb;
	}

	virtual size_t				Post(packet_content_t&& content) override;

private:
	int							ConsumeFramePage(const uint8_t *pBuf, int nLen, int nCapacity);
	int							ConsumePacket(const uint8_t *pBuf, int nLen);

private:
	ITcpConn *_refConn;
	got_packet_cb_t _gotPacketCb;

	FRAME_PAGE_STATE _eRecvPageState = FRAME_PAGE_STATE_RECEIVE_LEADING;
	tcp_frame_page_leading_t _recv_page_leading;

	//std::deque<packet_content_t> _dqPost;

	packet_buffer_t *_ref_recv_buf = nullptr;
	packet_buffer_t *_ref_send_buf = nullptr;

public:
	static void					InitPacketBuffer();
	static void					DestroyPacketBuffer();

};

/*EOF*/