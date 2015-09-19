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

#ifndef HANDYSTATS_EVENTS_MEMORY_POOL_IMPL_HPP_
#define HANDYSTATS_EVENTS_MEMORY_POOL_IMPL_HPP_

namespace handystats { namespace events {

struct event_message_block;

}} // namespace handystats::events

namespace handystats { namespace events { namespace memory_pool {

event_message_block* acquire();
void acknowledge(events::event_message_block*);

void initialize();
void finalize();

}}} // namespace handystats::events::memory_pool

#endif // HANDYSTATS_EVENTS_MEMORY_POOL_IMPL_HPP_
