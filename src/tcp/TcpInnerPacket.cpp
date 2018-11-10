//------------------------------------------------------------------------------
//  TcpInnerPacket.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "TcpInnerPacket.h"

#include <string.h>
#include <stdio.h>

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

static got_packet_cb_t s_defaultGotPacketCb = [](const uint8_t *buf_in, size_t len) {};

static packet_buffer_t *
packet_buffer_alloc(packet_buffer_t **pp_free_list, size_t capacity_hint, int num_hint) {
	
	packet_buffer_t *entry = (*pp_free_list);

	if (NULL != entry) {
		(*pp_free_list) = entry->next;
		entry->size = 0;
	}
	else {
		packet_buffer_t *tmp;
		int i;
		for (i = 0; i < num_hint; ++i) {
			tmp = (packet_buffer_t *)malloc(capacity_hint);
			tmp->data = (uint8_t *)(tmp + 1);
			tmp->cap_size = capacity_hint - sizeof(packet_buffer_t);
			tmp->size = 0;

			tmp->next = (*pp_free_list);
			(*pp_free_list) = tmp;
		}
		entry = packet_buffer_alloc(pp_free_list, capacity_hint, num_hint);
	}
	return entry;
}

static void
packet_buffer_free(packet_buffer_t **pp_free_list, packet_buffer_t *entry) {
	entry->next = (*pp_free_list);
	(*pp_free_list) = entry;
}

static void
packet_buffer_real_free(packet_buffer_t **pp_free_list) {
	packet_buffer_t *tmp;
	while ((*pp_free_list)) {
		tmp = (*pp_free_list);
		(*pp_free_list) = tmp->next;
		free(tmp);
	}
}

const size_t s_RECV_PACKET_BUFFER_CAPACITY_HINT = FULL_PACKET_MAX_SIZE_HINT;
const size_t s_SEND_PACKET_BUFFER_CAPACITY_HINT = PER_FRAME_PAGE_SIZE_MAX * 2;

static packet_buffer_t *s_recv_packet_buffer_free_list = nullptr;
static packet_buffer_t *s_send_packet_buffer_free_list = nullptr;

//------------------------------------------------------------------------------
/**

*/
CTcpInnerPacket::CTcpInnerPacket(ITcpConn *pConn)
	: _refConn(pConn) {
	// default cb
	SetGotPacketCb(s_defaultGotPacketCb);

	memset(&_recv_page_leading, 0, sizeof(_recv_page_leading));
}

//------------------------------------------------------------------------------
/**

*/
CTcpInnerPacket::~CTcpInnerPacket() {
	Clear();
}

