/*
 * ThreadData.cpp
 *
 *  Created on: 31.05.2022
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "ThreadData.h"

std::mutex ThreadData::mtx;
ReversiCodeSet* ThreadData::pset;
ReversiCodeSetCI ThreadData::first;
int ThreadData::proceed;

Chain ThreadData::getNextChain(int n) {
	Chain c;
	mtx.lock();
	if (first == pset->end()) {
		c.finish = true;
		c.proceed=size();
	} else {
		c.finish = false;
		c.begin = first;
		c.proceed=proceed;
		while(n>0){
			first++;
			n--;
			proceed++;
			if(first==pset->end()){
				break;
			}
		}
		c.end = first;
	}
	mtx.unlock();
	return c;
}

void ThreadData::init(ReversiCodeSet *p) {
	pset=p;
	first=p->begin();
}
