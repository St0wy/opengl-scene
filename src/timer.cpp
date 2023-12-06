/**
 * @file timer.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the Timer and Duration classes.
 * @version 1.0
 * @date 08/11/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <chrono>

export module timer;

import number_types;

namespace stw
{
#pragma region duration
export class Duration
{
public:
	static constexpr f64 MicroToSecond = 0.000001;
	static constexpr f64 MicroToMilli = 0.001;
	static constexpr f64 MilliToMicro = 1'000;
	static constexpr f64 SecondToMicro = 1'000'000;

	[[nodiscard]] static constexpr Duration FromMicroSeconds(f64 microseconds);
	[[nodiscard]] static constexpr Duration FromMilliseconds(f64 milliseconds);
	[[nodiscard]] static constexpr Duration FromSeconds(f64 seconds);

	[[nodiscard]] constexpr f64 GetInMicroseconds() const;
	[[nodiscard]] constexpr f64 GetInMilliseconds() const;
	[[nodiscard]] constexpr f64 GetInSeconds() const;

private:
	explicit constexpr Duration(double microseconds);

	double m_DurationInMicroSeconds = 0.0;
};

constexpr f64 Duration::GetInSeconds() const { return GetInMicroseconds() * MicroToSecond; }

constexpr f64 Duration::GetInMilliseconds() const { return GetInMicroseconds() * MicroToMilli; }

constexpr f64 Duration::GetInMicroseconds() const { return m_DurationInMicroSeconds; }

constexpr Duration::Duration(const f64 microseconds) : m_DurationInMicroSeconds(microseconds) {}

constexpr Duration Duration::FromMicroSeconds(const f64 microseconds) { return Duration{ microseconds }; }

constexpr Duration Duration::FromMilliseconds(const f64 milliseconds)
{
	return Duration{ milliseconds * MilliToMicro };
}

constexpr Duration Duration::FromSeconds(const f64 seconds) { return Duration{ seconds * SecondToMicro }; }
#pragma endregion duration

export class Timer
{
public:
	Timer();

	void Start();
	void Stop();
	void Restart();
	[[nodiscard]] bool IsStopped() const;
	Duration RestartAndGetElapsedTime();
	Duration GetElapsedTime();

private:
	std::chrono::steady_clock::time_point m_StartTime{};
	std::chrono::steady_clock::time_point m_EndTime{};
	bool m_Stopped = true;
};

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
