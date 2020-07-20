#include "header.hpp"
#include "player.hpp"

void Player::init_player()
{
	info.score = info.stock = info.skill = info.ojama = 0;
	info.timeleft = 1800000;
	memset(info.board, 0, sizeof(info.board));
}

void Player::input_player()
{
	init_player();
	std::string str;
	int b;

	cin >> info.timeleft >> info.stock >> info.skill >> info.score;
	for (Rank r = RANK_16; r >= RANK_1; r--)
		for (File f = FILE_1; f <= FILE_10; f++)
		{
			cin >> b;
			info.board[r] |= Bit(b, f);
			if (b == OJAMA)
				info.ojama++;
		}
	cin >> str;
	assert(str == "END");
}

void Player::print_player()
{
	Block block;
	uint64_t bb;

	cerr << info.timeleft << endl;
	cerr << info.stock << endl;
	cerr << info.skill << endl;
	cerr << info.score << endl;

	for (int r = RANK_16; r >= RANK_1; r--)
	{
		bb = info.board[r];
		for (int f = FILE_1; f <= FILE_10; f++)
		{
			bb = bb >> 4;
			block = bb & 0xf;
			if (block == OJAMA)
				cerr << "x ";
			else
				cerr << (bb & 0xf) << ' ';
		}
		cerr << endl;
	}
	cerr.flush();
}

void Player::output_order(Order order)
{
	if (order == USE_SKILL)
	{
		assert(info.skill >= 80);
		cout << "S" << endl;
		cout.flush();
	}
	else
	{
		cout << pos_of(order) << ' ' << rot_of(order) << endl;
		cout.flush();
	}
}

Block Player::block_num(File x, Rank y)
{
	uint64_t bb = info.board[y] >> (x * 4);
	return bb & 0xf;
}

//xは0から9まで
uint64_t Player::three_blocks(int x, Rank y)
{
	assert(0 <= x and x <= 9);

	uint64_t bb = info.board[y] >> (x * 4);
	return bb & 0xfff;
}

int Player::count_block()
{
	int sum = 0;
	for (File x = FILE_1; x <= 10; x++)
		sum += rank_block(x);
	return sum - info.ojama;
}

bool Player::danger_line()
{
	return info.board[RANK_17] or info.board[RANK_18];
}

Rank Player::rank_block(File x)
{
	Rank y;
	for (y = RANK_18; y >= RANK_1; y--)
	{
		if (block_num(x, y) != EMPTY)
			break;
	}
	return y;
}

Data Player::drop_block(Block b, File x)
{
	int chain;
	assert(FILE_1 <= x and x <= FILE_10);
	Rank y = rank_block(x), low[FILE_NB] = {};
	info.board[y + 1] |= Bit(b, x);

	for (File f = FILE_1; f <= FILE_10; f++)
		low[f] = -1;
	low[x] = y + 1;
	chain = delete_block(0, low);
	return data_set(chain, -1);
}

Data Player::drop_pack(Pack pack, Order order)
{
	int chain;
	Rank r1, r2, low[FILE_NB] = {};
	uint64_t bit;
	Pos p = pos_of(order);
	Rot r = rot_of(order);
	rot_block(&pack, r);
	r1 = rank_block(p + 1);
	r2 = rank_block(p + 2);
	if (p + 1 < FILE_1 or p + 2 < FILE_1 or p + 1 > FILE_10 or p + 2 > FILE_10)
	{
		cerr << "error" << endl;
		cerr.flush();
		exit(EXIT_FAILURE);
	}

	info.board[r1 + 2] |= Bit(pack.block[0], p + 1);
	info.board[r1 + 1] |= Bit(pack.block[2], p + 1);
	info.board[r2 + 2] |= Bit(pack.block[1], p + 2);
	info.board[r2 + 1] |= Bit(pack.block[3], p + 2);

	for (int i = 0; i < FILE_NB; i++)
		low[i] = -1;
	low[p + 1] = r1 + 1;
	low[p + 2] = r2 + 1;
	chain = delete_block(0, low);
	return data_set(chain, 0);
}

