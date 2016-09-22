#include "stdafx.h"
#include "Rocket League Trainer Library.h"

namespace rocket_league_trainer {
	const std::vector<unsigned char> car_signature = { 0x6B, 0x0B, 0x5E, 0x5D, 0xD0, 0x03 };
	const std::vector<unsigned char> ball_signature = { 0x6B, 0x0B, 0x5E, 0x5D, 0x10, 0x07 };

	const int signature_position_offset = -132;
	const int signature_velocity_offset = 140;
	const int signature_rotation_offset = -180;
	const int signature_angular_velocity_offset = 156;

	cascade_pointer ball_position = {
		0x0169d4d0,
		{ 0x130, 0x4c, 0x150, 0x64, 0x40, }
	};
	cascade_pointer car_position = {
		0x01631864,
		{ 0xfc, 0x668, 0xe4, 0x40 }
	};

	const unsigned int scoring_code_address = 0xad33b5;

	rocket_league_info rli = {};

	unsigned char* cascade_pointer::get_final_address() {
		unsigned char* current(rli.base_address + start);

		for (int i(0); i < offsets.size(); ++i) {

			if (ReadProcessMemory(rli.process, current, &current, 4, NULL) == 0) { return NULL; };
			current += offsets[i];
		}
		return current;
	}

	bool attach_to_process() {
		/* Currently not needed as there is no text output.

		// Fix std::cout not working from the dll.
		// From here http://stackoverflow.com/questions/311955/redirecting-cout-to-a-console-in-windows/25927081#25927081 .
		// I dont know what exactly it does.
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		std::wcout.clear();
		std::cout.clear();
		std::wcerr.clear();
		std::cerr.clear();
		std::wcin.clear();
		std::cin.clear();*/

		DWORD pid;

		// Find the pid of the process with image "RocketLeague.exe"
		{
			HANDLE processes = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (processes == INVALID_HANDLE_VALUE) {
				return false;
			}
			PROCESSENTRY32 process_entry;
			process_entry.dwSize = sizeof(PROCESSENTRY32);
			if (!Process32First(processes, &process_entry)) {
				CloseHandle(processes);
				return false;
			}

			do {
				if (wcscmp(process_entry.szExeFile, L"RocketLeague.exe") == 0) {
					pid = process_entry.th32ProcessID;
					break;
				}
			} while (Process32Next(processes, &process_entry));
			CloseHandle(processes);
		}

		// Finding the main module of the process like here http://stackoverflow.com/questions/11564148/how-to-get-the-starting-base-address-of-a-process-in-c/25355713#25355713
		{
			HANDLE modules = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE, pid);
			if (modules == INVALID_HANDLE_VALUE) {
				return false;
			}
			MODULEENTRY32 module_entry;
			module_entry.dwSize = sizeof(MODULEENTRY32);
			if (!Module32First(modules, &module_entry)) {
				CloseHandle(modules);
				return false;
			}

			do {
				if (wcscmp(module_entry.szModule, L"RocketLeague.exe") == 0) {
					rli.base_address = module_entry.modBaseAddr;
					break;
				}
			} while (Module32Next(modules, &module_entry));
			CloseHandle(modules);
		}

