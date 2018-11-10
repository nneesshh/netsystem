#pragma once

/**
* A type wide enough to hold the output of "socket()" or "accept()".  On
* Windows, this is an intptr_t; elsewhere, it is an int. */
#ifdef _WIN32
#define socket_t intptr_t
#else
#define socket_t int
#endif

#define TCP_STREAM_READ_SIZE 8192

/*EOF*/