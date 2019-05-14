// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"

void forkThreadFunc(int num){
    printf("forkThreadFunc start nachos thread:[%s].\n",currentThread->getName());
    machine->Run();
}

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------

void
StartProcess1(char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;
    
    if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        return;
    }
    space = new AddrSpace(executable);
    currentThread->space = space;
    
    delete executable;            // close file
    
    space->InitRegisters();        // set the initial register values
    space->RestoreState();        // load page table register
    
    machine->Run();            // jump to the user progam
    ASSERT(FALSE);            // machine->Run never returns;
    // the address space exits
    // by doing the syscall "exit"
}

void
StartProcess(char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);
    if (!executable) {
        printf("1 Unable to open file %s\n", filename);
        return;
    }
    printf("initial first thread space\n");
    AddrSpace *space = new AddrSpace(executable);
    delete executable;            // close file

    // second thread
    OpenFile *executable1 = fileSystem->Open(filename);
    if (!executable1) {
        printf("2 Unable to open file %s\n", filename);
        return;
    }
    printf("initial second thread space\n");
    AddrSpace *space1 = new AddrSpace(executable1);
    delete executable1;            // close file

    currentThread->space = space;

    space1->InitRegisters();        // set the initial register values
    space1->RestoreState();        // load page table register
    Thread *thread1 = new Thread("2ndThread");
    thread1->space = space1;
    thread1->Fork(forkThreadFunc,1);
    
    currentThread->Yield(); // 主线程让出cpu
    
    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register
    printf("StartProcess start nachos thread:[%s].\n",currentThread->getName());
    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}
