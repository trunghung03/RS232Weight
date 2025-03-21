#include <iostream>
#include <iomanip>
#include <numbers>
#include <array>
#include <string>
#include <sstream>
#include <ranges>
#include <algorithm>

#ifdef WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include "SimpleSerial.hpp"

void test_com(std::array<uint8_t, 15>& input) {
	std::string str = reinterpret_cast<char*>(input.data());
	str.resize(15);

	try {

		SimpleSerial serial("COM6", 115200);

		serial.writeString(str);

		std::cout << "Sent: " << str;

	}
	catch (boost::system::system_error& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}
}

float exponential_decay(const float N_start, const float lambda, const int t) {
	float N_result = N_start * std::pow(std::numbers::e_v<float>, -lambda * t);
	return N_result;
}

std::array<uint8_t, 7> float_to_char(const float number) {
	int int_part = (int)number;

	std::stringstream buffer{};
	buffer << std::setw(3) << std::setfill('0') << int_part % 1000;

	int decimal_part = (number - int_part) * 1000;
	buffer << '.' << std::setw(3) << std::setfill('0') << decimal_part;

	std::string result = buffer.str();
	std::array<uint8_t, 7> final_array{};
#ifdef min
#undef min
#endif
	std::copy_n(result.begin(), std::min(result.size(), final_array.size()), final_array.begin());
	return final_array;
}

std::array<uint8_t, 15> convert_to_array(const float weight) {
	std::array<uint8_t, 15> result{};

	result.at(0) = 'W';
	result.at(1) = 'G';

	auto weight_string = float_to_char(weight);
	for (int i = 2, n = 0; i <= 8; i++, n++) {
		result.at(i) = weight_string.at(n);
	}

	result.at(9) = 'k';
	result.at(10) = 'g';

	result.at(11) = 'x';
	result.at(12) = 'x';

	result.at(13) = 0x0d;
	result.at(14) = 0x0a;

	return result;
}

void print_bytes(const std::array<uint8_t, 15>& bytes) {
	for (auto& byte : bytes) {
		std::cout << char(byte);
	}
	//std::cout << "\t";
	//for (auto& byte : bytes) {
	//	std::cout << " 0x" << std::hex << int(byte);
	//}
	//std::cout << std::endl;
}

void loop() {
	float target = 0.0f; // kg
	while (target <= 0.0f) {
		std::cout << "Enter weight: ";
		std::cin >> target;
	}

	float imprecision = 0.20f;

	std::cout << std::setprecision(3) << std::fixed;

	float N_start = target * imprecision;
	float lambda = 2;
	float max_steps = 100;
	for (int step = 0; step < max_steps; step++) {
		float n = exponential_decay(N_start, lambda, step);
		float scale_number = target + n;

		auto scale_string = convert_to_array(scale_number);
		//print_bytes(scale_string);
		test_com(scale_string);

		if (n < 0.0004) break;
	}
	std::cout << std::endl;
}

int main()
{
	while (true) {
		loop();
	}

	return 0;
}