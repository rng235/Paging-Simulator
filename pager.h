/************************************************************************
* Paging Lab 4 -- Operating Systems
* Author		: Ricardo N. Guntur <ricardo.guntur@nyu.edu>
* Semester		: Fall 2015
*
* Class: pager.h
/************************************************************************/

#ifndef LAB4_V2_PAGER_H
#define LAB4_V2_PAGER_H
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>

using namespace std;

class pager {


public:

    int lastFrameIn; // Last frame placed (used for lifo)

    //Replacement Algorithms
    int lru(int numFrames, int pTimeSinceLastReference[]);  //Least Recently Used
    int random_algo(int numFrames, int seed);               //Random
    int lifo(int numFrames);                                //Last In First Out
    void initialize_lastFrameIn();

};
#endif //LAB4_V2_PAGER_H
