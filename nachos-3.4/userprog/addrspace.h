// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"

#define UserStackSize		1024 	// increase this as necessary!

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    // 保存用户程序空间的状态
    void SaveState();			// Save/restore address space-specific
    // 恢复处理机用户程序空间的状态
    void RestoreState();		// info on a context switch 

    unsigned int getNumPages(){ return numPages; } // add by yangyu
    
  private:
    // 读入可执行文件的时候，根据可执行文件大小建立好线性页表，虚拟页号和物理页号相同
    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    
    // 每一个可执行文件所需要的页数
    unsigned int numPages;		// Number of pages in the virtual 
					// address space
};

#endif // ADDRSPACE_H
