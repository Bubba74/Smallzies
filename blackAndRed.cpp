
#include <cstring>
#include <string>
#include <stdlib.h>
#include <chrono>
	int64_t time();
#include <iostream>
	using namespace std;

#define RECT_CACHE 0
#define RECURSION  1
#define LINE_CACHE 2

#define METHOD LINE_CACHE


#if METHOD == LINE_CACHE
int SIZE = 12000;
int REDS = SIZE;
int BLACKS = SIZE;
#else
const int SIZE = 12000;
const int REDS = SIZE;
const int BLACKS = SIZE;
#endif
const bool debug = false;

const char GREEN[20] = "\033[1;32m";
const char RED[20] = "\033[1;31m";
const char RESET[10] = "\033[0m";

enum Output {ALL, GRID, TIME, PROFIT};
bool outputs[4] = {false, false, false, false};

bool print = false;
bool print_nums = true;


#if METHOD == RECT_CACHE || METHOD == RECURSION
int64_t cache_start = time();
double *cache[REDS+1];
int64_t cache_end = time();

void clear_cache(){
	int64_t clear_start = time();
	for (int i=0; i<REDS+1; i++){
		cache[i] = (double *) malloc( sizeof(double)*(BLACKS+1) );
		for (int j=0; j<BLACKS+1; j++)
			cache[i][j] = -1;
	}
	int64_t clear_end = time();
	int64_t dur = (cache_end-cache_start)+(clear_end-clear_start);
	cout << "Generated cache took: " << dur << "ms" << endl;
}

double cache_val(int r, int b, double val){
	cache[r][b] = val;
	return val;
}

double ev(int r,int b){
	if (cache[r][b] != -1) return cache[r][b];

	if (debug)
	if (r + b == 34) //Have picked 10 cards so far
		cout << "Red: " << r << "\tBlack: " << b << endl;

	if(r==0)
		return cache_val(r,b,0);
	if(b==0)
		return cache_val(r,b,r);

	double ep=0;
	ep+=(ev(r-1,b)+1)*((0.0+r)/(r+b));
	ep+=(ev(r,b-1)-1)*((0.0+b)/(r+b));

	if(ep<0)
		return cache_val(r,b,0);
	return cache_val(r,b,ep);
}

double fill(int r,int b){
        if(r==0)
                return cache_val(r,b,0);
        if(b==0)
                return cache_val(r,b,r);

        double ep=0;
        ep += ( cache[r-1][b] +1) * ((0.0+r)/(r+b));
        ep += ( cache[r][b-1] -1) * ((0.0+b)/(r+b));

        if(ep<0)
                return cache_val(r,b,0);
        return cache_val(r,b,ep);
}

#elif METHOD == LINE_CACHE
double *cache[2] = {nullptr, nullptr};

int max(int a, int b){
	return a>b?a:b;
}
int min(int a, int b){
	return a<b?a:b;
}

double scanto(int red, int black){
	int r,b;
	int layers = red+black+1;
	int xoff = 0, length = 0;

	for (int i=0; i<layers; i++){
		if (outputs[Output::ALL])
			if (i % 1000  == 0) cout << "Reached layer: " << i << endl;

		//Free first layer cache if it exists
		if (cache[0] != nullptr){
			free(cache[0]);
			cache[0] = nullptr;
		}
		//Shift second layer cache into first layer
		cache[0] = cache[1];


		//If the right bound exceeds B, decrease length by 1
		if (length + xoff > black)
			length--;

		//If the bottom bound exceeds red, increase xoff by one and decrease length by 1
		if (i - xoff > red){
			xoff++;
			length--;
		}
		
		cache[1] = (double *) malloc( (length+1) * sizeof(double) );

		for (int j=0; j<length+1; j++){
			r = i-xoff-j;	b = xoff+j;
			if(r==0){
				cache[1][j] = 0;
				continue;
        		} 
			if(b==0){
				cache[1][j] = r;
				continue;
			}

        		double ep=0;
        		ep += ( cache[0][j+(xoff>0?1:0)]   + 1) * ((0.0+r)/(r+b));
        		ep += ( cache[0][j+(xoff>0?1:0)-1] - 1) * ((0.0+b)/(r+b));
	
        		if(ep<0){
				cache[1][j] = 0;
				continue;
			}
			cache[1][j] = ep;
		}

		if (print && outputs[Output::ALL]){
			for (int i=0; i<xoff; i++)
				printf("%6s ", "");
			for (int i=0; i<length+1; i++)
				printf("%6.2f ", cache[1][i]);
			cout << endl;
		}

		//If the layer still starts at the bottom left, increase the length 1
		if (xoff == 0)
			length++;
		else //Otherwise increase xoff to keep right bound
			xoff++;
	}

	return cache[1][0];
}//scanto
#endif


