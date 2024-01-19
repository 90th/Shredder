#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <filesystem>

namespace file = std::filesystem;

inline std::vector<char> generateRandomPattern(std::size_t size) {
	std::vector<char> pattern(size);
	std::mt19937 engine(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()));
	std::uniform_int_distribution<int> distribution(500, 5000);

	for (auto& element : pattern) {
		element = static_cast<char>(distribution(engine));
	}

	return pattern;
}

inline static void shredFile(const std::string& filePath, int iterations) {
	try {
		auto fileSize = file::file_size(filePath);
		std::vector<char> buffer(fileSize, 0);

		for (int iteration = 0; iteration < iterations; ++iteration) {
			std::ifstream inputFile(filePath, std::ios::binary);
			inputFile.read(buffer.data(), fileSize);
			inputFile.close();

			std::ofstream outputFile(filePath, std::ios::binary | std::ios::trunc);

			auto pattern = generateRandomPattern(fileSize);
			for (int i = 0; i < 10; ++i) {
				auto offset = static_cast<std::size_t>(std::rand() % static_cast<int>(fileSize - pattern.size() + 1));
				std::copy(pattern.begin(), pattern.end(), buffer.begin() + offset);
			}

			outputFile.write(buffer.data(), fileSize);
			outputFile.close();

			// Print progress information for the user
			float percentComplete = static_cast<float>(iteration + 1) / iterations * 100.0f;
			std::cout << "Shredding file... " << std::fixed << percentComplete << "% complete\n";
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
		std::exit(1);
	}
}

int main(int argc, char* argv[]) {
	if (argc == 1) {
		std::cout << "Usage: shredder -f <file location> -i <number of iterations>\n"
			<< "The more times you shred the file, the harder it will be to recover.\n"
			<< "Default number of iterations is 3.\n";
		std::exit(0);
	}

	if (argc != 5) {
		std::cout << "Invalid command-line arguments.\n";
		std::exit(1);
	}

	std::string filePath = argv[2];
	int iterations = std::atoi(argv[4]);

	if (iterations < 1 || iterations > 250) {
		std::cout << "Invalid number of iterations.\n";
		std::exit(1);
	}

	if (!file::exists(filePath)) {
		std::cout << "File not found.\n";
		std::exit(1);
	}

	try {
		std::size_t fileSize = file::file_size(filePath);

		if (fileSize < 1 || fileSize > 100'000'000) {
			std::cout << "Invalid file size.\n";
			return 1;
		}

		if ((file::status(filePath).permissions() & file::perms::owner_write) == file::perms::none) {
			std::cout << "You do not have permission to shred this file.\n";
			return 1;
		}

		shredFile(filePath, iterations);

		file::remove(filePath);
		std::cout << "File: " << filePath << " -> shredded successfully.\n";
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}
