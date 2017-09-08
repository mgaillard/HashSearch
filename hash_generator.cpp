#include <iostream>
#include <random>
#include <cstdlib>

using namespace std;

int main(int argc, char* argv[])
{
	ios_base::sync_with_stdio(false);
	cin.tie(0);
	
	if (argc != 2)
	{
		cout << "Invalid input args.\nFirst argument: number of hashes to generate." << endl;
		return 0;
	}
	
	int N = strtol(argv[1], NULL, 10);
	
	// Fixed seed for reproducibility.
	seed_seq fixed_seed{11, 16};
    mt19937_64 random_generator(fixed_seed);
    uniform_int_distribution<uint64_t> uniform_distribution;
    
    for (int i = 0; i < N; i++) {
		cout << uniform_distribution(random_generator) << "\n";
	}
	
	return 0;
}