void fill_grid(){
#if METHOD == RECURSION
	cout << "Solving for [" << REDS << "," << BLACKS << "] recursively" << endl << flush;
	cout << endl << ev(REDS,BLACKS) << endl;
#elif METHOD == RECT_CACHE
	cout << "Solving for [" << REDS << "," << BLACKS << "] progressively" << endl << flush;
	for (int i=0; i<REDS+1; i++)
		for (int j=0; j<BLACKS+1; j++)
			fill(i,j);
	cout << endl << cache[REDS][BLACKS] << endl;
#elif METHOD == LINE_CACHE
	if (outputs[Output::ALL])
		cout << "Solving for [" << REDS << "," << BLACKS << "] scan-progressively" << endl << flush;

	double profit = scanto(REDS, BLACKS);

	if (outputs[Output::ALL])
		cout << "Expected profit: " << profit << endl;

	if (outputs[Output::PROFIT])
		cout << profit << " ";
#endif
}//fill_grid

int main(int argc, char *argv[]){
	if (argc > 1){
#if METHOD != LINE_CACHE
		cout << "Can only specify command-line deck size if process is a linear cache" << endl;
#else
		int size = stoi(argv[1]);
		SIZE = size;
		BLACKS = size;
		REDS = size;
#endif
		if (argc > 2){
			if (strcmp(argv[2], "all") == 0)
				outputs[Output::ALL] = true;
			else if (strcmp(argv[2], "time") == 0)
				outputs[Output::TIME] = true;
			else if (strcmp(argv[2], "profit") == 0)
				outputs[Output::PROFIT] = true;
			else if (strcmp(argv[2], "sum") == 0){
				outputs[Output::GRID] = true;
				outputs[Output::TIME] = true;
				outputs[Output::PROFIT] = true;
			}
			else
				outputs[Output::ALL] = true;

		} else
			outputs[Output::ALL] = true;
	} else outputs[Output::ALL] = true;


#if METHOD == RECT_CACHE || METHOD == RECURSION
	clear_cache();

	int64_t start = time();
	fill_grid();
	int64_t end = time();

	cout << "Process took: " << (end-start) << "ms" << endl;

	if (print)
	for (int i=0; i<REDS+1; i++){
		for (int j=0; j<BLACKS+1; j++){
			if (cache[i][j] > 0){
				cout << GREEN;
				// << cache[i][j] << "   ";
				if (print_nums) printf("%7.2f", cache[i][j]);
				else cout << "X";
			} else {
				cout << RED;
				// << cache[i][j] << "   ";
				if (print_nums) printf("%7.2f", cache[i][j]);
				else cout << "X";
			}
		}
		cout << endl;
	}
	cout << RESET;

	for (int i=0; i<BLACKS+1; i++)
		if (cache[REDS][i] == 0){
			cout << "Do not play if the probability of getting a red is: " << (REDS/(REDS+i+0.0)) << endl;
			break;
		}

#elif METHOD == LINE_CACHE
	if (outputs[Output::GRID])
		cout << REDS << " x " << BLACKS << " ";

	int64_t start = time();
	fill_grid();
	int64_t end = time();

	if (outputs[Output::ALL])
		cout << "Process took: " << (end-start) << "ms" << endl;
	else if (outputs[Output::TIME])
		cout << (end-start);

	for (int i=0; i<4; i++)
		if (outputs[i]) {
			cout << endl;
			break;
		}
#endif
	
}

int64_t time(){
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

// Statistics
// complexity:	O(N^2) -- Must generate rectange of reds*blacks (where reds+blacks == N)
// memory:	O(N)   -- Must generate 2 layers of appr. min(reds,blacks) where max(min(reds,blacks)) is 0.5N
//
// Expected output of original 26r+26b deck is 2.62448
//
// Finding expected output of deck with 120,000 red cards and 120,000 black cards is 180.832
// 	The original scan-processing method of using the previous pascal-layer to calculate the next layer took 788003ms
// 	The optimized scan-processing method of confining each layer within the red*black rectangle took 441979ms






