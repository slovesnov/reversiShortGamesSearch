/*
 * defines.h
 *
 *  Created on: 31.05.2022
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef DEFINES_H_
#define DEFINES_H_

/* allow even boardSize up to 16 if boardSize more than 16 need to change
 * type of boardSize from unsigned char to short because of fillForFlip() function
 */
const int boardSize = 12;

//cut all positions without board chips on this layer, if not defined never cut
//#define BOARD_LAYER 9
//const int maxLayer1=15;
//const int maxLayer=17;

const int maxLayer1=11;
const int maxLayer=15;



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

const bool searchBWOnly=1;


//#define BORDER_COUNT
static const char black = 0;
static const char white = 1;
static const char empty = 2;

//when REVERSI_CODE_MOVE_INSIDE is NOT defined programs faster 1-(6*60+10)/(6*60+55)=10.8%
//REVERSI_CODE_MOVE_INSIDE is smaller sizeof(ReversiCode)
//#define REVERSI_CODE_MOVE_INSIDE




#endif /* DEFINES_H_ */
