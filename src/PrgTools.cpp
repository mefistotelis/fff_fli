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
  printf("  !!! There was an error during program execution !!!\n");
 if (ErrNum==errImpossible)
  {
  printf("\n    IMPOSSIBLE EVENT HAS HAPPEN.\n  PROGRAM IS BADLY MODIFIED OR YOUR COMPUTER IS UNSTABLE.\n");
  }
 else
  {
  printf("    When ");
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
    printf("allocating memory required");
   else
    printf("processings");
  if (ErrNum & 0x0ff0)
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
    if (ErrNum & errAddHdr)
      printf("Main FLI header additional segments");
     else
    if (ErrNum & errFramePosTbl)
      printf("a Frame Position Table");
     else
    if (ErrNum & errPlainData)
      printf("a plain data");
     else
      printf("unknown");
    };
  printf(",\n    the exception has occured:\n");
  printf("    %s\n",ErrText);
  if (ErrNum & errDataNotExist)
    printf("    The expected data couldn't be found.\n");
  if (ErrNum & errCannotComplete)
    printf("    The operation couldn't be completed.\n");
  if (ErrNum & errOnlyParsing)
    printf("    This happen on parsing only, not on final processing.\n    Parsing cancelled.\n");
  }; //end if (ErrNum==errImposs...

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

void SaveBlockToNewFile(const char *FName,void *Buf,ulong BufSize)
{
  int File=open(FName,O_RDWR|O_CREAT|O_TRUNC|O_BINARY,S_IREAD|S_IWRITE);
  ulong nWritten=write(File,Buf,BufSize);
  if (nWritten<BufSize) ShowError(errFileWrite,"Funkcja SaveBlock nie mogˆa wykona† operacji.");
  close(File);
}

void SaveDataToFile(void *BufDest,ulong Size,int DestFile)
{
  ulong nWritten=write(DestFile,BufDest,Size);
  if (nWritten<Size) ShowError(errFileWrite+errCritical,strerror(errno));
}

int LoadDataFromFile(int File,void *Buf,ulong BytesToRead,int ErrNum,int Options)
/*
 Wczytuje okre˜lon¥ liczb© bajt¢w z pliku do wcze˜niej zaallokowanego bufora.
*/
{
  if (BytesToRead==0) return 1;
  unsigned int ReadOK=read(File,Buf,BytesToRead);
  if ((ReadOK!=BytesToRead) && !(Options & poIgnoreExceptions))
    {
    char *ErrMsg=(char *)malloc(256);
    if (ErrMsg!=NULL)
      {
      sprintf(ErrMsg,"Couldn't read more than %u bytes, %lu needed. EOF or read error.",ReadOK,BytesToRead);
      ShowError(errFileRead|ErrNum,ErrMsg);
      free(ErrMsg);
      }
     else
      ShowError(errFileRead|ErrNum,"Couldn't read enought bytes, also some memory faults.");
    };
  if (ReadOK!=BytesToRead)
    return 0;
   else
    return 1;
}

void *AllocateMem(ulong Size,int ErrNum,int Options)
{
  void *Data=malloc(Size);
  if ((Data==NULL) && !(Options & poIgnoreExceptions))
   {
    char *ErrMsg=(char *)malloc(128);
    if (ErrMsg!=NULL)
      {
      if (Size<8192)
        sprintf(ErrMsg,"Cannot allocate %lu bytes. Whole memory has been allocated before.",Size);
       else
        sprintf(ErrMsg,"Cannot allocate %lu bytes. Maybe U should modify FLI file to have less frames.",Size);
      ShowError(errMemAlloc|ErrNum,ErrMsg);
      free(ErrMsg);
      }
     else
      ShowError(errMemAlloc|ErrNum,"Cannot allocate any memory. Try reboot your machine.");
   };
  return Data;
}

#endif	// __PrgTools_Cpp
