//
// Created by stowy on 10/03/2023.
//

#include "job_system.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "thread_safe_ring_buffer.hpp"

namespace stw::job_system
{
std::uint32_t NumThreads = 0;
ThreadSafeRingBuffer<std::function<void()>, 256> JobPool;
std::condition_variable WakeCondition;
std::mutex WakeMutex;
std::uint64_t CurrentLabel = 0;
std::atomic<std::uint64_t> FinishedLabel;

void Initialize()
{
	FinishedLabel.store(0);

	auto numCores = std::thread::hardware_concurrency() - 1;
	NumThreads = std::max(1u, numCores);

	for (std::uint32_t threadId = 0; threadId < NumThreads; ++threadId)
	{
		std::thread worker([]() {
			while (true)
			{
				if (auto jobOpt = JobPool.PopFront())
				{
					auto& job = jobOpt.value();
					job();
					FinishedLabel.fetch_add(1, std::memory_order_acquire);
				}
				else
				{
					std::unique_lock<std::mutex> lock(WakeMutex);
					WakeCondition.wait(lock);
				}
			}
		});

		worker.detach();
	}
}

void Poll()
{
	WakeCondition.notify_one();
	std::this_thread::yield();
}

void Execute(const std::function<void()>& job)
{
	CurrentLabel += 1;

	while (!JobPool.PushBack(job))
	{
		Poll();
	}

	WakeCondition.notify_one();
}

bool IsBusy() { return FinishedLabel.load() < CurrentLabel; }

void Wait()
{
	while (IsBusy())
	{
		Poll();
	}
}

void Dispatch(std::uint32_t jobCount, std::uint32_t groupSize, const std::function<void(JobDispatchArgs)>& job)
{
	if (jobCount == 0 || groupSize == 0)
	{
		return;
	}

	const auto groupCount = (jobCount + groupSize - 1) / groupSize;
	CurrentLabel += groupCount;

	for (std::uint32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex)
	{
		auto jobGroup = [jobCount, groupSize, job, groupIndex]() {
			const auto groupJobOffset = groupIndex * groupSize;
			const auto groupJobEnd = std::min(groupJobOffset + groupSize, jobCount);

			JobDispatchArgs args{};
			args.groupIndex = groupIndex;

			for (std::uint32_t i = groupJobOffset; i < groupJobEnd; ++i)
			{
				args.jobIndex = i;
				job(args);
			}
		};

		while (!JobPool.PushBack(jobGroup))
		{
			Poll();
		}

		WakeCondition.notify_one();
	}
}
}// namespace stw::job_system
