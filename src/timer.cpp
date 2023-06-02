//////////////////////////////////////////////////////////////////////////////
// Timer.cpp
// =========
// High Resolution Timer.
// This timer is able to measure the elapsed time with 1 micro-second accuracy
// in both Windows, Linux and Unix system 
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2003-01-13
// UPDATED: 2017-03-30
//
// Copyright (c) 2003 Song Ho Ahn
//////////////////////////////////////////////////////////////////////////////

#include "timer.hpp"
#include <stdlib.h>

namespace stw
{
Timer::Timer()
	: m_StartTimeInMicroSec(0),
	m_EndTimeInMicroSec(0),
	m_Stopped(0),
#if defined(WIN32) || defined(_WIN32)
	m_Frequency(),
	m_StartCount(),
	m_EndCount()
#else
	m_StartCount(),
	m_EndCount()
#endif
{
#if defined(WIN32) || defined(_WIN32)
	QueryPerformanceFrequency(&m_Frequency);
	m_StartCount.QuadPart = 0;
	m_EndCount.QuadPart = 0;
#else
    m_StartCount.tv_sec = m_StartCount.tv_usec = 0;
    m_EndCount.tv_sec = m_EndCount.tv_usec = 0;
#endif
}


void Timer::Start()
{
	m_Stopped = 0; // reset stop flag
#if defined(WIN32) || defined(_WIN32)
	QueryPerformanceCounter(&m_StartCount);
#else
    gettimeofday(&m_StartCount, NULL);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// stop the timer.
// endCount will be set at this point.
///////////////////////////////////////////////////////////////////////////////
void Timer::Stop()
{
	m_Stopped = 1; // set timer stopped flag

#if defined(WIN32) || defined(_WIN32)
	QueryPerformanceCounter(&m_EndCount);
#else
    gettimeofday(&m_EndCount, NULL);
#endif
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
#if defined(WIN32) || defined(_WIN32)
	if (!m_Stopped)
		QueryPerformanceCounter(&m_EndCount);

	const auto startCountDouble = static_cast<f64>(m_StartCount.QuadPart);
	const auto endCountDouble = static_cast<f64>(m_EndCount.QuadPart);
	const auto frequencyDouble = static_cast<f64>(m_Frequency.QuadPart);

	m_StartTimeInMicroSec = startCountDouble * (1000000.0 / frequencyDouble);
	m_EndTimeInMicroSec = endCountDouble * (1000000.0 / frequencyDouble);
#else
    if(!stopped)
        gettimeofday(&m_EndCount, NULL);

    startTimeInMicroSec = (m_StartCount.tv_sec * 1000000.0) + m_StartCount.tv_usec;
    endTimeInMicroSec = (m_EndCount.tv_sec * 1000000.0) + m_EndCount.tv_usec;
#endif

	return Duration::FromMicroSeconds(m_EndTimeInMicroSec - m_StartTimeInMicroSec);
}
}
