/* Reversi.cpp
 *
 *  Created on: 08.05.2022
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */
#include <cassert>

#include "Reversi.h"
#include "aslov.h"

std::vector<int> Reversi::cells,Reversi::possibleMoves;
std::set<ReversiCode> Reversi::layerSet[maxLayer+1];
std::set<ReversiCode> Reversi::found[3];
std::set<ReversiCode> Reversi::foundEndCount[3];
int Reversi::foundMinTurns[3];
int Reversi::borderCount;

#ifdef STORE_MOVE
std::vector<std::string> Reversi::foundString[3];
#endif

#ifdef USE_SYMMETRY
int Reversi::flip[7][boardSize*boardSize];
#endif

const int Reversi::direction[] = { -lineSize-1, -lineSize, -lineSize+1, -1, 1, lineSize-1, lineSize, lineSize+1 };
const char outChar[]=".bw";

const char *gameTypeString[]={
		"black only","white only","black and white"
};

bool ReversiCode::operator<(ReversiCode const &o) const {
	auto p=o.c;
	for(auto&a:c){
		if(a!=*p){
			return a<*p;
		}
		p++;
	}
	return moveColor<o.moveColor;
}

bool ReversiCode::operator ==(const ReversiCode &o) const {
	auto p = o.c;
	for (auto &a : c) {
		if (a != *p) {
			return false;
		}
		p++;
	}
	return moveColor == o.moveColor;
}

std::string ReversiCode::toString()const{
	std::string s;
	for (auto &a : c) {
		s += format("%llx ", a);
	}
	if(moveColor==Reversi::black){
		s+="black";
	}
	else if(moveColor==Reversi::white){
		s+="white";
	}
	else{
		s+=forma(int(moveColor));
	}
	return s;
}

std::ostream& operator<<(std::ostream& os, const ReversiCode& a){
    os << a.toString();
    return os;
}

void Reversi::staticInit() {
	int i, j, k;
	i=upleftCenter;
	int a[]={i,i+1,i+lineSize,i+lineSize+1};

	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			k=i * lineSize + j;
			cells.push_back(k);
			if (!ONE_OF(k, a)) {
				possibleMoves.push_back(k);
			}
		}
	}
#ifdef USE_SYMMETRY
	Reversi r;
	k=0;
	for (j = 0; j < 2; j++) {
		r.fillForFlip();
		if (j) {
			r = r.flipHorisontal();
		}
		for (i = 0; i < 4; i++) {
			if(j!=0 || i!=0){
				r.setFlip(k++);
			}
			r = r.rotate90();
		}
	}
#endif
}

//see index function
Reversi::Reversi() {
	int i;
	for (i = 0; i <= lineSize; i++) {
		board[i] = empty;
	}
	for (i = 2 * lineSize; i <= boardSize * lineSize; i += lineSize) {
		board[i] = empty;
	}
	for (; i < boardSize2; i++) {
		board[i] = empty;
	}
}

void Reversi::init(int type/*=0*/) {
	for (char &a : board) {
		a = empty;
	}
	int i = upleftCenter;
	int d = i + lineSize;
	std::vector<int> a;
	if (type < 2) {
		a = { i, d + 1 };
	} else if (type < 4) {
		a = { i, i + 1 };
	} else {
		a = { i + 1, d + 1 };
	}

	for (auto &e : { i, i + 1, d, d + 1 }) {
		board[e] = oneOf(e, a) == type % 2 ? white : black;
	}

	moveColor = black;
}

void Reversi::print() const {
	int i, j;
	const int k=boardSize>=10?2:1;
	char c;
	std::string s = " ";
	if(k==2){
		s+=" ";
	}
	for (i = 0, c = 'a'; i < boardSize; i++, c++) {
		s += c;
	}
	s += " \n";
	printf("%s", s.c_str());
	for (i = 1; i <= boardSize; i++) {
		printf("%*d",k, i);
		for (j = 1; j <= boardSize; j++) {
			printf("%c", outChar[int(board[i * lineSize + j])]);
		}
		printf("%d\n", i);
	}
	printf("%s", s.c_str());
}