Data Player::burst_block()
{
	info.skill = 0;

	uint64_t x_mask[RANK_NB] = {};
	uint64_t bit;
	Rank low[FILE_NB] = {};
	int before, after, chain;
	before = count_block();

	for (int y = RANK_18; y >= RANK_1; y--)
	{
		if (info.board[y] == 0)
			continue;
		for (int x = FILE_1; x <= FILE_10; x++)
		{
			if (block_num(x, y) != BLOCK_5)
				continue;

			//左右3マス
			bit = three_blocks(x - 1, y);
			x_mask[y] |= Bit(LRburst_pattern_of(bit), x - 1);

			//上方向3マス
			bit = three_blocks(x - 1, y + 1);
			x_mask[y + 1] |= Bit(UDburst_pattern_of(bit), x - 1);

			//下方向3マス
			bit = three_blocks(x - 1, y - 1);
			x_mask[y - 1] |= Bit(UDburst_pattern_of(bit), x - 1);
		}
	}

	bool done = false;
	for (Rank y = RANK_18; y >= RANK_1; y--)
	{
		if (x_mask[y] == 0)
			continue;
		done = true;
		info.board[y] ^= x_mask[y];
	}

	//fix board
	int zero_y;
	for (File x = FILE_1; x <= FILE_10; x++)
	{
		low[x] = -1;
		done = false;
		for (Rank y = RANK_1; y <= rank_block(x); y++)
		{
			Block block = block_num(x, y);
			if (!done and block == EMPTY)
			{
				done = true;
				low[x] = zero_y = y;
			}
			if (done and block != EMPTY)
			{
				info.board[y] ^= Bit(block, x);
				info.board[zero_y++] ^= Bit(block, x);
			}
		}
	}

	after = count_block();
	assert(before - after >= 0);
	chain = delete_block(0, low);
	return data_set(chain, before - after);
}

int Player::delete_block(int chain, Rank *low)
{

	uint64_t x_mask[RANK_NB] = {};
	uint64_t bit;
	UDdelete_patterns dp;
	Rank new_low[FILE_NB] = {};

	for (File x = 0; x < FILE_NB; x++)
	{
		if (low[x] == -1)
			continue;
		for (Rank y = low[x];; y++)
		{
			if (block_num(x, y) == EMPTY)
				break;

			//左右3マス
			bit = three_blocks(x - 1, y);
			x_mask[y] |= Bit(LRdelete_pattern_of(bit), x - 1);

			//上方向3マス
			bit = three_blocks(x - 1, y + 1);
			dp = UDdelete_pattern_of(block_num(x, y), bit);
			x_mask[y] |= Bit(dp.target, x);
			x_mask[y + 1] |= Bit(dp.three_blocks, x - 1);

			//下方向3マス
			bit = three_blocks(x - 1, y - 1);
			dp = UDdelete_pattern_of(block_num(x, y), bit);
			x_mask[y] |= Bit(dp.target, x);
			x_mask[y - 1] |= Bit(dp.three_blocks, x - 1);
		}
	}

	bool done = false;
	for (Rank y = RANK_1; y <= RANK_18; y++)
	{
		if (x_mask[y] == 0)
			continue;
		done = true;
		info.board[y] ^= x_mask[y];
	}

	if (done == false)
		return chain;

	//fix board
	int zero_y;
	for (File x = FILE_1; x <= FILE_10; x++)
	{
		new_low[x] = -1;
		done = false;
		for (Rank y = RANK_1; y <= rank_block(x); y++)
		{
			Block block = block_num(x, y);
			if (!done and block == EMPTY)
			{
				done = true;
				new_low[x] = zero_y = y;
			}
			if (done and block != EMPTY)
			{
				info.board[y] ^= Bit(block, x);
				info.board[zero_y++] ^= Bit(block, x);
			}
		}
	}

	return delete_block(chain + 1, new_low);
}

Data Player::data_set(int chain, int burst_consumed)
{
	Data data = {0, 0, 0};
	data.chain = chain;
	data.chainscore = chainscore_of(chain);
	if (burst_consumed > 0)
		data.burstscore = (int)floor(25 * pow(2, (double)burst_consumed / 12));

	info.score += data.chainscore + data.burstscore;

	//set PlayerInfo
	if (data.chain > 0)
		info.skill = std::min(info.skill + 8, 100);
	info.stock -= data.chainscore / 2 + data.burstscore / 2;
	info.stock = std::max(info.stock, 0);

	return data;
}

