/*
    Program tool functions
    (c) 2004 Tomasz Lis
*/
#if !defined( __PrgTools_Cpp )
#define __PrgTools_Cpp

#include <stdlib.h>

long filesize(FILE *stream)
{
   long curpos, length;

   curpos = ftell(stream);
   fseek(stream, 0L, SEEK_END);
   length = ftell(stream);
   fseek(stream, curpos, SEEK_SET);
   return length;
}

void ShowError(int ErrNum,char ErrText[])
{
  printf("  !!! There was an error during program execution !!!\n    When ");
  if (ErrNum & errFileRead)
    printf("reading from a file");
   else
  if (ErrNum & errFileWrite)
    printf("writing to a file");
   else
  if (ErrNum & errFileOpen)
    printf("opening a file");
   else
  if (ErrNum & errMemAlloc)
    printf("allocating memory for");
   else
    printf("processings");
  if (ErrNum & 0x00f0)
    {
    printf(" the element: ");
    if (ErrNum & errMainHdr)
      printf("Main FLI header");
     else
    if (ErrNum & errFrameHdr)
      printf("a frame header");
     else
    if (ErrNum & errChunkHdr)
      printf("a Chunk (part of frame) header");
     else
    if (ErrNum & errChunkData)
      printf("a Chunk (part of frame) data");
     else
      printf("unknown");
    };
  printf(", the exception has occured:\n");
  printf("    %s\n",ErrText);

if (ErrNum & errCritical)
  {
   printf("   With this error, program is unable to continue.\n");
   printf("   Program will now exit. Bye.\n");
   printf(" (Press any key to quit)\n");
   getch();
   exit(EXIT_FAILURE);
  }
  else
  {
   printf("   Program will now try to continue, but some data may be invalid.\n\n");
  };
}

void LoadPalette(char *FName,void *Buf,ulong BufSize,int Options)
{
  int File=open(FName,O_RDONLY|O_BINARY);
  if (File<1)
    { ShowError(errFileOpen,strerror(errno)); return; };
  ulong Count=3*256;
  if (BufSize<Count) Count=BufSize;
  ulong ReadOK=read(File,Buf,Count);
  if ((ReadOK!=Count) && !(Options & poIgnoreExceptions))
    ShowError(errFileRead,"I couldn't read enought bytes of palette file to reconstruct chunk.");
  if (Options & poDisplayAllInfo)
    printf("    ==>Palette chunk constructed.\n");
  close(File);
}




#endif	// __PrgTools_Cpp
