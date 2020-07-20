#include "header.hpp"
#include "game.hpp"

//テスト用
void user_test()
{
}

int main()
{
	UDdelete_calc();
	LRdelete_calc();
	UDburst_calc();
	LRburst_calc();

	cout << PROGRAM_NAME << endl;
	cout.flush();
	//user_test();

	Game game;
	game.init_game();
	game.competition();

	return EXIT_SUCCESS;
}