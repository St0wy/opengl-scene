#include "timer.hpp"

namespace stw
{
Timer::Timer() = default;

void Timer::Start()
{
	m_Stopped = false;
	m_StartTime = std::chrono::steady_clock::now();
}

void Timer::Stop()
{
	m_Stopped = true;
	m_EndTime = std::chrono::steady_clock::now();
}

void Timer::Restart()
{
	Stop();
	Start();
}

Duration Timer::RestartAndGetElapsedTime()
{
	const Duration duration = GetElapsedTime();
	Restart();
	return duration;
}

Duration Timer::GetElapsedTime()
{
	m_EndTime = std::chrono::steady_clock::now();
	const i64 timeInMicroSeconds =
		std::chrono::duration_cast<std::chrono::microseconds>(m_EndTime - m_StartTime).count();
	return Duration::FromMicroSeconds(static_cast<f64>(timeInMicroSeconds));
}

bool Timer::IsStopped() const { return m_Stopped; }
}// namespace stw
