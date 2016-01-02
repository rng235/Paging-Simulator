/************************************************************************
* Paging Lab 4 -- Operating Systems
* Author    : Ricardo N. Guntur <ricardo.guntur@nyu.edu>
* Semester	: Fall 2015
*
* Program Description:
* Simulate demand paging and see how the number of page
* faults depends on page size, program size, replacement algorithm, and 
* job mix.
* 
* The arguments:
* M, the machine size in words
* P, the page size in words
* S, the process size, i.e, the references are to virtual addresses 0...S-1
* J, the "job mix", which determines A, B, and C, as described below
* N, the number of references for each process
* R, the replacement algorithm, LIFO, RANDOM, or LRU
/************************************************************************/
#include "pager.h"

pager pager;

/************************ Functions ***************************/

void print_input();

void handleJobMix();

void frameTable();

void handleReferences();

void nextReference(int process);

void checkPageFault(int process);

void increment_TimeTable();

void print_results();

/************************** Global Variables ****************************/

int programTime;

// USER ARGUMENTS
int machineSize;            // Machine size
int pageSize;               // Page size
int processSize;            // Process size
int jobMix;                 // Determines A, B, and C
int numReferences;          // # of references for each process
string algo;                // Replacement algorithm, LIFO, RANDOM, or LRU

// JOB MIX VARIABLES
int numProcesses;           // Total Number of processes
double probabilities[4][4]; // Handles probabilities for case 4
ifstream randNumGenerator;  // Random number generator

// PAGING VARIABLES
int *pagingTable;           // The frame table for paging
int *ownerTable;            // stores the owners of a frame in the paging table
int *timeTable;             // stores the time of a frames residency
int numFrames;              // # of frames system can hold
int freeFrames;             // # of free frames

int residencyTime[4];       // Time a process had a page
int pageFaults[4];          // # of page faults for process i
int pageEvictions[4];       // # of page evictions for process i

// REPLACEMENT ALGORITHM VARIABLES
int *TimeSinceLastRef;

// REFERENCING VARIABLES
int Reference[4];           // The address referenced

//DEBUG BOOL
bool debug;

//-----------------------------------------------------------------------

/*-----------------------------------------------------------------------
 * Function Name : Main
 * Driver function of the program.  It will take in
 * 7 arguments; <program-name> <M> <P> <S> <J> <N> <R> <D>
 * Invocation defined above
-----------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
    // Local constants
    const int args_needed = 8;
    const string random_numFile = "random-numbers.txt";
    const int QUANTUM = 3;  //Program uses quantums of 3 as per instructions

    // Local variables
    istringstream strStream;    // Used to convert str -> int
    // could've also used atoi (y)


    // check if program had right number of arguments passed
    if (argc == args_needed) {

        // Initialize variables while converting to int
        strStream.str(argv[1]);
        strStream >> machineSize;
        strStream.clear();
        strStream.str(argv[2]);
        strStream >> pageSize;
        strStream.clear();
        strStream.str(argv[3]);
        strStream >> processSize;
        strStream.clear();
        strStream.str(argv[4]);
        strStream >> jobMix;
        strStream.clear();
        strStream.str(argv[5]);
        strStream >> numReferences;
        strStream.clear();
        strStream.str(argv[6]);
        strStream >> algo;
        strStream.clear();
        strStream.str(argv[7]);
        strStream >> debug;

        programTime = 1;

        // Prints out arguments as per lab instructions
        print_input();

        // Initialize job mix
        handleJobMix();

        // Initialize frame table
        frameTable();

        // Initialize first references of each process
        handleReferences();

        // Random number generator
        randNumGenerator.open(random_numFile.c_str());

        // Loop for number of refs / quantum
        for (int i = 0; i < numReferences / QUANTUM; i++) {
            // for each process
            for (int j = 0; j < numProcesses; j++) {
                // for quantum of 3
                for (int q = 0; q < QUANTUM; q++) {
                    // Simulate reference for this process
                    checkPageFault(j);

                    // Get next reference for this process
                    nextReference(j);

                    // Increment program time
                    increment_TimeTable();
                    programTime++;
                }
            }
        }

        // Loop through all processes
        for (int j = 0; j < numProcesses; j++) {
            for (int i = 0; i < numReferences % QUANTUM; i++) {
                // Simulate reference for this process
                checkPageFault(j);

                // Get next reference for this process
                nextReference(j);

                // Increment program time
                increment_TimeTable();
                programTime++;
            }
        }


        // Print out results
        print_results();
    }

        //If program was invoked incorrectly, print the right way to do it
    else {
        // Print out correct program invocation
        cout << "You passed the wrong things meng" << endl;
        cout << "Usage: PAGING <M> <P> <S> <J> <N> <R> <D>\n";
        cout << "    M - Machine size\n"
        << "    P - Page size\n"
        << "    S - Process size\n"
        << "    J - Job mix\n"
        << "    N - Number of references\n"
        << "    R - Replacement algorithm (lru,lifo,random)\n"
        << "    D - Debug (0,1)" << endl;
    }

}


/*-----------------------------------------------------------------------
* Function Name : Print Arguments
* Prints user inputs
-----------------------------------------------------------------------*/
void print_input() {
    // Print arguments
    cout << "The machine size is " << machineSize << ".\n"
    << "The page size is " << pageSize << ".\n"
    << "The process size is " << processSize << ".\n"
    << "The job mix number is " << jobMix << ".\n"
    << "The number of references per process is " << numReferences << ".\n"
    << "The replacement algorithm is " << algo << ".\n\n";

}


