// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"

extern void getFormatDate(char out[]);

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space

    int needDirect = numSectors > NumDirect ? NumDirect : numSectors;

    for (int i = 0; i < needDirect; i++)
        dataSectors[i] = freeMap->Find();
    
    if(numSectors > NumDirect){
        // NumDirect = 14,NumFirst = 1
        dataSectors[NumDirect] = freeMap->Find();
        int direct_first[SectorSize / sizeof(int)] = {0}; // 一级索引表含表项为 SectorSize / sizeof(int)
        for(int i = 0; i < numSectors - NumDirect; ++i){
            direct_first[i] = freeMap->Find();
        }
        synchDisk->WriteSector(dataSectors[NumDirect],(char*)direct_first);
    }
    
    setCreateTime();
    
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
    int needDirect = numSectors > NumDirect ? NumDirect : numSectors;
    
    for (int i = 0; i < needDirect; i++) {
	ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	freeMap->Clear((int) dataSectors[i]);
    }
    
    if(numSectors > NumDirect){
        // NumDirect = 14,NumFirst = 1
        char* direct_first = new char[SectorSize];
        bzero(direct_first,0);
        synchDisk->ReadSector(dataSectors[NumDirect],direct_first);

        // 释放一级索引所有表项
        for(int i = 0; i < numSectors - NumDirect; ++i){
            ASSERT(freeMap->Test((int) direct_first[i * 4]));  // ought to be marked!
            freeMap->Clear((int) direct_first[i * 4]);
        }
        
        // 释放一级索引
        ASSERT(freeMap->Test((int) dataSectors[NumDirect]));  // ought to be marked!
        freeMap->Clear((int) dataSectors[NumDirect]);
    }
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    if(offset <= NumDirect * SectorSize)
        return(dataSectors[offset / SectorSize]);
    else{
        int pos = (offset - NumDirect * SectorSize) / SectorSize;
        // NumDirect = 14,NumFirst = 1
        char* direct_first = new char[SectorSize];
        bzero(direct_first,0);
        synchDisk->ReadSector(dataSectors[NumDirect],direct_first);
        return int(direct_first[pos * 4]);
    }
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];
    int needDirect = numSectors > NumDirect ? NumDirect : numSectors;
    
    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < needDirect; i++)
        printf("%d ", dataSectors[i]);
    
    if(numSectors > NumDirect){
        printf("\ndirect_first:[%d]\n",dataSectors[NumDirect]);
        // NumDirect = 14,NumFirst = 1
        char* direct_first = new char[SectorSize];
        bzero(direct_first,0);
        synchDisk->ReadSector(dataSectors[NumDirect],direct_first);
        
        for(int i = 0; i < numSectors - NumDirect; ++i){
            printf("%d ",(int)direct_first[i * 4]);
        }
    }
    printf("\nFile contents:\n");
    
    for (i = k = 0; i < needDirect; i++) {
        synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
            if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
                printf("%c", data[j]);
            else
                printf("\\%x", (unsigned char)data[j]);
        }
        printf("\n"); 
    }
    
    if(numSectors > NumDirect){
        char* direct_first = new char[SectorSize];
        bzero(direct_first,0);
        synchDisk->ReadSector(dataSectors[NumDirect],direct_first);
        for(i = 0; i < numSectors - NumDirect; ++i){
            printf("Sector:[%d]\n",(int)direct_first[i * 4]);
            synchDisk->ReadSector((int)direct_first[i * 4],data);
            for(j = 0; (j < SectorSize) && (k < numBytes); ++j, ++k){
                if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
                    printf("%c", data[j]);
                else
                    printf("\\%x", (unsigned char)data[j]);
            }
        }
        printf("\n");
    }
    
    delete [] data;
}

void FileHeader::setCreateTime()
{
    getFormatDate(createTime);
}

void FileHeader::setUpdateTime()
{
    getFormatDate(updateTime);
}

void FileHeader::setAccessTime()
{
    getFormatDate(accessTime);
}
