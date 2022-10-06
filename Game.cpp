#include "Game.hpp"

#include "Connection.hpp"

#include <stdexcept>
#include <iostream>
#include <cstring>

#include <glm/gtx/norm.hpp>

Game::Game() {};

void Game::init() {
	player_count = 0;
	for (uint8_t i = 0; i < 8; i++) {
		players[i].status = PlayerStatus::ABSENT;
	}
}

uint8_t Game::spawn_player(Connection *c) {
	Player& player = players[player_count];
	player.status = PlayerStatus::GUESSING;
	srand((unsigned int)time(NULL));

	for (char c = 'A'; c <= 'Z'; c++) {
		player.scramble[c - 'A'] = c;
	}
	if (player_count % 2 == 0) {
		// This is a jumbled player
		unsigned seed = rand();
		std::shuffle(player.status.begin(), player.status.end(), std::default_random_engine(seed));
	}

	player_count++;
	player.number = player_count;
	player.connection = c;
	player.password = passwords[rand() % 20];
	player.progress = 0;
	send_password_message(player_count);
	for (auto& other_player : players) {
		if (other_player.status != PlayerStatus::ABSENT) {
			send_status_message(PlayerStatus::GUESSING, player_count);
		}
	}
	return player_count;
}

void Game::remove_player(uint8_t player_number) {
	players[player_number - 1].status = PlayerStatus::ABSENT;
	for (auto& player : players) {
		if (player.status != PlayerStatus::ABSENT) {
			send_status_message(PlayerStatus::ABSENT, player_number);
		}
	}
}

void Game::send_password_message(uint8_t player_number) const {
	auto& connection = *(players[player_number - 1].connection);
	std::string& pass = players[player_number - 1].password;
	size_t length = pass.length();

	// Indicate the type of message
	connection.send(Message::S2C_PASSWORD);
	connection.send(uint8_t(length));
	for (int i = 0; i < length; i++) {
		connection.send(pass[i]);
	}
}

void Game::send_status_message(Connection *c, PlayerStatus status, uint8_t player_number) const {
	assert(c);
	auto& connection = *c;

	// Indicate the type of message
	connection.send(Message::S2C_STATUS);
	// Say which player it's for
	connection.send(player_number);
	// Send the status
	connection.send(status);
}

void Game::send_key_message(Connection *c char key, uint8_t player_number) const {
	assert(c);
	auto& connection = *c;
	// Indicate the type of message
	connection.send(Message::S2C_KEY);
	// Say which player it's for
	connection.send(player_number);
	// Send the character
	connection.send(key);
}

bool Game::recv_key_message(uint8_t player_number) {
	auto& player = players[player_number - 1];
	auto& connection = *(player.connection);
	auto& buffer = connection.recv_buffer;

	// Keys are two characters long
	if (buffer.size() < 2) return false;
	// Ensure we have the right header
	if (buffer[0] != uint8_t(Message::C2S_KEY)) return false;
	// Grab the character
	char guess = player.scramble[buffer[1] - 'A'];
	// Tell everyone what was typed by who
	// But don't tell the person who typed it!
	for (auto& other_player : players) {
		if (other_player.number != player_number && players[other_player].status != PlayerStatus::ABSENT) {
			send_key_message(other_player.connection, guess, player_number);
		}
	}
	// Check if that guess was correct
	if (guess == player.password[player.progress]) {
		player.progress++;
		// Send solve message if necessary
		if (player.progress == player.password.length()) {
			player.status = PlayerStatus::SOLVED;
			for (auto& other_player : players) {
				send_status_message(other_player.connection, PlayerStatus::SOLVED, player_number);
			}
		}
	} else {
		player.progress = 0;
	}

	buffer.erase(buffer.begin(), buffer.begin() + 2);

	return true;
}
