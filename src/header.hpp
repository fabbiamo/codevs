#pragma once
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <ctime>
#include <immintrin.h>
#include <iostream>
#include <istream>
#include <tuple>
#include <vector>

#define PROGRAM_NAME "fabbiamo"
#define SCORE_MAX 1000000
#define RECORD_SIZE 500
#define PACK_SIZE 4

using std::cerr;
using std::cin;
using std::cout;
using std::endl;

typedef int Block;
typedef int Pos;
typedef int Rot;
typedef int Order;
typedef int File;
typedef int Rank;

enum
{
	SELF = 0,
	ENEMY = 1,
};

enum
{
	EMPTY = 0,
	BLOCK_1,
	BLOCK_2,
	BLOCK_3,
	BLOCK_4,
	BLOCK_5,
	BLOCK_6,
	BLOCK_7,
	BLOCK_8,
	BLOCK_9,
	BLOCK_NB,
	BLOCK_DELETE = 10,
	OJAMA = 11,
	BLOCK_ZERO = 0
};

struct Pack
{
	Block block[PACK_SIZE];
};

struct Data
{
	int chain, chainscore, burstscore;
};

struct UDdelete_patterns
{
	uint64_t target;
	uint64_t three_blocks;
};

typedef uint64_t LRdelete_patterns;
typedef uint64_t UDburst_patterns;
typedef uint64_t LRburst_patterns;

enum
{
	FILE_0,
	FILE_1,
	FILE_2,
	FILE_3,
	FILE_4,
	FILE_5,
	FILE_6,
	FILE_7,
	FILE_8,
	FILE_9,
	FILE_10,
	FILE_11,
	FILE_NB,
	FILE_ZERO = 0,
};

enum
{
	RANK_0,
	RANK_1,
	RANK_2,
	RANK_3,
	RANK_4,
	RANK_5,
	RANK_6,
	RANK_7,
	RANK_8,
	RANK_9,
	RANK_10,
	RANK_11,
	RANK_12,
	RANK_13,
	RANK_14,
	RANK_15,
	RANK_16,
	RANK_17,
	RANK_18,
	RANK_19,
	RANK_NB,
	RANK_ZERO = 0,
};

inline bool on_board(File x, Rank y)
{
	return (FILE_1 <= x and x <= FILE_10) and (RANK_1 <= y and y <= RANK_18);
}

enum
{
	POS_0,
	POS_1,
	POS_2,
	POS_3,
	POS_4,
	POS_5,
	POS_6,
	POS_7,
	POS_8,
	POS_NB,
	POS_ZERO = 0,
	POS_SKILL = 10,
};

enum
{
	ROT_0,
	ROT_1,
	ROT_2,
	ROT_3,
	ROT_NB,
	ROT_ZERO = 0,
	ROT_SKILL = 10,
};

enum
{
	ORDER_0,
	ORDER_1,
	ORDER_2,
	ORDER_3,
	ORDER_4,
	ORDER_5,
	ORDER_6,
	ORDER_7,
	ORDER_8,
	ORDER_9,
	ORDER_10,
	ORDER_11,
	ORDER_12,
	ORDER_13,
	ORDER_14,
	ORDER_15,
	ORDER_16,
	ORDER_17,
	ORDER_18,
	ORDER_19,
	ORDER_20,
	ORDER_21,
	ORDER_22,
	ORDER_23,
	ORDER_24,
	ORDER_25,
	ORDER_26,
	ORDER_27,
	ORDER_28,
	ORDER_29,
	ORDER_30,
	ORDER_31,
	ORDER_32,
	ORDER_33,
	ORDER_34,
	ORDER_35,
	USE_SKILL,
	ORDER_NB,
	ORDER_ZERO = 0,
	ORDER_NULL = -1,
};

const Pos OrderToPos[ORDER_NB] = {
	POS_0,
	POS_0,
	POS_0,
	POS_0,
	POS_1,
	POS_1,
	POS_1,
	POS_1,
	POS_2,
	POS_2,
	POS_2,
	POS_2,
	POS_3,
	POS_3,
	POS_3,
	POS_3,
	POS_4,
	POS_4,
	POS_4,
	POS_4,
	POS_5,
	POS_5,
	POS_5,
	POS_5,
	POS_6,
	POS_6,
	POS_6,
	POS_6,
	POS_7,
	POS_7,
	POS_7,
	POS_7,
	POS_8,
	POS_8,
	POS_8,
	POS_8,
	POS_SKILL,
};

