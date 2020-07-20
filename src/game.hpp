#pragma once
#include "header.hpp"
#include "player.hpp"

struct Game
{
	Player player[2];
	void init_game();
	void input_game();
	void print_game();
	void competition();
	SearchInfo set_search_info();
	int set_req_score();
};