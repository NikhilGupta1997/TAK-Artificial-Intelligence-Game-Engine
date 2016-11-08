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
	float diff = 5;
	float val1 = stof(argv[1]);
	float val2 = stof(argv[2]);
	int countme = stoi(argv[3]);
	int countyou = stoi(argv[4]);
	if(countme - countyou > 25) {
		if(val1 < val2) {
			val2 = val1;
			val1 -= diff*(countme - countyou)/25;
		}
		else {
			val2 = val1;
			val1 += diff*(countme - countyou)/25;
		}
	}
	else if(countyou - countme > 25){
		if(val1 < val2) {
			val1 = val2 + diff*(countyou - countme)/25;
		}
		else {
			val1 = val2 - diff*(countyou - countme)/25;
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