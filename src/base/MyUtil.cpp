//------------------------------------------------------------------------------
//  MyUtil.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include <vector>
#include "MyUtil.h"

#include "snprintf/mysnprintf.h"

static void
string_replace_all(std::string& s, std::string const& t, std::string const& w) {
	std::string::size_type pos = s.find(t), t_size = t.size(), r_size = w.size();
	while (pos != std::string::npos) { // found   
		s.replace(pos, t_size, w);
		pos = s.find(t, pos + r_size);
	}
}

char
int_to_hex_char(int x) {
	static const char HEX[16] = {
		'0', '1', '2', '3',
		'4', '5', '6', '7',
		'8', '9', 'A', 'B',
		'C', 'D', 'E', 'F'
	};
	return HEX[x];
}

int
hex_char_to_int(char hex) {

	int nHex = toupper(hex);

	if (isdigit(nHex))
		return (hex - '0');
	if (isalpha(nHex))
		return (hex - 'A' + 10);
	return 0;
}

std::string
bytes_to_hex_string(const byte *in, size_t size) {
	std::string str;
	for (size_t i = 0; i < size; ++i) {
		int t = in[i];
		int a = t / 16;
		int b = t % 16;
		str.append(1, int_to_hex_char(a));
		str.append(1, int_to_hex_char(b));
		if (i != size - 1)
			str.append(1, ' ');
	}
	return str;
}

size_t
hex_string_to_bytes(const std::string &str, byte *out) {
	int nErrorNum = 0;
	try {
		std::vector<std::string> vec;

		nErrorNum++;

		std::string::size_type currPos = 0, prevPos = 0;
		while ((currPos = str.find(' ', prevPos)) != std::string::npos) {
			std::string b(str.substr(prevPos, currPos - prevPos));
			vec.emplace_back(b);
			prevPos = currPos + 1;
		}

		nErrorNum++;

		if (prevPos < str.size()) {
			std::string b(str.substr(prevPos));
			vec.emplace_back(b);
		}

		nErrorNum++;

		using sz_type = std::vector<std::string>::size_type;
		sz_type size = vec.size();

		nErrorNum++;

		for (sz_type i = 0; i < size; ++i) {
			int a = hex_char_to_int(vec[i][0]);
			int b = hex_char_to_int(vec[i][1]);
			out[i] = a * 16 + b;

			nErrorNum++;
		}

		nErrorNum++;

		return size;
	}
	catch (...) { //ADD 2009-6-7 BY ZM
		char temp[128] = { 0 };
		o_snprintf(temp, sizeof(temp), "Error: Util::hexStringToBytes() failed, nErrorNum:%d£¡\r\n", nErrorNum);
		//OutputDebugString(temp);
	}
	return 0;
}

int
split(const char *str, int str_len, char **av, int av_max, char c) {
	int i, j;
	char *ptr = (char*)str;
	int count = 0;

	if (!str_len) str_len = strlen(ptr);

	for (i = 0, j = 0; i < str_len&& count < av_max; ++i) {
		if (ptr[i] != c)
			continue;
		else
			ptr[i] = 0x0;

		av[count++] = &(ptr[j]);
		j = i + 1;
		continue;
	}

	if (j < i) av[count++] = &(ptr[j]);
	return count;
}

int
split2d(const char *str, int str_len, char *(*av)[32], int av_max, char c1, char c2, int *out_arr_n2) {
	char* arr1[256];
	int i, n1, n2;

	int av1_max = std::min<int>(av_max, 256);
	int av2_max = 32;

	n1 = split(str, str_len, arr1, av1_max, c1);
	for (i = 0;i < n1; ++i) {
		n2 = split(arr1[i], strlen(arr1[i]), av[i], av2_max, c2);
		out_arr_n2[i] = n2;
	}
	return n1;
}

int
split_ip(const char *str, int str_len, ip_num_array_t *av) {
	int i, j;
	char *ptr = (char*)str;
	int nAvCount = 0;
	int nMaxAv = sizeof(av->_num);

	if (!str_len) str_len = strlen(ptr);

	for (i = 0, j = 0; i < str_len&& nAvCount < nMaxAv; ++i) {
		if (ptr[i] != '.')
			continue;
		else
			ptr[i] = 0x0;

		av->_num[nAvCount++] = atoi(&(ptr[j]));
		j = i + 1;
		continue;
	}

	if (j < i)
		av->_num[nAvCount++] = atoi(&(ptr[j]));

	return nAvCount;
}

