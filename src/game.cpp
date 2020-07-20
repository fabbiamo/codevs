#include "header.hpp"
#include "game.hpp"

int N_PLY;
Block history[RECORD_SIZE][PACK_SIZE];
UDdelete_patterns UDdelete[12][size];
uint64_t LRdelete[size];
uint64_t UDburst[size];
uint64_t LRburst[size];

void Game::init_game()
{
	N_PLY = 0;
	player[0].init_player();
	player[1].init_player();
}

void Game::input_game()
{
	cin >> N_PLY;
	player[0].input_player();
	player[1].input_player();
}

void Game::print_game()
{
	cout << N_PLY;
	player[0].print_player();
	player[1].print_player();
}

void Game::competition()
{
	FutureOrder future;
	SearchInfo beam;
	bool done, drop;
	int result[RECORD_SIZE] = {};
	int time_level;
	int last_fire = 0;
	done = drop = false;

	cerr << "game start" << endl;

	input_history();
	for (int ply = 0; ply < RECORD_SIZE; ply++)
	{
		input_game();
		beam = set_search_info();

		if (player[0].info.timeleft > 120 * 1000) //17
			time_level = 0;
		else if (player[0].info.timeleft > 60 * 1000) // 13
			time_level = 1;
		else if (player[0].info.timeleft > 30 * 1000) // 9
			time_level = 2;
		else if (player[0].info.timeleft > 15 * 1000) // 2
			time_level = 3;
		else
			time_level = 4;

		if ((not future.fire) or future.historder.empty() or (future.historder.front() == USE_SKILL and player[0].info.stock < 80))
			done = false;
		if ((not drop) and player[0].info.stock >= 10)
			done = false;
		drop = player[0].info.stock >= 10; //次のターンに使う

		if (not done)
		{
			future = player[0].beam_search(beam, time_level, last_fire);
			result[ply] = future.max_score;
		}

		if (not future.historder.empty())
		{
			player[0].output_order(future.historder.front());
			future.historder.erase(future.historder.begin());
		}
		else
			player[0].output_order(ORDER_0);
		done = future.fire;
		if (done and future.historder.empty())
			last_fire = ply; //N_PLY
		if (ply >= 1 and result[ply - 1] == result[ply])
			done = true; // 2回連続結果が同じ場合強制発火
	}
}

//ビームサーチの深さと幅について
//プレイヤのスコアなども判断材料にさせたい
SearchInfo Game::set_search_info()
{
	SearchInfo beam;
	beam.width = 900;
	if (N_PLY == 0)
	{
		beam.depth = 15;
		beam.req_chain = 12;
	}
	else if (player[0].info.timeleft < 15 * 1000)
	{
		beam.depth = 2;
		beam.width = 50;
		beam.req_chain = 1;
	}
	else if (player[0].info.score - player[1].info.score >= 30)
	{
		beam.depth = 14;
		beam.req_chain = 11;
	}
	else
	{
		int sum = 0, max = 0;
		for (File x = FILE_1; x <= FILE_10; x++)
		{
			Rank rank = player[0].rank_block(x);
			max = std::max(max, rank);
			sum += rank;
		}
		if (max >= 14 and sum >= 130)
		{
			beam.depth = 10;
			beam.req_chain = 1;
		}
		else if (sum >= 100)
		{
			beam.depth = 12;
			beam.req_chain = 3;
		}
		else
		{
			beam.depth = 14;
			beam.req_chain = 12;
		}
	}
	return beam;
}

void input_history()
{
	std::string str;
	Block b;
	for (int i = 0; i < RECORD_SIZE; i++)
	{
		for (int j = 0; j < PACK_SIZE; j++)
		{
			cin >> b;
			history[i][j] = b;
		}
		cin >> str;
		assert(str == "END");
	}
}

Pack get_pack(int ply)
{
	Pack pack = {history[ply][0], history[ply][1], history[ply][2], history[ply][3]};
	return pack;
}

void UDdelete_calc()
{
	int e, f, g, h, i, j;
	for (int a = 0; a < 12; a++)
		for (int b = 0; b < 12; b++)
			for (int c = 0; c < 12; c++)
				for (int d = 0; d < 12; d++)
				{
					e = f = g = h = 0;
					if (a + b == 10)
						e = a, f = b;
					if (a + c == 10)
						e = a, g = c;
					if (a + d == 10)
						e = a, h = d;
					i = (b << 8) + (c << 4) + d;
					j = (f << 8) + (g << 4) + h;
					UDdelete[a][i].target = e;
					UDdelete[a][i].three_blocks = j;
				}
}

void LRdelete_calc()
{
	int d, e, f, i, j;
	for (int a = 0; a < 12; a++)
		for (int b = 0; b < 12; b++)
			for (int c = 0; c < 12; c++)
			{
				d = e = f = 0;
				if (a + b == 10)
					d = a, e = b;
				if (b + c == 10)
					e = b, f = c;
				i = (a << 8) + (b << 4) + c;
				j = (d << 8) + (e << 4) + f;
				LRdelete[i] = j;
			}
}

void UDburst_calc()
{
	int d, e, f, i, j;
	for (int a = 0; a < 12; a++)
		for (int b = 0; b < 12; b++)
			for (int c = 0; c < 12; c++)
			{
				d = e = f = 0;
				if (a != 11)
					d = a;
				if (b != 11)
					e = b;
				if (c != 11)
					f = c;
				i = (a << 8) + (b << 4) + c;
				j = (d << 8) + (e << 4) + f;
				UDburst[i] = j;
			}
}

void LRburst_calc()
{
	int c, d, i, j;
	for (int a = 0; a < 12; a++)
		for (int b = 0; b < 12; b++)
		{
			c = d = 0;
			if (a != 11)
				c = a;
			if (b != 11)
				d = b;
			i = (a << 8) + (5 << 4) + b;
			j = (c << 8) + (5 << 4) + d;
			LRburst[i] = j;
		}
}

void printb(uint64_t v)
{
	uint64_t mask = (uint64_t)1 << (sizeof(v) * 8 - 1);
	int c = 0;
	do
	{
		putchar(mask & v ? '1' : '0');
		if (c % 8 == 7)
			putchar(' ');
		c++;
	} while (mask >>= 1);
	putchar('\n');
}