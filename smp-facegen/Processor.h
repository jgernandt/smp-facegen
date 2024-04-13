#pragma once
#include <atomic>
#include <filesystem>
#include <mutex>
#include <queue>
#include <semaphore>
#include <thread>

#include "RefFiles.h"
#include "Timer.h"

class Processor
{
private:
	struct Task
	{
		std::filesystem::path in;
		std::filesystem::path out;
	};

public:
	Processor(int threads, const RefFiles& refs);
	Processor(const Processor&) = delete;

	~Processor();

	void join();
	void process(const std::filesystem::directory_entry& entry,
		const std::filesystem::path& iroot, const std::filesystem::path& oroot);
	void report() const;

private:
	void process_nif(const std::filesystem::path& in, const std::filesystem::path& out);
	void push(const std::filesystem::path& in, const std::filesystem::path& out);
	void worker();

private:
	const RefFiles& m_refs;

	std::vector<std::thread> m_threads;
	std::queue<Task> m_tasks;
	std::mutex m_mutex;
	std::counting_semaphore<> m_signal{ 0 };

	std::atomic_int m_written{ 0 };
	std::atomic_int m_skipped{ 0 };

	Timer<double> m_timer;
};
