/*
* Copyright (c) YANDEX LLC. All rights reserved.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 3.0 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library.
*/

#include <chrono>
#include <algorithm>
#include <thread>
#include <sys/prctl.h>
#include <handystats/atomic.hpp>

#include <handystats/chrono.hpp>
#include <handystats/core.hpp>
#include <handystats/core.h>

#include "events/event_message_impl.hpp"
#include "events/memory_pool_impl.hpp"
#include "message_queue_impl.hpp"
#include "internal_impl.hpp"
#include "metrics_dump_impl.hpp"
#include "config_impl.hpp"

#include "core_impl.hpp"

namespace handystats {

std::mutex operation_mutex;
std::atomic<bool> enabled_flag(false);

bool is_enabled() {
	return config::core_opts.enable && enabled_flag.load(std::memory_order_acquire);
}


chrono::time_point last_message_timestamp;
std::thread processor_thread;

static void process_message_queue() {
	auto* message = message_queue::pop();

	if (message) {
		last_message_timestamp = std::max(last_message_timestamp, message->timestamp);
		internal::process_event_message(*message);

		events::delete_event_message(message);
	}
}

static void run_processor() noexcept {
	char thread_name[16];
	memset(thread_name, 0, sizeof(thread_name));

	sprintf(thread_name, "handystats");

	prctl(PR_SET_NAME, thread_name);

	while (is_enabled()) {
		if (!message_queue::empty()) {
			process_message_queue();
		}
		else {
			last_message_timestamp = std::max(last_message_timestamp, chrono::tsc_clock::now());
			std::this_thread::sleep_for(std::chrono::microseconds(1000));
		}

		metrics_dump::update(chrono::tsc_clock::now(), last_message_timestamp);
	}
}

void initialize() {
	std::lock_guard<std::mutex> lock(operation_mutex);
	if (enabled_flag.load(std::memory_order_acquire)) {
		return;
	}

	metrics_dump::initialize();
	internal::initialize();
	events::memory_pool::initialize();
	message_queue::initialize();

	if (!config::core_opts.enable) {
		return;
	}

	enabled_flag.store(true, std::memory_order_release);

	last_message_timestamp = chrono::time_point();

	processor_thread = std::thread(run_processor);
}

void finalize() {
	std::lock_guard<std::mutex> lock(operation_mutex);
	enabled_flag.store(false, std::memory_order_release);

	if (processor_thread.joinable()) {
		processor_thread.join();
	}

	internal::finalize();
	message_queue::finalize();
	events::memory_pool::finalize();
	metrics_dump::finalize();
	config::finalize();
}

} // namespace handystats


extern "C" {

void handystats_initialize() {
	handystats::initialize();
}

void handystats_finalize() {
	handystats::finalize();
}

} // extern "C"
