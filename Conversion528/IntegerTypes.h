/* Copyright Zaurmann Software 2010 */
#ifndef _MY_INTEGER_TYPES_H_
#define _MY_INTEGER_TYPES_H_

#include "StaticAssertion.h"

	//64 bit architecture is not supported yet.

	typedef unsigned char	uint8;
	typedef signed char		int8;
	typedef uint8			byte;

	typedef unsigned short  uint16;
	typedef	signed short	int16;

	typedef unsigned int	uint32;
	typedef signed int		int32;

	typedef long long int	int64;

	CheckSize(uint8, 1);
	CheckSize(byte, 1);
	CheckSize(int8, 1);
	CheckSize(uint16, 2);
	CheckSize(int16, 2);
	CheckSize(uint32, 4);
	CheckSize(int32, 4);
	CheckSize(int64, 8);

#endif

