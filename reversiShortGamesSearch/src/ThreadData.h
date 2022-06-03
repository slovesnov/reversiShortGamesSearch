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

#include <mutex>

#include "ReversiCode.h"

class Chain{
public:
	ReversiCodeSetCI begin,end;
	bool finish;
	int proceed;
	operator bool(){
		return !finish;
	}
};

class ThreadData{
	static int proceed;
public:
	int index;
	ReversiCodeSet foundEndCount[maxLayer-maxLayer1+1][3];
	ReversiCode root;
	clock_t start;
	static std::mutex mtx;
	static ReversiCodeSet* pset;
	static ReversiCodeSetCI first;
	static Chain getNextChain(int n);
	static void init(ReversiCodeSet*p);
	static size_t size(){
		return pset->size();
	}
};

#endif /* THREADDATA_H_ */
