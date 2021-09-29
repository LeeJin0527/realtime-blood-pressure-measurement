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

/*
 * TODO:
 * Add a function to enqueue data block instead of one by one.
 * Write function definitions in the header file as doxygen format
 * Init function will also allocate memory for queue buffer, providing the buffer will not necessary
 *
 * */

#include "queue.h"
#include "mbed.h"
#include "Peripherals.h"

int queue_reset(struct queue_t *q)
{
	if (!q)
		return -EINVAL;

	q->wr = q->base;
	q->rd = q->base;
	q->num_item = 0;
	q->ovf_item = 0;
#ifdef QUEUE_USAGE_STATS
	q->pop_cnt = 0;
	q->push_cnt = 0;
	q->stats_period_cnt = 100; // Default
#endif
	return 0;
}

int queue_update_items_size(struct queue_t *q, int item_size)
{
	if (!q)
		return -EINVAL;

	queue_reset(q);
	q->item_size = item_size;
	q->buffer_size = q->max_buffer_size - (q->max_buffer_size % item_size);
	return 0;
}

int queue_len(struct queue_t *q)
{
	int num_elements;

	if (!q)
		return -EINVAL;

	num_elements = q->num_item;
	return num_elements;
}


int queue_init(struct queue_t *q, void *buf, int item_size, int buffer_size)
{
	if (!q || !buf)
		return -EINVAL;

	if (buffer_size % item_size != 0)
		return -EINVAL; // Padding problem

	q->num_item = 0;
	q->ovf_item = 0;
	q->base = (char *)buf;
	q->wr =  (char *)buf;
	q->rd =  (char *)buf;
	q->item_size = item_size;
	q->buffer_size = buffer_size;
	q->max_buffer_size = buffer_size;
	q->name = NULL;

#ifdef QUEUE_USAGE_STATS
	q->pop_cnt = 0;
	q->push_cnt = 0;
	q->stats_period_cnt = 100; // Default
#endif

	return 0;
}

int queue_init_by_name(struct queue_t *q, void *buf, int item_size, int buffer_size, const char *name)
{
	int ret = queue_init(q, buf, item_size, buffer_size);

	if (ret < 0)
		return ret;

	q->name = (char *)name;
	return 0;
}


void queue_destroy(struct queue_t *q)
{
/* TODO: This is placeholder function, double check the implementation */
	free((void *)q->base);
	free((void *)q);
}

int enqueue(struct queue_t *q, void *data)
{
	int ret = 0;

	if (!q || !data)
		return -EINVAL; // Invalid pointer

	if (q->wr == q->rd)
		ret = (q->num_item != 0) ? -2 : 0; // Is FIFO Full or Empty?

	if (q->wr >= (q->base + q->buffer_size))
		q->wr = q->base;

	memcpy((void *)q->wr, data, q->item_size);
	q->wr = q->wr + q->item_size;
	q->num_item++;
	int fifo_size = q->buffer_size / q->item_size;
	if (q->num_item > fifo_size)
		q->ovf_item = q->num_item - fifo_size;
	else
		q->ovf_item = 0;
	return ret;
}

int enqueue_test(struct queue_t *q, void *data)
{
	if (!q || !data)
		return -EINVAL; // Invalid pointer

	q->num_item++;
	return 0;
}


int dequeue_test(struct queue_t *q, void *data)
{
	if (!q || !data)
		return -EINVAL;

	if (q->num_item > 0)
		q->num_item--;
	else
		return -2;
	return 0;
}

int dequeue(struct queue_t *q, void *data)
{
	int fifo_size = q->buffer_size / q->item_size;

	if (!q || !data)
		return -EINVAL;

	if (q->num_item <= 0) {
		return -2;
	}

	if (q->num_item > fifo_size) {
		uint32_t curr_rd_off = ((uint32_t)(q->rd - q->base) + q->num_item * q->item_size);
		q->rd = (q->base + (curr_rd_off % q->buffer_size));
		q->num_item = fifo_size; // OVF number samples are already gone.

#ifdef QUEUE_USAGE_STATS
		q->pop_cnt++;

		if ((q->pop_cnt % q->stats_period_cnt) == 0) {
			if (q->name) {
				pr_info("%s:%d (%s) - %d samples lost, avail:%d \n",__func__, __LINE__, q->name, q->ovf_item, q->num_item);
			} else {
				pr_info("%s:%d - %d samples lost, avail:%d \n",	__func__, __LINE__, q->ovf_item, q->num_item);
			}
		}
#endif
	}

	if (q->rd >= (q->base + q->buffer_size))
		q->rd = q->base;

	memcpy(data, (void *)q->rd, q->item_size);
	q->rd = q->rd + q->item_size;
	q->num_item--;
	if (q->num_item <= fifo_size)
		q->ovf_item = 0;

#if defined(QUEUE_DEBUG)
	do {
		static int cnt;

		if (cnt++ % 100 == 0) {
			if (q->name) {
				pr_debug("%s - $ Fifo size: %d, usage: %d\n", q->name, fifo_size, q->num_item);
			} else {
				pr_debug("$ Fifo size: %d, usage: %d\n", fifo_size, q->num_item);
			}
		}
	} while(0);
#endif

	return 0;
}