void Player::drop_ojamablock()
{
	assert(info.stock >= 0);
	if (info.stock < 10)
		return;
	info.stock -= 10;
	info.ojama += 10;

	for (File x = FILE_1; x <= FILE_10; x++)
	{
		Rank y = rank_block(x);
		info.board[y + 1] |= Bit(OJAMA, x);
	}
}

std::tuple<int, int> Player::erasing_block(int before_count)
{
	PlayerInfo init = info;
	int max, chain, eff_consumed, consumed, before;
	max = eff_consumed = 0;

	for (File x = FILE_1; x <= FILE_10; x++)
	{
		for (Rank y = RANK_1; y <= rank_block(x); y++)
		{
			Block block = block_num(x, y);
			if (block == EMPTY or block == OJAMA)
				continue;
			bool eraseble = false;
			eraseble |= on_board(x - 1, y + 1) and block_num(x - 1, y + 1) == EMPTY;
			eraseble |= on_board(x, y + 1) and block_num(x, y + 1) == EMPTY;
			eraseble |= on_board(x + 1, y + 1) and block_num(x + 1, y + 1) == EMPTY;
			if (eraseble)
			{
				Rank low[FILE_NB];
				for (File f = FILE_0; f < FILE_NB; f++)
					low[f] = -1;
				low[x] = y;
				Rank dy = y;
				info.board[y] ^= Bit(block, x);
				for (Rank r = y; r <= rank_block(x); r++)
				{
					Block b = block_num(x, r);
					if (b != EMPTY and block_num(x, dy) == EMPTY)
					{
						info.board[dy++] ^= Bit(b, x);
						info.board[r] ^= Bit(b, x);
					}
				}
				chain = delete_block(0, low);
				if (max <= chain)
				{
					consumed = before_count - count_block();
					if (max < chain or consumed < eff_consumed)
					{
						max = chain;
						eff_consumed = consumed;
					}
				}
				info = init;
			}
		}
	}
	info = init;
	return std::make_tuple(max, eff_consumed);
}

auto Player::gen_legalmoves(int ply)
{
	std::vector<Order> moves;
	for (auto order : legal_order)
	{
		if (order == USE_SKILL)
		{
			if (info.skill >= 80)
				moves.push_back(order);
		}
		else
		{
			int p = pos_of(order);
			if (rank_block(p + 1) <= RANK_14 and rank_block(p + 2) <= RANK_14)
				moves.push_back(order);
		}
	}
	return moves;
}

