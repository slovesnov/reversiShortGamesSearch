/* Reversi.cpp
 *
 *  Created on: 08.05.2022
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */
#include <cassert>
#include <cstring>
#include <iostream>

#include "Reversi.h"
#include "aslov.h"

extern std::vector<ThreadData> threadData;

std::vector<int> Reversi::cells,Reversi::possibleMoves;
ReversiCodeSet Reversi::layerSet[maxLayer+1];
ReversiCodeSet Reversi::found[3];
ReversiCodeSet Reversi::foundEndCount[maxLayer+1][3];
int Reversi::foundMinTurns[3]={255,255,255};
int Reversi::maxMinChips=-1;
#ifdef BORDER_COUNT
int Reversi::borderCount;
#endif

#ifdef STORE_MOVE
std::vector<std::string> Reversi::foundString[3];
#endif

int Reversi::flip[7][boardSize*boardSize];

#ifdef SEARCH_MOVES
ReversiCodeSet Reversi::endCode;
bool Reversi::searchFromStart;
#endif

const int Reversi::direction[] = { -lineSize-1, -lineSize, -lineSize+1, -1, 1, lineSize-1, lineSize, lineSize+1 };
const char outChar[]="bw.";

const char *gameTypeString[]={
		"black only","white only","black and white"
};

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
#if defined(USE_SYMMETRY) || defined(SEARCH_MOVES)
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

Reversi::Reversi(ReversiCode const& code){
	*this=code;
}

