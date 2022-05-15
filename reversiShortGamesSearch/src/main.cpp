#include "aslov.h"
#include "Reversi.h"

#include <set>

//https://thesaurus.altervista.org/reversi?tz=airyqU6pTShgK78C&fl=if#e3
// D3E3F4G3F3C5H3F2C4C3E2E1B3H4H5A3 16turns

int main(){
	const bool standard = 0;
	const bool bwOnly=1;
	const bool showProceed1million=1;
	int type = standard ? 1 : 3; //type=1 standard, type=3 non standard
	Reversi r;
	std::string s,q;
	int i,j;
	size_t psize, ssize;
	clock_t begin;
	aslovSetOutputWidth(60);

//	r.test();
//	return 0;

	Reversi::staticInit();
	Reversi::initFirst2Layers(type,bwOnly);
	preventThreadSleep();

	s=type<2?"standard":"non standard";
	s+=format(" board %dx%d",boardSize,boardSize);
//#ifdef NDEBUG
//	s+=" NDEBUG";
//#endif
#ifdef USE_SYMMETRY
	s+=" symmetry="+forma(USE_SYMMETRY);
#endif
	s+=" bwOnly="+std::to_string(bwOnly);

	s+=" storeMove=";
#ifdef STORE_MOVE
	s+="1";
#else
	s+="0";
#endif
	//printan(s)
	printl(s)

	for (i = 2; i <= maxLayer; i++) {
		begin = clock();
		auto& previousSet = Reversi::layerSet[i-1];
		auto& set = Reversi::layerSet[i];
		psize = previousSet.size();
		j=0;
		for (auto &code : previousSet) {
			r.fromCode(code);
			r.addAllMoves(i,code);
			if(showProceed1million && ++j%1'000'000==0){
				printf("%d.",(j/1'000'000)%10);
				fflush(stdout);
			}
		}

		ssize=set.size();
		if(showProceed1million && j>1'000'000){
			printf("\n");
		}

		s=format("%2d %11s bf=%.2lf", i,toString(ssize,',').c_str(),double(ssize)/psize );
		q=Reversi::endGameCounts();
		if(q!="0+0+0=0"){
			s+=" "+q;
		}
		j=Reversi::borderCount;
		if(j){
			s+=" border="+toString(j,',');
		}
		s+=" "+secondsToString(begin);
		printl(s);
		//println("%2d %11s bf=%.2lf %s", i,toString(ssize,',').c_str(),double(ssize)/psize,secondsToString(begin).c_str() )
		fflush(stdout);

		for(j=0;j<3;j++){
			Reversi::foundEndCount[j].clear();
			Reversi::borderCount=0;
		}
		#ifndef STORE_MOVE
		previousSet.clear();
		#endif

		if(Reversi::allFound(s)){
//			break;
		}

	}

//	s = "";
//	for (i = 0; i < 3; i++) {
//		s += std::to_string(Reversi::foundMinTurns[i]) + " ";
//	}
//	printl(s)

#ifdef STORE_MOVE
	printl(Reversi::forHtml())
#endif
}