FutureOrder Player::beam_search(SearchInfo const &beam, const int time_level, const int last_fire)
{
	const int beam_depth = beam.depth;
	const int beam_width = beam.width;
	const int req_chain = beam.req_chain;
	const int req_score = chainscore_of(req_chain);
	const int fire_chain = 6;
	double seconds;
	switch (time_level)
	{
	case 0:
		seconds = 17.0;
		break;
	case 1:
		seconds = 13.0;
		break;
	case 2:
		seconds = 9.0;
		break;
	case 3:
		seconds = 2.0;
		break;
	case 4:
		seconds = 0.1;
		break;
	default:
		break;
	}

	cerr << "n_ply: " << N_PLY << ", depth: " << beam_depth << ", width: " << beam_width << endl;

	std::chrono::system_clock::time_point start, end;
	start = std::chrono::system_clock::now();

	Status best_state[beam_depth + 1] = {};
	int max_score[beam_depth + 1] = {}, dropped_ojama[beam_depth + 1] = {};
	int upper;
	PlayerInfo root;
	FutureOrder future;
	Status new_state;
	Data data;
	Pack pack;
	std::vector<Status> Q[beam_depth + 1];
	new_state.info = info;
	Q[0].push_back(new_state);

	for (int depth = 0; depth < beam_depth; depth++)
	{
		pack = get_pack(N_PLY + depth);
		for (auto state : Q[depth])
		{
			info = state.info;
			drop_ojamablock();
			dropped_ojama[depth + 1] = info.ojama;
			root = info;
			auto moves = gen_legalmoves(N_PLY + depth);
			if (moves.empty())
				continue;
			for (auto order : moves)
			{
				info = root;
				if (order == USE_SKILL)
					data = burst_block();
				else
					data = drop_pack(pack, order);

				new_state.info = info;
				new_state.eval = evaluate();
				new_state.historder = state.historder;
				new_state.historder.push_back(order);

				if (new_state.eval > -SCORE_MAX)
				{
					if (data.chain < fire_chain)
						Q[depth + 1].push_back(new_state);
					if (max_score[depth + 1] < data.chainscore + data.burstscore)
					{
						max_score[depth + 1] = data.chainscore + data.burstscore;
						best_state[depth + 1] = new_state;
					}
				}
				end = std::chrono::system_clock::now();
				double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
				if (elapsed >= seconds * 1000)
				{
					cerr << depth << " break" << endl;
					cerr.flush();
					upper = depth + 1;
					goto skip;
				}
			}
		}
		if (max_score[depth + 1] >= chainscore_of(15))
		{
			//15連鎖以上を見つけたらそこで打ち切る
			upper = depth + 1;
			goto skip;
		}
		if (Q[depth + 1].empty())
		{
			future.fire = false;
			int d = depth;
			for (d = depth; d >= 1; d--)
			{
				if (not best_state[d].historder.empty())
				{
					future.historder = best_state[d].historder;
					future.max_score = max_score[d];
					future.fire = (future.max_score >= req_score) ? true : false;
					break;
				}
			}
			if (future.historder.empty())
			{
				future.historder.push_back(ORDER_0);
				future.max_score = -1;
				future.fire = false;
			}
			return future;
		}
		if (depth < beam_depth - 1)
		{
			std::sort(Q[depth + 1].begin(), Q[depth + 1].end(), cmp);
			if (Q[depth + 1].size() > beam_width)
				Q[depth + 1].erase(Q[depth + 1].begin() + beam_width, Q[depth + 1].end());
		}
	}
	upper = beam_depth;
	goto skip; // ?
skip:
	int best_depth = 0;
	double acc, max_acc = 0.0;

	for (int depth = 0; depth < upper; depth++)
	{
		if (max_score[depth + 1] >= req_score)
		{
			best_depth = depth;
			break;
		}
	}
	if (N_PLY + best_depth < last_fire + 13 or best_depth == 0)
	{
		for (int depth = best_depth; depth < upper; depth++)
		{
			if (max_score[depth + 1] == 0)
				continue;
			acc = 10 * pow(1.4, -depth) * max_score[depth + 1];
			//fprintf(stderr, "+%dt %dpt %.3f\n", depth + 1, max_score[depth + 1], acc);
			if (max_acc < acc)
			{
				max_acc = acc;
				best_depth = depth;
			}
		}
	}

	future.historder = best_state[best_depth + 1].historder;
	future.max_score = max_score[best_depth + 1];
	future.fire = (future.max_score >= req_score) ? true : false;

	end = std::chrono::system_clock::now();
	double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	fprintf(stderr, "result: +%ldt %dpt\n", future.historder.size(), future.max_score);
	cerr << elapsed << "ms passed" << endl;
	if (future.fire)
		cerr << "fire." << endl;
	cerr.flush();

	return future;
}

double Player::evaluate()
{
	srand((unsigned int)time(NULL));
	Data data;
	double score = 0.0;
	int count, chain, consumed;

	if (danger_line())
		return -SCORE_MAX;

	if (info.board[RANK_14] or info.board[RANK_15])
		score -= 5000;

	count = count_block();
	std::tie(chain, consumed) = erasing_block(count);

	assert(chain >= 0);
	//score += (double)rand() / RAND_MAX; // [0.0, 1.0]
	score += 100000 * chain;
	score += 1000 * count;
	score -= 100 * consumed / (chain + 1);

	return score;
}

void rot_block(Pack *p, Rot rot)
{
	switch (rot)
	{
	case ROT_0:
		break;
	case ROT_1:
		std::swap(p->block[0], p->block[2]);
		std::swap(p->block[2], p->block[3]);
		std::swap(p->block[1], p->block[3]);
		break;
	case ROT_2:
		std::swap(p->block[0], p->block[3]);
		std::swap(p->block[1], p->block[2]);
		break;
	case ROT_3:
		std::swap(p->block[0], p->block[1]);
		std::swap(p->block[1], p->block[3]);
		std::swap(p->block[2], p->block[3]);
		break;
	}
	if (p->block[2] == EMPTY)
		std::swap(p->block[0], p->block[2]);
	if (p->block[3] == EMPTY)
		std::swap(p->block[1], p->block[3]);
}