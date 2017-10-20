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

 
void jacobi_thread(std::vector<std::vector<float>>& a, std::vector<float>& b,
	std::vector<float>& x, std::vector<float>& x_new, std::vector<float>& x_diff,
	std::vector<bool>& wait, std::vector<int>& iter, int i, int n, int start, int nrows, bool *finish/*,
        int *curr_iter, std::mutex& m, std::condition_variable& cv, std::mutex& m2,
        std::condition_variable& cv2, int *done*/){
		float acc;
		while (! *finish){
			std::unique_lock<std::mutex> lk(m,  std::defer_lock);
			cv.wait(lk, [&]{return (curr_iter == iter[i]);});
			if (lk.owns_lock()) lk.unlock(); 
			cv.notify_all();
			//wait[i] = true;
			std::cout << "Thread " << i << " is computing iter " << iter[i] << std::endl;
			// Compute X_new
			[&] () {for(int i=start; i<start+nrows; i++){
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
			iter[i] += 1;	
			//{
				
       				//std::lock_guard<std::mutex> lk(m);
       				m.lock();
       				std::cout << "Thread " << i << " has acquired lock on m " << iter[i] << std::endl;
       				done++;
       				m.unlock();
    		//	}		
    			cv.notify_all();  //TODO: should work with notify_one()
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
	bool finish = false;
	//std::mutex m, m2;
	//std::condition_variable cv, cv2;
	//int curr_iter = -1;
	
	for(int i=0; i<pd; i++){
		rows[i] = base;
		if (remainder != 0){  //load balance the remaining rows
			rows[i]++;
			remainder--;
		}
		Workers.push_back(std::thread(jacobi_thread, std::ref(a), std::ref(b), std::ref(x), 
			std::ref(x_new), std::ref(x_diff), std::ref(wait), std::ref(iter_v),
			 i, n, start, rows[i], &finish/*, &curr_iter,
			 std::ref(m), std::ref(cv), std::ref(m2), std::ref(cv2), &done*/));
		start += rows[i];
	}	
	float epsilon = 10.0; 
	int k = 0;
	//std::unique_lock<std::mutex> lk(m);
	while (k < iter && epsilon > tollerance){
		//init_bool_vec(wait, pd, false);
		// Tell threads to do an iteration
	//	{
			//std::lock_guard<std::mutex> lk(m);
			
			curr_iter = k;	
			std::cout << "Main acquired lock on m at iter  " << curr_iter << std::endl;
			
	//	}
		cv.notify_all();
		// Wait for all threads to conclude computation
		std::unique_lock<std::mutex> lk(m, std::defer_lock);
		cv.wait(lk, [&]{return (done == pd);}); //Think that after this line, LOCK IS NO MORE HELD
		// Now iteration ended, so done=0
		m.lock();
		done = 0;
		std::cout << "Main acquired lock on m after wait() at iter  " << curr_iter << std::endl;
		m.unlock();
		if (lk.owns_lock()) lk.unlock();
		
		/*do {
			done = 0;
			for (int i=0; i<pd; i++ )
				if(iter_v[i] == k) done++;
		} while (done < pd);
		*/
		k++;
		float max_diff= 0.0;
		for (int i=0; i< pd; i++)
			if(max_diff < x_diff[i]) max_diff = x_diff[i];
		if(epsilon > max_diff) epsilon = max_diff; 
		x.swap(x_new);	
	}
	curr_iter++;
	finish = true;
	//init_bool_vec(wait, pd, false);
	std::cout << "Main exited loop and is going to notify the others, at iter  " << curr_iter << std::endl;
	for (int i=0; i< pd; i++)
		std::cout << "The value of iter is:  " << iter_v[i] << std::endl;
	cv.notify_all();
	for (int i=0; i< pd; i++)
			Workers[i].join();
		
	print_vec(x,n);
	for (int i=0; i<pd; i++)
		std::cout << "\nNum iterations done = " << iter_v[i] << std::endl;
	std::cout << "Num iterations that should be done = " << iter << std::endl;
	


}








	
