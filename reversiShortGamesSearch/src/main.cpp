#include "aslov.h"
#include "Reversi.h"

//https://thesaurus.altervista.org/reversi?tz=airyqU6pTShgK78C&fl=if#e3
// D3E3F4G3F3C5H3F2C4C3E2E1B3H4H5A3 16turns

int main(){
	const bool standard = 1;
	const bool bwOnly=1;
	const int proceedN=10'000'000;
	const bool showProceedN=1;
	int type = standard ? 1 : 3; //type=1 standard, type=3 non standard
	Reversi r;
	std::string s,q;
	int i,j;
	size_t psize, ssize;
	clock_t begin;

	aslovSetOutputWidth(60);
	Reversi::staticInit();
	Reversi::initFirst2Layers(type,bwOnly);
	preventThreadSleep();

//	r.test();
//	return 0;
//	r.init(1);
//	r.makeMoves("g6g7g8h8i8i9i10j10e4");
//	r.print();
//	return 0;

	s=type<2?"standard":"non standard";
	s+=format(" board %dx%d",boardSize,boardSize);
#ifndef NDEBUG
	s+=" DEBUG";
#endif
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

#ifdef BOARD_LAYER
	s+=" boardLayer="+std::to_string(BOARD_LAYER);
#endif

	s+=" maxLayer="+std::to_string(maxLayer);
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
			if(showProceedN && ++j%proceedN==0){
				printf("%d.",j/proceedN);
				fflush(stdout);
			}
		}

		ssize=set.size();
		if(showProceedN && j>proceedN){
			printf("\n");
		}

		s=format("%2d %11s bf=", i,i==maxLayer ? "?": toString(ssize,',').c_str() );
		s+=i==maxLayer ? "?.??" : format("%.2lf",double(ssize)/psize);
		q=Reversi::endGameCounts(i);
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
			Reversi::borderCount=0;
		}
		#ifndef STORE_MOVE
		previousSet.clear();
		#endif

		if(Reversi::allFound(s)){
//			break;
		}

	}
	printi

	printl(	Reversi::shortestEndGameCounts())

}

/* file parser for html
 int main() {
	std::ifstream f("in.txt");
	std::string s,line,q,a[14][6];
	int layer,i,j;
	for(i=0;i<14;i++){
		for(j=0;j<6;j++){
			a[i][j]="*";
		}
	}

	std::regex re(R"((([\d,]+)(\{nb[\d,]+\})?[+=]){3})");
	std::cmatch m;
	std::regex re1(R"(\{nb[\d,]+\})");

	int k=-3;
	while(std::getline(f, line)){
		if(line.find("bf=")!=std::string::npos && (layer=stoi(line))>=7){
			if(layer==7){
				k+=3;
			}
			auto*p=a[layer]+k;
			if(std::regex_search (line.c_str(), m, re)){
				q=m[0];
				q=q.substr(0,q.length()-1);
				s=std::regex_replace(q, re1, "");
				auto v=split(s,"+");
				for(auto&a:v){
					*p++=a;
				}
			}
			else{
				p[0]=p[1]=p[2]="0";
			}
			printl(line)
		}
	}

	for(i=7;i<14;i++){
		printf("<tr><td>%d",i);
		for(j=0;j<6;j++){
			printf("<td>%s",a[i][j].c_str());
		}
		printf("\n");
	}

}

 */
