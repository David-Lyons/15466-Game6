
#include "Connection.hpp"

#include "hex_dump.hpp"

#include "Game.hpp"

#include <chrono>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <unordered_map>

#ifdef _WIN32
extern "C" { uint32_t GetACP(); }
#endif
int main(int argc, char **argv) {
#ifdef _WIN32
	{ //when compiled on windows, check that code page is forced to utf-8 (makes file loading/saving work right):
		//see: https://docs.microsoft.com/en-us/windows/apps/design/globalizing/use-utf8-code-page
		uint32_t code_page = GetACP();
		if (code_page == 65001) {
			std::cout << "Code page is properly set to UTF-8." << std::endl;
		} else {
			std::cout << "WARNING: code page is set to " << code_page << " instead of 65001 (UTF-8). Some file handling functions may fail." << std::endl;
		}
	}

	//when compiled on windows, unhandled exceptions don't have their message printed, which can make debugging simple issues difficult.
	try {
#endif

	//------------ argument parsing ------------

	if (argc != 2) {
		std::cerr << "Usage:\n\t./server <port>" << std::endl;
		return 1;
	}

	//------------ initialization ------------

	Server server(argv[1]);

	//------------ main loop ------------

	//keep track of which connection is controlling which player:
	std::unordered_map< Connection *, Player * > connection_to_player;
	//keep track of game state:
	Game game;
	game.init();
	auto start_time = std::chrono::steady_clock::now();

	while (true) {
		static auto next_tick = std::chrono::steady_clock::now() + std::chrono::duration< double >(Game::Tick);
		//process incoming data from clients until a tick has elapsed:
		while (true) {
			auto now = std::chrono::steady_clock::now();
			if (!game.game_over) {
				game.game_time = std::chrono::duration<float>(now - start_time).count();
			}
			double remain = std::chrono::duration< double >(next_tick - now).count();
			if (remain < 0.0) {
				next_tick += std::chrono::duration< double >(Game::Tick);
				break;
			}

			//helper used on client close (due to quit) and server close (due to error):
			auto remove_connection = [&](Connection *c) {
				Player *player = connection_to_player[c];
				assert(player != nullptr);
				game.remove_player(player->number);
				connection_to_player[c] = nullptr;
			};

			server.poll([&](Connection* c, Connection::Event evt) {
				if (evt == Connection::OnOpen) {
					//client connected:
					uint8_t player = game.spawn_player(c);
					if (game.game_over) {
						c->close();
					}
					if (player != 0) {
						connection_to_player[c] = &game.players[player - 1];
					} else {
						c->close();
					}

				} else if (evt == Connection::OnClose) {
					//client disconnected:

					remove_connection(c);

				} else { 
					assert(evt == Connection::OnRecv);
					//got data from client:

					//look up in players list:
					Player* player = connection_to_player[c];
					assert(player != nullptr);

					//handle messages from client:
					try {
						bool handled_message;
						do {
							handled_message = false;
							if (game.recv_key_message(player->number)) handled_message = true;
						} while (handled_message);
					} catch (std::exception const &e) {
						std::cout << "Disconnecting client:" << e.what() << std::endl;
						c->close();
						remove_connection(c);
					}
				}
			}, remain);
		}
	}


	return 0;

#ifdef _WIN32
	} catch (std::exception const &e) {
		std::cerr << "Unhandled exception:\n" << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "Unhandled exception (unknown type)." << std::endl;
		throw;
	}
#endif
}
