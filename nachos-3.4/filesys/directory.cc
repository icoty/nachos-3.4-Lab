// directory.cc 
//	Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//	Fixing this is one of the parts to the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "directory.h"
//----------------------------------------------------------------------
// Directory::Directory
// 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	"size" is the number of entries in the directory
//----------------------------------------------------------------------

Directory::Directory(int size)
{
    table = new DirectoryEntry[size];
    tableSize = size;
    for (int i = 0; i < tableSize; i++)
	table[i].inUse = FALSE;
}

//----------------------------------------------------------------------
// Directory::~Directory
// 	De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{
    if(table){
        delete [] table;
        table = NULL;
    }
} 

//----------------------------------------------------------------------
// Directory::FetchFrom
// 	Read the contents of the directory from disk.
//
//	"file" -- file containing the directory contents
//----------------------------------------------------------------------

void
Directory::FetchFrom(OpenFile *file)
{
    /*
    for(int i = 0; i < tableSize; ++i){
        (void) file->ReadAt((char*)&table[i], (int)sizeof(DirectoryEntry), 0);
        
        int len = 0;
        (void) file->ReadAt((char *)&len, sizeof(int), 0);
        
        table[i].name = new char(len + 1);
        (void) file->ReadAt((char *)table[i].name, len * sizeof(char), 0);
    }*/
    (void) file->ReadAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::WriteBack
// 	Write any modifications to the directory back to disk
//
//	"file" -- file to contain the new directory contents
//----------------------------------------------------------------------

void
Directory::WriteBack(OpenFile *file)
{
    /*
    for(int i = 0; i < tableSize; ++i){
        (void) file->WriteAt((char*)&table[i], (int)sizeof(DirectoryEntry), 0);
        (void) file->WriteAt((char *)strlen(table[i].name), sizeof(int), 0);
        (void) file->WriteAt((char *)table[i].name, strlen(table[i].name) * sizeof(char), 0);
    }*/
    (void) file->WriteAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::FindIndex
// 	Look up file name in directory, and return its location in the table of
//	directory entries.  Return -1 if the name isn't in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::FindIndex(char *name)
{
    for (int i = 0; i < tableSize; i++){
        //printf("%s %s %d %d\n",table[i].name, name,table[i].inUse,i);

        if (table[i].inUse && !strncmp(table[i].name, name,FileNameMaxLen)) //!strncmp(table[i].name, name, FileNameMaxLen))
	    return i;
    }
    return -1;		// name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
// 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't 
//	in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::Find(char *name)
{
    int i = FindIndex(name);
    if (i != -1)
	return table[i].sector;
    return -1;
}

//----------------------------------------------------------------------
// Directory::Add
// 	Add a file into the directory.  Return TRUE if successful;
//	return FALSE if the file name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional file names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool
Directory::Add(char *name, int newSector)
{
    //ASSERT(strlen(name) < FileNameMaxLen);
    // -1不存在同名文件
    if (FindIndex(name) != -1)
	return FALSE;
    
    for (int i = 0; i < tableSize; i++)
        if (!table[i].inUse) {
            table[i].inUse = TRUE;
            
            strncpy(table[i].name,name,FileNameMaxLen);
            // 深拷贝;
            /*table[i].name = new char(strlen(name) + 1);
            bzero(table[i].name,strlen(name) + 1);
            strncpy(table[i].name,name,strlen(name));*/
            
            table[i].sector = newSector;
            DEBUG('f',"Add fileName:[%s],DirectoryTable[%d].sector:[%d]\n",\
                   table[i].name,i,table[i].sector);
        return TRUE;
	}
    return FALSE;	// no space.  Fix when we have extensible files.
}

//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return TRUE if successful;
//	return FALSE if the file isn't in the directory. 
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------

bool
Directory::Remove(char *name)
{ 
    int i = FindIndex(name);

    if (i == -1)
	return FALSE; 		// name not in directory
    table[i].inUse = FALSE;
    bzero(table[i].name,FileNameMaxLen);
    return TRUE;
}

//----------------------------------------------------------------------
// Directory::List
// 	List all the file names in the directory. 
//----------------------------------------------------------------------

void
Directory::List()
{
   for (int i = 0; i < tableSize; i++)
       if (table[i].inUse)
           printf("isDirectory:[%d],fileName:[%s],hdrSector:[%d],hdrItem inUse:[%d]\n", table[i].inUse,table[i].name,table[i].sector,table[i].inUse);
}

//----------------------------------------------------------------------
// Directory::Print
// 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void
Directory::Print()
{ 
    FileHeader *hdr = new FileHeader;

    printf("Directory contents:\n");
    for (int i = 0; i < tableSize; i++)
	if (table[i].inUse) {
        printf("isDirectory:[%d], Name:[%s], hdrSector:[%d]\n", table[i].inUse,table[i].name, table[i].sector);
	    hdr->FetchFrom(table[i].sector);
	    hdr->Print();
	}
    printf("\n");
    delete hdr;
}
