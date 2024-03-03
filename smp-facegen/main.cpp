#include <filesystem>
#include <iostream>

#include <conio.h>

#include "Processor.h"
#include "RefFiles.h"
#include "Timer.h"

constexpr const char* PROJECT_NAME = "SMP FaceGen";
constexpr const char* VERSION_STR = "1.1.1";

int main()
{
	std::cout << PROJECT_NAME << " version " << VERSION_STR << "\n\n";

	std::filesystem::path root(std::filesystem::current_path());
	std::filesystem::path in(root / "in");
	std::filesystem::path out(root / "out");
	std::filesystem::path ref(root / "ref");

	try {
		Timer<double> m_timer;

		RefFiles refs;
		Processor proc(std::max((int)std::thread::hardware_concurrency(), 1), refs);

		refs.readExclusions(root / "exclude.txt");
		refs.readDirectory(ref);

		proc.process(std::filesystem::directory_entry(in), in, out);
		proc.join();

		std::cout << "Created " << proc.processed() << " file(s) in " 
			<< std::setprecision(2) << m_timer.elapsed() << " seconds\n\n";
	}
	catch (const std::exception& e) {
		std::cout << e.what() << "\n\n";
	}

	std::cout << "Press any key to exit...";
	while (!_kbhit()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return 0;
}
