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

#include <boost/thread/tss.hpp>

#include "core_impl.hpp"

#include "events/gauge_impl.hpp"
#include "events/counter_impl.hpp"
#include "events/timer_impl.hpp"
#include "events/attribute_impl.hpp"

#include "events/event_message_block_impl.hpp"
#include "events/memory_pool_impl.hpp"

#include "events/event_message_impl.hpp"


namespace handystats { namespace events {

namespace {

void local_event_message_block_cleanup(event_message_block* block) {
	// condition: block is not NULL (POSIX guarantees)
	// thus block is not exhausted
	if (is_enabled()) {
		// remaining messages from the block can be reused
		// that's why allocated is not reseted
		memory_pool::acknowledge(block);
	}
	else {
		// this thread "owns" block
		delete block;
	}
}

} // unnamed namespace

event_message* allocate_event_message() {
	static boost::thread_specific_ptr<event_message_block> local_block(local_event_message_block_cleanup);

	auto* block = local_block.get();
	if (!block) {
		block = memory_pool::acquire();
		local_block.reset(block);
	}

	auto* message = block->allocate();

	if (block->allocated == event_message_block::BLOCK_SIZE) {
		local_block.release();
	}

	return message;
}

void delete_event_message(event_message* message) {
	if (!message) {
		return;
	}

	switch (message->destination_type) {
		case event_destination_type::COUNTER:
			counter::delete_event(message);
			break;
		case event_destination_type::GAUGE:
			gauge::delete_event(message);
			break;
		case event_destination_type::TIMER:
			timer::delete_event(message);
			break;
		case event_destination_type::ATTRIBUTE:
			attribute::delete_event(message);
			break;
		default:
			break;
	}

	auto* block = message->block;
	block->acknowledge(message);

	if (block->processed == event_message_block::BLOCK_SIZE) {
		// block is exhausted and can be reused
		block->allocated = 0;
		block->processed = 0;

		memory_pool::acknowledge(block);
	}
}

}} // namespace handystats::events
