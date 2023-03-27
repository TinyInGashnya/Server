// Module Name: nbcommon.h
//
// Purpose:
//    This header file contains the function prototypes for a set
//    of functions that implement some of the most common NetBIOS
//    functions such as enumerating LANAs, adding names, removing
//    names, etc.  The functions are implemented in Nbcommon.c
//    
#include <windows.h>
#include <nb30.h>

extern "C" int Recv(int lana, int lsn, char *buffer, DWORD *len);
extern "C" int Send(int lana, int lsn, char *data, DWORD len);
extern "C" int AddName(int lana, char *name, int *num);
extern "C" int DelName(int lana, char *name);
extern "C" int AddGroupName(int lana, char *name, int *num);
extern "C" int ResetAll(LANA_ENUM *lenum, UCHAR ucMaxSession, 
	     UCHAR ucMaxName, BOOL bFirstName);
extern "C" int LanaEnum(LANA_ENUM *lenum);
extern "C" int Hangup(int lana, int lsn);
extern "C" int Cancel(PNCB pncb);
extern "C" int FormatNetbiosName(char *nbname, char *outname);

extern "C" char NbCommonErrorMsg[];