#pragma once
#include "header.hpp"
struct PlayerInfo
{
	uint64_t board[RANK_NB];
	int timeleft, score, stock, skill, ojama;
};

struct SearchInfo
{
	int depth;
	int width;
	int req_chain;
};

struct Status
{
	PlayerInfo info;
	double eval;
	std::vector<Order> historder;
};
inline bool cmp(const Status &a, const Status &b) { return a.eval > b.eval; }

struct FutureOrder
{
	bool fire;
	int max_score;
	std::vector<Order> historder;
};

struct Player
{
	PlayerInfo info;

	void init_player();
	void input_player();
	void print_player();
	void output_order(Order order);
	Block block_num(File x, Rank y);
	uint64_t three_blocks(int x, Rank y);
	int count_block();
	bool danger_line();
	Rank rank_block(File x);

	//simulation
	Data drop_block(Block b, File x);
	Data drop_pack(Pack p, Order order);
	Data burst_block();
	int delete_block(int chain, Rank *lower);
	Data data_set(int chain, int burst_consumed);
	void drop_ojamablock();
	std::tuple<int, int> erasing_block(int before_count);

	//search
	auto gen_legalmoves(int ply);
	FutureOrder beam_search(SearchInfo const &beam, const int time_level, const int last_fire);
	double evaluate();
};

void rot_block(Pack *p, Rot rot);