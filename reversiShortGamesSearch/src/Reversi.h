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

//allow boardSize>8
const int boardSize = 8;

/* Note. Program do not add endgame positions to layerSet so set not contains all positions
 * all end game positions showed in form 0+66+0=66
 */
const int maxLayer=14;

/* ifdef USE_SYMMETRY much slower but needs less memory, also "move string" will be invalid
 * 1 or 2 or not defined if symmetry not used
 * USE_SYMMETRY 1 faster than USE_SYMMETRY 2
 *
 * for non standard start position 8x8 STORE_MOVE is not defined
 * USE_SYMMETRY not defined layer 11  42,881,764 bf=6.70 2,475+0+7=2,482 0:23 main.cpp:65 main()
 * USE_SYMMETRY 1 layer 11  36,098,556 bf=6.50 2,199+0+7=2,206 0:51 main.cpp:65 main()
 * USE_SYMMETRY 2 layer 11  36,098,556 bf=6.50 2,199+0+7=2,206 1:06 main.cpp:65 main()
 */
//#define USE_SYMMETRY 1

#ifndef USE_SYMMETRY
//STORE_MOVE is valid only if USE_SYMMETRY isn't defined
	#define STORE_MOVE
#endif

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
	//move here "char moveColor;" to make lower sizeof(ReversiCode)
	char moveColor;
	bool operator<(ReversiCode const &o) const;
	bool operator==(ReversiCode const &o) const;
	std::string toString()const;
};
std::ostream& operator<<(std::ostream& os, const ReversiCode& a);

class Reversi {
public:
	static const int lineSize = boardSize + 1;
	static const int boardSize2 = (boardSize + 2) * lineSize + 1;
	static const int upleftCenter = boardSize / 2 * lineSize + boardSize / 2; //analog d3 on 8x8 table
	static const char empty = 0;
	static const char black = 1;
	static const char white = 2;
	static const int direction[8];
	static const int BLACK_ONLY=0;
	static const int WHITE_ONLY=1;
	static const int BLACK_AND_WHITE=2;

	static std::vector<int> cells, possibleMoves;
	static std::set<ReversiCode> layerSet[maxLayer+1];
	static std::set<ReversiCode> found[3];
	static std::set<ReversiCode> foundEndCount[3];
	static int foundMinTurns[3];
	static int borderCount;
#ifdef STORE_MOVE
	static std::vector<std::string> foundString[3];
#endif
#ifdef USE_SYMMETRY
	static int flip[7][boardSize*boardSize];
#endif

	char board[boardSize2];
	char moveColor;

	Reversi();
	void init(int type=0);
	static void staticInit();
	void assign(Reversi const &re, char _move);
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
	void addAllMoves(int layer,ReversiCode const& parentCode);
	int endGameType()const;
	static void setSearchOnlyBlackAndWhite();
	static bool allFound(std::string& s);
	static std::string forHtml();

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
	static std::string endGameCounts();
	void test();
	int countBorderChips()const;
	void fillForFlip();
#ifdef USE_SYMMETRY
	void setFlip(int n);
#endif
};

#endif /* REVERSI_H_ */
