#pragma once
//------------------------------------------------------------------------------
/**
@class CMyUtil

(C) 2016 n.lee
*/
#include <string>
#include <cmath>
#include <cstdlib>

//------------------------------------------------------------------------------
/**
@brief CMyUtil
*/
#include <algorithm>

template<typename T>
T clamp(T _min, T x, T _max) {
	return (std::max)((std::min)(x, _max), _min);
}

/*
*convert int to hex char.
*example:10 -> 'A',15 -> 'F'
*/
char int_to_hex_char(int x);

/*
*convert hex char to int.
*example:'A' -> 10,'F' -> 15
*/
int hex_char_to_int(char hex);

typedef unsigned char byte;

/*
*convert a byte array to hex string.
*hex string format example:"AF B0 80 7D"
*/
std::string bytes_to_hex_string(const byte *in, size_t size);

/*
*convert a hex string to a byte array.
*hex string format example:"AF B0 80 7D"
*/
size_t hex_string_to_bytes(const std::string &str, byte *out);

/*
*	split
*/
int split(const char *str, int str_len, char **av, int av_max, char c);

/*
*	split2d
*/
int split2d(const char *str, int str_len, char *(*av)[32], int av_max, char c1, char c2, int *out_arr_n2);

/*
*	split ip string into numbers, such as "192.168.1.1" to {192, 168, 1, 1}*
*/
typedef struct _ip_num_array_t {
	unsigned char _num[8];
} ip_num_array_t;

typedef struct _ip_pair_t {
	ip_num_array_t _ip;
	ip_num_array_t _mask;
} ip_pair_t;

int split_ip(const char *str, int str_len, ip_num_array_t *av);

/*
*	split formula into variables, such as "a > b" to {"a", "b", >}
*/
typedef enum _FORMULA_TEST_FLAG_tag {
	FORMULA_TEST_UNKNOWN = 0,
	FORMULA_TEST_EQUAL,
	FORMULA_TEST_NOT_EQUAL,
	FORMULA_TEST_LESS_THAN,
	FORMULA_TEST_GREATER_THAN,
	FORMULA_TEST_LESS_OR_EQ,
	FORMULA_TEST_GREATER_OR_EQ,

} FORMULA_TEST_FLAG;

typedef struct _simple_formula_t {
	char				_left[64];
	char				_right[64];
	FORMULA_TEST_FLAG	_test_flag;
} simple_formula_t;

int split_simple_formula(const char *formula, int formula_len, simple_formula_t *out);

/*
next_pot function
returns the Next Power of Two value.

Examples:
- If "value" is 15, it will return 16.
- If "value" is 16, it will return 16.
- If "value" is 17, it will return 32.
*/
unsigned long next_pot(unsigned long value);

/************************************************************************/
/*                                                                      */
/************************************************************************/
#define SLERP_XYZ( p0, p1, t, out )	\
	out.x = p0.x+(p1.x-p0.x)*t, out.y = p0.y+(p1.y-p0.y)*t, out.z = p0.z+(p1.z-p0.z)*t

#define  DISTANCE_XYZ_PWR( p0, p1, out )	\
	out = (p1.x-p0.x)*(p1.x-p0.x) + (p1.y-p0.y)*(p1.y-p0.y) + (p1.z-p0.z)*(p1.z-p0.z)

#define  DISTANCE_XYZ( p0, p1, out )	\
	out = sqrt((p1.x-p0.x)*(p1.x-p0.x) + (p1.y-p0.y)*(p1.y-p0.y) + (p1.z-p0.z)*(p1.z-p0.z))


/*
*
*/
size_t util_strlcpy(char* dst, const char* src, size_t size);
size_t util_strlcat(char* dst, const char* src, size_t size);

bool util_float_equal(float f1, float f2, float tolerance = FLT_EPSILON);
bool util_float_greater_than(float f1, float f2, float tolerance = FLT_EPSILON);
bool util_float_greater_or_eq(float f1, float f2, float tolerance = FLT_EPSILON);
bool util_float_less_than(float f1, float f2, float tolerance = FLT_EPSILON);
bool util_float_less_or_eq(float f1, float f2, float tolerance = FLT_EPSILON);

/*EOF*/