//------------------------------------------------------------------------------
/**

*/
void
CTcpInnerPacket::Clear() {
	
	_eRecvPageState = FRAME_PAGE_STATE_RECEIVE_LEADING;

	if (_recv_buf) {
		packet_buffer_free(&s_recv_packet_buffer_free_list, _recv_buf);
		_recv_buf = nullptr;
	}

	if (_send_buf) {
		packet_buffer_free(&s_send_packet_buffer_free_list, _send_buf);
		_send_buf = nullptr;
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CTcpInnerPacket::EncodeStream(uint64_t uInnerUuid, uint8_t uSerialNo, const char *sTypeName, size_t szTypeNameLen, size_t szBodyLen, int& nOutBodyOffset) {

	const int nDataSize = SIZE_OF_TCP_INNER_PACKET_LEADING + szTypeNameLen + szBodyLen;
	nOutBodyOffset = SIZE_OF_FRAME_PAGE_LEADING + SIZE_OF_TCP_INNER_PACKET_LEADING + szTypeNameLen;

	assert(nOutBodyOffset < _send_buf->cap_size);

	tcp_frame_page_leading_t *frame_page_leading = (tcp_frame_page_leading_t *)_send_buf->data;
	frame_page_leading->page_start = 1;
	frame_page_leading->page_end = 0;
	frame_page_leading->data_size = nDataSize;

	tcp_inner_packet_leading_t *packet_leading = (tcp_inner_packet_leading_t *)(frame_page_leading + 1);
	packet_leading->inner_name_len = szTypeNameLen;
	packet_leading->inner_serial_no = uSerialNo;
	packet_leading->inner_uuid = uInnerUuid;

	memcpy((void *)(packet_leading + 1), sTypeName, szTypeNameLen);
	_send_buf->size = nOutBodyOffset + szBodyLen;
}

//------------------------------------------------------------------------------
/**

*/
int
CTcpInnerPacket::DecodeStream(bip_buf_t& bb) {
	int nConsumed = ConsumeFramePage(
		(const uint8_t *)bip_buf_get_contiguous_block(&bb),
		bip_buf_get_committed_size(&bb),
		bip_buf_get_capacity(&bb));

	//
	bip_buf_decommit(&bb, nConsumed);
	return nConsumed;
}

//------------------------------------------------------------------------------
/**

*/
size_t
CTcpInnerPacket::Post(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {

	size_t sz = 0;
	size_t szTypeNameLen = sTypeName.length();
	size_t szBodyLen = sBody.length();

	_send_buf = packet_buffer_alloc(&s_send_packet_buffer_free_list, s_SEND_PACKET_BUFFER_CAPACITY_HINT, 1);

	// encode stream
	int nOffset = 0;
	EncodeStream(uInnerUuid, uSerialNo, sTypeName.c_str(), szTypeNameLen, szBodyLen, nOffset);

	// post
	const uint8_t *ptr = (uint8_t *)sBody.c_str();
	int nremains = (int)szBodyLen;
	int nsends;

	//
	tcp_frame_page_leading_t *tmp_page_leading = (tcp_frame_page_leading_t *)_send_buf->data;
	while (0 == tmp_page_leading->page_end) {

		nsends = std::min<int>(PER_FRAME_PAGE_SIZE_MAX - nOffset, nremains);
		nremains -= nsends;

		memcpy(_send_buf->data + nOffset, ptr, nsends);
		_send_buf->size = nOffset + nsends;

		tmp_page_leading->page_end = (nremains <= 0);
		tmp_page_leading->data_size = _send_buf->size - SIZE_OF_FRAME_PAGE_LEADING;

		sz += _refConn->SendRaw(_send_buf->data, (size_t)_send_buf->size);
		ptr += nsends;

		// for next page, page_start must be 0, and offset contains only page leading
		tmp_page_leading->page_start = 0;
		nOffset = SIZE_OF_FRAME_PAGE_LEADING;
	}

#ifdef _DEBUG
	//StatsSend( chPacket, nPacketLength);
#endif

	packet_buffer_free(&s_send_packet_buffer_free_list, _send_buf);
	_send_buf = nullptr;
	return sz;
}

//------------------------------------------------------------------------------
/**

*/
unsigned int
CTcpInnerPacket::ConsumeFramePage(const uint8_t *pBuf, int nLen, int nCapacity) {
	unsigned int nconsumed = 0;
	const uint8_t *ptr = pBuf;
	int nremains = nLen;
	int nwants, nskip;

	while (1) {
		switch (_eRecvPageState) {
		case FRAME_PAGE_STATE_RECEIVE_LEADING: {
			nwants = SIZE_OF_FRAME_PAGE_LEADING;
			if (nremains < nwants) {
				// hungry for leading
				goto loop_halt;
			}

			// got frame leading success
			_recv_page_leading = *(tcp_frame_page_leading_t *)ptr;
			if (nwants + (int)_recv_page_leading.data_size <= nCapacity) {
				// got header
				_eRecvPageState = FRAME_PAGE_STATE_RECEIVE_BODY;
				continue;
			}
			else {

				fprintf(stderr, "\n [CTcpOuterPacket::ConsumeFramePage()] page overflow(%d + %d > %d)!!!!!!!!\n",
					nwants, (int)_recv_page_leading.data_size, nCapacity);

				// page overflow
				nskip = nremains;
				_recv_page_leading.data_size -= (nskip - nwants);

				//
				_eRecvPageState = FRAME_PAGE_STATE_SKIP;
				ptr += nskip;
				nremains -= nskip;
				nconsumed += nskip;
				goto loop_halt;
			}
		}

		case FRAME_PAGE_STATE_RECEIVE_BODY: {
			nwants = SIZE_OF_FRAME_PAGE_LEADING + (int)_recv_page_leading.data_size;
			if (nremains < nwants) {
				// hungry for body
				goto loop_halt;
			}

			// got frame page body success, consume packet
			if (0 != ConsumePacket((const uint8_t *)(ptr + SIZE_OF_FRAME_PAGE_LEADING), (int)_recv_page_leading.data_size)) {
				// consume error
				goto loop_halt;
			}

			// 
			_eRecvPageState = FRAME_PAGE_STATE_RECEIVE_LEADING;
			ptr += nwants;
			nremains -= nwants;
			nconsumed += nwants;
			continue;
		}

		case FRAME_PAGE_STATE_SKIP: {
			nskip = std::min<int>(nremains, _recv_page_leading.data_size);
			_recv_page_leading.data_size -= nskip;

			if (0 >= _recv_page_leading.data_size) {
				// go on to process next message
				_eRecvPageState = FRAME_PAGE_STATE_RECEIVE_LEADING;
			}

			//
			ptr += nskip;
			nremains -= nskip;
			nconsumed += nskip;
			goto loop_halt;
		}

		default:
			break;
		}
	}

loop_halt:
	return nconsumed;
}

//------------------------------------------------------------------------------
/**

*/
int
CTcpInnerPacket::ConsumePacket(const uint8_t *pBuf, int nLen) {
	int rc = 0;
	tcp_inner_packet_leading_t *packet_leading;

	if (_recv_page_leading.page_start > 0
		&& _recv_page_leading.page_end > 0) {
		// process packet
		packet_leading = (tcp_inner_packet_leading_t *)pBuf;
		if (packet_leading->inner_name_len > 0
			&& nLen >= SIZE_OF_TCP_INNER_PACKET_LEADING + packet_leading->inner_name_len) {
			//
			_gotPacketCb(pBuf, nLen);
		}
		else {
			rc = -1;
		}
	}
	else if (_recv_page_leading.page_start > 0) {
		// it is first page, alloc new buffer
		_recv_buf = packet_buffer_alloc(&s_recv_packet_buffer_free_list, s_RECV_PACKET_BUFFER_CAPACITY_HINT, 1);

		assert(_recv_buf);
		if (_recv_buf->size + nLen <= _recv_buf->cap_size) {
			memcpy(_recv_buf->data + _recv_buf->size, pBuf, nLen);
			_recv_buf->size += nLen;
		}
		else {
			// recv buffer overflow
			fprintf(stderr, "\n [CTcpOuterPacket::ConsumePacket()] recv buffer overflow(%d + %d > %d)!!!!!!!!\n",
				(int)_recv_buf->size, nLen, (int)_recv_buf->cap_size);

			rc = -2;

			// free
			packet_buffer_free(&s_recv_packet_buffer_free_list, _recv_buf);
			_recv_buf = nullptr;
		}
	}
	else if (_recv_page_leading.page_end > 0) {
		// it is last page
		assert(_recv_buf);
		if (_recv_buf->size + nLen <= _recv_buf->cap_size) {
			memcpy(_recv_buf->data + _recv_buf->size, pBuf, nLen);
			_recv_buf->size += nLen;

			// process packet
			packet_leading = (tcp_inner_packet_leading_t *)_recv_buf->data;
			if (packet_leading->inner_name_len > 0
				&& _recv_buf->size >= SIZE_OF_TCP_INNER_PACKET_LEADING + packet_leading->inner_name_len) {
				//
				_gotPacketCb((const uint8_t *)_recv_buf->data, _recv_buf->size);
			}
			else {
				rc = -1;
			}

			// free
			packet_buffer_free(&s_recv_packet_buffer_free_list, _recv_buf);
			_recv_buf = nullptr;
		}
		else {
			// recv buffer overflow
			fprintf(stderr, "\n [CTcpOuterPacket::ConsumePacket()] recv buffer overflow(%d + %d > %d)!!!!!!!!\n",
				(int)_recv_buf->size, nLen, (int)_recv_buf->cap_size);

			rc = -2;

			// free
			packet_buffer_free(&s_recv_packet_buffer_free_list, _recv_buf);
			_recv_buf = nullptr;
		}
	}
	else {
		// it is middle page
		assert(_recv_buf);
		if (_recv_buf->size + nLen <= _recv_buf->cap_size) {
			memcpy(_recv_buf->data + _recv_buf->size, pBuf, nLen);
			_recv_buf->size += nLen;
		}
		else {
			// recv buffer overflow
			fprintf(stderr, "\n [CTcpOuterPacket::ConsumePacket()] recv buffer overflow(%d + %d > %d)!!!!!!!!\n",
				(int)_recv_buf->size, nLen, (int)_recv_buf->cap_size);

			rc = -2;

			// free
			packet_buffer_free(&s_recv_packet_buffer_free_list, _recv_buf);
			_recv_buf = nullptr;
		}
	}

	return rc;
}

//------------------------------------------------------------------------------
/**

*/
void
CTcpInnerPacket::InitPacketBuffer() {

	packet_buffer_t *tmp;

	// recv buf -- default 1
	tmp = packet_buffer_alloc(&s_recv_packet_buffer_free_list, s_RECV_PACKET_BUFFER_CAPACITY_HINT, 1);
	packet_buffer_free(&s_recv_packet_buffer_free_list, tmp);

	// send buf -- only need one
	tmp = packet_buffer_alloc(&s_send_packet_buffer_free_list, s_SEND_PACKET_BUFFER_CAPACITY_HINT, 1);
	packet_buffer_free(&s_send_packet_buffer_free_list, tmp);
}

//------------------------------------------------------------------------------
/**

*/
void
CTcpInnerPacket::DestroyPacketBuffer() {

	packet_buffer_real_free(&s_recv_packet_buffer_free_list);
	packet_buffer_real_free(&s_send_packet_buffer_free_list);
}

/* -- EOF -- */