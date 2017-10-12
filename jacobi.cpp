#include <iostream>
#include <vector>
#include <string.h>
#include <math.h>
#include <ff/farm.hpp>
#include <chrono>
using namespace ff;

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
	while (k < max_iter && epsilon > tollerance){
		//std::cout << "\nIteration " << k << std::endl;
		for(int i=0; i< n; i++){
			acc = 0;
			for (int j=0; j<n; j++){
				if (i != j)
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
	float *svc(float * task){
		int channel = lb -> get_channel_id();		
		if (channel >= 0){
			//std::cout << "Task received from worker " << channel << std::endl;
			const float& partial_diff =  *task;
			//std:: cout << partial_diff << std::endl;
			received++;
			if (partial_diff > max_diff) max_diff=partial_diff;
			if (received == pd){  // One iteration is ended
				//print_vec(x,n);
				//print_vec(x_new,n);
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
			}
		}
		else {
			std::cout << "Task received from channel" << channel << std::endl;
			lb -> broadcast_task(GO_ON); //TODO: GO_ON is not propagated, but if inside this method it is?
			return GO_ON; //TODO: ok or useless this line?		
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
	float *svc(float *){					
		for(int i=start; i<start+nrows; i++){
			acc = 0;
			for (int j=0; j<n; j++){
				if (i != j)
					acc += a[i][j] * x[j];
			}
			x_new[i] = (1.0/a[i][i]) * (b[i] - acc);
		}		
		return (new float (compute_difference()));
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


void display_result(std::vector<float>& x, int n, bool sequential, std::chrono::duration<double>& time){
	std::cout << "\n\nAfter computing Jacobi with";
	if (sequential)
		std::cout << "out parellization, vector X =" << std::endl;
	else
		std::cout << " parellization, vector X =" << std::endl;
	print_vec(x,n);
	std::cout << "\nComputed in " << time.count() << "s" << std::endl;
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
	int iter = atoi(argv[3]);
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
	/*
	std::cout << "Printing A: "  << std::endl;
	print_matrix(a,n);
	std::cout << "Printing X: "  << std::endl;
	print_vec(x,n);
	std::cout << "\nPrinting B: "  << std::endl;
	print_vec(b,n);	  
	std::cout << std::endl;
	*/
	if (sequential){ 
		auto start_t = std::chrono::system_clock::now();	
		jacobi(a,b,x,n,iter,tollerance);
		auto end = std::chrono::system_clock::now();
		time = end-start_t;
	}
	else {
		auto start_t = std::chrono::system_clock::now();
		std::vector<float> x_new(n);
		init_zero_vec(x_new,n);
		int base = n/pd;
		int remainder = n - (base * pd);
		int start = 0;
		int rows[pd]; 
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
		Emitter E(farm.getlb(), iter, tollerance, x, x_new, pd, n);
		farm.add_emitter(E);
		farm.remove_collector();
		farm.wrap_around();
		if (farm.run_and_wait_end() < 0) return -1;
		auto end = std::chrono::system_clock::now();
		time = end-start_t;			
	}
	display_result(x, n, sequential, time);
	return 0;	
	
}
