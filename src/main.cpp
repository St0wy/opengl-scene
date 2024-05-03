#include <memory>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

import number_types;
import window;
import ssao_scene;

int main(int, char*[])
{
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
	console_sink->set_level(spdlog::level::info);
#ifndef NDEBUG
	console_sink->set_level(spdlog::level::debug);
#endif

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>("logs/logs.txt", true);
	file_sink->set_level(spdlog::level::trace);

	spdlog::sinks_init_list sinks{ console_sink, file_sink };
	auto logger = std::make_shared<spdlog::logger>("OpenGL Scene", sinks);
	logger->set_level(spdlog::level::debug);

	spdlog::set_default_logger(logger);

	stw::Window<stw::SsaoScene> window("OpenGL Scene");
	window.Loop();
	return 0;
}
