//
// Created by stowy on 10/03/2023.
//

#pragma once

#include <functional>

namespace stw
{
struct JobDispatchArgs
{
	std::size_t jobIndex;
	std::size_t groupIndex;
};

namespace job_system
{
void Initialize();
void Execute(const std::function<void()>& job);
void Dispatch(std::uint32_t jobCount, std::uint32_t groupSize, const std::function<void(JobDispatchArgs)>& job);
bool IsBusy();
void Wait();
}
}