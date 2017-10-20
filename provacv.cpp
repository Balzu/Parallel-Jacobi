#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <math.h>
#include <string.h>
#include <utility>
#include <tuple>
#include <functional>
#include <mutex>
#include <condition_variable>

std::mutex m;
std::condition_variable cv;
int curr_iter = -1;
int done = 0;
int max_iter = 100;
int pd = 2;
int n= 10;
int seed = 123;
int max = 16;
bool dd = true;


void init_rand_matrix(std::vector<std::vector<float>>& a, int n, int max, bool diag_dominant){
	if (diag_dominant == false){   // Build a completely random matrix
		for (int i=0; i<n ; i++){
			for (int j=0; j<n; j++)
				a[i][j] = ((float) (rand() % max));		
		}
	}
	else {  // Else build a random matrix, but diagonally dominant
		float sum;
		for (int i=0; i<n ; i++){
			sum = 0;
			for (int j=0; j<n; j++){
				a[i][j] = ((float) (rand() % max));
				sum += a[i][j];
			}
			a[i][i] = sum;		
		}		
	}	
}

void init_zero_vec(std::vector<float>& x, int n){
	for (int i=0; i<n; i++)
		x[i] = 0.0;		
}

void init_int_vec(std::vector<int>& x, int n, int value){
	for (int i=0; i<n; i++)
		x[i] = value;		
}

void init_bool_vec(std::vector<bool>& x, int n, bool value){
	for (int i=0; i<n; i++)
		x[i] = value;		
}

void init_rand_vec(std::vector<float>& b, int n, int max){
	for (int i=0; i<n; i++)
		b[i] =  ((float) (rand() % max));	
}

void print_matrix(std::vector<std::vector<float>>& a, int n){
	for (int i=0; i<n ; i++){
		for (int j=0; j<n; j++)
			std::cout << a[i][j] << " ";
		std::cout << std::endl;
	}
}


void print_vec(std::vector<float>& a, int n){
	for (int i=0; i<n ; i++)		
		std::cout << a[i] << " " ;
}

void worker_thread(int num, int n, std::vector<float>& x, std::vector<std::vector<float>>& a,
	std::vector<float>& b, std::vector<float>& x_new, std::vector<float>& x_diff, int start, int nrows)
{
    float acc;
    for(int i=0; i<max_iter; i++){
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&]{return (i==curr_iter);});    
        std::cout << "Worker " << num << " is doing its work for the " << i << " time \n";        
        [&] () {
            for(int i=start; i<start+nrows; i++){
	        acc = 0.0;
	        for (int j=0; j<n; j++){
		    if (i != j)
		        acc += a[i][j] * x[j];
	        }
	        x_new[i] = (1.0/a[i][i]) * (b[i] - acc);
	    }	
	} ();	       
    	done++;
    	lk.unlock();
   	cv.notify_all();
    }    
}

int main(){
    
    std::vector<std::vector<float>> a(n, std::vector<float>(n));
    init_rand_matrix(a, n, max, dd);
    std::vector<float> x(n);
    init_zero_vec(x,n);
    std::vector<float> b(n);
    init_rand_vec(b,n, max);
    std::vector<float> x_new(n);
    init_zero_vec(x_new,n);
    std::vector<float> x_diff(pd);
    init_zero_vec(x_diff,pd);
    int base = n/pd;
    int remainder = n - (base * pd);
    int start = 0 /*, done = 0*/;
    int rows[pd]; 
    std::vector<std::thread> Workers;
    for(int i=0; i<pd; i++){
        rows[i] = base;
	if (remainder != 0){  //load balance the remaining rows
	    rows[i]++;
	    remainder--;
	}
	Workers.push_back(std::thread(worker_thread, i, n, std::ref(x), std::ref(a), std::ref(b),  
			std::ref(x_new), std::ref(x_diff), start, rows[i]));
		start += rows[i];
	}
    int k = 0;
    while (k < max_iter){
        // wait for the worker          
        curr_iter = k;
        cv.notify_all();        
        std::unique_lock<std::mutex> lk(m); // creates the unique lock and LOCKS THE MUTEX
        //std::cout << " Main Holds the lock?  " << lk.owns_lock()  << std::endl;
        //std::cout << " done =  " << done << ", pd = " << pd  << std::endl;
        cv.wait(lk, [&]{return (done==pd);});  // wait should be done WHEN LOCK
        done = 0;
	    std::cout << "Main is doing its work for the " << k << " time\n";
	    k++;
	    x.swap(x_new);
	    lk.unlock();        
    }
    for (int i=0; i< pd; i++)
	Workers[i].join();
    std::cout << "Main is going to end.\n X = ";
    
    print_vec(x,n);
}