void Reversi::init(int type/*=0*/) {
	for (auto &a : board) {
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
	std::cout<<toString();
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
	assert(s.length() == 2 || (boardSize>=10 && s.length() == 3));
	char a = tolower(s[0]);
	assert(a >= 'a' && a < 'a'+boardSize);
	std::string q=s.substr(1);
	int d=0;
#ifndef NDEBUG
		bool b =
#endif
	parseString(q,d);//can be "j10"
	assert(b);
	assert(d >= 1 && d < 1+boardSize);
	return (a-'a'+1) + d * lineSize;
}

std::string Reversi::indexToString(int index) {
	return std::string(1, char((index % lineSize) + 'a' - 1))+std::to_string(index / lineSize);
}

void Reversi::makeMoves(const std::string &_s) {
	std::string s=replaceAll(_s, " ", "");
	size_t i;
	for (i = 0; i < s.length(); ) {
		int j=2+isdigit(s[i+2]);
#ifndef NDEBUG
		bool b =
#endif
				makeMove(index(s.substr(i, j)));
		//printl(s.substr(i, j));
		assert(b);
		i+=j;
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

void Reversi::operator=(ReversiCode const& c){
	const uint64_t *p = c.c;
#ifdef REVERSI_CODE_MOVE_INSIDE
	moveColor=c.getMove();
#else
	moveColor=c.moveColor;
#endif
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
#ifdef REVERSI_CODE_MOVE_INSIDE
	if(moveColor==black){
		board[upleftCenter]&=1;
	}
#endif
}

std::string Reversi::toString()const{
	int i, j,l;
	const int k=boardSize>=10?2:1;
	char c;
	std::string s,q = " ";
	if(k==2){
		q+=" ";
	}
	for (i = 0, c = 'a'; i < boardSize; i++, c++) {
		q += c;
	}
	q += " \n";
	s=q;
	for (i = 1; i <= boardSize; i++) {
		s+=format("%*d",k, i);
		for (j = 1; j <= boardSize; j++) {
			l=board[i * lineSize + j];
			if(l<3){
				s+=format("%c", outChar[l]);
			}
			else{
				s+=format("%x", l);
			}
		}
		s+=format("%d\n", i);
	}
	s+=q;
	return s;
}

void Reversi::addAllMoves(int layer, ReversiCode const &parentCode) const {
#ifdef BORDER_COUNT
	int j;
#endif
	int l;
#ifndef SEARCH_MOVES
	int k;
#endif
	bool f = false;
	Reversi t;
	ReversiCode tcode;
	std::string s;

	Reversi p;
	p.setPotentialMoves(*this);

	for (l = 0; l < 2; l++) {
		t.assign(*this, l == 0 ? moveColor : oppositeColor(moveColor));
		for (int i : possibleMoves) {
			if(p.board[i]!=1){
				continue;
			}

			if (t.makeMove(i)) {
				f = true;
				tcode=t.code();

#ifdef BORDER_COUNT
				j=t.countBorderChips();
				if(j>0){
					borderCount++;
				}
#endif

#ifdef SEARCH_MOVES
				if(endCode.find(tcode)!=endCode.end()){
					s="";
					tcode=parentCode;
					//auto a = this->code();
					for (int j = layer - 1; j > (searchFromStart?0:1); j--) {
						auto it = layerSet[j].find(tcode);
						assert(it != layerSet[j].end());
						auto p=it->parent;
						for(auto&a:tcode.c){
							a=*p++;
						}
#ifndef REVERSI_CODE_MOVE_INSIDE
						tcode.moveColor = it->parentColor;
#endif
						s = indexToString(it->move) + "" + s;
					}
					s += indexToString(i);
					printl(s);
					exit(0);
				}
#else//SEARCH_MOVES326

				if (t.isEnd()) {
					//use same color for operator< because game is over
#ifdef	REVERSI_CODE_MOVE_INSIDE
					tcode.setBlackMove();
#else
					tcode.moveColor=black;
#endif
					k = t.endGameType();
					if( (searchBWOnly && k == BLACK_AND_WHITE) || !searchBWOnly ){
						foundEndCount[layer][k].insert(tcode);
					}
					//output/count all positions with black and white colors
					bool b = ((searchBWOnly && k == BLACK_AND_WHITE) || !searchBWOnly) && found[k].empty();
					if (!b) {
						if (k == BLACK_AND_WHITE) {
							b = foundMinTurns[k] >= layer
									&& found[k].find(tcode) == found[k].end();
						}
					}
					if(b){
						found[k].insert(tcode);
						foundMinTurns[k]=layer;
#ifdef STORE_MOVE
						s="";
						tcode=parentCode;
						//auto a = this->code();
						for (int j = layer - 1; j > 0; j--) {
							auto it = layerSet[j].find(tcode);
							assert(it != layerSet[j].end());
							auto p=it->parent;
							for(auto&a:tcode.c){
								a=*p++;
							}
#ifndef REVERSI_CODE_MOVE_INSIDE
							tcode.moveColor = it->parentColor;
#endif
							s = indexToString(it->move) + "" + s;
						}
						s += indexToString(i);
						foundString[k].push_back(s);
//						exit(0);
#else
						s="lastmove "+indexToString(i);
#endif
//						println("found %s %d turns %s", gameTypeString[k], layer,s.c_str());

						//Reversi r=parentCode;
//						t.print();

						fflush(stdout);
					}
				}

#endif//SEARCH_MOVES326

				else if(layer!=maxLayer){
#ifdef BOARD_LAYER
					if(layer!=BOARD_LAYER || t.countBorderChips()>0){
						insert(layer, parentCode, i, tcode);
					}
#else
					insert(layer, parentCode, i, tcode);
#endif
				}
				t.assign(*this, l == 0 ? moveColor : oppositeColor(moveColor));
			}//if (t.makeMove(i))
		}
		if (f) {
			return;
		}
	}
}

void Reversi::addAllMoves(ReversiCode const &parentCode, int depth,
		ThreadData &data, Reversi &p) const {
	int k, l;
	bool f = false;
	Reversi t;
	ReversiCode tcode;
	std::string s;
	const int layer=maxLayer-maxLayer1-depth;
	Reversi p1;

	for (l = 0; l < 2; l++) {
		t.assign(*this, l == 0 ? moveColor : oppositeColor(moveColor));
		for (int i : possibleMoves) {
			if(p.board[i]!=1){
				continue;
			}

			if (t.makeMove(i)) {
				f = true;
				if (depth == 1) {
					if (t.isEnd()) {
						tcode=t.code();
						//use same color for operator< because game is over
#ifdef	REVERSI_CODE_MOVE_INSIDE
						tcode.setBlackMove();
#else
						tcode.moveColor = black;
#endif
						k = t.endGameType();
						bool b=true;
						if((searchBWOnly && k==BLACK_AND_WHITE) || !searchBWOnly){
							b=data.foundEndCount[layer][k].insert(tcode).second;
						}
						if (/*boardSize >= 12 &&*/ k == BLACK_AND_WHITE && b) {
							outSaveFoundedToFile(tcode,data,__LINE__,i);
						}
					}
				}
				else{
					tcode = t.code();
					p1.copyBoard(p);
					p1.addPotentialMove(i);
					t.addAllMoves(tcode, depth - 1, data, p1);
				}
				t.assign(*this, l == 0 ? moveColor : oppositeColor(moveColor));
			}
		}
		if (f) {
			return;
		}
	}

	if(layer>0){
		tcode=parentCode;
		//use same color for operator< because game is over
#ifdef	REVERSI_CODE_MOVE_INSIDE
		tcode.setBlackMove();
#else
		tcode.moveColor=black;
#endif
		k = endGameType();
		bool b=true;
		if ((searchBWOnly && k == BLACK_AND_WHITE) || !searchBWOnly) {
			b=data.foundEndCount[layer - 1][k].insert(tcode).second;
		}
		if (/*boardSize >= 12 &&*/ k == BLACK_AND_WHITE && b) {
			outSaveFoundedToFile(tcode,data,__LINE__,-1);
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

void Reversi::assign(const Reversi &re,char _move) {
	moveColor = _move;
// min lineSize+1, max boardSize*(lineSize+1), need to copy max-min+1
	memcpy(board+lineSize+1, re.board+lineSize+1, ((boardSize-1)*(lineSize+1)+1) * sizeof(char));
//	memcpy(board, re.board, boardSize2 * sizeof(char));
}

void Reversi::copyBoard(const Reversi &re) {
	memcpy(board+lineSize+1, re.board+lineSize+1, ((boardSize-1)*(lineSize+1)+1) * sizeof(char));
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
#ifdef REVERSI_CODE_MOVE_INSIDE
	if(moveColor==black){
		c.setBlackMove();
	}
	else{
		c.setWhiteMove();
	}
#else
	c.moveColor=moveColor;
#endif
	return c;
}

void Reversi::insert(int layer, ReversiCode const &parentCode, char move,
		ReversiCode &code) {
#ifdef STORE_MOVE
	code.move=move;
	auto p=parentCode.c;
	for(auto&a:code.parent){
		a=*p++;
	}
#ifndef REVERSI_CODE_MOVE_INSIDE
	code.parentColor = parentCode.moveColor;
#endif

#endif
	layerSet[layer].insert(code);
}

void Reversi::initFirst2Layers(int type) {
	int i;
	Reversi r, ro;
	ReversiCode co, c;

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

std::string Reversi::endGameCounts(int layer,bool showNoBorder/*=true*/) {
	std::string s;
	int i, noBorderCount;
	size_t j, k = 0;
	for (i = 0; i < 3; i++) {
		if(searchBWOnly && i!=BLACK_AND_WHITE){
			continue;
		}
		auto &set = foundEndCount[layer][i];
		j = set.size();
		k += j;
		if (i && !searchBWOnly) {
			s += '+';
		}
		s += ::toString(j, ',');
		if (j) {
			noBorderCount= 0;
			for (auto &a : set) {
				Reversi r;
				r=a;
				if( r.countBorderChips()==0){
					noBorderCount++;
				}
			}
			if(showNoBorder){
				s+="{nb"+  ::toString(noBorderCount,',')+"}";
			}
		}
	}
	if (!searchBWOnly) {
		s += '=' + ::toString(k, ',');
	}
	return s;
}

std::string Reversi::shortestEndGameCounts(){
	std::string s;
	int i, layer;
	size_t j;
	for (i = 0; i < 3; i++) {
		for (layer = 2; layer <= maxLayer; layer++) {
			j = foundEndCount[layer][i].size();
			if (j) {
				s += ::toString(j, ',') + ",";
				break;
			}
		}
		if(layer==maxLayer+1){
			s+="?,";
		}
	}
	return s;
}

bool Reversi::test(int p){
	int i, j,k;
	std::string s;
	Reversi r;
	init(1);
	makeMoves("d3");
	print();
	r.setPotentialMoves(*this);
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			k=i * lineSize + j;
			auto c=r.board[k];
			if(c==1){
				s+=indexToString(k)+" ";
			}
		}
	}
	printl(s);
	return false;
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

int Reversi::countBorderChips(char color) const{
	int i,k=0;
	for (i = 1; i <= boardSize; i++) {
		if(board[1 * lineSize + i] ==color){
			k++;
		}
		if(board[boardSize * lineSize + i] ==color){
			k++;
		}
	}
	for (i = 2; i < boardSize; i++) {
		if(board[i * lineSize + 1] ==color){
			k++;
		}
		if(board[i * lineSize + boardSize] ==color){
			k++;
		}
	}
	return k;
}

int Reversi::countChips(char color) const {
	int i,j;
	int c=0;
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			if(board[i * lineSize + j]==color){
				c++;
			}
		}
	}
	return c;
}


void Reversi::fillForFlip() {
	int i,j;
	//#if USE_SYMMETRY!=1 when USE_SYMMETRY not defined or USE_SYMMETRY==2
#if USE_SYMMETRY!=1 && !defined(SEARCH_MOVES)
	int k=0;
#endif
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
#if USE_SYMMETRY==1 || defined(SEARCH_MOVES)
			board[i * lineSize + j]=i * lineSize + j;
#else
			board[i * lineSize + j]=k++;
#endif
		}
	}
}

void Reversi::setFlip(int n){
	int i,j,k=0;
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			flip[n][k++]=board[i * lineSize + j];
		}
	}
}