bool Reversi::makeMove(int index) {
	int i, j = index;
	if (board[j] != empty) {
		return false;
	}

	bool r = false;
	const char o = oppositeColor(moveColor);
	for (auto &d : direction) {
		i = j + d;
		if (board[i] == o) {
			do {
				i += d;
			} while (board[i] == o);
			if (board[i] == moveColor) {
				for (; i != j; i -= d) {
					board[i] = moveColor;
				}
				r = true;
			}
		}
	}
	if (r) {
		board[j] = moveColor;
		moveColor = o;
	}
	return r;
}

char Reversi::getCenterMove(bool type){
	return upleftCenter - lineSize - !type;
}

bool Reversi::possibleMove(int index, char move) const {
	int i, j = index;
	if (board[j] != empty) {
		return false;
	}

	const char o = oppositeColor(move);
	for (auto &d : direction) {
		i = j + d;
		if (board[i] == o) {
			do {
				i += d;
			} while (board[i] == o);
			if (board[i] == move) {
				return true;
			}
		}
	}
	return false;
}

/* a1=lineSize+1 ... h1=a1+boardSize-1=a1+linesize-2=2*liveSize-1
 * a2=2*lineSize+1 ...
 * ...
 * a8=boardSize*lineSize+1 h8=a8+boardSize-1=a8+linesize-2=boardSize*lineSize-1
 *
 * empty 0..linesize,[2...boardsize]*linesize,boardsize*linesize+1 to the end
 */
int Reversi::index(const std::string &s) {
	assert(s.length() == 2);
	char a = tolower(s[0]);
	assert(a >= 'a' && a < 'a'+boardSize);
	char d = s[1];
	assert(d >= '1' && d < '1'+boardSize);
	a -= 'a';
	d -= '1';
	return (a+1) + (d+1) * lineSize;
}

std::string Reversi::indexToString(int index) {
	std::string s(1, char((index % lineSize) + 'a' - 1));
	return s + char(index / lineSize + '1' - 1);
}

void Reversi::makeMoves(const std::string &_s) {
	std::string s=replaceAll(_s, " ", "");
	assert(s.length() % 2 == 0);
	size_t i;
	for (i = 0; i < s.length(); i += 2) {
#ifndef NDEBUG
		bool b =
#endif
				makeMove(index(s.substr(i, 2)));
		//printl(i/2,b,s.substr(i, 2));
		assert(b);
	}
}

char Reversi::oppositeColor(char c) {
	assert(c == black || c == white);
	return c == black ? white : black;
}

bool Reversi::isEnd() const {
	int k;
	char m = moveColor;
	for (k = 0; k < 2; k++) {
		for (int i : possibleMoves) {
			if (possibleMove(i, m)) {
				return false;
			}
		}
		m = oppositeColor(m);
	}
	return true;
}

void Reversi::operator =(const Reversi &re) {
	assign(re,re.moveColor);
}

void Reversi::addAllMoves(int layer,ReversiCode const& parentCode)  {
	int k, l;
	bool f = false;
	Reversi t;
	ReversiCode tcode;
	std::string s;
	for (l = 0; l < 2; l++) {
		t.assign(*this, l == 0 ? moveColor : oppositeColor(moveColor));
		for (int i : possibleMoves) {
			if (t.makeMove(i)) {
				f = true;
				tcode=t.code();

//				if (l == 1) {
//					print();
//					printl("no moves switch",turnsMade(),"turns",getMoves())
//					exit(0);
//				}

				if(t.countBorderChips()>0){
					borderCount++;
				}

				if (t.isEnd()) {
					k = t.endGameType();
					foundEndCount[k].insert(tcode);
					//output/count all positions with black and white colors
					if (found[k].empty()
							|| (k == BLACK_AND_WHITE /*&& foundMinTurns[k] == layer*/
									&& found[k].find(tcode) == found[k].end())) {
						found[k].insert(tcode);
						foundMinTurns[k]=layer;
//						if(t.countBorderChips()<3 && k==BLACK_AND_WHITE){
//							t.print();
//						}
//						t.print();
#ifdef STORE_MOVE
						s="";
						int j;
						tcode=parentCode;
						//auto a = this->code();
						for (j = layer - 1; j > 0; j--) {
							auto it = layerSet[j].find(tcode);
							assert(it != layerSet[j].end());
							auto p=it->parent;
							for(auto&a:tcode.c){
								a=*p++;
							}
							tcode.moveColor = it->parentColor;
							s = indexToString(it->move) + "" + s;
						}
						s += indexToString(i);
						foundString[k].push_back(s);
//						exit(0);
#else
						s="lastmove "+indexToString(i);
#endif
						println("found %s %d turns %s", gameTypeString[k], layer,s.c_str());
						fflush(stdout);
					}
				} else {
					insert(layer, parentCode, i, tcode);
				}
				t.assign(*this, l == 0 ? moveColor : oppositeColor(moveColor));
			}
		}
		if (f) {
			return;
		}
	}
}