// 与えられたOrderに対応するPosを返す。
// →　行数は長くなるが速度面においてテーブルを用いる。
inline Pos pos_of(Order ord)
{
	assert(ord >= ORDER_ZERO);
	assert(ord < ORDER_NB);
	return OrderToPos[ord];
}

const Rot OrderToRot[ORDER_NB] = {
	ROT_0,
	ROT_1,
	ROT_2,
	ROT_3,
	ROT_0,
	ROT_1,
	ROT_2,
	ROT_3,
	ROT_0,
	ROT_1,
	ROT_2,
	ROT_3,
	ROT_0,
	ROT_1,
	ROT_2,
	ROT_3,
	ROT_0,
	ROT_1,
	ROT_2,
	ROT_3,
	ROT_0,
	ROT_1,
	ROT_2,
	ROT_3,
	ROT_0,
	ROT_1,
	ROT_2,
	ROT_3,
	ROT_0,
	ROT_1,
	ROT_2,
	ROT_3,
	ROT_0,
	ROT_1,
	ROT_2,
	ROT_3,
	ROT_SKILL,
};
// 与えられたOrderに対応するRotを返す。
// →　行数は長くなるが速度面においてテーブルを用いる。
inline Rot rot_of(Order ord)
{
	assert(ord >= ORDER_ZERO);
	assert(ord < ORDER_NB);
	return OrderToRot[ord];
}

// bbを(x * 4) 回だけ左シフトしたビット列を返す
// xはFileであることに注意
inline uint64_t Bit(Block b, File x)
{
	uint64_t bb = (uint64_t)b << x * 4;
	return bb;
}

const int ChainToChainScore[41] = {
	0,   //0
	1,   //1
	2,   //2
	4,   //3
	6,   //4
	9,   //5
	13,  //6
	19,  //7
	27,  //8
	37,  //9
	50,  //10
	67,  //11
	90,  //12
	120, //13
	159, //14
	210, //15
	276, 362, 474, 620, 810,
	1057, 1378, 1795, 2337, 3042,
	3959, 5151, 6701, 8716, 11335,
	14740, 19167, 24923, 32405, 42132,
	54778, 71218, 92590, 120373,
	156491, //40
};
inline int chainscore_of(int chain)
{
	assert(chain >= 0);
	if (chain > 40)
		chain = 40;
	return ChainToChainScore[chain];
}

const Order legal_order[] = {
	USE_SKILL,
	ORDER_8, ORDER_9, ORDER_10, ORDER_11,
	ORDER_12, ORDER_13, ORDER_14, ORDER_15,
	ORDER_16, ORDER_17, ORDER_18, ORDER_19,
	ORDER_20, ORDER_21, ORDER_22, ORDER_23,
	ORDER_24, ORDER_25, ORDER_26, ORDER_27,
	ORDER_0, ORDER_1, ORDER_2, ORDER_3,
	ORDER_4, ORDER_5, ORDER_6, ORDER_7,
	ORDER_28, ORDER_29, ORDER_30, ORDER_31,
	ORDER_32, ORDER_33, ORDER_34, ORDER_35};

//グローバル変数・関数
const int size = 5000;
extern int N_PLY;
extern int BEAM_DEPTH;
extern int BEAM_WIDTH;
extern Block history[RECORD_SIZE][PACK_SIZE];
extern UDdelete_patterns UDdelete[12][size];
extern uint64_t LRdelete[size];
extern uint64_t UDburst[size];
extern uint64_t LRburst[size];

void input_history();
Pack get_pack(int ply);
void UDdelete_calc();
void LRdelete_calc();
void UDburst_calc();
void LRburst_calc();
void printb(uint64_t v);

inline UDdelete_patterns UDdelete_pattern_of(uint64_t x, uint64_t y) { return UDdelete[x][y]; }
inline uint64_t LRdelete_pattern_of(uint64_t x) { return LRdelete[x]; }
inline uint64_t UDburst_pattern_of(uint64_t x) { return UDburst[x]; }
inline uint64_t LRburst_pattern_of(uint64_t x) { return LRburst[x]; }
