#pragma once

#include <chrono>

template<typename tick_t = long long, typename period_t = std::ratio<1>>
class Timer
{
private:
	using clock_t = std::chrono::steady_clock;
	using duration_t = std::chrono::duration<tick_t, period_t>;
	using time_t = std::chrono::time_point<clock_t>;

public:
	Timer()	{ m_startTime = clock_t::now(); }
	~Timer() = default;

	tick_t elapsed() const { return std::chrono::duration_cast<duration_t>(clock_t::now() - m_startTime).count(); }
	void reset() { m_startTime = clock_t::now(); }

private:
	time_t m_startTime;
};