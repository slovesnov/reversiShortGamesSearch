/*
 * ReversiCode.h
 *
 *  Created on: 31.05.2022
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef REVERSICODE_H_
#define REVERSICODE_H_

#include <set>
#include <cstdint>
#include <string>

#include "defines.h"

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
	void operator=(ReversiCode const &o);
	std::string toString()const;
#ifdef REVERSI_CODE_MOVE_INSIDE
	static const int d3 = (boardSize * (boardSize / 2 - 1) + boardSize / 2) * 2
			- 1;
	static const uint64_t moveN = 1ull << (d3 % u64bits);
	char getMove()const;
	void setBlackMove();
	void setWhiteMove();
#endif
	void test();
	int turns()const;
};

using ReversiCodeSet = std::set<ReversiCode>;
using ReversiCodeSetCI = ReversiCodeSet::const_iterator;

std::ostream& operator<<(std::ostream& os, const ReversiCode& a);

#endif /* REVERSICODE_H_ */
