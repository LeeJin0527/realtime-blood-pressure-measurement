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
#ifdef ENABLE_EVENTSTATS
#include "EventStats.h"
#include "Peripherals.h"

EventStats::EventStats() :
	single_evt(), all_evts()
{
	name = "";
	report_period = 1000000;
	num_evts = 0;
	total_evt_time = 0;

	evt_in_prog = false;
}

EventStats::EventStats(int report_period, const char* name) :
	single_evt(), all_evts()
{
	if (name)
		this->name = name;
	else
		this->name = "";

	this->report_period = report_period;
	num_evts = 0;
	total_evt_time = 0;
	evt_in_prog = false;

}

void EventStats::start()
{
	if (evt_in_prog) {
		pr_debug("EventStats::start() called twice without calling EventStats::stop()!");
	}


	if (!num_evts)
		all_evts.start();

	single_evt.start();
	evt_in_prog = true;

}

void EventStats::stop()
{
	if (!evt_in_prog) {
		pr_debug("EventStats::stop() called before EventStats::start()!");
	}

	single_evt.stop();
	num_evts++;
	evt_in_prog = false;
	total_evt_time += single_evt.read_us();
	single_evt.reset();

	int time_since_start = all_evts.read_us();
	if (time_since_start > report_period) {
		print_events(true);
	}
}

void EventStats::print_events(bool reset)
{
	if (num_evts > 0) {
		int time_since_start = all_evts.read_us();
		int avg = total_evt_time / num_evts;

		pr_debug("[Event (%s) hit %d times in %dus. Avg: %dus, Total: %dus\r\n",
				name,
				num_evts,
				time_since_start,
				avg,
				total_evt_time);
	} else {
		pr_debug("Event (%s) has not yet occurred\r\n", name);
	}

	if (reset) {
		num_evts = 0;
		all_evts.reset();
		total_evt_time = 0;
	}
}

#endif //ENABLE_EVENTSTATS
