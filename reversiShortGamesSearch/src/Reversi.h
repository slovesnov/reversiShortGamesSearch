/*
 * Reversi.h
 *
 *  Created on: 08.05.2022
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef REVERSI_H_
#define REVERSI_H_

#include <set>
#include <vector>
#include <string>

/* allow even boardSize up to 16 if boardSize more than 16 need to change
 * type of boardSize from unsigned char to short because of fillForFlip() function
 */
const int boardSize = 12;

//cut all positions without board chips on this layer, if not defined never cut
//#define BOARD_LAYER 11

const int maxLayer1=8;//11
const int maxLayer=maxLayer1+5;//maxLayer1+5


//const int boardSize = 12;
//#define BOARD_LAYER 9
//const int maxLayer1=11+3;//11
//const int maxLayer=maxLayer1+3;

//#define SEARCH_MOVES

/* ifdef USE_SYMMETRY much slower but needs less memory, also "move string" will be invalid
 * 1 or 2 or not defined if symmetry not used
 * USE_SYMMETRY 1 faster than USE_SYMMETRY 2
 *
 * for non standard start position 8x8 STORE_MOVE is not defined
 * USE_SYMMETRY not defined layer 11  42,881,764 bf=6.70 2,475+0+7=2,482 0:23 main.cpp:65 main()
 * USE_SYMMETRY 1 layer 11  36,098,556 bf=6.50 2,199+0+7=2,206 0:51 main.cpp:65 main()
 * USE_SYMMETRY 2 layer 11  36,098,556 bf=6.50 2,199+0+7=2,206 1:06 main.cpp:65 main()
 */
#ifndef SEARCH_MOVES
#define USE_SYMMETRY 1
#endif

#ifndef USE_SYMMETRY
	//STORE_MOVE is valid only if USE_SYMMETRY isn't defined
	#define STORE_MOVE
#endif


//#define BORDER_COUNT
static const char black = 0;
static const char white = 1;
static const char empty = 2;

//when REVERSI_CODE_MOVE_INSIDE is NOT defined programs faster 1-(6*60+10)/(6*60+55)=10.8%
//#define REVERSI_CODE_MOVE_INSIDE

//when POTENTIAL_MOVES is defined programs faster 1-(6*60+10)/(7*60+34)=18.5%
#define POTENTIAL_MOVES

class ReversiCode{
public:
	static const int bits = boardSize * boardSize * 2;
	static const int u64bits = sizeof(uint64_t) * 8;
	static const int size = bits / u64bits + (bits % u64bits != 0);
	uint64_t c[size];
#ifdef STORE_MOVE
	uint64_t parent[size];
	char parentColor;
	char move;
#endif

#ifndef REVERSI_CODE_MOVE_INSIDE
	//move here "char moveColor;" to make lower sizeof(ReversiCode)
	char moveColor;
#endif
	ReversiCode() {}
	ReversiCode(std::initializer_list<uint64_t> array
#ifndef REVERSI_CODE_MOVE_INSIDE
			,char move
#endif
			);

	bool operator<(ReversiCode const &o) const;
	bool operator==(ReversiCode const &o) const;
	bool operator!=(ReversiCode const &o) const;
	//void operator=(ReversiCode const &o);
	std::string toString()const;
#ifdef REVERSI_CODE_MOVE_INSIDE
	static const int d3 = (boardSize * (boardSize / 2 - 1) + boardSize / 2) * 2
			- 1;
	static const uint64_t moveN = 1ull << (d3 % u64bits);
	char getMove()const;
	void setZero();
	void setBlackMove();
	void setWhiteMove();
#endif
	void test();
	int turns()const;
};

using ReversiCodeSet = std::set<ReversiCode>;
using ReversiCodeSetCI = ReversiCodeSet::const_iterator;

class ThreadData{
public:
	ReversiCodeSetCI begin,end;
	ReversiCodeSet foundEndCount[maxLayer-maxLayer1+1][3];
	ReversiCode base;
	clock_t start;
	static int size;
};

std::ostream& operator<<(std::ostream& os, const ReversiCode& a);

class Reversi {
public:
	static const int lineSize = boardSize + 1;
	static const int boardSize2 = (boardSize + 2) * lineSize + 1;
	static const int upleftCenter = boardSize / 2 * lineSize + boardSize / 2; //analog d3 on 8x8 table
	static const int direction[8];
	static const int BLACK_ONLY=0;
	static const int WHITE_ONLY=1;
	static const int BLACK_AND_WHITE=2;

	static std::vector<int> cells, possibleMoves;
	static ReversiCodeSet layerSet[maxLayer+1];
	static ReversiCodeSet found[3];
	static ReversiCodeSet foundEndCount[maxLayer+1][3];
	static int foundMinTurns[3];
	static int maxMinChips;
#ifdef BORDER_COUNT
	static int borderCount;
#endif
#ifdef STORE_MOVE
	static std::vector<std::string> foundString[3];
#endif
#ifdef USE_SYMMETRY
	static int flip[7][boardSize*boardSize];
#endif

	//unsigned char allow board size up to 16x16
	unsigned char board[boardSize2];
	char moveColor;
#ifdef SEARCH_MOVES
	static ReversiCode endCode;
#endif

	Reversi();
	Reversi(ReversiCode const& code);
	void init(int type=0);
	static void staticInit();
	void assign(Reversi const &re, char _move);
	void copyBoard(Reversi const &re);
	void print()const;
	bool makeMove(int index);
	static char getCenterMove(bool type);
	bool possibleMove(int index,char move)const;
	static int index(std::string const& s);
	static std::string indexToString(int index);
	void makeMoves(std::string const& s);
	static char oppositeColor(char c);
	bool isEnd()const;
	void operator=(Reversi const &re);
	void operator=(ReversiCode const& code);
	void addAllMoves(int layer,ReversiCode const& parentCode)const;
	void addAllMoves(ReversiCode const& parentCode,int depth,ThreadData&data
#ifdef POTENTIAL_MOVES
			,Reversi& p
#endif
	)const;
	int endGameType()const;
	static void setSearchOnlyBlackAndWhite();
	static bool allFound(std::string& s);

	void fillChars();
	void printChars();
	Reversi flipHorisontal();
	Reversi rotate90();
	std::string toCharString();
	ReversiCode code1()const;
	ReversiCode code()const;
	void fromCode(ReversiCode const& c);
	static void insert(int layer, ReversiCode const &parentCode, char move,
			ReversiCode &code);

	static void initFirst2Layers(int type,bool bwOnly=false);
	static std::string endGameCounts(int layer,bool showNoBorder=true);
	static std::string shortestEndGameCounts();
	bool test(int p=0);
	int countBorderChips()const;
	void fillForFlip();
#ifdef USE_SYMMETRY
	void setFlip(int n);
#endif
	int getMinChips()const;
	int turns()const;
	void setPotentialMoves(Reversi const& r);
	void addPotentialMove(int i);
};

#endif /* REVERSI_H_ */
