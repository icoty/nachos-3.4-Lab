// directory.h 
//	Data structures to manage a UNIX-like directory of file names.
// 
//      A directory is a table of pairs: <file name, sector #>,
//	giving the name of each file in the directory, and 
//	where to find its file header (the data structure describing
//	where to find the file's data blocks) on disk.
//
//      We assume mutual exclusion is provided by the caller.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "openfile.h"

#define FileNameMaxLen 		11	// for simplicity, we assume
					// file names are <= 9 characters long
#define DictoryMaxLayer     4
#define PathMaxLen      (((1 + FileNameMaxLen) * DictoryMaxLayer) - 1)  // = 47, 最大支持四节目录。/a/b/c/d，目录c下只能是文件，根/ a/ b/ c/共四级

// The following class defines a "directory entry", representing a file
// in the directory.  Each entry gives the name of the file, and where
// the file's header is to be found on disk.
//
// Internal data structures kept public so that Directory operations can
// access them directly.

// 内存对齐，内存空间中占用64个字节
// 每一个表项对应一个文件的目录项，目录项的格式是（filename,sector）
// 即每一个目录项都记录了一个文件名和此文件名的头文件锁占据扇区的扇区号
class DirectoryEntry {
  public:
    bool type; // 0为目录 1为文件

    // 因为需要内存对齐，inUse实际会预留4个字节
    bool inUse;				// Is this directory entry in use?
    int sector;				// type = 1 Location on disk to find the FileHeader for this file
                            // type = 0 说明为目录，sector存放目录名为name的目录文件所在的扇区
    //char name[FileNameMaxLen + 1];	// Text name for file, with +1 for
					// the trailing '\0'
    //add by ayngyu
    char name[FileNameMaxLen + 1] = {0};
    //char path[PathMaxLen + 1] = {0};
};

// The following class defines a UNIX-like "directory".  Each entry in
// the directory describes a file, and where to find it on disk.
//
// The directory data structure can be stored in memory, or on disk.
// When it is on disk, it is stored as a regular Nachos file.
//
// The constructor initializes a directory structure in memory; the
// FetchFrom/WriteBack operations shuffle the directory information
// from/to disk. 

class Directory {
  public:
    Directory(int size); 		// Initialize an empty directory
					// with space for "size" files
    ~Directory();			// De-allocate the directory

    void FetchFrom(OpenFile *file);  	// Init directory contents from disk
    void WriteBack(OpenFile *file);	// Write modifications to 
					// directory contents back to disk

    int Find(char *name);		// Find the sector number of the 
					// FileHeader for file: "name"

    bool Add(char *name, int newSector);  // Add a file name into the directory

    bool Remove(char *name);		// Remove a file from the directory

    void List();			// Print the names of all the files
					//  in the directory
    void Print();			// Verbose print of the contents
					//  of the directory -- all the file
					//  names and their contents.

  private:
    int tableSize;			// Number of directory entries
    DirectoryEntry *table;		// Table of pairs: 
					// <file name, file header location> 

    int FindIndex(char *name);		// Find the index into the directory 
					//  table corresponding to "name"
};

#endif // DIRECTORY_H
