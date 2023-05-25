//////////////////////////////////////////////////////////////////////////////
// Timer.h
// =======
// High Resolution Timer.
// This timer is able to measure the elapsed time with 1 micro-second accuracy
// in both Windows, Linux and Unix system 
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2003-01-13
// UPDATED: 2017-03-30
// UPDATED: 2023-05-25 (by stowy)
//
// Copyright (c) 2003 Song Ho Ahn
//////////////////////////////////////////////////////////////////////////////

#ifndef TIMER_H_DEF
#define TIMER_H_DEF

#if defined(WIN32) || defined(_WIN32)   // Windows system specific
#include <windows.h>
#else          // Unix based system specific
#include <sys/time.h>
#endif

#include "number_types.hpp"

namespace stw
{
#pragma region duration
class Duration
{
public:
	[[nodiscard]] static constexpr Duration FromMicroSeconds(double microseconds);
	[[nodiscard]] static constexpr Duration FromMilliseconds(double milliseconds);
	[[nodiscard]] static constexpr Duration FromSeconds(double seconds);

	[[nodiscard]] constexpr double GetInMicroseconds() const;
	[[nodiscard]] constexpr double GetInMilliseconds() const;
	[[nodiscard]] constexpr double GetInSeconds() const;

private:
	explicit constexpr Duration(double microseconds);

	double m_DurationInMicroSeconds = 0.0;
};

constexpr f64 Duration::GetInSeconds() const
{
	return GetInMicroseconds() * 0.000001;
}

constexpr f64 Duration::GetInMilliseconds() const
{
	return GetInMicroseconds() * 0.001;
}

constexpr f64 Duration::GetInMicroseconds() const
{
	return m_DurationInMicroSeconds;
}

constexpr Duration::Duration(const f64 microseconds)
	: m_DurationInMicroSeconds(microseconds)
{
}

constexpr Duration Duration::FromMicroSeconds(const f64 microseconds)
{
	return Duration{microseconds};
}

constexpr Duration Duration::FromMilliseconds(const f64 milliseconds)
{
	return Duration{milliseconds * 1'000};
}

constexpr Duration Duration::FromSeconds(const f64 seconds)
{
	return Duration{seconds * 1'000'000};
}
#pragma endregion duration

class Timer
{
public:
	Timer();

	void Start();
	void Stop();
	void Restart();
	Duration RestartAndGetElapsedTime();
	Duration GetElapsedTime();

private:
	f64 m_StartTimeInMicroSec;
	f64 m_EndTimeInMicroSec;
	i32 m_Stopped;
#if defined(WIN32) || defined(_WIN32)
	LARGE_INTEGER m_Frequency;
	LARGE_INTEGER m_StartCount;
	LARGE_INTEGER m_EndCount;
#else
    timeval startCount;                         
    timeval endCount;                           
#endif
};
}
#endif // TIMER_H_DEF
