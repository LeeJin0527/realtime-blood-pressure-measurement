/*******************************************************************************
* Copyright (C) Maxim Integrated Products, Inc., All rights Reserved.
* 
* This software is protected by copyright laws of the United States and
* of foreign countries. This material may also be protected by patent laws
* and technology transfer regulations of the United States and of foreign
* countries. This software is furnished under a license agreement and/or a
* nondisclosure agreement and may only be used or reproduced in accordance
* with the terms of those agreements. Dissemination of this information to
* any party or parties not specified in the license agreement and/or
* nondisclosure agreement is expressly prohibited.
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

#ifndef _QUEUE_H_
#define _QUEUE_H_
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#ifndef UNIT_TEST
#include "platform.h"
#else
#ifndef enter_critical_section
#define enter_critical_section()
#endif

#ifndef exit_critical_section
#define exit_critical_section()
#endif

#ifndef pr_info
#define pr_info(...) printf(__VA_ARGS__)
#endif
#endif

#define QUEUE_USAGE_STATS

struct queue_t {
	char *wr; // write pointer
	char *rd; // read pointer
	char *base; // buffer base pointer
	int num_item; // number of data item
	int ovf_item; // Number of overflowed data
	int buffer_size; // buffer size in bytes
	int max_buffer_size; // buffer size in bytes
	int item_size; // data size
	char *name;
#ifdef QUEUE_USAGE_STATS
	int pop_cnt;
	int push_cnt;
	int stats_period_cnt;
#endif
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief		Queue initialization.
 * @param[in]   *q Points to the queue handle
 * @param[in]   *buf Points to external queue buffer
 * @param[in]   item_size Data size
 * @param[in]   buffer_size Total buffer size in bytes
 * @param[out]  *pDst points to output matrix structure
 * @return	The function returns 0: success
 *               -EINVAL (-22): Invalid Pointer, data or parameters
 *               -2: Queue buffer is full, no more space
 **/
int queue_init(struct queue_t *q, void *buf, int item_size, int buffer_size);



/**
 * @brief		Queue initialization by name.
 * @param[in]   *q Points to the queue handle
 * @param[in]   *buf Points to external queue buffer
 * @param[in]   item_size Data size
 * @param[in]   buffer_size Total buffer size in bytes
 * @param[out]  *pDst points to output matrix structure
 * @param[in]   Set queue name for debugging
 * @return      The function returns 0: success
 *               -EINVAL (-22): Invalid Pointer, data or parameters
 *               -2: Queue buffer is full, no more space
 **/
int queue_init_by_name(struct queue_t *q, void *buf, int item_size, int buffer_size, const char *name);



/**
 * @brief		Reset queue
 * @param[in]   *q Points to the queue handle
 * @param[in]   *data Points to any type of data to put FIFO
 * @param[out]  *pDst Points to output matrix structure
 * @return      The function returns 0: success
 *               -EINVAL (-22): Invalid Pointer
 *
 **/
int queue_reset(struct queue_t *q);



/**
 * @brief		Update item size and reset queue. Usign of this command requires exta cauion.
 * @param[in]   *q Points to the queue handle
 * @return     The function returns 0: success
 *					-EINVAL (-22): Invalid Pointer
 *
 **/
int queue_update_items_size(struct queue_t *q, int item_size);



/**
 * @brief		Data enqueue.
 * @param[in]   *q points to the queue handle
 * @param[in]   *data points to any type of data to put FIFO
 * @return      The function returns 0: success
 *               -EINVAL (-22): Invalid Pointer
 *               -2: Queue buffer is full, no more space
 **/
int enqueue(struct queue_t *q, void *data);



/**
 * @brief		Data dequeue.
 * @param[in]   *q points to the queue handle
 * @param[in]   *data points to any type of data to put FIFO
 * @param[out]  *data pop data from Queue
 * @return      The function returns 0: success
 *               -EINVAL (-22): Invalid Pointer or data
 *               -2: Queue buffer is empty
 **/
int dequeue(struct queue_t *q, void *data);



/**
 * @brief		Queue Destroy
 * @param[in]   *q points to the queue handle
 **/
void queue_destroy(struct queue_t *q);



/**
 * @brief		Number of elements in Queue
 * @param[in]   *q points to the queue handle
 * @return		number of elements
 **/
int queue_len(struct queue_t *q);



/**
 * @brief		Copies an item from the front of queue to data, but does not remove it from queue
 * @param[in]  *q points to the queue handle
 * @param[out]  Copy of item from front of the queue
 * @return		if value is greater than 0, return value is number of elements.
				If value is less than 0, returns -EINVAL (-22)
 **/
int queue_front(struct queue_t *q, void *data);



/**
 * @brief		Copies n items from the front of the queue, but does not remove them
 * @param[in]	*q - points to the queue handle
 * @param[out]	*data - The buffer to hold copied data
 * @param[in]	n - the number of items to remove
 * @param[in]	buf_sz - input *data buffer size
 * @return		0: success
 *                -EINVAL (-22): Invalid pointer
 *                -2: Queue contains less than n items
 */
int queue_front_n(struct queue_t *q, void *data, int n, int buf_sz);



/**
 * @brief		Removes an item from front of queue
 * @param[in]	*q points to the queue handle
 * @return		status, success or fail
 **/
int queue_pop(struct queue_t *q);

/**
 * @brief		Removes n items from the front of the queue
 * @param[in]	*q - points to the queue handle
 * @param[in]	n - the number of items to remove
 * @return		0: success
 *                -EINVAL (-22): Invalid pointer
 *                -2: Queue contains less than n items
 */
int queue_pop_n(struct queue_t *q, int n);



/**
 * @brief		Checks if queue is fill
 * @param[in]	*q points to the queue handle
 * @return		true (full), false (not full)
 *
 **/
bool queue_is_full(struct queue_t *q);



/**
 * @brief		returns fifo usage info
 * @param[in]	*q points to the queue handle
 * @param[out]	*total returns total FIFO size in number of elements
 * @param[out]	*nm_item returns number of elements in FIFO
 * @return		status, success or fail
 *               -EINVAL (-22): Invalid Pointer, data or parameters
 **/
int queue_usage(struct queue_t *q, int *total, int *nm_item);



/**
 * @brief		Pops out delimiter terminated string
 * @param[in]	*q points to the queue handle
 * @param[out]	*buf output char array to write
 * @param[in]	buffer_size Maximum buffer size to write the output char array
 * @return		status, string length if positive or fail if negative
 *               -EINVAL (-22): Invalid Pointer, data or parameters
 **/
int dequeue_string(struct queue_t *q, char *buf, int buffer_size);



/**
 * @brief		Pushes null terminated string (char array)
 * @param[in]	*q points to the queue handle
 * @param[in]	*data string(char array) to add it to the circullar buffer
 * @param[in]	sz 'data' length
 * @return		status, success or fail
 *               -EINVAL (-22): Invalid Pointer, data or parameters
 **/
int enqueue_string(struct queue_t *q, char *data, int sz);


/**
 * @brief		Counts length of string in queue
 * @param[in]	*q points to the queue handle
 * @return		status, success or fail
 *               -EINVAL (-22): Invalid Pointer, data or parameters
 *               if ret >= 0, string length
 **/
int queue_str_len(struct queue_t *q);



void queue_n_test(void);

int enqueue_test(struct queue_t *q, void *data);
int dequeue_test(struct queue_t *q, void *data);

#ifdef __cplusplus
}
#endif
#endif //_QUEUE_H_
