#include "Mode.hpp"

#include "Connection.hpp"
#include "Game.hpp"

#include <glm/glm.hpp>

#include <array>

struct PlayMode : Mode {
	PlayMode(Client &client);
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override; 
	bool check_for_message(Connection* c); 
	bool check_for_password(Connection* c);
	bool check_for_solve(Connection* c);

	//----- game state -----

	struct ClientPlayer {
		std::string last_key;
		bool exists;
		bool solved;
	};

	std::array<ClientPlayer, 8> players;
	std::string my_password;
	char my_key;
	bool key_ready;
	bool i_solved;
	bool game_over;
	bool found_password;

	//connection to server:
	Client &client;

};