bool queue_is_full(struct queue_t *q)
{
	if (!q)
		return -EINVAL;

	int num_items = q->buffer_size / q->item_size;


	if (num_items > q->num_item)
		return false;
	return true;
}

int queue_usage(struct queue_t *q, int *total, int *nm_item)
{
	if (!q)
		return -EINVAL;

	*total = q->buffer_size / q->item_size;
	*nm_item = q->num_item;

	return 0;
}

int queue_pop(struct queue_t *q)
{
	int fifo_size = q->buffer_size / q->item_size;

	if (!q)
		return -EINVAL;

	if (q->num_item <= 0) {
		return -2;
	}

	if (q->num_item > fifo_size) {
		uint32_t curr_rd_off = ((uint32_t)(q->rd - q->base) + q->num_item * q->item_size);
		q->ovf_item = q->num_item - fifo_size;
		q->rd = q->base + (curr_rd_off % q->buffer_size);
		q->num_item = fifo_size; // OVF number samples are already gone.

#ifdef QUEUE_USAGE_STATS
		q->push_cnt++;

		if ((q->push_cnt % q->stats_period_cnt) == 0) {
			if (q->name) {
				pr_info("%s:%d (%s) - %d samples lost, avail:%d \n",__func__, __LINE__, q->name, q->ovf_item, q->num_item);
			} else {
				pr_info("%s:%d - %d samples lost, avail:%d \n",	__func__, __LINE__, q->ovf_item, q->num_item);
			}
		}
#endif
	} else
		q->ovf_item = 0;

	if (q->rd >= (q->base + q->buffer_size))
		q->rd = q->base;

	q->rd = q->rd + q->item_size;
	q->num_item--;

#if defined(QUEUE_DEBUG)
	do {
		static int cnt;

		if (cnt++ % 100 == 0) {
			if (q->name)
				pr_debug("%s - $ Fifo size: %d, usage: %d\n", q->name, fifo_size, q->num_item);
			else
				pr_debug("$ Fifo size: %d, usage: %d\n", fifo_size, q->num_item);
		}
	} while(0);
#endif

	return 0;
}

int queue_pop_n(struct queue_t *q, int n)
{
	int fifo_size = q->buffer_size / q->item_size;

	if (!q || n < 1)
		return -EINVAL;

	if (q->num_item < n) {
		return -2;
	}

	if (q->num_item > fifo_size) {
		if (q->name) {
			pr_info("%s:%d ( %s ) - %d samples lost, avail:%d \n",
				__func__, __LINE__, q->name, q->num_item - fifo_size, fifo_size);
		} else {
			pr_info("%s:%d - %d samples lost, avail:%d \n",
				__func__, __LINE__, q->num_item - fifo_size, fifo_size);
		}
		q->num_item = fifo_size; // OVF number samples are already gone.
		n = fifo_size;
		uint32_t curr_rd_off = (uint32_t)(q->rd - q->base) + q->num_item * q->item_size;
		q->ovf_item = q->num_item - fifo_size;
		q->rd = q->base + curr_rd_off % q->buffer_size;
	} else {
		q->ovf_item = 0;
	}

	if (q->rd >= (q->base + q->buffer_size))
		q->rd = q->base;


	uint32_t rd_sz = q->item_size * n;
	uint32_t to_end = (uint32_t)(q->base + q->buffer_size - q->rd);
	if (to_end < rd_sz) {
		rd_sz -= to_end;
		q->rd = q->base;
	}

	q->rd = q->rd + rd_sz;
	q->num_item -= n;

	return 0;
}

