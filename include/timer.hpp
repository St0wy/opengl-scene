#pragma once

#include <chrono>

#include "number_types.hpp"

namespace stw
{
#pragma region duration
class Duration
{
public:
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

constexpr f64 Duration::GetInSeconds() const { return GetInMicroseconds() * 0.000001; }

constexpr f64 Duration::GetInMilliseconds() const { return GetInMicroseconds() * 0.001; }

constexpr f64 Duration::GetInMicroseconds() const { return m_DurationInMicroSeconds; }

constexpr Duration::Duration(const f64 microseconds) : m_DurationInMicroSeconds(microseconds) {}

constexpr Duration Duration::FromMicroSeconds(f64 microseconds) { return Duration{ microseconds }; }

constexpr Duration Duration::FromMilliseconds(f64 milliseconds) { return Duration{ milliseconds * 1'000 }; }

constexpr Duration Duration::FromSeconds(f64 seconds) { return Duration{ seconds * 1'000'000 }; }
#pragma endregion duration

class Timer
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
}// namespace stw
