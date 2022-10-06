#pragma once

#include <glm/glm.hpp>

#include <string>
#include <random>
#include <array>

struct Connection;

//Game state, separate from rendering.

//Currently set up for a "client sends controls" / "server sends whole state" situation.

enum Message : uint8_t {
	C2S_KEY,
	S2C_KEY,
	S2C_STATUS,
	S2C_PASSWORD
};

enum PlayerStatus : uint8_t {
	GUESSING,
	SOLVED,
	ABSENT
};

std::array<std::string, 20> passwords = ["APPLE", "DOG", "COMPUTER", "HOUSE", "JUNGLE",
	"SHADOW", "BRIDGE", "FRIEND", "MONSTER", "NEEDLE",
	"WATCH", "SHELL", "GHOST", "DONUT", "LANTERN",
	"MOON", "PLANE", "BOOK", "METAL", "OVEN"];

//state of one player in the game:
struct Player {
	PlayerStatus status;
	std::array<char, 26> scramble;
	uint8_t number;
	Connection *connection;
	std::string password;
};

struct Game {
	std::array<Player, 8> players;
	uint8_t player_count;
	uint8_t spawn_player(); //add player the end of the players list (may also, e.g., play some spawn anim)
	void remove_player(uint8_t player_number); //remove player from game (may also, e.g., play some despawn anim)

	Game();
	void init();

	//constants:
	//the update rate on the server:
	inline static constexpr float Tick = 1.0f / 30.0f;

	//---- communication helpers ----
	void send_password_message(uint8_t player_number) const;
	void send_status_message(PlayerStatus status, uint8_t player_number) const; 
	void send_key_message(char key, uint8_t player_number) const;
	bool recv_key_message(uint8_t player_number);
};
