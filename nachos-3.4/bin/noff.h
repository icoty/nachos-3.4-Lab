/* noff.h 
 *     Data structures defining the Nachos Object Code Format
 *
 *     Basically, we only know about three types of segments:
 *	code (read-only), initialized data, and unitialized data
 */

#define NOFFMAGIC	0xbadfad 	/* magic number denoting Nachos 
					 * object code file 
					 */
// add by yangyu https://icoty.github.io/about/
// 定义了elf文件格式的数据结构
typedef struct segment {
  int virtualAddr;		/* location of segment in virt addr space */
  int inFileAddr;		/* location of segment in this file */
  int size;			/* size of segment */
} Segment;

typedef struct noffHeader {
   int noffMagic;		/* should be NOFFMAGIC */
   Segment code;		/* executable code segment */ 
   Segment initData;		/* initialized data segment */
   Segment uninitData;		/* uninitialized data segment --
				 * should be zero'ed before use 
				 */
} NoffHeader;