/*-----------------------------------------------------------------------
 * Function Name : Initialize job mix
 * The job mix input determines the set of probability values used
-----------------------------------------------------------------------*/
void handleJobMix() {
    // Local constants
    const int a = 0;
    const int b = 1;
    const int c = 2;
    const int d = 3;


    // Case based on job mix argument passed in
    switch (jobMix) {
        case 1:    //  1 process sequentially
        {
            // Set probabilities and num of processes
            probabilities[0][a] = 1;
            probabilities[0][b] = 0;
            probabilities[0][c] = 0;
            probabilities[0][d] = 1 - (probabilities[0][a] + probabilities[0][b] + probabilities[0][c]);
            numProcesses = 1;
            break;
        }

        case 2:    // 4 processes sequentially
        {
            // Set probabilities and num of processes
            probabilities[0][a] = probabilities[1][a] = probabilities[2][a] = probabilities[3][a] = 1;
            probabilities[0][b] = probabilities[1][b] = probabilities[2][b] = probabilities[3][b] = 0;
            probabilities[0][c] = probabilities[1][c] = probabilities[2][c] = probabilities[3][c] = 0;
            probabilities[0][d] = probabilities[1][d] = probabilities[2][d] = probabilities[3][d] =
                    1 - (probabilities[0][a] + probabilities[0][b] + probabilities[0][c]);
            numProcesses = 4;
            break;
        }

        case 3:    // 4 processes fully random
        {
            // Set probabilities and num of processes
            probabilities[0][a] = probabilities[1][a] = probabilities[2][a] = probabilities[3][a] = 0;
            probabilities[0][b] = probabilities[1][b] = probabilities[2][b] = probabilities[3][b] = 0;
            probabilities[0][c] = probabilities[1][c] = probabilities[2][c] = probabilities[3][c] = 0;
            probabilities[0][d] = probabilities[1][d] = probabilities[2][d] = probabilities[3][d] =
                    1 - (probabilities[0][a] + probabilities[0][b] + probabilities[0][c]);
            numProcesses = 4;
            break;
        }

        case 4:    // 4 processes with annoying accesses below
        {
            // Set probabilities and num of processes
            // Process 1
            probabilities[0][a] = 0.75;
            probabilities[0][b] = 0.25;
            probabilities[0][c] = 0;
            probabilities[0][d] = 1 - (probabilities[0][a] + probabilities[0][b] + probabilities[0][c]);

            // Process 2
            probabilities[1][a] = 0.75;
            probabilities[1][b] = 0;
            probabilities[1][c] = 0.25;
            probabilities[1][d] = 1 - (probabilities[1][a] + probabilities[1][b] + probabilities[1][c]);

            // Process 3
            probabilities[2][a] = 0.75;
            probabilities[2][b] = 0.125;
            probabilities[2][c] = 0.125;
            probabilities[2][d] = 1 - (probabilities[2][a] + probabilities[2][b] + probabilities[2][c]);

            // Process 4
            probabilities[3][a] = 0.5;
            probabilities[3][b] = 0.125;
            probabilities[3][c] = 0.125;
            probabilities[3][d] = 1 - (probabilities[3][a] + probabilities[3][b] + probabilities[3][c]);

            numProcesses = 4;
            break;
        }

    }
}


