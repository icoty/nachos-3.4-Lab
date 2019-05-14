// filehdr.h 
//	Data structures for managing a disk file header.  
//
//	A file header describes where on disk to find the data in a file,
//	along with other information about the file (for instance, its
//	length, owner, etc.)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef FILEHDR_H
#define FILEHDR_H

#include "disk.h"
#include "bitmap.h"

#define DateLength 20

// file max length (14+1*32)*128=5888 Bytes
#define NumFirst 1  // 一级索引数1项,可存放128/4=32个直接索引，1793~5888 Bytes
#define NumDirect 	((SectorSize - 2 * sizeof(int) - 3 * DateLength * sizeof(char)) / sizeof(int) - NumFirst) // 直接索引数14项,0~1792 Bytes
#define MaxFileSize 	((NumDirect + NumFirst * SectorSize / sizeof(int)) * SectorSize) // 5888 Bytes

// The following class defines the Nachos "file header" (in UNIX terms,  
// the "i-node"), describing where on disk to find all of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks. 
//
// The file header data structure can be stored in memory or on disk.
// When it is on disk, it is stored in a single sector -- this means
// that we assume the size of this data structure to be the same
// as one disk sector.  Without indirect addressing, this
// limits the maximum file length to just under 4K bytes.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.
// 文件头128Bytes，减去属性所占用空间后，剩余空间用于直接索引的数量，一个索引指向一个扇区
class FileHeader {
  public:
    bool Allocate(BitMap *bitMap, int fileSize);// Initialize a file header, 
						//  including allocating space 
						//  on disk for the file data
    void Deallocate(BitMap *bitMap);  		// De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk

    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte

    int FileLength();			// Return the length of the file 
					// in bytes

    void Print();			// Print the contents of the file.
    
    // add by yangyu
    char* getCreateTime(){ return createTime; }
    char* getUpdateTime(){ return updateTime; }
    char* getAccessTime(){ return accessTime; }
    void setCreateTime();
    void setUpdateTime();
    void setAccessTime();

  private:
    int numBytes;			// Number of bytes in the file
    int numSectors;			// Number of data sectors in the file
    
    // 形如2018-12-12 12:25:45，长度20,内存已经对齐
    char createTime[DateLength] = {0};
    char updateTime[DateLength] = {0};
    char accessTime[DateLength] = {0};
    
    // NumDirect是文件头内部可以直接索引的数量，一个索引指向一个扇区
    int dataSectors[NumDirect + NumFirst];		// Disk sector numbers for each data
					// block in the file
};

#endif // FILEHDR_H
