/*
    Program tool structures
    (c) 2004 Tomasz Lis
*/
#if !defined( __PrgTools_H )
#define __PrgTools_H

#include <stdio.h>

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

    errDataNotExist=    0x1000,
    errCannotComplete=  0x2000,
    errCritical=        0x8000
    };

enum ProgOptions
    {
    poManualSeeking=    0x0001,//Alternatywna metoda szukania poˆo¾eä w plikach
    poDisplayAllInfo=   0x0002,//Wy˜wietla mn¢stwo tekst¢w informacyjnych
    poExpandPatette=    0x0004,//Powoduje pomno¾enie w znalezionych nieskompresowanych paletach wszystkiego *4
    poRecostructPatette=0x0008,
    poFixMainHeader=    0x0010,//Okre˜la czy MainHeader b©dzie "urzeczywistniany"
    poFixFrameHeaders=  0x0020,//FrameHeaders i tak s¥ poprawiane, ale to zwi©ksza rygor
    poIgnoreExceptions= 0x0100 //do wewn©trznego urzycia, nie pokazuje bˆ©d¢w gdy wyst©puj¥
    };

const int
  kbEnter     = 0x0d,
  kbEscape    = 27;

typedef unsigned long ulong;

//No i funkcje

long filesize(FILE *stream);
void ShowError(int ErrNum,char ErrText[]);



#endif	// __PrgTools_H
