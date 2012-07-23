//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <stdlib.h> 
#include "Utils.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: WriteMsg
//
// Type: Function
//
// Applies To :
//
// Description: Display a message and print the calling function name and line
//              number in the title.
//
//
//
// Usage: void WriteMsg(char *FuncName,int LineNum,char *Message)
//
//    -@- Char *FuncName : Name of the calling function.
//    -@- int LineNum : Line number from where this function is called.
//    -@- char *Message : Message to display.
//
// Returns:
//
// Remarks:
//
//
// System: MATLAB - Win98
// Author: Marchal F
//
// Date: 18/10/1999
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void WriteMsg(char *FuncName,int LineNum,char *Message,...)
{
  char Title[80];
  char Text[800],*Ptr;
  va_list ap;

  Ptr=strrchr(FuncName,'\\');
  if (Ptr)
   Ptr++;
  else
   Ptr=FuncName;
  sprintf(Title,"%s (%d)",Ptr,LineNum);
  va_start(ap,Message);
  vsprintf(Text,Message,ap);
  va_end(ap);
  Application->MessageBox(Text,Title,MB_OK | MB_TASKMODAL);
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: Purge
//
// Type: Function
//
// Applies To :
//
// Description: Free a pointer to a double and reset the pointer to NULL.
//
//
//
// Usage: void Purge(double &Ptr)
//
//    -@- double &Ptr: Pointer to purge.
//
// Returns:
//
// Remarks:
//
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 21/11/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void Purge(double *&Ptr)
{
  if (!Ptr) return;
  free(Ptr);
  Ptr=NULL;
}
