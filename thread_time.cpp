#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <utility>
#include <tuple>

using namespace std;

void loop(){
	
	    
}


int main(int argc, char * argv[]){
	if (argc == 1){
		cout << "Usage is: " << argv[0] << "nt " << endl;
		return (-1);
	}
	int nt = atoi (argv[1]);
	vector <thread> tvec;
	auto start = std::chrono::system_clock::now();
	for (int i=0; i< nt; i++)
	    tvec.push_back(thread(loop));
	for (int i=0; i< nt; i++)
		tvec[i].join();
	auto end = std::chrono::system_clock::now();
	chrono::duration<double> time = end-start;
	std::cout << "Time to setup " << nt << "threads = " << time.count() << " s\n";
	std::cout << "Avg time to setup one thread " << ((double)time.count())/((double)nt) << " s\n";
	return 0;
}
