/************************************************************************
* Paging Lab 4 -- Operating Systems
* Author		: Ricardo N. Guntur <ricardo.guntur@nyu.edu>
* Semester		: Fall 2015
*
* Class: pager.cpp
* Includes all the replacement algorithms required for this lab.
* For this case they are: lru, random and lifo
/************************************************************************/

#include "pager.h"

/*-----------------------------------------------------------------------
 * Function Name : Least recently used
 * When a page fault occurs, this algorithm will choose
 * the page that has been unused for the longest time as the victim.
-----------------------------------------------------------------------*/
int pager::lru(int numFrames, int pTimeSinceLastReference[])
{

    // Local variables
    int lruFrame = 0;	// Least recently used frame
    int timeUnused = 0;	// Time since frame's last use



    // FOR every occupied frame
    for (int i = 0; i < numFrames; i++)
    {
        // IF time since frame's last reference > current highest last reference time
        if (pTimeSinceLastReference[i] > timeUnused)
        {
            // This frame has been unused for a longer amount of time
            timeUnused = pTimeSinceLastReference[i];

            // Save index of frame
            lruFrame = i;
        }
    }

    // Return least recently used frame
    return lruFrame;

}

/*-----------------------------------------------------------------------
 * Function Name : Random algorithm
 * A page replacement algorithm that will select a random frame to evict.
-----------------------------------------------------------------------*/
int pager::random_algo(int numFrames, int seed)
{
    // Local variables
    int victimFrame;


    // Use random number to select a random frame
    victimFrame = seed % numFrames;

    // Return victim frame index
    return victimFrame;
}


/*-----------------------------------------------------------------------
 * Function Name : LIFO
 * Function Desc : LIFO will evict pages on a Last in first out basis
-----------------------------------------------------------------------*/
int pager::lifo(int numFrames)
{

    // Local variables
    int victimPage;


    // Select the first page that was placed to evict
    victimPage = lastFrameIn;

    // Set the next page that came in as first page
    lastFrameIn++;

    // IF current victim is 0
    if (victimPage == 0)
    {
        // Wrap around
        lastFrameIn = 0;
    }

    // Return victim page
    return victimPage;
}

void pager::initialize_lastFrameIn() {
    lastFrameIn = 0;
}
