/// \file
/// \brief Main file for comparing time performance of in-place matrix transposition algorithms

#include <chrono>
#include <assert.h>
#include <functional>
#include "utilities.h"
#include "transpose.h"
#include "Matrix.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char* argv[])
{
    auto verbose = false;
    auto long_timing = false;
    if (argc > 1) {
        for(auto i = 0; i < argc; i++){
            if (string(argv[i]) == "-v" || string(argv[i]) == "--verbose") {
                verbose = true;
            }
            if (string(argv[i]) == "-l" || string(argv[i]) == "--long") {
                long_timing = true;
            }
        }
    }
    if (verbose) {printf("Maximum threads: %d\n", getNumThreadsEnvVar() );}
    printf("Timing algorithms...\n");

    // Overall system time elapsed
	clock_t start_time = clock();

    // Output setup
	string fileOutName = "timings.txt";
	ofstream output_file(fileOutName, ios::out | ios::trunc);
	if (!output_file.is_open()) { cerr << "Unable to open file:" << fileOutName << endl; return -1;}

    // Output column titles
	auto width = 20;
	output_file << setw(width) << left << "#N0=N1";
	output_file << setw(width) << left << "Serial";
	output_file << setw(width) << left << "OpenMP:Naive";
    output_file << setw(width) << left << "OpenMP:Diagonal";
	output_file << setw(width) << left << "OpenMP:Block";
    output_file << setw(width) << left << "PThreads:Naive";
    output_file << setw(width) << left << "PThreads:Diagonal";
	output_file << setw(width) << left << "PThreads:Block";
	output_file << endl;
    
    // MT algorithms to time:
    vector<function<void (Matrix)>> vector_of_transpose_functions;     // vector of pointers to functions
    vector_of_transpose_functions.push_back(transposeMatrixSerial);
    vector_of_transpose_functions.push_back(transposeMatrixNaiveOpenMP);
    vector_of_transpose_functions.push_back(transposeMatrixDiagonalOpenMP);
    vector_of_transpose_functions.push_back(transposeMatrixBlockOpenMP);
    vector_of_transpose_functions.push_back(transposeMatrixNaivePThread);
    vector_of_transpose_functions.push_back(transposeMatrixDiagonalPThread);
    vector_of_transpose_functions.push_back(transposeMatrixBlockPThread);

    // Matrices to use for timing
    // (N must be a power of two, and larger than the number of threads in OMP_NUM_THREADS)
    vector<int> sizes = {128, 1024, 2048, 4096};
    if (long_timing){
        sizes.clear();
        sizes = {64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
    }

    for (auto& N : sizes)
    {
        // N must be a power of two
        assert(isPowerOfTwo(N));                                        
        Matrix A(N);
        A.randomizeValues();
        string validationFile = "data.txt";
        writeMatrixToFile(validationFile, A);
        Matrix B(N);
        B = readMatrixfromFile(validationFile);

        output_file << setw(width) << left << N;
        if (verbose) { printf("\nN = %d: \n", N);}
        auto num_of_functions = vector_of_transpose_functions.size();

        for(auto i = 0; i < num_of_functions; i++){
            if (verbose){printf("Timing algorithm %d/%d... \n", i+1, num_of_functions);}
            steady_clock::time_point t1 = steady_clock::now();
            vector_of_transpose_functions[i](A);                            // perform the algorithm
            steady_clock::time_point t2 = steady_clock::now();
            auto time_taken = duration_cast<duration<double>>(t2 - t1);     // record the time delta
            transposeMatrixSerial(A);                                       // restore the original matrix
            assert(matricesAreEqual(A, B));                                 // confirm the algorithm works
            output_file << setw(width) << left << setprecision(7) << fixed << time_taken.count();
            
        }
        output_file << endl;
    }

	output_file.close();
    
    cout << endl;
    if (verbose) {
	    cout << "Executable Runtime: " << double(clock() - start_time) / (double)CLOCKS_PER_SEC;
        cout << " seconds" << endl;
    }
	cout << "Timing complete: to view run 'cat timings.txt'" << endl;

}