int Reversi::endGameType() const {
	bool b = false, w = false;
	char c;
	for (int i : cells) {
		c = board[i];
		if (c == white) {
			w = true;
		} else if (c == black) {
			b = true;
		}
	}
	if (b) {
		return w ? BLACK_AND_WHITE : BLACK_ONLY;
	} else {
		return WHITE_ONLY;
	}
}

void Reversi::setSearchOnlyBlackAndWhite() {
	ReversiCode c;
	//prevents warning
	c.moveColor=black;
	found[BLACK_ONLY].insert(c);
	found[WHITE_ONLY].insert(c);
}

void Reversi::assign(const Reversi &re,char _move) {
	moveColor = _move;
// min lineSize+1, max boardSize*(lineSize+1), need to copy max-min+1
	memcpy(board+lineSize+1, re.board+lineSize+1, ((boardSize-1)*(lineSize+1)+1) * sizeof(char));
//	memcpy(board, re.board, boardSize2 * sizeof(char));
}

bool Reversi::allFound(std::string& s) {
	s="";
	for(auto&a:found){
		if(a.empty()){
			return false;
		}
		s+=" "+std::to_string(a.size());
	}
	return true;
}

std::string Reversi::forHtml() {
#ifdef STORE_MOVE
	int a[] = { BLACK_ONLY, WHITE_ONLY, BLACK_AND_WHITE };
	std::string s, q;
	for (int i : a) {
		q = foundString[i][0];
		s += "<td>" + q + " " + std::to_string(q.size() / 2);
	}
	return s;
#else
	return "";
#endif
}

void Reversi::fillChars() {
	int i, j;
	char c = '0';
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			board[i * lineSize + j] = c++;
		}
	}
}

void Reversi::printChars() {
	int i, j;
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			printf("%c",board[i * lineSize + j]);
		}
		printf("\n");
	}
}

Reversi Reversi::flipHorisontal() {
	int i, j;
	Reversi r;
	r.moveColor=moveColor;
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			r.board[i * lineSize + boardSize - j + 1] = board[i * lineSize + j];
		}
	}
	return r;
}

Reversi Reversi::rotate90() {
	int i, j;
	Reversi r;
	r.moveColor=moveColor;
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			r.board[j * lineSize + boardSize - i + 1] = board[i * lineSize + j];
		}
	}
	return r;
}

std::string Reversi::toCharString() {
	std::string s;
	int i, j;
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			s+= board[i * lineSize + j];
		}
	}
	return s;
}

