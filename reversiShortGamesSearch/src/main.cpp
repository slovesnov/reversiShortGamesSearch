#include <thread>

#include "aslov.h"
#include "Reversi.h"
/*
 * 8x8 https://thesaurus.altervista.org/reversi?tz=airyqU6pTShgK78C&fl=if#e3
 * D3E3F4G3F3C5H3F2C4C3E2E1B3H4H5A3 16turns
 */

std::vector<ThreadData> threadData;

std::string getOutFileName(int i){
	return "o"+forma(i)+".txt";
}
ReversiCodeSet proceedFile();

void threadf(int t, int layer) {
	auto &a = threadData[t];
	a.start = clock();
	double x, time;
	Reversi r;
	int i = 0,j;
	Reversi p;
	std::string s, q;
	const int N = maxLayer <= 15 ? 100'000 : 20'000;
	int size = ThreadData::size();
	Chain c;
	while ( (c = ThreadData::getNextChain(N)) ) {
		for (auto it = c.begin; it != c.end; it++) {
			auto const &code = *it;
			a.root = code;
			r = code;
			p.setPotentialMoves(r);
			r.addAllMoves(code, maxLayer - maxLayer1, a, p);
		}
		time = timeElapse(a.start);
		i = c.proceed+N;
		j=i-ThreadData::start_it;
		//printl(i,j)
		//j - time
		//left size-j left=time*(size-j)/j
		x = time * (size - j) / j;
		q = timeToString("%d%b%Y %H:%M:%S", true);
		s = format("t%d %4.1lf%% %dk/%dk ",t, i * 100. / size, i / 1000,
				size / 1000)+q+" left "+secondsToString(x);
		if (t == 0) {
			printl(s)
			fflush(stdout);
		}

		std::ofstream f(getOutFileName(t));
		f <<c.proceed<<"\n"<<s << "\n";
		f.close();
	}
	q = timeToString("%d%b%Y %H:%M:%S", true);
	println("t%d %s finished", t, q.c_str());
	fflush(stdout);
}

int main(int argc, char *argv[]){
	const bool standard = 1;
	const bool showNoBorder=false;
	const int type = standard ? 1 : 3;
	const int equalCharRepeat=60;
	Reversi r;
	std::string s,q;
	int i, j, l;
	size_t psize, ssize;
	clock_t begin;
	int threads;
	bool useStartIt=1;
	if(argc==1){
		threads = getNumberOfCores()-1;
	}
	else{
		i=1;
		if(!parseString(argv[i],j) || j<0 || j>getNumberOfCores()){
			printl("invalid argument",argv[i]);
			return 0;
		}
		threads=j;
		if(argc>2){
			useStartIt=0;
		}
	}

	aslovSetOutputWidth(60);
	Reversi::staticInit();
	Reversi::initFirst2Layers(type);
	preventThreadSleep();

	l=0;
	if(useStartIt){
		for(i=0;i<threads;i++){
			std::ifstream f(getOutFileName(i));
			if(f.is_open()){
				f>>j;
				if(j<l || i==0){
					l=j;
				}
			}
		}
	}
	const int start_it=l;
	/*
	 #define SEARCH_MOVES for both cases
	 to find first code moves sequence
	 Reversi::searchMoves(code11);

	 to find second code moves sequence
	 Reversi::searchMoves(code11,code16);
	 */

//	ReversiCode code16({0x8aaaa22aaa889aaaull, 0xaa8a82aaa202aaa8ull, 0xaa8aaaaa82aa2a82ull, 0xaaaaaaaaaaaaaaaaull, 0xaaaaaaaaull},white);
//	ReversiCode code11({0x8aaaa62aaaaaaaaaull, 0xaaaa82aaa242aaa9ull, 0xaa8aaaaa82aaaa82ull, 0xaaaaaaaaaaaaaaaaull, 0xaaaaaaaaull},white);
//	Reversi::searchMoves(code11);
//	Reversi::searchMoves(code11,code16);

//	ReversiCode code12({0x9aaaaaa6aaaaa9aaull, 0xaaa98aaaa66aaaa8ull, 0xaa8aaa9a82aaa642ull, 0xaaaaaaaaaaaaaaaaull, 0xaaaaaaaaull},black);
//	ReversiCode code16({0x8aaaa2a2aaaaa8aaull, 0xaaa88aaaa22aaaa8ull, 0x6aa2aa9a82aaa600ull, 0xaaaaaaaaaaaaa8aaull, 0xaaaaaaaaull},white);
//
//	r=code16;
//	printl(r,r.turns())
//
//	r=code12;
//	printl(r,r.turns())

	//Reversi::searchMoves(code12);
//	Reversi::searchMoves(code12,code16);


/*
	ReversiCodeSet set=proceedFile();
	printl(set.size())
	for(auto a:set){
		r=a;
		if(r.code()!=a){
			printei
		}
		if(r.turns()!=16){
			printel(r.turns())
		}
		printl(r.countBorderChips(),r.countBorderChips(black),'/',r.countChips(black),r.countBorderChips(white),'/',r.countChips(white))
		r.print();
	}
	return 0;
*/


	s=timeToString("%d%b%Y %H:%M:%S",true);

	s+=" REVERSI_CODE_MOVE_INSIDE=";
#ifdef REVERSI_CODE_MOVE_INSIDE
	s+="1";
#else
	s+="0";
#endif

	s+=" start="+toString(start_it,',');
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
			r=code;
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

	if(maxLayer1>=maxLayer){
		return 0;
	}
	int layer=i;
	s=std::string(equalCharRepeat,'=');
	printl(s);
	fflush(stdout);

	std::vector<std::thread> vt;

	begin = clock();
	ThreadData::init(&Reversi::layerSet[i-1],start_it);
	threadData.resize(threads);
	for (i = 0; i < threads; ++i) {
		threadData[i].index=i;
		vt.push_back(std::thread(threadf, i,layer));
	}

	for (auto& a : vt){
		a.join();
	}

	ReversiCodeSet a;
	for (i = layer; i <= maxLayer; i++) {
		s = format("%2d ", i);
		psize = 0;
		for (j = searchBWOnly?2:0; j < 3; j++) {
			a.clear();
			for (l = 0; l < threads; l++) {
				a.merge(threadData[l].foundEndCount[i - layer][j]);
			}
			psize += a.size();
			s += toString(a.size(), ',');
			if (!searchBWOnly) {
				s += j == 2 ? '=' : '+';
			}
		}
		if (!searchBWOnly) {
			s += toString(psize, ',');
		}
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
}

ReversiCodeSet proceedFile(){
	std::ifstream f("found.txt");
	std::string s,ps,q;
	//std::string color;
	size_t p,p1;
	uint64_t u=0;
	VUint64 v;
	ReversiCodeSet set;

	while(std::getline(f, s)){
		if(startsWith(ps, " turns16")){
			p=s.find('{');
			p1=s.find('}');
//			color=s.substr(p1+2,5);
			//			if(color!="black" && color!="white"){
			//				throw 0;
			//			}
			q=s.substr(p+1, p1-p-1);
//			printl(q,'!')
			v.clear();
			VString vs=split(q,", ");
			for(auto&a:vs){
				q=a.substr(2, a.length()-5);
				if(parseString(q,u,16)){
//					printl("%llx",u)
					v.push_back(u);
				}
				else{
					printei
					throw 0;
				}
			}
			set.insert(ReversiCode(v
					//since game over color can be any
#ifndef REVERSI_CODE_MOVE_INSIDE
			,black
#endif
					));
		}
		ps=s;
	}
	return set;
//	printl(joinV(threadOrder))

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
