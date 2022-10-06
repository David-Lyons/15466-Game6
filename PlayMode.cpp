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
		players[i].last_key = ' ';
	}
	key_ready = false;
	game_over = false;
	found_password = false;
	my_password = "";
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_a) {
			my_key = 'A';
			return true;
		} else if (evt.key.keysym.sym == SDLK_b) {
			my_key = 'B';
			return true;
		} else if (evt.key.keysym.sym == SDLK_c) {
			my_key = 'C';
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			my_key = 'D';
			return true;
		} else if (evt.key.keysym.sym == SDLK_e) {
			my_key = 'E';
			return true;
		} else if (evt.key.keysym.sym == SDLK_f) {
			my_key = 'F';
			return true;
		} else if (evt.key.keysym.sym == SDLK_g) {
			my_key = 'G';
			return true;
		} else if (evt.key.keysym.sym == SDLK_h) {
			my_key = 'H';
			return true;
		} else if (evt.key.keysym.sym == SDLK_i) {
			my_key = 'I';
			return true;
		} else if (evt.key.keysym.sym == SDLK_j) {
			my_key = 'J';
			return true;
		} else if (evt.key.keysym.sym == SDLK_k) {
			my_key = 'K';
			return true;
		} else if (evt.key.keysym.sym == SDLK_l) {
			my_key = 'L';
			return true;
		} else if (evt.key.keysym.sym == SDLK_m) {
			my_key = 'M';
			return true;
		} else if (evt.key.keysym.sym == SDLK_n) {
			my_key = 'N';
			return true;
		} else if (evt.key.keysym.sym == SDLK_o) {
			my_key = 'O';
			return true;
		} else if (evt.key.keysym.sym == SDLK_p) {
			my_key = 'P';
			return true;
		} else if (evt.key.keysym.sym == SDLK_q) {
			my_key = 'Q';
			return true;
		} else if (evt.key.keysym.sym == SDLK_r) {
			my_key = 'R';
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			my_key = 'S';
			return true;
		} else if (evt.key.keysym.sym == SDLK_t) {
			my_key = 'T';
			return true;
		} else if (evt.key.keysym.sym == SDLK_u) {
			my_key = 'U';
			return true;
		} else if (evt.key.keysym.sym == SDLK_v) {
			my_key = 'V';
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			my_key = 'W';
			return true;
		} else if (evt.key.keysym.sym == SDLK_x) {
			my_key = 'X';
			return true;
		} else if (evt.key.keysym.sym == SDLK_y) {
			my_key = 'Y';
			return true;
		} else if (evt.key.keysym.sym == SDLK_z) {
			my_key = 'Z';
			return true;
		}
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
	for (uint8_t i = 2; i < 2 + size; i++) {
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
		players[buffer[1] - 1].last_key = buffer[2];
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
		for (uint8_t i = 0; i < 8; i++) {
			game_over = true;
			if (players[i].exists && !players[i].solved) {
				game_over = false;
			}
			if (game_over) printf("We have a winner!");
		}
		return true;
	}
	return false;
}

void PlayMode::update(float elapsed) {
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
			std::cout << "[" << c->socket << "] closed (!)" << std::endl;
			throw std::runtime_error("Lost connection to server!");
		} else { 
			assert(event == Connection::OnRecv);
			//std::cout << "[" << c->socket << "] recv'd data. Current buffer:\n" << hex_dump(c->recv_buffer); std::cout.flush(); //DEBUG
			bool handled_message;
			try {
				do {
					handled_message = false;
					if (check_for_password(c)) handled_message = true;
					if (check_for_message(c)) handled_message = true;
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

	static std::array< glm::vec2, 16 > const circle = [](){
		std::array< glm::vec2, 16 > ret;
		for (uint32_t a = 0; a < ret.size(); ++a) {
			float ang = a / float(ret.size()) * 2.0f * float(M_PI);
			ret[a] = glm::vec2(std::cos(ang), std::sin(ang));
		}
		return ret;
	}();

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	
	//figure out view transform to center the arena:
	float aspect = float(drawable_size.x) / float(drawable_size.y);
	float scale = std::min(
		2.0f * aspect / (Game::ArenaMax.x - Game::ArenaMin.x + 2.0f * Game::PlayerRadius),
		2.0f / (Game::ArenaMax.y - Game::ArenaMin.y + 2.0f * Game::PlayerRadius)
	);
	glm::vec2 offset = -0.5f * (Game::ArenaMax + Game::ArenaMin);

	glm::mat4 world_to_clip = glm::mat4(
		scale / aspect, 0.0f, 0.0f, offset.x,
		0.0f, scale, 0.0f, offset.y,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	{
		DrawLines lines(world_to_clip);

		//helper:
		auto draw_text = [&](glm::vec2 const &at, std::string const &text, float H) {
			lines.draw_text(text,
				glm::vec3(at.x, at.y, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0x00, 0x00, 0x00, 0x00));
			float ofs = (1.0f / scale) / drawable_size.y;
			lines.draw_text(text,
				glm::vec3(at.x + ofs, at.y + ofs, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		};

		draw_text(glm::vec2(drawable_size.x / 3, drawable_size.y - (drawable_size.y / 8)), my_password, 0.1f);

		for (uint8_t i = 0; i < 8; i++) {
			if (players[i].exists) {
				draw_text(glm::vec2(drawable_size.x / 10.0f, drawable_size.y / 16.0f + (drawable_size.y / 8.0f) * i), "Player " + std::to_string(i + 1), 0.1f);
			}
		}
	}
	GL_ERRORS();
}