int queue_front(struct queue_t *q, void *data)
{
#if 0
	return queue_front_n(q, data, 1);
#endif

	int fifo_size = q->buffer_size / q->item_size;
	void *rd = 0;

	if (!q || !data)
		return -EINVAL;

	if (q->num_item <= 0) {
		return -2;
	}

	if (q->num_item > fifo_size) {
		uint32_t curr_rd_off = (uint32_t)(q->rd - q->base) + q->num_item * q->item_size;
		rd = q->base + (curr_rd_off % q->buffer_size);
		if (q->name) {
			pr_info("%s:%d ( %s )- %d samples lost, avail:%d cap:%d \n",
				__func__, __LINE__, q->name, q->ovf_item, q->num_item, fifo_size);
		} else {
			pr_info("%s:%d - %d samples lost, avail:%d cap:%d \n",
				__func__, __LINE__, q->ovf_item, q->num_item, fifo_size);
		}
	} else {
		q->ovf_item = 0;
		rd = q->rd;
	}

	if (q->rd >= (q->base + q->buffer_size))
		rd = q->base;

	memcpy(data, (void *)rd, q->item_size);
	return 0;
}

int queue_front_n(struct queue_t *q, void *data, int n, int buf_sz)
{
	int fifo_size = q->buffer_size / q->item_size;
	char *rd = 0;
	char *wr = (char *)data;

	if (!q || !data || n < 1)
		return -EINVAL;

	if (q->num_item < n) {
		return -2;
	}

	if (q->num_item > fifo_size) {
		if (q->name) {
			pr_info("\n%s:%d ( %s ) - %d samples lost, avail:%d \n",
					__func__, __LINE__, q->name, q->num_item - fifo_size, fifo_size);
		} else {
			pr_info("\n%s:%d - %d samples lost, avail:%d \n",
				__func__, __LINE__, q->num_item - fifo_size, fifo_size);
		}
		q->num_item = fifo_size;
		n = fifo_size;
		uint32_t curr_rd_off = (uint32_t)(q->rd - q->base) + q->num_item * q->item_size;
		rd = q->base + (curr_rd_off % q->buffer_size);
	} else {
		q->ovf_item = 0;
		rd = q->rd;
	}

	if (q->rd >= (q->base + q->buffer_size))
		rd = q->base;

	uint32_t rd_sz = q->item_size * n;

	if (buf_sz < rd_sz) {
		if (q->name) {
			pr_info("\n%s:%d ( %s ) - Insufficient buffer size: %d\n",
					__func__, __LINE__, q->name, buf_sz);
		} else {
			pr_info("\n%s:%d - Insufficient buffer size: %d\n",
				__func__, __LINE__, buf_sz);
		}
		return -EINVAL;
	}

	uint32_t to_end = (uint32_t)(q->base + q->buffer_size - q->rd);
	if (to_end < rd_sz) {
		memcpy(wr, rd, to_end);
		rd_sz -= to_end;
		rd = q->base;
		wr += to_end;
		memcpy(wr, rd, rd_sz);

	} else {
		memcpy(wr, rd, rd_sz);
	}

	return 0;
}

int enqueue_string(struct queue_t *q, char *data, int sz)
{
	int ret = 0;
	int buf_index;
	char *wr_ptr;

	if (!q || !data || sz <= 0)
		return -EFAULT; // Invalid parameters

	if (q->wr == q->rd)
		ret = (q->num_item != 0) ? -2 : 0; // Is FIFO Full or Empty?

	if (q->wr >= (q->base + q->buffer_size))
		q->wr = q->base;

	if ((q->num_item + sz) > q->buffer_size) {
#if defined(QUEUE_DEBUG)
		{
			char buf[128];
			int len;

			if (q->name)
				len = sprintf(buf, "\r\n**** %s - ( %s ) - Fifo is full. num_item: %d, sz: %d, buffer size: %d\r\n",
						__func__, q->name, q->num_item, sz, q->buffer_size);
			else
				len = sprintf(buf, "\r\n**** %s - Fifo is full. num_item: %d, sz: %d, buffer size: %d\r\n",
						__func__, q->num_item, sz, q->buffer_size);

			UART_Write(UART_PORT, (uint8_t*)buf, len);
		}
#endif
		return -ENOMEM;
	}

	buf_index = (uint32_t)(q->wr - q->base);
	wr_ptr = q->base;
	q->num_item += sz;
	while(sz--)
		wr_ptr[buf_index++ % q->buffer_size] = *data++;

	q->wr = q->base + buf_index % q->buffer_size;
	return ret;
}

int dequeue_string(struct queue_t *q, char *buf, int buffer_size)
{
	char *rd_ptr;
	int buf_index;
	int len;

	if (!q || !buf || buffer_size <= 0)
		return -EFAULT;

	if (q->num_item <= 0) {
		return -EPERM;
	}

	rd_ptr = (char *)q->base;
	buf_index = (uint32_t)(q->rd - q->base);
	len = q->num_item;

	while (buffer_size-- && q->num_item--) {
		char tmp = rd_ptr[buf_index % q->buffer_size];
		rd_ptr[buf_index % q->buffer_size] = 0; // Remove this later on
		buf_index++;
		*buf++ = tmp;
		if (tmp == '\0')
			break;
	}

	if (q->num_item < 0) {
		/* Data corruption in FIFO */
		q->num_item = 0;
	} else
		len -= q->num_item;

	q->rd = q->base + buf_index % q->buffer_size;

	return len;
}