		rli.process = OpenProcess(
			PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION,
			false,
			pid);
		return rli.process != NULL;
	}

	bool is_in_freeplay() {
		return ball_position.get_final_address() != NULL && car_position.get_final_address() != NULL;
	}

	void disable_scoring() {
		unsigned char *address(rli.base_address + scoring_code_address);
		unsigned char code[] = { 0xE9, 0xB7, 0x06, 0x00, 0x00 };
		WriteProcessMemory(rli.process, address, code, 5, NULL);
	}
	void enable_scoring() {
		unsigned char *address(rli.base_address + scoring_code_address);
		unsigned char code[] = { 0x0F, 0x84, 0xB6, 0x06, 0x00, 0x00 };
		WriteProcessMemory(rli.process, address, code, 6, NULL);
	}

	void read_vec3(unsigned char* from, vec3& to) {
		ReadProcessMemory(rli.process, from, &to, sizeof(vec3), NULL);
	}

	void write_vec3(unsigned char* to, vec3 const& from) {
		WriteProcessMemory(rli.process, to, &from, sizeof(vec3), NULL);
	}

	entity get_entity_from_signature(unsigned char* signature_address) {
		entity e;
		read_vec3(signature_address + signature_position_offset, e.position);
		read_vec3(signature_address + signature_velocity_offset, e.velocity);
		read_vec3(signature_address + signature_rotation_offset, e.rotation_row_0);
		// There is one zero byte after every row of the rotation matrix
		read_vec3(signature_address + signature_rotation_offset + 1 * 4 * 4, e.rotation_row_1);
		read_vec3(signature_address + signature_rotation_offset + 2 * 4 * 4, e.rotation_row_2);
		read_vec3(signature_address + signature_angular_velocity_offset, e.angular_velocity);
		return e;
	}

	void set_entity_from_signature(unsigned char* signature_address, entity const& e) {
		write_vec3(signature_address + signature_position_offset, e.position);
		write_vec3(signature_address + signature_velocity_offset, e.velocity);
		write_vec3(signature_address + signature_rotation_offset, e.rotation_row_0);
		write_vec3(signature_address + signature_rotation_offset + 1 * 4 * 4, e.rotation_row_1);
		write_vec3(signature_address + signature_rotation_offset + 2 * 4 * 4, e.rotation_row_2);
		write_vec3(signature_address + signature_angular_velocity_offset, e.angular_velocity);
	}

	entity get_entity_from_position(unsigned char* position_address) {
		return get_entity_from_signature(position_address - signature_position_offset);
	}

	void set_entity_from_position(unsigned char* position_address, entity const& e) {
		return set_entity_from_signature(position_address - signature_position_offset, e);
	}

	bool get_ball(entity& out_e) {
		unsigned char* address = ball_position.get_final_address();
		if (address != NULL) {
			out_e = get_entity_from_position(address);
			return true;
		}
		return false;
	}

	bool set_ball(entity const& e) {
		unsigned char* address = ball_position.get_final_address();
		if (address != NULL) {
			set_entity_from_position(address, e);
			return true;
		}
		return false;
	}

	bool get_car(entity& out_e) {
		unsigned char* address = car_position.get_final_address();
		if (address != NULL) {
			out_e = get_entity_from_position(address);
			return true;
		}
		return false;
	}

	bool set_car(entity const& e) {
		unsigned char* address = car_position.get_final_address();
		if (address != NULL) {
			set_entity_from_position(address, e);
			return true;
		}
		return false;
	}

	bool signature_scan(unsigned char*& car_signature_address, unsigned char*& ball_signature_address) {
		if (rli.process == NULL) {
			return false;
		}

		unsigned char* current_address = NULL;
		MEMORY_BASIC_INFORMATION info;

		bool car_found(false);
		bool ball_found(false);

		for (current_address = NULL;
			VirtualQueryEx(rli.process, current_address, &info, sizeof(info)) == sizeof(info);
			current_address += info.RegionSize)
		{
			if (info.State == MEM_COMMIT && info.Type == MEM_PRIVATE && info.AllocationProtect == PAGE_READWRITE) {
				std::vector<unsigned char> buffer;

				SIZE_T bytes_read;
				buffer.resize(info.RegionSize);
				ReadProcessMemory(rli.process, current_address, &buffer[0], info.RegionSize, &bytes_read);
				buffer.resize(bytes_read);

				if (!car_found) {
					auto result = std::search(buffer.begin(), buffer.end(), car_signature.begin(), car_signature.end());
					if (result != buffer.end()) {
						car_signature_address = current_address + (result - buffer.begin());
						car_found = true;
						if (ball_found) { break; }
					}
				}

				if (!ball_found) {
					auto result = std::search(buffer.begin(), buffer.end(), ball_signature.begin(), ball_signature.end());
					if (result != buffer.end()) {
						ball_signature_address = current_address + (result - buffer.begin());
						ball_found = true;
						if (car_found) { break; }
					}
				}
			}
		}
		return car_found && ball_found;
	}

	std::string vec3::to_string() const
	{
		return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
	}

	std::string entity::to_string() const
	{
		return "position: " + position.to_string() +
			   "\nvelocity: " + velocity.to_string() +
			   "\nrotation:\n" +
			   rotation_row_0.to_string() + "\n" +
			   rotation_row_1.to_string() + "\n" + 
			   rotation_row_2.to_string() + 
			   "\nangular_velocity: " + angular_velocity.to_string();
	}
}