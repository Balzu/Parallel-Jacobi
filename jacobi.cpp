#include <iostream>
#include <vector>
#include <string.h>
#include <math.h>
#include <ff/farm.hpp>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
using namespace ff;

std::mutex m;
std::condition_variable cv;
int curr_iter = -1;
int done = 0;
int max_iter;
int pd;
int n;
int seed = 123;
int max = 16;
bool dd;
float tollerance;

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


void jacobi(std::vector<std::vector<float>>& a, std::vector<float>& b,
	std::vector<float>& x, int n, int max_iter, float tollerance){
	int k = 0;
	float epsilon = a[0][0];  // initialization of epsilon is arbitrary 
	float acc;
	std::vector<float> x_new(n);
	auto start_t = std::chrono::system_clock::now();
	auto end_t = std::chrono::system_clock::now();
	std::chrono::duration<double> time;
	while (k < max_iter && epsilon > tollerance){
		//std::cout << "\nIteration " << k << std::endl;
		start_t = std::chrono::system_clock::now();
		for(int i=0; i< n; i++){
			acc = 0;
			for (int j=0; j<n; j++){
				if (i != j) //TODO: maybe this if prevents use of vectorization
					acc += a[i][j] * x[j];
			}
			x_new[i] = (1.0/a[i][i]) * (b[i] - acc);
		}
		k++;
//x contains the elements previously assigned to x_new. x_new instead contains the elements previously assigned to x,
//but this does not matter, since they will be overwritten. The cost of swap() is constant :).
		x.swap(x_new);
		//print_vec(x,n);
		float max_diff = 0.0;  // Store the max difference btw corrispondent items btw iterations k and k-one
		float abs_value;
		for (int i=0; i<n; i++){
			abs_value = fabs(x_new[i] - x[i]);
			if(abs_value > max_diff) max_diff = abs_value;   // max_diff must take the maximum 
		}
		if (epsilon > max_diff) epsilon = max_diff;  // epsilon must take the minimum
		end_t = std::chrono::system_clock::now();
		time = end_t - start_t;
		std::cout << "\nIter " << k  << " Computed in " << time.count() << "s" <<  std::endl;
	}
}



struct Emitter: ff_node_t<float> {
	Emitter( ff_loadbalancer *const lb,  int max_iter, float tollerance,
		std::vector<float>& x, std::vector<float>& x_new, int pd, int n):
		lb(lb),max_iter(max_iter), tollerance(tollerance), pd(pd), x(x), x_new(x_new), n(n) {}
	ff_loadbalancer *const lb;
	int max_iter, pd, n;
	int k = 0, received = 0;
	float tollerance;  	
	std::vector<float>& x;
	std::vector<float>& x_new;
	// max_diff holds the maximum difference btw 2 corresponding items (x[i] and x_new[i]) at a given iteration	
	float epsilon = 10.0, max_diff= 0.0;  // initialization of epsilon is arbitrary 
	std::chrono::time_point<std::chrono::system_clock> start_t, end_t;
	std::chrono::duration<double> time;
	float *svc(float * task){
		int channel = lb -> get_channel_id();		
		if (channel >= 0){
			//std::cout << "Task received from worker " << channel << std::endl;
			if (received == 0) start_t = std::chrono::system_clock::now();
			const float& partial_diff =  *task;
			//std:: cout << partial_diff << std::endl;
			received++;
			if (partial_diff > max_diff) max_diff=partial_diff;
			if (received == pd){  // One iteration is ended
				//print_vec(x,n);
				//print_vec(x_new,n);
				end_t = std::chrono::system_clock::now();
				//time = end_t - start_t;
				//std::cout << "\nIter p" << k  << " Computed in " << time.count() << "s" <<  std::endl;
				received = 0;
				if (epsilon > max_diff) epsilon = max_diff;
				max_diff = 0.0;
				k++;
				x.swap(x_new);
				//std::cout << "\nIteration " << k << std::endl;
				//print_vec(x,n);
				if (k < max_iter && epsilon > tollerance)
					lb -> broadcast_task((float *) GO_ON);
				else
					lb -> broadcast_task((float *) EOS);
				time = end_t - start_t;
				std::cout << "\nIter p" << k  << " Computed in " << time.count() << "s" <<  std::endl;
			}
		}
		else {
			std::cout << "Task received from channel" << channel << std::endl;
			start_t = std::chrono::system_clock::now();
			lb -> broadcast_task(GO_ON); //TODO: GO_ON is not propagated, but if inside this method it is?	
			end_t = std::chrono::system_clock::now();	
			time = end_t - start_t;
			std::cout << "\nTime to broadcast tasks: "  << time.count() << "s" <<  std::endl;
		}
		return GO_ON;
	}
	
	
	
};

