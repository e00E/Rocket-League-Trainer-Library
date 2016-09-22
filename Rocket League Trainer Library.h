#ifdef ROCKETLEAGUETRAINERLIBRARY_EXPORTS
#define ROCKETLEAGUETRAINERLIBRARY_API __declspec(dllexport)
#else
#define ROCKETLEAGUETRAINERLIBRARY_API __declspec(dllimport)
#endif

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <vector>

namespace rocket_league_trainer {
	struct ROCKETLEAGUETRAINERLIBRARY_API vec3 {
		float x, y, z;

		std::string to_string() const;
	};

	struct ROCKETLEAGUETRAINERLIBRARY_API entity {
		vec3 position;
		vec3 velocity;
		vec3 rotation_row_0;
		vec3 rotation_row_1;
		vec3 rotation_row_2;
		vec3 angular_velocity;

		std::string to_string() const;
	};

	// Attach to the Rocket League process.
	// Returns true on success, false on fail.
	// Must be called with success before the other functions.
	ROCKETLEAGUETRAINERLIBRARY_API bool attach_to_process();

	ROCKETLEAGUETRAINERLIBRARY_API bool is_in_freeplay();

	// Return true on success and false on fail
	// Should work if and only if RL is in freeplay.
	ROCKETLEAGUETRAINERLIBRARY_API bool get_ball(entity& out_e); // Retrieve the ball from memory
	ROCKETLEAGUETRAINERLIBRARY_API bool set_ball(entity const& e); // Commit the ball to memory

	ROCKETLEAGUETRAINERLIBRARY_API bool get_car(entity& out_e);
	ROCKETLEAGUETRAINERLIBRARY_API bool set_car(entity const& e);

	// Enable or diable scoring goals in free play.
	ROCKETLEAGUETRAINERLIBRARY_API void disable_scoring();
	ROCKETLEAGUETRAINERLIBRARY_API void enable_scoring();


	// The following declarations are used by this library internally.
	// You can completely ignore them but they might be useful sometimes.

	// Multiple pointers in a row with offsets between.
	// Start at the offset start compared to the process' main module's base address
	// then follow pointers at offsets.
	struct cascade_pointer {
		unsigned int start;
		std::vector<int> offsets;

		ROCKETLEAGUETRAINERLIBRARY_API unsigned char* get_final_address(); // or null if the pointer is null at any point in the cascade
	};

	// All the state the library keeps.
	struct ROCKETLEAGUETRAINERLIBRARY_API rocket_league_info {
		HANDLE process; // Handle to the Rocket League process
		unsigned char* base_address; // Base address of the main module used for the cascade_pointers .
	};
	ROCKETLEAGUETRAINERLIBRARY_API extern rocket_league_info rli;

	// Hard coded cascade_pointers to the car and ball.
	// They are supposed to be null at some point in their chain
	// if Rocket League is not in freeplay which is why the are used
	// to detect freeplay too.
	ROCKETLEAGUETRAINERLIBRARY_API extern cascade_pointer car_position;
	ROCKETLEAGUETRAINERLIBRARY_API extern cascade_pointer ball_position;

	// The following functions and variables are used to find the memory addresses of
	// the car and ball should a Rocket League update invalidate the hard coded pointers.
	
	// Find the signatures of the ball and car and store them in the parameters.
	// Returns true iff both signatures were found.
	ROCKETLEAGUETRAINERLIBRARY_API bool signature_scan(unsigned char*& car_signature_address, unsigned char *& ball_signature_address);

	// Byte arrays which occur only near the ball and car respectively.
	ROCKETLEAGUETRAINERLIBRARY_API extern const std::vector<unsigned char> car_signature;
	ROCKETLEAGUETRAINERLIBRARY_API extern const std::vector<unsigned char> ball_signature;

	// Offsets from the above start of the signature to the values we are interested in.
	// For example to get the address of the position when  we already found a signature:
	// address = signature_address + signature_position_offset
	ROCKETLEAGUETRAINERLIBRARY_API extern const int signature_position_offset;
	ROCKETLEAGUETRAINERLIBRARY_API extern const int signature_velocity_offset;
	ROCKETLEAGUETRAINERLIBRARY_API extern const int signature_rotation_offset;
	ROCKETLEAGUETRAINERLIBRARY_API extern const int signature_angular_velocity_offset;

	// Work with entities using their signature's address
	ROCKETLEAGUETRAINERLIBRARY_API entity get_entity_from_signature(unsigned char* signature_address);
	ROCKETLEAGUETRAINERLIBRARY_API void set_entity_from_signature(unsigned char* signature_address, entity const& e);

	// Work with entities using their position's address
	ROCKETLEAGUETRAINERLIBRARY_API entity get_entity_from_position(unsigned char* signature_address);
	ROCKETLEAGUETRAINERLIBRARY_API void set_entity_from_position(unsigned char* signature_address, entity const& e);

	// Read and write vec3 from and to Rocket League's memory
	ROCKETLEAGUETRAINERLIBRARY_API void read_vec3(unsigned char* from, vec3& to);
	ROCKETLEAGUETRAINERLIBRARY_API void write_vec3(unsigned char* to, vec3& from);
}