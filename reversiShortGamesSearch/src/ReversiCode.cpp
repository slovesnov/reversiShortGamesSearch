/*
 * ReversiCode.cpp
 *
 *  Created on: 31.05.2022
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "aslov.h"
#include "ReversiCode.h"
#include "Reversi.h"

ReversiCode::ReversiCode(std::initializer_list<uint64_t> array
#ifndef REVERSI_CODE_MOVE_INSIDE
			,char move
#endif
		){
	auto p=c;
	for(auto&a:array){
		*p++=a;
	}
#ifndef REVERSI_CODE_MOVE_INSIDE
	moveColor=move;
#endif

}

bool ReversiCode::operator<(ReversiCode const &o) const {
	auto p=o.c;
	for(auto&a:c){
		if(a!=*p){
			return a<*p;
		}
		p++;
	}
#ifdef REVERSI_CODE_MOVE_INSIDE
	return false;
#else
	return moveColor<o.moveColor;
#endif
}

bool ReversiCode::operator ==(const ReversiCode &o) const {
	auto p = o.c;
	for (auto &a : c) {
		if (a != *p) {
			return false;
		}
		p++;
	}
#ifdef REVERSI_CODE_MOVE_INSIDE
	return true;
#else
	return moveColor == o.moveColor;
#endif
}

bool ReversiCode::operator !=(const ReversiCode &o) const {
	return !(*this == o);
}
/*
void ReversiCode::operator =(const ReversiCode &o) {
	int i=0;
	for (auto &a : c) {
		a= o.c[i++];
	}
#ifdef STORE_MOVE
	//not tested
	i=0;
	for (auto &a : parent) {
		a= o.parent[i++];
	}
	parentColor=o.parentColor;
	move=o.move;
#endif
	moveColor=o.moveColor;
}*/

std::string ReversiCode::toString()const{
	std::string s;
	int i=0;
	for (auto &a : c) {
		s += format("%s0x%llxull",i?", ":"{", a);
		i++;
	}
	s+="};";
#ifdef REVERSI_CODE_MOVE_INSIDE
	auto moveColor=getMove();
#endif
	if(moveColor==black){
		s+="black";
	}
	else if(moveColor==white){
		s+="white";
	}
	else{
		s+=forma(int(moveColor));
	}
	return s;
}

#ifdef REVERSI_CODE_MOVE_INSIDE
char ReversiCode::getMove()const{
	return (c[d3/u64bits]&moveN)!=0 ? black:white;
}

void ReversiCode::setBlackMove(){
	c[d3/u64bits] |= moveN;
}

void ReversiCode::setWhiteMove(){
	c[d3/u64bits] &= ~moveN;
}
#endif

void ReversiCode::test(){
}

int ReversiCode::turns()const{
	Reversi r(*this);
	return r.turns();
}

std::ostream& operator<<(std::ostream& os, const ReversiCode& a){
    os << a.toString();
    return os;
}
