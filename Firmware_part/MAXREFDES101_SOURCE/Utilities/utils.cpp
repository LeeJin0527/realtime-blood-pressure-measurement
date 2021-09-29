/***************************************************************************
* Copyright (C) 2017 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
****************************************************************************
*/

#include "utils.h"
#include <ctype.h>
#include "Peripherals.h"

/*
 * @brief Parse DeviceStudio get_reg command 
 * @details format is "get_reg <type> <addr>"
 *
 * @return 0 on success, -1 on failure
 */
int parse_get_reg_cmd(const char* str, const char* dev_type, uint8_t* addr)
{
	const char* num_start = str + strlen("get_reg") + strlen(dev_type) + 2;
	unsigned int addr32;

	int num_found = sscanf(num_start, "%x", &addr32);
	if (num_found == 1) {
		*addr = (uint8_t)addr32;
		return 0;
	} else {
		return -1;
	}
}

/*
 * @brief Parse DeviceStudio set_reg command 
 * @details format is "set_reg <type> <addr> <val>"
 *
 * @return 0 on success, -1 on failure
 */
int parse_set_reg_cmd(const char* str, const char* dev_type, uint8_t* addr, uint8_t* val)
{
	const char* num_start = str + strlen("set_reg") + strlen(dev_type) + 2;
	unsigned int addr32, val32;

	int num_found = sscanf(num_start, "%x %x", &addr32, &val32);
	if (num_found == 2) {
		*addr = (uint8_t)addr32;
		*val = (uint8_t)val32;
		return 0;
	} else {
		return -1;
	}
}
int parse_set_reg_cmd(const char* str, const char* dev_type, uint8_t* addr, uint16_t* val)
{
	const char* num_start = str + strlen("set_reg") + strlen(dev_type) + 2;
	unsigned int addr32, val32;

	int num_found = sscanf(num_start, "%x %x", &addr32, &val32);
	if (num_found == 2) {
		*addr = (uint8_t)addr32;
		*val = (uint16_t)val32;
		return 0;
	} else {
		return -1;
	}
}
int parse_set_reg_cmd(const char* str, const char* dev_type, uint8_t* addr, uint32_t* val)
{
	const char* num_start = str + strlen("set_reg") + strlen(dev_type) + 2;
	unsigned int addr32, val32;

	int num_found = sscanf(num_start, "%x %x", &addr32, &val32);
	if (num_found == 2) {
		*addr = (uint8_t)addr32;
		*val = val32;
		return 0;
	} else {
		return -1;
	}
}

/// TODO: additional for float
/*
int parse_cmd_data(const char* str, const char* cmd, float *vals, int vals_sz, bool hex)
{
	const char* sptr = str + strlen(cmd);
	int found = 0;
	int ssfound;
	unsigned int val32;

	while (found < vals_sz) {
		while (*sptr != ' ' && *sptr != '\0') { sptr++; }
		if (*sptr == '\0')
			break;
		sptr++;

		if (hex)
			ssfound = sscanf(sptr, "%x", &val32);
		else
			ssfound = sscanf(sptr, "%d", &val32);
		if (ssfound != 1)
			break;
		*(vals + found) = (uint8_t)val32;
		found++;
	}

	return found;
}
*/
int parse_cmd_data(const char* str, const char* cmd, uint8_t *vals, int vals_sz, bool hex)
{
	const char* sptr = str + strlen(cmd);
	int found = 0;
	int ssfound;
	unsigned int val32;

	while (found < vals_sz) {
		while (*sptr != ' ' && *sptr != '\0') { sptr++; }
		if (*sptr == '\0')
			break;
		sptr++;

		if (hex)
			ssfound = sscanf(sptr, "%x", &val32);
		else
			ssfound = sscanf(sptr, "%d", &val32);
		if (ssfound != 1)
			break;
		*(vals + found) = (uint8_t)val32;
		found++;
	}

	return found;
}

int parse_cmd_data(const char* str, const char* cmd, uint16_t *vals, int vals_sz, bool hex)
{
	const char* sptr = str + strlen(cmd);
	int found = 0;
	int ssfound;
	unsigned int val32;

	while (found < vals_sz) {
		while (*sptr != ' ' && *sptr != '\0') { sptr++; }
		if (*sptr == '\0')
			break;
		sptr++;

		if (hex)
			ssfound = sscanf(sptr, "%x", &val32);
		else
			ssfound = sscanf(sptr, "%d", &val32);
		if (ssfound != 1)
			break;
		*(vals + found) = (uint16_t)val32;
		found++;
	}

	return found;
}

int parse_cmd_data(const char* str, const char* cmd, uint32_t *vals, int vals_sz, bool hex)
{
	const char* sptr = str + strlen(cmd);
	int found = 0;
	int ssfound;

	while (found < vals_sz) {
		while (*sptr != ' ' && *sptr != '\0') { sptr++; }
		if (*sptr == '\0')
			break;
		sptr++;

		if (hex)
			ssfound = sscanf(sptr, "%x", vals + found);
		else
			ssfound = sscanf(sptr, "%d", vals + found);
		if (ssfound != 1)
			break;
		found++;
	}

	return found;
}

bool starts_with(const char* str1, const char* str2)
{
	while (*str1 && *str2) {
		if (*str1 != *str2)
			return false;
		str1++;
		str2++;
	}

	if (*str2)
		return false;

	return true;
}
