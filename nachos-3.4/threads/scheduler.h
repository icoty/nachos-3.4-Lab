// scheduler.h 
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "list.h"
#include "thread.h"

// The following class defines the scheduler/dispatcher abstraction -- 
// the data structures and operations needed to keep track of which 
// thread is running, and which threads are ready but not running.

class Scheduler {
  public:
    Scheduler();			// Initialize list of ready threads 
    ~Scheduler();			// De-allocate ready list

    void ReadyToRun(Thread* thread);	// Thread can be dispatched.
    // bySlepp = true则是由睡眠的线程调用，不判断时间片大小，直接从队列取
    // bySlepp = false则是由正在运行的线程yield调用，需要判断时间片大小是否切换线程
    Thread* FindNextToRun(bool bySlepp);		// Dequeue first thread on the ready
					// list, if any, and return thread.
    void Run(Thread* nextThread);	// Cause nextThread to start running
    void Print();			// Print contents of ready list
    
    List* getReadyList(){ return readyList; }  // add by yangyu
    int getLastSwitchTick(){ return lastSwitchTick; }
    int lastSwitchTick;
  private:
    List *readyList;  		// queue of threads that are ready to run,
				// but not running
};

#endif // SCHEDULER_Hm
