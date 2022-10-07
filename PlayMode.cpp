#include "PlayMode.hpp"

#include "DrawLines.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "hex_dump.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <random>
#include <array>

PlayMode::PlayMode(Client &client_) : client(client_) {
	for (uint8_t i = 0; i < 8; i++) {
		players[i].solved = false;
		players[i].exists = false;
		players[i].last_key = "";
	}
	key_ready = false;
	game_over = false;
	found_password = false;
	i_solved = false;
	time = 0.0f;
	my_password = "";
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (game_over || key_ready) {
		return false;
	}
	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_a) {
			my_key = 'A';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_b) {
			my_key = 'B';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_c) {
			my_key = 'C';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			my_key = 'D';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_e) {
			my_key = 'E';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_f) {
			my_key = 'F';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_g) {
			my_key = 'G';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_h) {
			my_key = 'H';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_i) {
			my_key = 'I';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_j) {
			my_key = 'J';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_k) {
			my_key = 'K';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_l) {
			my_key = 'L';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_m) {
			my_key = 'M';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_n) {
			my_key = 'N';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_o) {
			my_key = 'O';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_p) {
			my_key = 'P';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_q) {
			my_key = 'Q';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_r) {
			my_key = 'R';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			my_key = 'S';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_t) {
			my_key = 'T';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_u) {
			my_key = 'U';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_v) {
			my_key = 'V';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			my_key = 'W';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_x) {
			my_key = 'X';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_y) {
			my_key = 'Y';
			key_ready = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_z) {
			my_key = 'Z';
			key_ready = true;
			return true;
		}
	}

	return false;
}

bool PlayMode::check_for_solve(Connection* c) {
	assert(c);
	auto& connection = *c;
	auto& buffer = connection.recv_buffer;
	if (buffer.size() >= 1 && buffer[0] == uint8_t(Message::S2C_SOLVE)) {
		i_solved = true;
		buffer.erase(buffer.begin(), buffer.begin() + 1);
		return true;
	} else if (buffer.size() >= 1 && buffer[0] == uint8_t(Message::S2C_GAMEOVER)) {
		i_solved = true;
		game_over = true;
		buffer.erase(buffer.begin(), buffer.begin() + 1);
		return true;
	}
	return false;
}

bool PlayMode::check_for_password(Connection* c) {
	assert(c);
	auto& connection = *c;
	auto& buffer = connection.recv_buffer;
	if (buffer.size() < 2) return false;
	if (buffer[0] != uint8_t(Message::S2C_PASSWORD)) return false;
	uint8_t size = buffer[1];
	if (buffer.size() < 2 + size) return false;
	for (uint8_t i = 2; i < uint8_t(2 + size); i++) {
		my_password += buffer[i];
	}
	buffer.erase(buffer.begin(), buffer.begin() + 2 + size);
	return true;
}

bool PlayMode::check_for_message(Connection *c) {
	assert(c);
	auto& connection = *c;
	auto& buffer = connection.recv_buffer;
	if (buffer.size() < 3) return false;
	if (buffer[0] == uint8_t(Message::S2C_KEY)) {
		if (players[buffer[1] - 1].last_key.length() == 0) {
			players[buffer[1] - 1].last_key += buffer[2];
		} else {
			players[buffer[1] - 1].last_key[0] = buffer[2];
		}
		buffer.erase(buffer.begin(), buffer.begin() + 3);
		return true;
	} else if (buffer[0] == uint8_t(Message::S2C_STATUS)) {
		if (buffer[2] == PlayerStatus::ABSENT) {
			players[buffer[1] - 1].exists = false;
		} else if (buffer[2] == PlayerStatus::GUESSING) {
			players[buffer[1] - 1].exists = true;
			players[buffer[1] - 1].solved = false;
		} else if (buffer[2] == PlayerStatus::SOLVED) {
			players[buffer[1] - 1].exists = true;
			players[buffer[1] - 1].solved = true;
		}
		buffer.erase(buffer.begin(), buffer.begin() + 3);
		return true;
	}
	return false;
}

void PlayMode::update(float elapsed) {
	if (game_over) {
		return;
	}
	time += elapsed;
	if (key_ready) {
		client.connection.send(Message::C2S_KEY);
		client.connection.send(my_key);
		key_ready = false;
	}

	//send/receive data:
	client.poll([this](Connection *c, Connection::Event event){
		if (event == Connection::OnOpen) {
			std::cout << "[" << c->socket << "] opened" << std::endl;
		} else if (event == Connection::OnClose) {
			std::cout << "FATAL: Server has died.\n";
			game_over = true;
		} else { 
			assert(event == Connection::OnRecv);
			try {
				bool handled_message;
				do {
					handled_message = false;
					if (check_for_password(c)) handled_message = true;
					if (check_for_message(c)) handled_message = true;
					if (check_for_solve(c)) handled_message = true;
				} while (handled_message);
			} catch (std::exception const &e) {
				std::cerr << "[" << c->socket << "] malformed message from server: " << e.what() << std::endl;
				//quit the game:
				throw e;
			}
		}
	}, 0.0);
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	float aspect = float(drawable_size.x) / float(drawable_size.y);
	DrawLines lines(glm::mat4(
		1.0f / aspect, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	));

	{
		//helper:
		auto draw_text = [&](glm::vec2 const &at, std::string const &text, float H, glm::u8vec4 color) {
			lines.draw_text(text,
				glm::vec3(at.x, at.y, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				color);
		};

		glm::u8vec4 white = glm::u8vec4(0xFF, 0xFF, 0xFF, 0xFF);
		glm::u8vec4 green = glm::u8vec4(0x00, 0xFF, 0x00, 0xFF);
		
		if (i_solved) {
			draw_text(glm::vec2(-0.25f, 0.5f), my_password, 0.1f, green);
		} else {
			draw_text(glm::vec2(-0.25f, 0.5f), my_password, 0.1f, white);
		}

		if (game_over) {
			draw_text(glm::vec2(-0.3f, 0.8f), "Everyone solved! Took " + std::to_string((int)time) + " seconds!", 0.1f, green);
		}

		for (uint8_t i = 0; i < 8; i++) {
			if (players[i].exists) {
				std::string message = "Player " + std::to_string(i + 1) + ": ";
				if (players[i].solved) {
					message += "Solved! ";
				} else {
					message += "Guessing...";
				}
				message += players[i].last_key;
				draw_text(glm::vec2(-0.25f, 0.25f - 0.15f * i), message, 0.1f, white);
			}
		}
	}
	GL_ERRORS();
}
