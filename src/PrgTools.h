/*
    Program tool structures
    (c) 2004 Tomasz Lis
*/
#if !defined( __PrgTools_H )
#define __PrgTools_H

#include <stdio.h>

typedef unsigned long ulong;
typedef unsigned int uint;

enum ProgError
    {
    errFileRead=        0x0001,
    errFileWrite=       0x0002,
    errFileOpen=        0x0004,
    errMemAlloc=        0x0008,

    errMainHdr=         0x0010,
    errFrameHdr=        0x0020,
    errChunkHdr=        0x0040,
    errChunkData=       0x0080,
    errAddHdr=          0x0100,
    errFramePosTbl=     0x0200,
    errPlainData=       0x0400,

    errDataNotExist=    0x1000,
    errCannotComplete=  0x2000,
    errOnlyParsing=     0x4000,
    errCritical=        0x8000,
    errImpossible=      0xffff
    };

enum ProgOptions
    {
    poManualSeeking=     0x0001,//Alternatywna metoda szukania poˆo¾eä w plikach
    poDisplayAllInfo=    0x0002,//Wy˜wietla mn¢stwo tekst¢w informacyjnych
    poExpandPatette=     0x0004,//Opcja wewnetrzna - powoduje zmiane typu palet COLOR256 na COLOR
    poRecostructPatette= 0x0008,//Opcja wewn©trzna - wˆ¥cza si© gdy w 1 klatce nie ma definicji palety kolor¢w
    poFixMainHeader=     0x0010,//Okre˜la czy MainHeader b©dzie "urzeczywistniany"
    poFixFramePositions= 0x0020,//Okre˜la, czy pozycje klatek maj¥ by† dopasowywane przez szukanie ich w pliku
    poUseFrameFinder=    0x0040,//FrameFinder to funkcja szukaj¥ca klatek, gdy znaleziono ich zbyt maˆo
    poFixFrameHeaders=   0x0080,//Bez tego funkcja naprawiaj¥ca sie sama koäczy
    poRadicalFrameHdrFix=0x0100,//zwi©ksza rygor poprawiania FrameHeaders
    poRemoveBadChunks=   0x0200,//Jak z chunksem jest co˜ nie tak, to go wywala
    poFixChunkHeaders=   0x0400,//Okre˜la czy chunksy maj¥ by† poprawiane
    poSimpleFix=         0x0800,//Do test¢w - program naprawia tylko nagˆ¢wek, reszt© kopiuje
    poNeverWaitForKey=   0x1000,//Nigdy nie prosi o naci˜ni©cie klawisza
    poIgnoreExceptions=  0x2000,//do wewn©trznego urzycia, nie pokazuje bˆ©d¢w gdy wyst©puj¥
    poNeverSkipFrames=   0x4000 //klatki «le wygl¥daj¥ce nie s¥ odrzucane
    };

const int
  kbEnter     = 0x0d,
  kbEscape    = 27;

typedef struct
  {
     unsigned int low;
     unsigned int high;
  } longAsInt;

//No i funkcje

long filesize(FILE *stream);
void ShowError(int ErrNum,char ErrText[]);
void LoadPalette(char *FName,void *Buf,ulong BufSize,int Options);
void SaveBlockToNewFile(const char *FName,void *Buf,ulong BufSize);
void SaveDataToFile(void *BufDest,ulong Size,int DestFile);
int LoadDataFromFile(int File,void *Buf,ulong BytesToRead,int ErrNum,int Options);
void *AllocateMem(ulong Size,int ErrNum,int Options);


#endif	// __PrgTools_H
