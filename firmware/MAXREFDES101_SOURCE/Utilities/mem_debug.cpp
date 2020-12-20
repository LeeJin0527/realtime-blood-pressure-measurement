/*******************************************************************************
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
 *******************************************************************************
 */

#include "mbed.h"

#if defined(MBED_MEM_TRACING_ENABLED )
#include "mbed_mem_trace.h"
#endif /* MBED_MEM_TRACING_ENABLED */

#if defined(MBED_HEAP_STATS_ENABLED)
#include "mbed_stats.h"
#endif /* MBED_HEAP_STATS_ENABLED */

#if defined(MBED_STACK_STATS_ENABLED)
#include "cmsis_os.h"
#endif /* MBED_STACK_STATS_ENABLED */


void mbed_mem_debug_init()
{
#if defined(MBED_MEM_TRACING_ENABLED )
	mbed_mem_trace_set_callback(mbed_mem_trace_default_callback);
#endif /* MBED_MEM_TRACING_ENABLED */
}

#if defined(MBED_HEAP_STATS_ENABLED)
void mbed_heap_stats(const char* func, int line)
{
	mbed_stats_heap_t heap_stats;
	mbed_stats_heap_get(&heap_stats);
	printf("  HEAP current: %lu, max: %lu, cumulative: %lu bytes (%s:%d)\n",
	       heap_stats.current_size,
	       heap_stats.max_size,
	       heap_stats.total_size,
	       func, line);
}
#endif /* MBED_HEAP_STATS_ENABLED */

#if defined(MBED_STACK_STATS_ENABLED)
void mbed_stack_stats(const char* func, int line)
{
	osThreadId main_id = osThreadGetId();

	osEvent info;
	info = _osThreadGetInfo(main_id, osThreadInfoStackSize);
	if (info.status != osOK) {
		printf("Could not get stack size (%s:%d)\n", func, line);
	}
	uint32_t stack_size = (uint32_t)info.value.v;
	info = _osThreadGetInfo(main_id, osThreadInfoStackMax);
	if (info.status != osOK) {
		printf("Could not get max stack (%s:%d)\n", func, line);
	}
	uint32_t max_stack = (uint32_t)info.value.v;

	printf("  STACK used: %li, max: %li bytes (%s:%d)\n",
	       max_stack,
	       stack_size,
	       func, line);
}
#endif /* MBED_STACK_STATS_ENABLED */
