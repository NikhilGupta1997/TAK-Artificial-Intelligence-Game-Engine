#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <algorithm>
#include <stack>
#include <limits.h>
#include <time.h>
#include <queue>

using namespace std;

int main(int argc, char** argv) {
	srand(time(NULL));
	float diff = 0.2;
	float val1 = stof(argv[1]);
	float val2 = stof(argv[2]);
	int countme = stoi(argv[3]);
	int countyou = stoi(argv[4]);
	if(countme > countyou) {
		if(val1 < val2) {
			val2 = val1;
			val1 -= diff;
		}
		else {
			val2 = val1;
			val1 += diff;
		}
	}
	else if(countme < countyou){
		if(val1 < val2) {
			val1 = val2 + diff;
		}
		else {
			val1 = val2 - diff;
		}
	}
	else {
		val1 = val2 - (val1 - val2) / 2;
		val2 = val2;
	}
	cout<<"val1 "<<val1<<"\n";
	cout<<"val2 "<<val2;
	return 0;
}