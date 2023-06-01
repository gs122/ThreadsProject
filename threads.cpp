// Filename: threads.cpp
// Compile command: gcc threads.cpp -o threads.exe -pthread 
// Run command: ./threads.exe instructionprj2.txt prose.txt

#include <iostream>
#include <sys/types.h>     
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <time.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <cstring>
using namespace std;

// Global variables
// File descriptors
int f1, f2, f3;	
// Instructions array
string instructions[20];
// Strings that we read
string readStrings[20];

// SIZE OF BUFFERS USED TO READ INSTRUCTIONS AND GIVE OUTPUT
const int outputSIZ = 150;
const int instructSIZ = 150;

// A function for converting strings to integers
int stringToInt (string inputString) {
	int result;
    result = 0;
    
    // Convert given string to a character array
    char* myArray = new char[inputString.size()+1];
    strcpy(myArray, inputString.c_str());
    
    // So long as we are currently pointing at a char that is a single digit number...
	// Add the digits one by one to the int we initialized as 0
    while(*myArray >= '0' && *myArray <= '9') {
        result = result*10 + (*myArray-'0');
        myArray++;
    }
    return result;
}


// Parameter class used for invoking pthread function.
	class MyParam {

		// Public variables, the bounds of each function
		public:
		  MyParam(int l, int u, int i) :   lb(l), ub(u), lid(i) { };
		  int getLower() { return lb;}
		  int getUpper() { return ub;}
		  int getLid()   { return lid;}
		
		// Private accessor variables
		private:
		  int lb, ub, lid;
	};
	
	// The function of each pthread
	void *myrunner (void *param) {
		
		MyParam *p = (MyParam *) param;
		
		// Parameters: lower bound, upper bound, thread ID
		   int lower = p->getLower();
		   int upper = p->getUpper();
		   int idx   = p->getLid();
		
		
		// Loop through 5 instructions per thread
		for (int a = lower; a < upper+1; a++) {
		
			// Store the instructions in an array and seekArgs
			string instruction = instructions[a];
			string seekArgs[4];
			int currArg = 0;
			
			// Put the instructions into seekArgs, disregarding commas
			for (int b = 0; b < instruction.length(); b++) {
				if (instruction[b] != ',') {
					seekArgs[currArg] += instruction[b];
				} else {
					currArg++;
				}
			}

			// The infoString holds information regarding our single thread output
			string infoString = "";
			
			// We need to change the idx and variables from ints to strings
			std::string idx_str = std::to_string(idx);
			std::string a_str = std::to_string(a);
			
			// Concatenate all of the instruction/thread related info for printing
			infoString = infoString + "Thread ID: " + idx_str + ", Instruction number: " 
			+ a_str + ", Executed instruction: "
			+ seekArgs[0] + ", " + seekArgs[1] + ", " + seekArgs[2] + "\n"; 

			// Each element in seekArgs represents i, j, and k
			int i = stringToInt(seekArgs[0]);
			int j = stringToInt(seekArgs[1]);
			int k = stringToInt(seekArgs[2]);
			
			// We perform the instruction, and store it in outputBuf
			int n;
			char outputBuf[outputSIZ];
			
			// We read 'k' characters with an offset of 'j'
			if (n = pread(f2, outputBuf, k, j) < 0) {
				cout << "Read failed";
				cout << "Errno: " << errno << ", Error String: " << strerror(errno) << "\n";
				pthread_exit(0);
			} 
		
			// Our outputBuf should end with a NULL character
			// Without it, the outputBuf may contain more than "k" characters
			outputBuf[k] = '\0';
			
			// We convert outputBuf to a string for output
			string outputString(outputBuf);
			infoString += outputString + "\n";
			
			// readStrings is an array holding all instructions
			readStrings[a] = outputString;
			cout << infoString;
			
		}
		
		// Exit the thread once everything's done
		pthread_exit(0);
	} 

int main(int argc, const char *argv[]) {

	// Checking for the correct number of arguments
	if (argc != 3){
		cout << "Wrong number of command line arguments\n";
		return 1;
	}
	
	// Declare file descriptors for:
	// Instructions file (instructions.txt)
	if ((f1 = open(argv[1], O_RDONLY, 0)) == -1){
		cout << "Can't open input file \n" ;
		return 2;
	} 
	
	// File to be read (prose.txt)
	if ((f2 = open(argv[2], O_RDONLY, 0)) == -1){
		cout << "Can't open input file \n" ;
		return 2;
	} 
	
	// Output file (output.txt)
	if ((f3 = open("output.txt", O_RDWR, 0)) == -1){
		cout << "Can't open input file \n" ;
		return 2;
	} 
	
	// Read the instructions and store them in instructBuf
	char instructBuf[instructSIZ];
	int n;
	while ((n = read(f1, instructBuf, instructSIZ)) > 0){
	}

	// 'x' is the position in the instructions array
	int x = 0; 
	
	// We store instructBuf in instructions[] array
	// Every time we get '\n' we change position w/ x++
	for (int i = 0; i < instructSIZ; i++) {
		if (instructBuf[i] != '\n') {
			instructions[x] += instructBuf[i];
		}
		else {
			x++;
		} 	
	}
	
	// Create the parameters for each of the 4 pthreads
	MyParam *p[4];
	p[0] = new MyParam(0, 4, 0);
	p[1] = new MyParam(5, 9, 1);
	p[2] = new MyParam(10, 14, 2);
	p[3] = new MyParam(15, 19, 3);
	
	
	// Create the pthreads
	pthread_t tid[4];
	pthread_attr_t attr[4];
	
	for (int i = 0; i < 4; ++i) {
	   pthread_attr_init(&(attr[i]));
	   pthread_create(&(tid[i]), &(attr[i]), myrunner, p[i]);
	}
	
	// Join pthreads
	for (int i = 0; i < 4; ++i) {
	   pthread_join(tid[i], NULL);
	}
	
	// finalString concatenates all the strings generated by pread
	string finalString = "";
	
	for (int i = 0; i < 20; i++) {
		finalString += readStrings[i];
	}
	
	// Print ending information and final concatenated string
	cout << "All threads completed actions." << endl;
	cout << "Strings: " << endl;
	for (int i = 0; i < 20; i++) {
		cout << readStrings[i] << endl;
	}
	cout << "Final concatenated string: " << finalString << endl;
	
	// Clean up the threads
	for (int i = 0; i < 4; ++i) delete p[i];
	
	// Convert finalString to character array
	char finalBuf[outputSIZ];
	strcpy(finalBuf, finalString.c_str());
	
	// Write the finalString to an output file, output.txt
	if (write(f3, finalBuf, outputSIZ) != outputSIZ)
	{
		cout << "Can't write file" ;
		close(f3);
		return 4;
	}
	

	return 0;
	
} 