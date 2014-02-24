#include <handystats/json_dump.hpp>

#include <handystats/internal_impl.hpp>
#include <handystats/json_impl.hpp>

namespace handystats {

extern metrics::gauge event_message_queue_size;
extern metrics::gauge monitors_size;
extern metrics::gauge message_processing_time;
extern metrics::gauge message_push_time;
extern metrics::gauge message_pop_time;
extern metrics::gauge message_delete_time;

}

namespace handystats { namespace json {

chrono::default_time_point dump_timestamp;
chrono::default_duration DUMP_INTERVAL = std::chrono::duration_cast<chrono::default_duration>(std::chrono::milliseconds(500));

std::mutex json_dump_mutex;
std::shared_ptr<const std::string> json_dump(new std::string());

std::shared_ptr<const std::string> get_json_dump() {
	std::lock_guard<std::mutex> lock(json_dump_mutex);
	return json_dump;
}

std::shared_ptr<const std::string> create_json_dump() {
	rapidjson::Value dump_value(rapidjson::kObjectType);

	dump_timestamp = chrono::default_clock::now();
	rapidjson::Value timestamp_value;
	write_to_json_value(dump_timestamp, &timestamp_value);
	dump_value.AddMember("dump-timestamp", timestamp_value, memoryPoolAllocator);

	for (auto monitor_entry : internal::monitors) {
		rapidjson::Value monitor_value;
		switch (monitor_entry.second.which()) {
			case internal::internal_monitor_index::INTERNAL_GAUGE:
				write_to_json_value(boost::get<internal::internal_gauge*>(monitor_entry.second), &monitor_value, memoryPoolAllocator);
				break;
			case internal::internal_monitor_index::INTERNAL_COUNTER:
				write_to_json_value(boost::get<internal::internal_counter*>(monitor_entry.second), &monitor_value, memoryPoolAllocator);
				break;
			case internal::internal_monitor_index::INTERNAL_TIMER:
				write_to_json_value(boost::get<internal::internal_timer*>(monitor_entry.second), &monitor_value, memoryPoolAllocator);
				break;
		}

		dump_value.AddMember(monitor_entry.first.c_str(), memoryPoolAllocator, monitor_value, memoryPoolAllocator);
	}

	{
		rapidjson::Value queue_size_value;
		write_to_json_value(&event_message_queue_size, &queue_size_value, memoryPoolAllocator);
		dump_value.AddMember("message-queue-size", queue_size_value, memoryPoolAllocator);
	}
	{
		rapidjson::Value monitors_size_value;
		write_to_json_value(&monitors_size, &monitors_size_value, memoryPoolAllocator);
		dump_value.AddMember("monitors-size", monitors_size_value, memoryPoolAllocator);
	}
	{
		rapidjson::Value processing_time_value;
		write_to_json_value(&message_processing_time, &processing_time_value, memoryPoolAllocator);
		dump_value.AddMember("message-processing-time", processing_time_value, memoryPoolAllocator);
	}
	{
		rapidjson::Value push_time_value;
		write_to_json_value(&message_push_time, &push_time_value, memoryPoolAllocator);
		dump_value.AddMember("message-push-time", push_time_value, memoryPoolAllocator);
	}
	{
		rapidjson::Value pop_time_value;
		write_to_json_value(&message_pop_time, &pop_time_value, memoryPoolAllocator);
		dump_value.AddMember("message-pop-time", pop_time_value, memoryPoolAllocator);
	}
	{
		rapidjson::Value delete_time_value;
		write_to_json_value(&message_delete_time, &delete_time_value, memoryPoolAllocator);
		dump_value.AddMember("message-delete-time", delete_time_value, memoryPoolAllocator);
	}

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	dump_value.Accept(writer);

	std::shared_ptr<const std::string> new_json_dump(new std::string(buffer.GetString(), buffer.GetSize()));
	return new_json_dump;
}

void update_json_dump() {
	if (std::chrono::duration_cast<chrono::default_duration>(chrono::default_clock::now() - dump_timestamp) > DUMP_INTERVAL) {
		auto new_json_dump = create_json_dump();
		{
			std::lock_guard<std::mutex> lock(json_dump_mutex);
			json_dump = new_json_dump;
		}
	}
}

}} // namespace handystats::json

