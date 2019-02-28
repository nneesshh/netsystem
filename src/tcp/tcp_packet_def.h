#pragma once
//------------------------------------------------------------------------------
/**
    @class CTcpPacketDef
    
    (C) 2016 n.lee
*/
#include <stdint.h>

#include "tcp_def.h"

#ifdef __cplusplus 
extern "C" { 
#endif

//////////////////////////////////////////////////////////////////////////
/// frame page state
enum FRAME_PAGE_STATE {
	FRAME_PAGE_STATE_RECEIVE_LEADING = 1,
	FRAME_PAGE_STATE_RECEIVE_BODY,
	FRAME_PAGE_STATE_SKIP,
};

//////////////////////////////////////////////////////////////////////////
/// packet state
enum PACKET_STATE {
	PACKET_STATE_PROCESS = 1,
	PACKET_STATE_PROCESS_OK,
	PACKET_STATE_ABORT,
};

///
struct packet_buffer_t {
	uint8_t *data;
	size_t   size;
	size_t   cap_size;

	struct packet_buffer_t	*next;
};

#pragma pack(1)

// full_packet = frame_page1 + frame_page2 + ... 
// frame_page = frame_page_leading + page(first_page/non-first_page)
// first_page = packet_leading + name + data
// non-first_page = data

#define PER_FRAME_PAGE_SIZE_MAX		TCP_STREAM_READ_SIZE
#define FULL_PACKET_MAX_SIZE_HINT	0xF0000	// 0x40000, 256k, 262144? 0x80000, 512k, 524288? 0xF0000, 960K, 983040? include frame page header and packet buffer entry header, ...

#define	SIZE_OF_FRAME_PAGE_LEADING	2
struct tcp_frame_page_leading_t {
	uint16_t page_start:1; // 1 = first page, 0 = more pages after first page
	uint16_t page_end:1; // 1 = first page, 0 = more pages after first page
	uint16_t data_size:14; // page data size
};

#define SIZE_OF_TCP_OUTER_PACKET_LEADING	2 // sizeof(tcp_outer_packet_leading_t)
struct tcp_outer_packet_leading_t {
	uint16_t name_len:8; // 256
	uint16_t serial_no:8;
};

#define SIZE_OF_TCP_INNER_PACKET_LEADING	16 // sizeof(tcp_inner_packet_leading_t)
struct tcp_inner_packet_leading_t {
	uint32_t inner_name_len:8; // 256
	uint32_t inner_serial_no:8;
	uint32_t _1:16; // skip
	uint32_t _2; // skip
	uint64_t inner_uuid;
};

#pragma pack()
#ifdef __cplusplus 
} 
#endif 

/*EOF*/