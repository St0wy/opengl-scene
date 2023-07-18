//
// Created by stowy on 10/03/2023.
//

#pragma once

#include <array>
#include <cstddef>
#include <mutex>
#include <optional>

namespace stw
{
template<typename T, std::size_t capacity>
class ThreadSafeRingBuffer
{
public:
	bool PushBack(const T& item)
	{
		bool result = false;
		m_Lock.lock();
		std::size_t next = (m_Head + 1) % capacity;
		if (next != m_Tail)
		{
			m_Data[m_Head] = item;
			m_Head = next;
			result = true;
		}

		m_Lock.unlock();
		return result;
	}

	std::optional<T> PopFront()
	{
		std::optional<T> result{};
		m_Lock.lock();
		if (m_Tail != m_Head)
		{
			result = m_Data[m_Tail];
			m_Tail = (m_Tail + 1) % capacity;
		}
		m_Lock.unlock();

		return result;
	}

private:
	std::array<T, capacity> m_Data;
	std::size_t m_Head = 0;
	std::size_t m_Tail = 0;
	std::mutex m_Lock;
};
}// namespace stw
