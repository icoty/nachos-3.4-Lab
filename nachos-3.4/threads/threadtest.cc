// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "elevatortest.h"
#include "synch.h"
#include "stats.h"
#include <string.h>
#include <stdio.h>
#include <string>
#include <iostream>
using std::string;
// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------
// 线程主动让出cpu,在FIFO调度策略下能够看到多个线程按顺序运行
void
SimpleThread(int which)
{
    for (int num = 0; num < 5; num++) {
        int ticks = stats->systemTicks - scheduler->getLastSwitchTick();
        printf("userId=%d,threadId=%d,prio=%d,loop:%d,lastSwitchTick=%d,systemTicks=%d,usedTicks=%d,TimerSlice=%d\n",\
               currentThread->getUserId(),currentThread->getThreadId(),currentThread->getPriority(),num,\
               scheduler->getLastSwitchTick(),stats->systemTicks,ticks,TimerSlice);
        // 时间片轮转算法，判断时间片是否用完，如果用完主动让出cpu，针对nachos内核线程算法
        if(ticks >= TimerSlice){
            printf("threadId=%d Yield\n",currentThread->getThreadId());
            currentThread->Yield();
        }
        
        // 非抢占模式下，多个线程同时执行该接口的话，会交替执行，交替让出cpu
        // currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, (void*)1);
    SimpleThread(0);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------
void
ThreadPriorityTest()
{
    Thread* t1 = new Thread("forkThread1", 1);
    printf("-->name=%s,threadId=%d\n",t1->getName(),t1->getThreadId());
    t1->Fork(SimpleThread, (void*)1);
    
    Thread* t2 = new Thread("forkThread2", 2);
    printf("-->name=%s,threadId=%d\n",t2->getName(),t2->getThreadId());
    t2->Fork(SimpleThread, (void*)2);
    
    Thread* t3 = new Thread("forkThread3", 3);
    printf("-->name=%s,threadId=%d\n",t3->getName(),t3->getThreadId());
    t3->Fork(SimpleThread, (void*)3);
    
    Thread* t4 = new Thread("forkThread4", 4);
    printf("-->name=%s,threadId=%d\n",t4->getName(),t4->getThreadId());
    t4->Fork(SimpleThread, (void*)4);
    
    currentThread->Yield();
    SimpleThread(0);
}

// 线程最多128个，超过128个终止运行
void
ThreadCountLimitTest()
{
    for (int i = 0; i <= maxThreadsCount; ++i) {
	Thread* t = new Thread("fork thread");
        printf("thread name = %s, userId = %d, threadId = %d\n", t->getName(), t->getUserId(), t->getThreadId());
    }
}


// 信号量解决生产者消费者问题
#define N 1024 // 缓冲区大小
Semaphore* empty = new Semaphore("emptyBuffer", N);
Semaphore* mutex = new Semaphore("lockSemaphore", 1);
Semaphore* full = new Semaphore("fullBuffer", 0);
int msgQueue = 0;

// Lab3 信号量实现生产者消费者问题
void Producer(int val){
    while(1) {
        empty->P();
        mutex->P();
        if(msgQueue >= N){ // 已经满了则停止生产
            printf("-->Product alread full:[%d],wait consumer.",msgQueue);
        }else{
            printf("-->name:[%s],threadId:[%d],before:[%d],after:[%d]\n",\
                   currentThread->getName(),currentThread->getThreadId(),msgQueue,msgQueue+1);
            ++msgQueue;
        }
        mutex->V();
        full->V();
        
        //sleep(val); // 休息下在生产
    }
}

void Customer(int val){
    while(1) {
        full->P();
        mutex->P();
        if(msgQueue <= 0){
            printf("Product alread empty:[%d],wait Producer.",msgQueue);
        }else{
            printf("name:[%s] threadId:[%d],before:[%d],after:[%d]\n",\
                   currentThread->getName(),currentThread->getThreadId(),msgQueue,msgQueue-1);
            --msgQueue;
        }
        mutex->V();
        empty->V();
        
        //sleep(val); // 休息下在消费
    }
}

void ThreadProducerConsumerTest(){
    DEBUG('t', "Entering ThreadProducerConsumerTest");
    Thread* p1 = new Thread("Producer1");
    Thread* p2 = new Thread("Producer2");
    p1->Fork(Producer, 1);
    p2->Fork(Producer, 3);
    
    Thread* c1 = new Thread("Consumer1");
    //Thread* c2 = new Thread("Consumer2");
    c1->Fork(Customer, 1);
    //c2->Fork(Customer, 2);
}


// 条件变量实现生产者消费者问题
Condition* condc = new Condition("ConsumerCondition");
Condition* condp = new Condition("ProducerCondition");
Lock* pcLock = new Lock("producerConsumerLock");
int shareNum = 0; // 共享内容，生产存值，消费后清零，互斥访问

// lab3 条件变量实现生产者消费者问题
void Producer1(int val){
#if 0
    for(int i = 1; i < N; ++i)
    {
        pcLock->Acquire();
        while(shareNum != 0){ // 为清零表示消费者还未取走,等待在条件变量上
            printf("Product alread full:[%d],threadId:[%d],wait consumer.\n",\
                   shareNum,currentThread->getThreadId());
            condp->Wait(pcLock);
        }
        printf("name:[%s],threadId:[%d],before:[%d],after:[%d]\n",\
               currentThread->getName(),currentThread->getThreadId(),shareNum,i);
        shareNum = i;

        condc->Signal(pcLock);
        pcLock->Release();
        sleep(val);
    }
#else
    while(1){
        pcLock->Acquire();
        while(shareNum >= N){ // 缓冲区已满则等待在条件变量上
            printf("Product alread full:[%d],threadId:[%d],wait consumer.\n",\
                   shareNum,currentThread->getThreadId());
            condp->Wait(pcLock);
        }
        printf("name:[%s],threadId:[%d],before:[%d],after:[%d]\n",\
               currentThread->getName(),currentThread->getThreadId(),shareNum,shareNum+1);
        ++shareNum;
        
        condc->Signal(pcLock);
        pcLock->Release();
        sleep(val);
    }
#endif
}

void Customer1(int val){
#if 0
    for(int i = 1; i < N; ++i)
    {
        pcLock->Acquire();
        while(shareNum == 0){ // 缓冲区为空则等待在条件变量上
            printf("-->Product alread empty:[%d],threadId:[%d],wait producer.\n",\
                   shareNum,currentThread->getThreadId());
            condc->Wait(pcLock);
        }
        printf("-->name:[%s],threadId:[%d],before:[%d],after:[%d]\n",\
               currentThread->getName(),currentThread->getThreadId(),shareNum,0);
        shareNum = 0;
        condp->Signal(pcLock);
        pcLock->Release();
        sleep(val);
    }
#else
    while(1){
        pcLock->Acquire();
        while(shareNum <= 0){ // 为零表示生产者者还未放数据,等待在条件变量上
            printf("-->Product alread empty:[%d],threadId:[%d],wait producer.\n",\
                   shareNum,currentThread->getThreadId());
            condc->Wait(pcLock);
        }
        printf("-->name:[%s],threadId:[%d],before:[%d],after:[%d]\n",\
               currentThread->getName(),currentThread->getThreadId(),shareNum,shareNum-1);
        --shareNum;
        condp->Signal(pcLock);
        pcLock->Release();
        sleep(val);
    }
#endif
}

void ThreadProducerConsumerTest1(){
    DEBUG('t', "Entering ThreadProducerConsumerTest1");
    Thread* p1 = new Thread("Producer1");
    Thread* p2 = new Thread("Producer2");
    p1->Fork(Producer1, 1);
    p2->Fork(Producer1, 3);
    
    Thread* c1 = new Thread("Consumer1");
    Thread* c2 = new Thread("Consumer2");
    c1->Fork(Customer1, 1);
    c2->Fork(Customer1, 2);
}


// lab3 条件变量实现barrier
Condition* barrCond = new Condition("BarrierCond");
Lock* barrLock = new Lock("BarrierLock");
int barrierCnt = 0;
const int barrierThreadNum = 5; // 当且仅当barrierThreadNum个线程同时到达时才能往下运行
void barrierFun(int num){
    /*while(1)*/
     {
        barrLock->Acquire();
        ++barrierCnt;
        if(barrierCnt == barrierThreadNum){
            printf("threadName:[%s%d],barrierCnt:[%d],needCnt:[%d],Broadcast.\n",\
                   currentThread->getName(),num,barrierCnt,barrierThreadNum);
            barrCond->Broadcast(barrLock);
            barrLock->Release();
        }else{
            printf("threadName:[%s%d],barrierCnt:[%d],needCnt:[%d],Wait.\n",\
                   currentThread->getName(),num,barrierCnt,barrierThreadNum);
            barrCond->Wait(barrLock);
            barrLock->Release();
        }
        printf("threadName:[%s%d],continue to run.\n", currentThread->getName(),num);
    }
}

void barrierThreadTest(){
    DEBUG('t', "Entering barrierThreadTest");
    for(int i = 0; i < barrierThreadNum; ++i){
        Thread* t = new Thread("barrierThread");
        t->Fork(barrierFun,i+1);
    }
}


// Lab3 锁实现读者写者问题
int rCnt = 0; // 记录读者数量
Lock* rLock = new Lock("rlock");
Semaphore* wLock = new Semaphore("wlock",1); // 必须用信号量，不能用锁，因为锁只能由枷锁的线程解锁
int bufSize = 0;
// Lab3 锁实现读者写者问题
void readFunc(int num){
    while(1) {
        rLock->Acquire();
        ++rCnt;
        if(rCnt == 1){ // 如果是第一个读者进入，需要竞争写锁，竞争成功才能进入临界区
            wLock->P();
        }
        rLock->Release();
        if(0 == bufSize){
            printf("threadName:[%s],Val:[%d],current not data.\n",currentThread->getName(),bufSize);
        }else{
            printf("threadName:[%s],readVal:[%d],exec read operation.\n",currentThread->getName(),bufSize);
        }
        rLock->Acquire();
        --rCnt;
        if(rCnt == 0){
            wLock->V();
        }
        rLock->Release();
        currentThread->Yield();
        sleep(num);
    }
}

void writeFunc(int num){
    while(1) {
        wLock->P();
        ++bufSize;
        printf("writerThread:[%s],before:[%d],after:[%d]\n", currentThread->getName(), bufSize-1, bufSize);
        wLock->V();
        currentThread->Yield();
        sleep(num);
    }
}

void readWriteThreadTest(){
    DEBUG('t', "Entering readWriteThreadTest");
    Thread * r1 = new Thread("read1");
    Thread * r2 = new Thread("read2");
    Thread * r3 = new Thread("read3");
    Thread * w1 = new Thread("write1");
    Thread * w2 = new Thread("write2");
    
    r1->Fork(readFunc,1);
    w1->Fork(writeFunc,1);
    r2->Fork(readFunc,1);
    w2->Fork(writeFunc,1);
    r3->Fork(readFunc,1);
}

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    case 2:
	ThreadCountLimitTest();
	break;
    case 3:
	ThreadPriorityTest();
	break;
    case 4:
	ThreadProducerConsumerTest();
	break;
    case 5:
        ThreadProducerConsumerTest1();
        break;
    case 6:
        barrierThreadTest();
        break;
    case 7:
        readWriteThreadTest();
        break;
    default:
	printf("No test specified.\n");
	break;
    }
}