/*-----------------------------------------------------------------------
 * Function Name : Initialize frame table
 * Allocates memory for the frame table and the owners table
-----------------------------------------------------------------------*/
void frameTable() {

    // # of frames
    numFrames = machineSize / pageSize;

    //Initializers
    pagingTable = new int[numFrames];
    ownerTable = new int[numFrames];
    timeTable = new int[numFrames];
    TimeSinceLastRef = new int[numFrames]; //for lru

    // Set default values for tables
    for (int i = 0; i < numFrames; i++) {
        pagingTable[i] = -1;
        ownerTable[i] = -1;
        timeTable[i] = 0;
        TimeSinceLastRef[i] = 0;
    }

    // Set default values for page variables
    for (int i = 0; i < numProcesses; i++) {
        residencyTime[i] = 0;
        pageFaults[i] = 0;
        pageEvictions[i] = 0;
    }

    // Set number of free frames
    freeFrames = numFrames;
    pager.initialize_lastFrameIn();
}


/*-----------------------------------------------------------------------
 * Function Name : Initialize references
 * Initializes the first reference for every process
 * which is (111 * process num) mod process size
-----------------------------------------------------------------------*/
void handleReferences() {
    for (int i = 0; i < numProcesses; i++) {
        // Initialize first reference of every process
        Reference[i] = (111 * (i + 1)) % processSize;
    }
}


/*-----------------------------------------------------------------------
 * Function Name : Set next reference
 * Takes an integer (process) and sets the next reference
 * for that process depending on the probability provided by the job mix
 * The four cases for the next reference are;
 * w + 1 mod S with probability A
 * w - 5 mod S with probability B
 * w + 4 mod S with probability C
 * A random value in 0... S - 1 each with probability (1 - A - B - C) / S
-----------------------------------------------------------------------*/
void nextReference(int process) {
    // Local constants
    const int MAX_INT = 2147483647;

    // Local variables
    int random_number;        // Random number from random number file
    double caseProbability;        // Probability of a case happening
    int new_reference;        // The new reference

    double a = probabilities[process][0];
    double b = probabilities[process][1];
    double c = probabilities[process][2];
    double d = probabilities[process][3];



    // Get a random number from random number file
    randNumGenerator >> random_number;

    if (debug) {
        //cout << process + 1 << " Random Number Used: " << random_number << endl;
    }

    // Get probability value
    caseProbability = (double) random_number / (MAX_INT + 1.0);
    //cout << "Case Probability : " << caseProbability << endl;


    // Case 1
    if (caseProbability < a) {
        new_reference = (Reference[process] + 1) % processSize;
    }

        // Case 2
    else if (caseProbability < a + b) {
        new_reference = (Reference[process] - 5 + processSize) % processSize;
    }

        // Case 3
    else if (caseProbability < a + b + c) {
        new_reference = (Reference[process] + 4) % processSize;
    }

        // Case 4
    else {
        randNumGenerator >> random_number;
        new_reference = random_number % processSize;
    }

    // Set next reference
    Reference[process] = new_reference;
}


