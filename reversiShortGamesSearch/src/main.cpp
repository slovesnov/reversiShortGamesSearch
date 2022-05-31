#include <thread>

#include "aslov.h"
#include "Reversi.h"
/*
 * 8x8 https://thesaurus.altervista.org/reversi?tz=airyqU6pTShgK78C&fl=if#e3
 * D3E3F4G3F3C5H3F2C4C3E2E1B3H4H5A3 16turns
 */

std::vector<ThreadData> threadData;

void threadf(int t, int layer) {
	auto &a = threadData[t];
	a.start=clock();
	double x,time;
	Reversi r;
	int i=0;
	Reversi p;
	std::string s,q;
	const int N=maxLayer<=15 ? 100'000 : 10'000;

	for (auto it = a.begin; it != a.end; it++) {
		auto const &code = *it;
		a.base=code;
		r.fromCode(code);
		p.setPotentialMoves(r);
		r.addAllMoves(code, maxLayer - maxLayer1, a, p);
		time=timeElapse(a.start);
		i++;
		//i - time
		//all - x  x=time*all/i
		//left x-time
		x = time * ThreadData::size / i;
		if (i % N == 0) {
			s=format("%4.1lf%%",i * 100. / ThreadData::size);
			q=timeToString("%d%b%Y %H:%M:%S",true);
			println("t%d %s %s proceed %s, left %s, all %s", t,s.c_str(),q.c_str(),
					 secondsToString(time).c_str(),
					secondsToString(x - time).c_str(),
					secondsToString(x).c_str());
			fflush(stdout);
			std::ofstream f("o"+std::to_string(t)+".txt");
			f<<i<<" "<<ThreadData::size<<" "<<s<<" "<<q;
		}
	}
}

int main(){
	const int threads = getNumberOfCores()-1;
	const bool standard = 1;
	const bool showNoBorder=false;
	const int type = standard ? 1 : 3; //type=1 standard, type=3 non standard
	const int equalCharRepeat=60;
	Reversi r;
	std::string s,q;
	int i, j, k, l;
	size_t psize, ssize;
	clock_t begin;

	aslovSetOutputWidth(60);
	Reversi::staticInit();
	Reversi::initFirst2Layers(type);
	preventThreadSleep();

//	r.init(type);
//	r.makeMoves("f5e4f4");
//	Reversi::endCode=r.code();

	//without symmetry
//	ReversiCode code14({0xaa1aaaaa26aaa02aull, 0xaaaaa12aaaaa62aaull, 0xaa4aaaaa92aaaa8eull, 0xaaaaaaaaaaaaaaaaull, 0xaaaaaaaaull});
//	ReversiCode code17({0xaa0aaaaa22aaa028ull, 0xaaaaa029aaaa26aaull, 0xa80aaaaa82aaaa82ull, 0xaaaaaaaaaaaaaaaaull, 0xaaaaaaaaull});
//	Reversi::layerSet[1].clear();
//	Reversi::layerSet[1].insert(code14);
//	Reversi::endCode=code17;
//
//	r.test();
//	return 0;

	s=timeToString("%d%b%Y %H:%M:%S",true);

	s+=" REVERSI_CODE_MOVE_INSIDE=";
#ifdef REVERSI_CODE_MOVE_INSIDE
	s+="1";
#else
	s+="0";
#endif
	printl(s);

	s=type<2?"standard":"non standard";
	s+=format(" board=%dx%d",boardSize,boardSize);
#ifndef NDEBUG
	s+=" DEBUG";
#endif
#ifdef USE_SYMMETRY
	s+=" symmetry="+forma(USE_SYMMETRY);
#endif
	s+=" searchBWOnly="+std::to_string(searchBWOnly);

	s+=" storeMove=";
#ifdef STORE_MOVE
	s+="1";
#else
	s+="0";
#endif

#ifdef BOARD_LAYER
	s+=" BOARD_LAYER="+std::to_string(BOARD_LAYER);
#endif

	s+=" maxLayer="+std::to_string(maxLayer1)+"/"+std::to_string(maxLayer);
	//printan(s)

	s+=" threads="+std::to_string(threads);
	printl(s)

	for (i = 2; i <= maxLayer1; i++) {
		begin = clock();
		auto &previousSet = Reversi::layerSet[i - 1];
		auto &set = Reversi::layerSet[i];
		psize = previousSet.size();
		j = 0;
		for (auto &code : previousSet) {
			r.fromCode(code);
			r.addAllMoves(i, code);
		}

		ssize = set.size();
		s = format("%2d %11s bf=", i,
				i == maxLayer ? "?" : toString(ssize, ',').c_str());
		s += i == maxLayer ? "?.??" : format("%.2lf", double(ssize) / psize);
		q = Reversi::endGameCounts(i, showNoBorder);
		if (q != "0+0+0=0" && q!="0") {
			s += " " + q;
		}
#ifdef BORDER_COUNT
		j=Reversi::borderCount;
		if(j){
			s+=" border="+toString(j,',');
		}
#endif
		s+=" "+secondsToString(begin);
		printl(s);
		//println("%2d %11s bf=%.2lf %s", i,toString(ssize,',').c_str(),double(ssize)/psize,secondsToString(begin).c_str() )
		fflush(stdout);

#ifdef BORDER_COUNT
		Reversi::borderCount=0;
#endif
		#ifndef STORE_MOVE
		previousSet.clear();
		#endif

		if(Reversi::allFound(s)){
//			break;
		}

	}

	int layer=i;
	s=std::string(equalCharRepeat,'=');
	printl(s);
	fflush(stdout);

	std::vector<std::thread> vt;

	begin = clock();
	auto& previousSet = Reversi::layerSet[i-1];
	j=previousSet.size()/threads;
	ThreadData::size=j;
	ReversiCodeSetCI it;
	l=0;
	k=0;
	it=previousSet.begin();
	threadData.resize(threads);
	for(k=0;k<threads;k++){
		threadData[k].begin=it;
		std::advance(it, j);
	}
	for(k=0;k<threads-1;k++){
		threadData[k].end=threadData[k+1].begin;
	}
	threadData[k].end=previousSet.end();

	for (i = 0; i < threads; ++i) {
		vt.push_back(std::thread(threadf, i,layer));
	}

	for (auto& a : vt){
		a.join();
	}

	ReversiCodeSet a;
	for (i = layer; i <= maxLayer; i++) {
		s = format("%2d ", i);
		psize = 0;
		for (j = 0; j < 3; j++) {
			a.clear();
			for (l = 0; l < threads; l++) {
				a.merge(threadData[l].foundEndCount[i - layer][j]);
			}
			psize += a.size();
			s += toString(a.size(), ',');
			s += j == 2 ? '=' : '+';
		}
		s += toString(psize, ',');
		printl(s)
		;
	}

	s=format(" layers %d-%d time ", maxLayer1 + 1, maxLayer)+ secondsToString(begin)+" ";
	i = equalCharRepeat - s.length();
	q = std::string( i / 2, '=');
	s = q + s + q;
	if ( i % 2 ) {
		s += '=';
	}
	printl(s)
	;
	fflush(stdout);
	//getchar();//if run not under eclipse

	//printl(	Reversi::shortestEndGameCounts())
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
