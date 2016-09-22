#include <iostream>
#include "Rocket League Trainer Library.h"

#include <thread>
#include <chrono>
#include <iomanip>

using namespace rocket_league_trainer;

int main()
{
	if (attach_to_process()) {
		std::cout << "Found Rocket League process.\n";
		unsigned char *car, *ball;
		signature_scan(car, ball);
		//std::cout << "ball pos " << std::hex << reinterpret_cast<size_t>(ball + signature_position_offset) << "\n";
		//std::cout << "car pos " << std::hex << reinterpret_cast<size_t>(car + signature_position_offset) << "\n";
		unsigned int sleep_milliseconds(50);
		std::cout << "Reading speed every " << sleep_milliseconds << "ms" << "\n";

		while (true) {
			entity e(get_entity_from_signature(car));
			float speed1 = std::sqrt(std::pow(e.velocity.x, 2) + std::pow(e.velocity.y, 2) + std::pow(e.velocity.z, 2));
			float speed2 = std::sqrt(std::pow(e.velocity.x, 2) + std::pow(e.velocity.y, 2));
			std::cout << "speed on x y z axis: " << std::fixed << std::setprecision(2) << e.velocity.x << " " << e.velocity.y << " " << e.velocity.z << "\n"
				<< "= total speed: " << speed1 << "\n"
				<< "without z: " << speed2 << "\n" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_milliseconds));
		}

	}
	std::cout << "Didnt find Rocket League process.\n";
	return 0;
}