/*-----------------------------------------------------------------------
 * Function Name : Check page fault
 * Determines if a page fault occurs. If there are no free frames for the faulting page,
 * a resident page is evicted using the inputted replacement algo.
 * For the placement question, to choose the same frame as everybody else
 * in the class, the highest numbered free frame is chosen.
-----------------------------------------------------------------------*/
void checkPageFault(int process) {
    // Local constants
    const string LRU = "lru";
    const string LIFO = "lifo";
    const string RANDOM = "random";

    // Local variables
    bool pageFound = false;
    int pageFrame;
    int pageReferenced = Reference[process] / pageSize;    // The page referenced




    if (debug) {
        cout << process + 1 << " references word " << Reference[process]
        << " (page " << pageReferenced << ") at time " << programTime << ": ";
    }

    // Look for resident page
    for (int i = 0; i < numFrames && !pageFound; i++) {
        // IF page is found
        if (pagingTable[i] == pageReferenced && ownerTable[i] == process) {
            // Page is resident
            pageFound = true;
            TimeSinceLastRef[i] = 0;

            if (debug) {
                //HIT!
                cout << "Hit in frame " << i << endl;
            }
        }
    }

    // IF page was not found :[
    if (!pageFound) {
        // Page fault
        pageFaults[process]++;

        // IF there is a free frame
        if (freeFrames != 0) {
            // Place the page in the highest numbered free frame
            pageFrame = --freeFrames;
            pagingTable[pageFrame] = pageReferenced;
            ownerTable[pageFrame] = process;
            TimeSinceLastRef[pageFrame] = 0;

            if (debug) {
                cout << "Fault, using free frame " << pageFrame << endl;
            }
        }

            // else no free frames
        else {
            // Use the replacement algorithm to find page to evict
            if (algo == LRU) {
                pageFrame = pager.lru(numFrames, TimeSinceLastRef);
            }

            else if (algo == LIFO) {
                pageFrame = pager.lifo(numFrames);
            }

            else if (algo == RANDOM) {
                int seed;
                randNumGenerator >> seed;
                pageFrame = pager.random_algo(numFrames, seed);
            }

            else {
                // Error
                cout << "You inputted the wrong replacement algo. Should be lifo, random or lru" << endl;
            }

            // Evict the page and store the time of residency
            residencyTime[ownerTable[pageFrame]] += timeTable[pageFrame];
            pageEvictions[ownerTable[pageFrame]]++;

            if (debug) {
                cout << "Fault, evicting page " << pagingTable[pageFrame] << " of " << ownerTable[pageFrame] + 1
                << " from frame " << pageFrame << endl;
            }

            // Replace the evicted page with the new referenced page
            pagingTable[pageFrame] = pageReferenced;
            ownerTable[pageFrame] = process;
            timeTable[pageFrame] = 0;
            TimeSinceLastRef[pageFrame] = 0;


        }
    }
}


/*-----------------------------------------------------------------------
 * Function Name : Increment time table
 * After one time unit has passed, this function updates all the frames with pages reflect the time
 * of residency per frame.  It starts with the highest free frame.
-----------------------------------------------------------------------*/
void increment_TimeTable() {

    // FOR every frame with a page in it
    for (int i = numFrames - 1; i >= freeFrames; i--) {
        // Increment time table
        timeTable[i]++;
        TimeSinceLastRef[i]++;
    }
}

/*-----------------------------------------------------------------------
 * Function Name : Print Results
 * At the end of the program's run, this function
 * will print out the results.  The results include each processes' faults
 * and average residency time as well as the overall number of faults
 * and average residency.
-----------------------------------------------------------------------*/
void print_results() {

    // Local variables
    double averageResidency;        // Average residency time of a process
    int overallFaults = 0;            // Overall number of faults
    int overallEvictions = 0;        // Overall number of evictions
    double overallResidency = 0;    // Overall average residency time





    cout << endl;

    // Loop through each process
    for (int i = 0; i < numProcesses; i++) {
        // Add to running sum
        overallFaults += pageFaults[i];
        overallEvictions += pageEvictions[i];
        overallResidency += residencyTime[i];

        // IF process did not have any evictions
        if (pageEvictions[i] == 0) {
            // Print number of faults
            cout << "Process " << i + 1 << " had " << pageFaults[i] << " faults.\n";

            // Print average residency time is undefined
            cout << "\tWith no evictions, the average residence is undefined.\n";
        }

            // ELSE process did have evictions
        else {
            // Print number of faults
            cout << "Process " << i + 1 << " had " << pageFaults[i] << " faults and ";

            // Print average residency time
            averageResidency = (double) residencyTime[i] / pageEvictions[i];

            cout << averageResidency << " average residency.\n";
        }
    }

    // IF overall there were no evictions
    if (overallEvictions == 0) {
        // Print number of faults
        cout << "\nThe total number of faults is " << overallFaults << ".\n";

        // Print overall average residency time is undefined
        cout << "\tWith no evictions, the average residence is undefined.\n";
    }

        // ELSE overall there were evictions
    else {
        // Print number of faults
        cout << "\nThe total number of faults is " << overallFaults;

        // Print overall average residency time
        overallResidency /= overallEvictions;

        cout << " and the overall average residency is " << overallResidency << ".\n";
    }

    cout << endl;
}