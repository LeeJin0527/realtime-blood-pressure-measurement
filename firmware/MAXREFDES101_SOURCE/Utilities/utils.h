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


#ifndef __UTILS__H_
#include <mbed.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)        (sizeof(arr)/sizeof(arr[0]))
#endif

/*
 * @brief Parse DeviceStudio get_reg command 
 * @details format is "get_reg <type> <addr>"
 *
 * @param[in] str - Pointer to start of command
 * @param[in] dev_type - device type, ie "ppg"
 * @param[in] addr - Parsed address
 *
 * @return 0 on success, -1 on failure
 */
int parse_get_reg_cmd(const char* str, const char* dev_type, uint8_t* addr);

/*
 * @brief Parse DeviceStudio set_reg command 
 * @details format is "set_reg <type> <addr> <val>"
 *
 * @param[in] str - Pointer to start of command
 * @param[in] dev_type - device type, ie "ppg"
 * @param[in] addr - Parsed address
 * @param[in] val - Parsed value
 *
 * @return 0 on success, -1 on failure
 */
int parse_set_reg_cmd(const char* str, const char* dev_type, uint8_t* addr, uint8_t* val);
int parse_set_reg_cmd(const char* str, const char* dev_type, uint8_t* addr, uint16_t* val);
int parse_set_reg_cmd(const char* str, const char* dev_type, uint8_t* addr, uint32_t* val);

/*
 * @brief Parse data values sent by DeviceStudio command
 * @details For a command format of "<cmd> <val1> <val2> ... <valN>",
 * 			This command will parse val1 - valN into vals array
 *
 * @param[in] str - The full string received
 * @param[in] cmd - The command only
 * @param[out] vals - The output array of values
 * @param[in] vals_sz - The maximum number of values the output array vals can hold
 * @param[in] hex - Set to true for hexidecimal values, false for decimal
 *
 * @return -1 on error, otherwise the number of values parsed
 */
int parse_cmd_data(const char* str, const char* cmd, uint8_t *vals, int vals_sz, bool hex);
int parse_cmd_data(const char* str, const char* cmd, uint16_t *vals, int vals_sz, bool hex);
int parse_cmd_data(const char* str, const char* cmd, uint32_t *vals, int vals_sz, bool hex);

/*
 * @brief Determine if str2 is a substring of str1 beginning at idx 0
 *
 * @param[in] str1 - The parent string
 * @param[in] str2 - The substring which should exist starting at index 0 of str1
 *
 * @return true if str1 starts with str2
 *
 * Examples:
 * 	str1 = "An apple", str2 = "An a", returns true
 * 	str1 = "A dog", str2 = "A a", returns false
 * 	str1 = "An apple", str2 = "An apple tree", returns false
 */
bool starts_with(const char* str1, const char* str2);

#endif
