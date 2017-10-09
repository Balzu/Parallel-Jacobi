#include <iostream>
#include <vector>
//#include <stdlib.h>
#include <math.h>

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

void init_zero_vec(std::vector<float>& x, int n, int max){
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
		std::cout << "\nIteration " << k << std::endl;
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
		print_vec(x,n);
		float max_diff = 0.0;  // Store the max difference btw corrispondent items btw iterations k and k-one
		for (int i=0; i<n; i++)
			max_diff = (fabs(x_new[i] - x[i]) > max_diff) ? fabs(x_new[i] - x[i]) : max_diff;  //TODO: abs() computed twice, can compute only once instead
		if (epsilon > max_diff)
			epsilon = max_diff;
		}
}

int main(int argc, char *argv[]){
	if (argc < 2){
		std::cout << "Usage: " << argv[0] << " <matrix_dimension> <PAR_DEGREE> <diag_dominant> <iterations> \n" << std::endl;
		return -1;
	}
	int n = atoi(argv[1]);
	int pd = atoi(argv[2]);
	int seed = 123;
	int max = 16;
	std::vector<std::vector<float>> a(n, std::vector<float>(n));
	std::vector<float> x(n);
	std::vector<float> b(n);
	init_rand_matrix(a, n, max, true);
	init_zero_vec(x,n,max);
	init_rand_vec(b,n,max);
	std::cout << "Printing A: "  << std::endl;
	print_matrix(a,n);
	std::cout << "Printing X: "  << std::endl;
	print_vec(x,n);
	std::cout << "\nPrinting B: "  << std::endl;
	print_vec(b,n);
	jacobi(a,b,x,n,100,0.001);
	std::cout << "\n\nAfter computing Jacobi:\n" << std::endl;
	std::cout << "Printing X:\n "  << std::endl;
	print_vec(x,n);

	
	
	
}
