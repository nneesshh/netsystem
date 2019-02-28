#pragma once
//------------------------------------------------------------------------------
/**
@class IPacket
Net IO handler for message

(C) 2016 n.lee
*/
#include <stdint.h>
#include <functional>

#include "../base/bip_buf.h"
#include "tcp_packet_def.h"

///
using packet_content_t = std::tuple<uint64_t, uint8_t, std::string, std::string>;
using got_packet_cb_t = std::function<void(const uint8_t *buf_in, size_t len)>;

//------------------------------------------------------------------------------
/**

*/
class IPacket {
public:
	virtual ~IPacket() noexcept {}

	virtual void				Clear() = 0;

	virtual void				EncodeStream(uint64_t uInnerUuid, uint8_t uSerialNo, const char *sTypeName, size_t szTypeNameLen, size_t szBodyLen, int& nOutBodyOffset) = 0;
	virtual int					DecodeStream(bip_buf_t& bb) = 0;
	virtual void				SetGotPacketCb(got_packet_cb_t cb) = 0;

	virtual size_t				Post(packet_content_t&& content) = 0;
};

/*EOF*/