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

#include <vector>
#include <mutex>

#include "ThreadData.h"

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
	static bool searchFromStart;
#endif
	static std::mutex mtx;

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
	void operator=(ReversiCode const& c);
	std::string toString()const;
	void addAllMoves(int layer,ReversiCode const& parentCode)const;
	void addAllMoves(ReversiCode const &parentCode, int depth, ThreadData &data,
			Reversi &p) const;
	int endGameType()const;
	static bool allFound(std::string& s);

	void fillChars();
	void printChars();
	Reversi flipHorisontal();
	Reversi rotate90();
	std::string toCharString();
	ReversiCode code1()const;
	ReversiCode code()const;
	static void insert(int layer, ReversiCode const &parentCode, char move,
			ReversiCode &code);

	static void initFirst2Layers(int type);
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
	static void outSaveFoundedToFile(const ReversiCode& code,ThreadData const&data,int line);
#ifdef SEARCH_MOVES
	static void searchMoves(ReversiCode const& code);
	static void searchMoves(ReversiCode const& from,ReversiCode const& to);
#endif
};

std::ostream& operator<<(std::ostream& os, const Reversi& a);

#endif /* REVERSI_H_ */