int
split_simple_formula(const char *formula, int formula_len, simple_formula_t *out) {
	int i, j, k = -1;
	char *ptr = (char*)formula;

	if (!formula_len) formula_len = strlen(ptr);

	for (i = 0, j = 0; i < formula_len; ++i) {
		if (j + 1 < sizeof(out->_left) && '>' != ptr[i] && '<' != ptr[i] && '=' != ptr[i] && '!' != ptr[i]) {
			// skip head blank
			if (j > 0 || (' ' != ptr[i] && '\t' != ptr[i]))
			{
				out->_left[j] = ptr[i];
				++j;
			}
			continue;
		}
		else {
			// trim tail blank
			out->_left[j] = '\0';
			while (j > 1 && (' ' == out->_left[j - 1] || '\t' == out->_left[j - 1])) {
				--j;
				out->_left[j] = '\0';
			}

			//
			if ('>' == ptr[i]) {
				if (i + 1 < formula_len && '=' == ptr[i + 1]) {
					out->_test_flag = FORMULA_TEST_GREATER_OR_EQ;
					k = i + 2;
				}
				else {
					out->_test_flag = FORMULA_TEST_GREATER_THAN;
					k = i + 1;
				}
			}
			else if ('<' == ptr[i]) {
				if (i + 1 < formula_len && '=' == ptr[i + 1]) {
					out->_test_flag = FORMULA_TEST_LESS_OR_EQ;
					k = i + 2;
				}
				else {
					out->_test_flag = FORMULA_TEST_LESS_THAN;
					k = i + 1;
				}
			}
			else if ('!' == ptr[i]) {
				if (i + 1 < formula_len && '=' == ptr[i + 1]) {
					out->_test_flag = FORMULA_TEST_NOT_EQUAL;
					k = i + 2;
				}
				else {
					out->_test_flag = FORMULA_TEST_UNKNOWN;
					k = i + 1;
				}
			}
			else if ('=' == ptr[i]) {
				if (i + 1 < formula_len && '=' == ptr[i + 1]) {
					out->_test_flag = FORMULA_TEST_EQUAL;
					k = i + 2;
				}
				else {
					out->_test_flag = FORMULA_TEST_EQUAL;
					k = i + 1;
				}
			}
			break;
		}
	}

	// go on to got right value
	out->_right[0] = '\0';
	for (i = k, j = 0; i < formula_len; ++i) {
		// skip head blank
		if (j > 0 || (' ' != ptr[i] && '\t' != ptr[i])) {
			out->_right[j] = ptr[i];
			++j;
		}
		continue;
	}

	// trim tail blank
	out->_right[j] = '\0';
	while (j > 1 && (' ' == out->_right[j - 1] || '\t' == out->_right[j - 1])) {
		--j;
		out->_right[j] = '\0';
	}

	return out->_test_flag;
}

// Round up to the next power of two
unsigned long
next_pot(unsigned long x) {
	--x;
	x |= x >> 1; // handle 2 bit numbers
	x |= x >> 2; // handle 4 bit numbers
	x |= x >> 4; // handle 8 bit numbers
	x |= x >> 8; // handle 16 bit numbers
	x |= x >> 16; // handle 32 bit numbers
	return ++x;
}

size_t
util_strlcpy(char* dst, const char* src, size_t size) {
	size_t n;

	if (size == 0)
		return 0;

	for (n = 0; n < (size - 1) && *src != '\0'; n++)
		*dst++ = *src++;

	*dst = '\0';

	return n;
}

size_t
util_strlcat(char* dst, const char* src, size_t size) {
	size_t n;

	if (size == 0)
		return 0;

	for (n = 0; n < size && *dst != '\0'; n++, dst++);

	if (n == size)
		return n;

	while (n < (size - 1) && *src != '\0')
		n++, *dst++ = *src++;

	*dst = '\0';

	return n;
}

bool util_float_equal(float f1, float f2, float tolerance) {
	float f = f1 - f2;
	return f < tolerance && f > -tolerance;
}

bool util_float_greater_than(float f1, float f2, float tolerance) {
	float f = f1 - f2;
	return f >= tolerance;
}

bool util_float_greater_or_eq(float f1, float f2, float tolerance) {
	float f = f1 - f2;
	return f > -tolerance;
}

bool util_float_less_than(float f1, float f2, float tolerance) {
	float f = f1 - f2;
	return f <= -tolerance;
}

bool util_float_less_or_eq(float f1, float f2, float tolerance) {
	float f = f1 - f2;
	return f < tolerance;
}

/* -- EOF -- */