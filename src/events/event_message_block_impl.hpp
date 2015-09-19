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

#ifndef HANDYSTATS_EVENT_MESSAGE_BLOCK_IMPL_HPP_
#define HANDYSTATS_EVENT_MESSAGE_BLOCK_IMPL_HPP_

#include "events/event_message_impl.hpp"

namespace handystats { namespace events {

struct event_message_block
{
	static const size_t BLOCK_SIZE = 1 << 10;

	event_message_block()
		: allocated(0)
		, processed(0)
	{
	}

	event_message* allocate() {
		messages[allocated].block = this;
		return &messages[allocated++];
	}

	void acknowledge(const event_message*) {
		processed++;
	}

	// members' layout tries to avoid false sharing
	// between allocated and processed

	// number of allocated messages
	uint64_t allocated;

	event_message messages[BLOCK_SIZE];

	// number of processed messages
	uint64_t processed;
};

}} // namespace handystats::events

#endif // HANDYSTATS_EVENT_MESSAGE_BLOCK_IMPL_HPP_