int Reversi::getMinChips() const {
	int i,j;
	int b=0,w=0;
	char c;
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			c=board[i * lineSize + j];
			if(c==black){
				b++;
			}
			else if(c==white){
				w++;
			}
		}
	}
	return std::min(b,w);
}

int Reversi::turns()const{
	int i,j,k=-4;
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			if(board[i * lineSize + j]!=empty){
				k++;
			}
		}
	}
	return k;
}

void Reversi::setPotentialMoves(Reversi const& re){
	memset(board+lineSize+1, 0, ((boardSize-1)*(lineSize+1)+1) * sizeof(char));
	int i,j,k=-4;
	for (i = 1; i <= boardSize; i++) {
		for (j = 1; j <= boardSize; j++) {
			k=i * lineSize + j;
			if(re.board[k]!=empty){
				addPotentialMove(k);
			}
		}
	}
}

void Reversi::addPotentialMove(int i) {
	board[i]|=2;
	for(auto&a:direction){
		board[i+a]|=1;
	}
}

std::ostream& operator<<(std::ostream& os, const Reversi& a){
    return os << a.toString();
}

void Reversi::outSaveFoundedToFile(const ReversiCode& code,
		const ThreadData &data,int line,int lastmove) {
	Reversi r;
	ReversiCode v[]={code,data.root};
	int i=0;
	for(auto&c:v){
		printl(c)
		r=c;
		r.print();
		println("%d turns", r.turns());
		if(!i){
			printl("found thread time "+secondsToString(data.start)+" line "+forma(line)," thread ",&data-&threadData[0])
		}
		i++;

	}
	fflush(stdout);

	const bool findAll=1;

	auto o=std::fstream::out;
	if(findAll){
		o|=std::fstream::app;
	}

	i=0;
	ThreadData::mtx.lock();
	std::ofstream f("found.txt", o);
	f << "at line" << line <<" lastmove "<<lastmove<<" "<< indexToString(lastmove) << "\n";
	for (auto &c : v) {
		Reversi r = c;
		if (i) {
			f << "parent\n";
		}
		f << r << " turns" << r.turns() << "\n" << c << "\n";
		i++;
	}
	f.close();
	ThreadData::mtx.unlock();

	if(!findAll){
		exit(0);
	}
}

#ifdef SEARCH_MOVES
void Reversi::searchMoves(ReversiCode const& code){
	searchFromStart = true;
	Reversi r=code;
	r.addSearchAllSymmetries(code);
}

void Reversi::searchMoves(ReversiCode const& from,ReversiCode const& to){
	searchFromStart = false;
	layerSet[1].clear();
	layerSet[1].insert(from);
	Reversi r=to;
	r.addSearchAllSymmetries(to);
}

void Reversi::addSearchAllSymmetries(ReversiCode const& code){
	Reversi r=code;
	endCode.insert(code);
	r.moveColor = moveColor;
	int i, j, i1, j1;
	for (i = 0; i < 7; i++) {
		j = 0;
		for (i1 = 1; i1 <= boardSize; i1++) {
			for (j1 = 1; j1 <= boardSize; j1++) {
				r.board[i1 * lineSize + j1] = board[flip[i][j++]];
			}
		}
		endCode.insert(r.code1());
	}
}
#endif