ReversiCode Reversi::code() const {
#if USE_SYMMETRY==1
	ReversiCode min = code1(), c;
	Reversi r;
	r.moveColor = moveColor;
	int i, j, i1, j1;
	for (i = 0; i < 7; i++) {
		j = 0;
		for (i1 = 1; i1 <= boardSize; i1++) {
			for (j1 = 1; j1 <= boardSize; j1++) {
				r.board[i1 * lineSize + j1] = board[flip[i][j++]];
			}
		}
		c = r.code1();
		if (c < min) {
			min = c;
		}
	}
	return min;
#elif USE_SYMMETRY==2
	ReversiCode o = code1();
	ReversiCode c, min = o;
	uint64_t *p;
	c.moveColor = moveColor;

	int i, j, k, i1;
	for (i = 0; i < 7; i++) {
		for (auto &a : c.c) {
			a = 0;
		}
		p = c.c;
		j = 0;
		for (i1 = 0; i1 < boardSize * boardSize; i1++) {
			k = flip[i][i1];
			//get i-th pair of bits, every uint64_t has 64bits or 32 pairs
			*p |= ((o.c[k / 32] >> (2 * (k % 32))) & 3) << j;
			j += 2;
			if (j == 64) {
				p++;
				j = 0;
			}
		}
		if (c < min) {
			min = c;
		}
	}
	min.moveColor = moveColor;
	return min;
#else
	return code1();
#endif
}

ReversiCode Reversi::code1() const {
	ReversiCode c;
	c.moveColor=moveColor;
	for(auto&a:c.c){
		a=0;
	}
	uint64_t *p = c.c;
	int i, j, k = 0;
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			*p |= uint64_t(board[i * lineSize + j]) << k;
			k += 2;
			if (k == 64) {
				k = 0;
				p++;
			}
		}
	}
	return c;
}

void Reversi::fromCode(const ReversiCode &c) {
	const uint64_t *p = c.c;
	moveColor=c.moveColor;
	int i, j, k = 0;
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			board[i * lineSize + j] = (*p >> k) & 3;
			k += 2;
			if (k == 64) {
				k = 0;
				p++;
			}
		}
	}
}

void Reversi::insert(int layer, ReversiCode const &parentCode, char move,
		ReversiCode &code) {
#ifdef STORE_MOVE
	code.move=move;
	auto p=parentCode.c;
	for(auto&a:code.parent){
		a=*p++;
	}
	code.parentColor = parentCode.moveColor;
#endif
	layerSet[layer].insert(code);
}

void Reversi::initFirst2Layers(int type,bool bwOnly/*=false*/) {
	int i;
	Reversi r, ro;
	ReversiCode co, c;

	if(bwOnly){
		setSearchOnlyBlackAndWhite();
	}

	ro.init(type);
	co = ro.code();
	insert(0, co, 0, co);
	assert(type == 1 || type == 3);
	for (i = 0; i < (type < 2 ? 1 : 2); i++) {
		char m = getCenterMove(i == 0);
		r.init(type);
		r.makeMove(m);
		c = r.code();
		insert(1, co, m, c);
	}
}

std::string Reversi::endGameCounts() {
	std::string s;
	int i;
	size_t j,k=0;
	for(i=0;i<3;i++){
		j=foundEndCount[i].size();
		k+=j;
		if(i){
			s+='+';
		}
		s+=toString(j,',');
	}
	s+='='+toString(k, ',');
	return s;
}

void Reversi::test(){
	//int i, j, k,i1;
	Reversi r;

	staticInit();

	init(1);
	makeMoves("D3E3F4G3F3C5H3F2C4C3E2E1B3H4H5A3");
	print();


}

int Reversi::countBorderChips() const{
	int i,k=0;
	for (i = 1; i <= boardSize; i++) {
		if(board[1 * lineSize + i] !=empty){
			k++;
		}
		if(board[boardSize * lineSize + i] !=empty){
			k++;
		}
	}
	for (i = 2; i < boardSize; i++) {
		if(board[i * lineSize + 1] !=empty){
			k++;
		}
		if(board[i * lineSize + boardSize] !=empty){
			k++;
		}
	}
	return k;
}

void Reversi::fillForFlip() {
	int i,j;
	//#if USE_SYMMETRY!=1 when USE_SYMMETRY not defined or USE_SYMMETRY==2
#if USE_SYMMETRY!=1
	int k=0;
#endif
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
#if USE_SYMMETRY==1
			board[i * lineSize + j]=i * lineSize + j;
#else
			board[i * lineSize + j]=k++;
#endif
		}
	}
}

#ifdef USE_SYMMETRY
void Reversi::setFlip(int n){
	int i,j,k=0;
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			flip[n][k++]=board[i * lineSize + j];
		}
	}
}
#endif
