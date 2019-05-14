// stats.h 
//	Data structures for gathering statistics about Nachos performance.
//
// DO NOT CHANGE -- these stats are maintained by the machine emulation
//
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef STATS_H
#define STATS_H

#include "copyright.h"

// The following class defines the statistics that are to be kept
// about Nachos behavior -- how much time (ticks) elapsed, how
// many user instructions executed, etc.
//
// The fields in this class are public to make it easier to update.

class Statistics {
  public:
    int totalTicks;      	// Total time running Nachos
    int idleTicks;       	// Time spent idle (no threads to run)
    int systemTicks;	 	// Time spent executing system code
    int userTicks;       	// Time spent executing user code
				// (this is also equal to # of
				// user instructions executed)

    int numDiskReads;		// number of disk read requests
    int numDiskWrites;		// number of disk write requests
    int numConsoleCharsRead;	// number of characters read from the keyboard
    int numConsoleCharsWritten; // number of characters written to the display
    int numPageFaults;		// number of virtual memory page faults
    int numPacketsSent;		// number of packets sent over the network
    int numPacketsRecvd;	// number of packets received over the network

    Statistics(); 		// initialize everything to zero

    void Print();		// print collected statistics
};

// Constants used to reflect the relative time an operation would
// take in a real system.  A "tick" is a just a unit of time -- if you 
// like, a microsecond.
//
// Since Nachos kernel code is directly executed, and the time spent
// in the kernel measured by the number of calls to enable interrupts,
// these time constants are none too exact.

// nachos执行每条用户指令的时间为1Tick
#define UserTick 	1	// advance for each user-level instruction
// 系统态无法进行指令计算，所以nachos系统态的一次中断调用或其他需要进行时间计算的单位设置为10Tick
#define SystemTick 	10 	// advance each time interrupts are enabled

// 磁头寻找超过一个扇区的时间
#define RotationTime 	500 	// time disk takes to rotate one sector

// 磁头寻找超过一个磁道的时间
#define SeekTime 	500    	// time disk takes to seek past one track
#define ConsoleTime 	100	// time to read or write one character
#define NetworkTime 	100   	// time to send or receive one packet

// 时钟中断间隔
#define TimerTicks 	   10 	// (average) time between timer interrupts

// 时间片大小200
#define TimerSlice  20       // 时间片轮转算法一个时间片大小
#endif // STATS_H
