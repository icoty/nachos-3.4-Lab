// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
void IncrementPCRegs(void);
void
ExceptionHandler(ExceptionType which)
{
    // 从r2寄存器读取系统调用编号
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
        printf("[%s] SC_Halt, initiated by user program.\n",currentThread->getName());
        machine->ClearAll();
        interrupt->Halt();
    }else if ((which == SyscallException) && (type == SC_Exit)) {
        printf("[%s] SC_Exit, initiated by user program.\n",currentThread->getName());
        machine->ClearAll();
        
        int status = machine->ReadRegister(4); // r4: first arguments to functions
        /*
        currentThread->setExitStatus(status);
        if (status == 0) {
            DEBUG('S', COLORED(GREEN, "User program exit normally. (status 0)\n"));
        } else {
            DEBUG('S', COLORED(FAIL, "User program exit with status %d\n"), status);
        }*/

        IncrementPCRegs();
        currentThread->Finish();

    } else if(which == PageFaultException){
        // BadVaddrReg寄存器内所存储的当出错陷入（Exception）时用户程序的逻辑地址
        int vaddr = machine->ReadRegister(BadVAddrReg);
        if(machine->tlb){ // 处理快表失效
            DEBUG('a', "tlb miss pagefault BadVAddrReg:[%llx]\n",vaddr);
            machine->tlbSwap(vaddr);
        }else{ // 线性页表失效
            
        }
    }else{
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
}


/**********************************************************************/
/*************************** Lab6: Syscall ****************************/
/**********************************************************************/

//----------------------------------------------------------------------
// IncrementPCRegs
//     Because when Nachos cause the exception. The PC won't increment
//  (i.e. PC+4) in Machine::OneInstruction in machine/mipssim.cc.
//  Thus, when invoking a system call, we need to advance the program
//  counter. Or it will cause the infinity loop.
//----------------------------------------------------------------------

void IncrementPCRegs(void) {
    // Debug usage
    //machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    
    // Advance program counter
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    //machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
}

//----------------------------------------------------------------------
// AddressSpaceControlHandler
//     Handling address space control related system call.
//  1. Exit
//  2. Exec
//  3. Join
//----------------------------------------------------------------------
/*
void AddressSpaceControlHandler(int type)
{
    if (type == SC_Exit) {
        
        PrintTLBStatus(); // TLB debug usage
        
        int status = machine->ReadRegister(4); // r4: first arguments to functions
        
        currentThread->setExitStatus(status);
        if (status == 0) {
            DEBUG('S', COLORED(GREEN, "User program exit normally. (status 0)\n"));
        } else {
            DEBUG('S', COLORED(FAIL, "User program exit with status %d\n"), status);
        }
        
        // TODO: release children
        
#ifdef USER_PROGRAM
        if (currentThread->space != NULL) {
            machine->freeMem();
            
            delete currentThread->space;
            currentThread->space = NULL;
        }
#endif // USER_PROGRAM
        // TODO: if it has parent, then set this to zombie and signal
        currentThread->Finish();
    }
}

//----------------------------------------------------------------------
// FileSystemHandler
//
//----------------------------------------------------------------------

void FileSystemHandler(int type)
{
    
}

//----------------------------------------------------------------------
// UserLevelThreadsHandler
//
//----------------------------------------------------------------------

void UserLevelThreadsHandler(int type)
{
    
}
*/
