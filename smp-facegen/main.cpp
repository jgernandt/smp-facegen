#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <conio.h>

#include "Processor.h"
#include "RefFiles.h"
#include "Timer.h"

constexpr const char* PROJECT_NAME = "SMP FaceGen";
constexpr const char* VERSION_STR = "1.1.2";

int main()
{
	std::setlocale(LC_ALL, "en_US.utf8");

	std::cout << PROJECT_NAME << " version " << VERSION_STR << "\n\n";

	std::filesystem::path root(std::filesystem::current_path());
	std::filesystem::path in(root / "in");
	std::filesystem::path out(root / "out");
	std::filesystem::path ref(root / "ref");

	std::ifstream input("locations.txt");
	char buf[256];

	while (input.getline(buf, sizeof(buf))) {

		auto str = mbtowstring(buf, sizeof(buf));

		if (str.starts_with(L"in=")) {
			auto t1 = str.substr(4);
			auto t2 = std::filesystem::path(str.substr(4));
			in = std::filesystem::absolute(std::filesystem::path(str.substr(3)));
		}
		else if (str.starts_with(L"out=")) {
			out = std::filesystem::absolute(std::filesystem::path(str.substr(4)));
		}
		else if (str.starts_with(L"ref=")) {
			ref = std::filesystem::absolute(std::filesystem::path(str.substr(4)));
		}
	}

	if (std::filesystem::directory_entry(in).exists()) {
		std::wcout << std::left << std::setw(12) << "Input:" << in.native() << '\n';
	}
	else {
		std::wcout << "Input directory " << in.native() << " does not exist.\n";
		return 0;
	}

	if (std::filesystem::directory_entry(out).exists()) {
		std::wcout << std::left << std::setw(12) << "Output:" << out.native() << '\n';
	}
	else {
		std::wcout << "Output directory " << out.native() << " does not exist. Create it? Y/N\n";
		char reply;
		while (true) {
			(std::cin >> reply).ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			if (std::tolower(static_cast<unsigned char>(reply)) == 'n') {
				return 0;
			}
			else if (std::tolower(static_cast<unsigned char>(reply)) == 'y') {
				std::filesystem::create_directories(out);
				break;
			}
		}
	}

	if (in == out) {
		std::wcout << "Overwrite files in input directory? Y/N";
		char reply;
		while (true) {
			(std::cin >> reply).ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			if (std::tolower(static_cast<unsigned char>(reply)) == 'n') {
				return 0;
			}
			else if (std::tolower(static_cast<unsigned char>(reply)) == 'y') {
				break;
			}
		}
	}

	if (std::filesystem::directory_entry(ref).exists()) {
		std::wcout << std::left << std::setw(12) << "References:" << ref.native() << '\n';
	}
	else {
		std::wcout << "Reference directory " << ref.native() << " does not exist.\n";
		return 0;
	}

	std::wcout << std::endl;

	try {
		Timer<double> m_timer;

		RefFiles refs;
		Processor proc(std::max((int)std::thread::hardware_concurrency(), 1), refs);

		refs.readExclusions("exclude.txt");
		refs.readReferences("references.txt", ref);

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