struct Worker: ff_node_t<float> {
	Worker(std::vector<std::vector<float>>& a, std::vector<float>& b, std::vector<float>& x, 
		std::vector<float>& x_new,int n, int start, int nrows):a(a),b(b),x(x),start(start),nrows(nrows),n(n),x_new(x_new) {}
	std::vector<std::vector<float>>& a;
	std::vector<float>& b;
	std::vector<float>& x;
	std::vector<float>& x_new;
	int start, nrows, n;
	float acc = 0;
	std::chrono::time_point<std::chrono::system_clock> start_t, end_t;
	std::chrono::duration<double> time;
	float *svc(float *){	
		start_t = std::chrono::system_clock::now();				
		for(int i=start; i<start+nrows; i++){
			acc = 0;
			for (int j=0; j<n; j++){
				if (i != j)
					acc += a[i][j] * x[j];
			}
			x_new[i] = (1.0/a[i][i]) * (b[i] - acc);
		}	
		auto start_cd = std::chrono::system_clock::now();
		float result = compute_difference();
		
		end_t = std::chrono::system_clock::now();
		time = end_t - start_t;
		std::cout << "\nTime to compute for Worker " << get_my_id() << " : "  << time.count() << "s" <<  std::endl;
		return (new float(result));	
		//return (new float (compute_difference()));  TODO: reinsert this return, I commented it just because I am measuring times
	}
	
	float compute_difference(){
		float max_diff = 0.0;  // Store the max difference btw corrispondent items btw iterations k and k-one
		float abs_value;
		for (int i=start; i<start+nrows; i++){
			abs_value = fabs(x_new[i] - x[i]);
			max_diff = (abs_value > max_diff) ? abs_value : max_diff; 
		}
		return max_diff;
	}
};

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


void display_result(std::vector<float>& x, int n, std::string parallelization, std::chrono::duration<double>& time){
	std::cout << "\n\nAfter computing Jacobi with";
	if (parallelization.compare("sequential") == 0 )
		std::cout << "out parellization, \n" << "vector X = { ";
	else if (parallelization.compare("fastflow") == 0)
		std::cout << " fastflow, \n" << "vector X = { ";
	else if (parallelization.compare("pthread") == 0)
		std::cout << " pthread, \n" << "vector X = { ";	
	print_vec(x,n);
	std::cout << " }" << std::endl;
	std::cout << "\nComputed in " << time.count() << "s" << std::endl;
}

int main(int argc, char *argv[]){
	if (argc < 2){
		std::cout << "Usage: " << argv[0] << " <matrix_dimension> <diag_dominant> <iterations> <tollerance> <parallelization> <PAR_DEGREE>    \n" << std::endl;
		return -1;
	}
	n = atoi(argv[1]);
	pd = atoi(argv[6]);
	const std::string parallelization =argv[5]; 
	dd = (strcmp(argv[2], "true") == 0) ? true : false; 
	max_iter = atoi(argv[3]);
	tollerance = atof(argv[4]);
	std::chrono::duration<double> time;
	std::vector<std::vector<float>> a(n, std::vector<float>(n));
	std::vector<float> x(n);
	std::vector<float> b(n);
	init_rand_matrix(a, n, max, dd);
	init_zero_vec(x,n);
	init_rand_vec(b,n, max);
	/*
	std::cout << "Printing A: "  << std::endl;
	print_matrix(a,n);
	std::cout << "Printing X: "  << std::endl;
	print_vec(x,n);
	std::cout << "\nPrinting B: "  << std::endl;
	print_vec(b,n);	  
	std::cout << std::endl;
	*/
	if (parallelization.compare("sequential") == 0){ 
	   
	    auto start_t = std::chrono::system_clock::now();	
	    jacobi(a,b,x,n,max_iter,tollerance);
	    auto end = std::chrono::system_clock::now();
	    time = end-start_t;
	}
	else 
	{			
		auto start_t = std::chrono::system_clock::now();
		std::vector<float> x_new(n);
		init_zero_vec(x_new,n);
		int base = n/pd;
		int remainder = n - (base * pd);
		int start = 0;
		int rows[pd]; 
		auto start_tp = std::chrono::system_clock::now();
		
		if (parallelization.compare("fastflow") == 0 )
		{
			std::vector<std::unique_ptr<ff_node>> Workers;
		    for(int i=0; i<pd; i++){
				rows[i] = base;
				if (remainder != 0){  //load balance the remaining rows
					rows[i]++;
					remainder--;
				}
				Workers.push_back(make_unique<Worker>(a,b,x,x_new,n,start,rows[i]));
				start += rows[i];
			}
			ff_Farm<> farm(std::move(Workers));
			Emitter E(farm.getlb(), max_iter, tollerance, x, x_new, pd, n);
			farm.add_emitter(E);
			farm.remove_collector();
			farm.wrap_around();
			auto end_tp = std::chrono::system_clock::now();
			time = end_tp-start_tp;			
			std::cout << "\nTime to setup parallel activities: " << time.count() << "s" << std::endl;
			if (farm.run_and_wait_end() < 0) return -1;
			auto end = std::chrono::system_clock::now();
			time = end-start_t;	
		}
		
		if (parallelization.compare("pthread") == 0 )
		{
		    std::vector<float> x_diff(pd);
   			init_zero_vec(x_diff,pd);
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
     		    cv.wait(lk, [&]{return (done==pd);});  // wait should be done WHEN LOCK
        		done = 0;
	    		std::cout << "Main is doing its work for the " << k << " time\n";
	    		k++;
	    		x.swap(x_new);
	    		lk.unlock();        
   			}
   			for (int i=0; i< pd; i++)
				Workers[i].join();
    		std::cout << "Main is going to end.\n";
    		auto end = std::chrono::system_clock::now();
			time = end-start_t;	
		}
				
	}
	display_result(x, n, parallelization, time);
	return 0;	
	
}
