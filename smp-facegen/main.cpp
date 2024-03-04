#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <conio.h>

#include "Processor.h"
#include "RefFiles.h"
#include "Timer.h"

constexpr const char* PROJECT_NAME = "SMP FaceGen";
constexpr const char* VERSION_STR = "1.2.0";

constexpr const char* EXCLUDE_NAME = "exclude.txt";
constexpr const char* PATHS_NAME = "paths.txt";
constexpr const char* REFS_NAME = "head parts.txt";

int main()
{
	try {
		std::setlocale(LC_ALL, "en_US.utf8");

		std::cout << PROJECT_NAME << " version " << VERSION_STR << "\n\n";

		std::filesystem::path root(std::filesystem::current_path());
		std::filesystem::path in;
		std::filesystem::path out;
		std::filesystem::path ref;

		std::ifstream input(PATHS_NAME);
		char buf[256];

		while (input.getline(buf, sizeof(buf))) {

			auto end = buf + input.gcount();

			auto str = mbtowstring(buf, end - buf);

			if (str.starts_with(L"in=")) {
				in = std::filesystem::absolute(std::filesystem::path(str.substr(3)));
			}
			else if (str.starts_with(L"out=")) {
				out = std::filesystem::absolute(std::filesystem::path(str.substr(4)));
			}
			else if (str.starts_with(L"data=")) {
				ref = std::filesystem::absolute(std::filesystem::path(str.substr(5)));
			}
		}

		if (in.empty()) {
			in = root;
		}
		if (out.empty()) {
			out = root;
		}
		if (ref.empty()) {
			ref = root;
		}

		std::wcout << std::left << std::setw(8) << "Input:" << in.native() << '\n';
		std::wcout << std::left << std::setw(8) << "Output:" << out.native() << '\n';
		std::wcout << std::left << std::setw(8) << "Data:" << ref.native() << '\n';

		std::wcout << std::endl;

		if (!std::filesystem::directory_entry(in).exists()) {
			throw std::runtime_error("Input directory does not exist.");
		}

		if (in == out) {
			throw std::runtime_error("Input and output directories cannot be the same.");
		}

		if (!std::filesystem::directory_entry(ref).exists()) {
			throw std::runtime_error("Data directory does not exist.");
		}

		Timer<double> m_timer;

		RefFiles refs;
		Processor proc(std::max((int)std::thread::hardware_concurrency(), 1), refs);

		refs.readExclusions(EXCLUDE_NAME);
		refs.readReferences(REFS_NAME, ref);

		proc.process(std::filesystem::directory_entry(in), in, out);
		proc.join();

		std::cout << "Wrote " << proc.processed() << " file(s) in " 
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
