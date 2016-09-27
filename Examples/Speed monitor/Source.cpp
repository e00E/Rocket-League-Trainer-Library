#include <iostream>
#include <fstream>
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

		int64_t nanosleeptime(0);
		std::cout << "Enter tick length in nano seconds: ";
		std::cin >> nanosleeptime;
		std::chrono::nanoseconds sleep_time(nanosleeptime); //128 tickrate would be 7812500 nano seconds.
		double sleep_seconds = nanosleeptime / 1000000000.0;
		std::cout << "Reading speed every " << sleep_seconds << "s" << "\n";

		std::ofstream output("speed log.txt", std::ios_base::trunc);
		output << "Read interval: " << sleep_seconds << "s" << "\n\n";

		//double last_speed1(0);
		//double last_speed2(0);

		std::chrono::time_point<std::chrono::high_resolution_clock> now(std::chrono::high_resolution_clock::now());

		while (true) {
			entity e(get_entity_from_signature(car));
			//std::cout << std::unitbuf;
			double speed1 = std::sqrt(std::pow(e.velocity.x, 2) + std::pow(e.velocity.y, 2) + std::pow(e.velocity.z, 2));
			double speed2 = std::sqrt(std::pow(e.velocity.x, 2) + std::pow(e.velocity.y, 2));

			//double acceleration1((speed1 - last_speed1) / sleep_seconds);
			//double acceleration2((speed2 - last_speed2) / sleep_seconds);

			std::cout << std::fixed << std::setprecision(std::numeric_limits<long double>::digits10 + 1) << "Speed:\n" << "x y z: " << e.velocity.x << " " << e.velocity.y << " " << e.velocity.z << "\n"
				<< "Total with z: " << speed1 << "\nWithout z   : " << speed2 << "\n"
				/*<< "Acceleration to last measure:\n" << "With z : " << acceleration1 << "\nWithout: " << acceleration2 << "\n" */<< std::endl;
			output << std::fixed << std::setprecision(8) << "Speed:\n" << "x y z: " << e.velocity.x << " " << e.velocity.y << " " << e.velocity.z << "\n"
				<< "Total with z: " << speed1 << "\nWithout z   : " << speed2 << "\n"
				/*<< "Acceleration to last measure:\n" << "With z : " << acceleration1 << "\nWithout: " << acceleration2 << "\n" */<< std::endl;
			//last_speed1 = speed1;
			//last_speed2 = speed2;
			now += sleep_time;
			std::this_thread::sleep_until(now);
		}

	}
	std::cout << "Didnt find Rocket League process.\n";
	return 0;
}