int queue_str_len(struct queue_t *q)
{
	char *rd_ptr;
	int buf_index;
	int len, i;

	if (!q)
		return -EFAULT;

	if (q->num_item <= 0) {
		return 0;
	}

	rd_ptr = q->base;
	buf_index = (uint32_t)(q->rd - q->base);
	i = q->num_item;
	len = 0;

	while (i--) {
		char tmp = rd_ptr[buf_index % q->buffer_size];
		buf_index++;
		if (tmp == '\0')
			break;
		len++;
	}

	return len;
}

#if 0
void queue_test(void)
{
	int ret;
	ppg_data_t ppg_test = { 0, };
	ppg_data_t ppg_test_out = { 0, };
	int i, j, ii, jj;
	static ppg_data_t ppg_data[10];
	static queue_t queue;

	srand((unsigned)time(NULL));
	ret = queue_init(&queue, &ppg_data, sizeof(ppg_data_t), sizeof(ppg_data));
	while (1) {
		ii = rand() % 20;
		for (i = 0; i < ii; i++) {
			/* Test data */
			ppg_test.timestamp++;
			ppg_test.ir++;
			ppg_test.red++;
			ppg_test.green++;
			/* Test functions */
			ret = enqueue(&queue, &ppg_test);
		}
		jj = rand() % 20;
		for (j = 0; j < jj; j++) {
			ret = dequeue(&queue, &ppg_test_out);
		}
	}
}
#endif
void queue_n_test(void)
{
	struct queue_t q;
	uint8_t buf[5];
	uint8_t peek_buf[5];
	int error;
	int i;
	error = queue_init(&q, &buf[0], 1, sizeof(buf));
	if (error)
		printf("queue_init error :(\r\n");

	uint8_t val = 0;
	enqueue(&q, &val);
	val = 1;
	enqueue(&q, &val);
	val = 2;
	enqueue(&q, &val);
	val = 3;
	enqueue(&q, &val);
	val = 4;
	enqueue(&q, &val);

	printf("enqueued 0,1,2,3,4\r\n");

	error = queue_front_n(&q, &peek_buf, 5, sizeof(peek_buf));
	if (error) {
		printf("queue_front_n n=5 error :(\r\n");
	} else {
		printf("queue_front_n n=5: ");
		for (i = 0; i < 5; i++) {
			printf("%d ", peek_buf[i]);
		}
		printf("\r\n");
	}

	error = queue_front_n(&q, &peek_buf, 6, sizeof(peek_buf));
	if (error)
		printf("queue_front_n n=6 error :)\r\n");
	else
		printf("queue_front_n n=6 succeeded :(\r\n");

	error = queue_pop_n(&q, 2);
	if (error)
		printf("queue_pop_n n=2 error :(\r\n");
	else
		printf("queue_pop_n n=2 succeeded :)\r\n");

	error = queue_front_n(&q, &peek_buf, 5, sizeof(peek_buf));
	if (error)
		printf("queue_front_n n=5 error :)\r\n");

	error = queue_front_n(&q, &peek_buf, 3, sizeof(peek_buf));
	if (error) {
		printf("queue_front_n n=3 error :(\r\n");
	} else {
		printf("queue_front_n n=3: ");
		for (i = 0; i < 3; i++) {
			printf("%d ", peek_buf[i]);
		}
		printf("\r\n");
	}

	val = 0;
	enqueue(&q, &val);
	val = 1;
	enqueue(&q, &val);

	printf("enqueued 0,1\r\n");

	error = queue_front_n(&q, &peek_buf, 5, sizeof(peek_buf));
	if (error) {
		printf("queue_front_n n=5 error :(\r\n");
	} else {
		printf("queue_front_n n=5: ");
		for (i = 0; i < 5; i++) {
			printf("%d ", peek_buf[i]);
		}
		printf("\r\n");
	}

	error = queue_pop_n(&q, 4);
	if (error)
		printf("queue_pop_n n=4 error :(\r\n");
	else
		printf("queue_pop_n n=4 succeeded :)\r\n");

	error = queue_front_n(&q, &peek_buf, 1, sizeof(peek_buf));
	if (error) {
		printf("queue_front_n n=1 error :(\r\n");
	} else {
		printf("queue_front_n n=1: ");
		for (i = 0; i < 1; i++) {
			printf("%d ", peek_buf[i]);
		}
		printf("\r\n");
	}
}
