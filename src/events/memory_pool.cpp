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

#include <deque>
#include <mutex>

#include <handystats/common.h>

#include "core_impl.hpp"

#include "events/event_message_block_impl.hpp"
#include "events/memory_pool_impl.hpp"

namespace handystats { namespace events { namespace memory_pool {

namespace {

std::deque<event_message_block*> free_list;
std::mutex free_list_lock;

} // unnamed namespace


event_message_block* acquire() {
	{
		std::lock_guard<std::mutex> guard(free_list_lock);
		if (!free_list.empty()) {
			auto* block = free_list.front();
			free_list.pop_front();
			return block;
		}
	}

	// free list is empty
	// allocate new block from system
	auto* block = new event_message_block();
	return block;
}

void acknowledge(event_message_block* block) {
	std::lock_guard<std::mutex> guard(free_list_lock);
	free_list.push_back(block);
}

void initialize() {
}

void finalize() {
	std::lock_guard<std::mutex> guard(free_list_lock);

	for (auto iter = free_list.begin(); iter != free_list.end(); ++iter) {
		delete *iter;
	}

	free_list.clear();
}

}}} // namespace handystats::events::memory_pool
