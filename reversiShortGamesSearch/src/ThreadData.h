/*
 * ThreadData.h
 *
 *  Created on: 31.05.2022
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef THREADDATA_H_
#define THREADDATA_H_

#include "ReversiCode.h"

class ThreadData{
public:
	int index;
	ReversiCodeSetCI begin,end;
	ReversiCodeSet foundEndCount[maxLayer-maxLayer1+1][3];
	ReversiCode root;
	clock_t start;
	static int size;
};

#endif /* THREADDATA_H_ */
