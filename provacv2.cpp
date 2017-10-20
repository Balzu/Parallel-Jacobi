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
//int pd = 2;

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


void worker_thread(int num,std::vector<std::vector<float>>& a, std::vector<float>& b,
	std::vector<float>& x, std::vector<float>& x_new, std::vector<float>& x_diff,
	std::vector<bool>& wait, std::vector<int>& iter, int i, int n, int start, int nrows)
{
    float acc;
    for(int i=0; i<max_iter; i++){
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&]{return (i==curr_iter);});    
        std::cout << "Worker " << num << " is doing its work for the " << i << " time\n";
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
	// Compute max difference: X_new - X, for current partition
	x_diff[i] = [&] () {
	    float max_diff = 0.0;  // Store the max difference btw corrispondent items btw iterations k and k-one
	    float abs_value;
	    for (int i=start; i<start+nrows; i++){
		abs_value = fabs(x_new[i] - x[i]);
		max_diff = (abs_value > max_diff) ? abs_value : max_diff; 
	    }
	    return max_diff;
	} ();
    	done++;
    	lk.unlock();
   	cv.notify_all();
    }    
}

int main(int argc, char *argv[]){
    if (argc < 2){
		std::cout << "Usage: " << argv[0] << " <matrix_dimension> <diag_dominant> <iterations> <tollerance> <sequential> <PAR_DEGREE>    \n" << std::endl;
		return -1;
	}
	int n = atoi(argv[1]);
	int pd = atoi(argv[6]);
	bool sequential = (strcmp(argv[5], "true") == 0) ? true : false; ; 
	bool dd = (strcmp(argv[2], "true") == 0) ? true : false; 
	int iter = atoi(argv[3]);  //TODO: can call this max_iter and call iter_v simply iter
	float tollerance = atof(argv[4]);
	int seed = 123;
	int max = 16;
	std::chrono::duration<double> time;
	std::vector<std::vector<float>> a(n, std::vector<float>(n));
	std::vector<float> x(n);
	std::vector<float> b(n);
	init_rand_matrix(a, n, max, dd);
	init_zero_vec(x,n);
	init_rand_vec(b,n, max);
	std::vector<float> x_new(n);
	init_zero_vec(x_new,n);
	std::vector<int> iter_v(pd);
	init_int_vec(iter_v,pd,0);
	std::vector<float> x_diff(n);
	init_zero_vec(x_diff,n);
	std::vector<bool> wait(pd);
	init_bool_vec(wait,pd,true);
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
		Workers.push_back(std::thread(worker_thread, iter, std::ref(a), std::ref(b), std::ref(x), 
			std::ref(x_new), std::ref(x_diff), std::ref(wait), std::ref(iter_v),
			 i, n, start, rows[i]/*, &curr_iter,
			 std::ref(m), std::ref(cv), std::ref(m2), std::ref(cv2), &done*/));
		start += rows[i];
	}
    int k = 0;
    while (k < max_iter){
        // wait for the worker        
        std::unique_lock<std::mutex> lk(m); // creates the unique lock and LOCKS THE MUTEX
        curr_iter = k;
        cv.wait(lk, [&]{return (done==pd);});  // wait should be done WHEN LOCK IS ACQUIRED
        done = 0;
	std::cout << "Main is doing its work for the " << k << " time\n";
	k++;
	lk.unlock();
   	cv.notify_all();	
        
    }
    for (int i=0; i< pd; i++)
	Workers[i].join();
    std::cout << "Main is going to end.\n";
}
