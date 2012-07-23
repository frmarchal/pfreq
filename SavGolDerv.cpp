//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "SavGolDerv.h"
#include "utils.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

int SavGolDervCalc(double *data,double *dest,int POrder,int Neigh)
{
  double *Filter, huge *Resp, intcp, slope;
  long int i;

   int convlv(double huge *data, long int n, double huge *respns, long int m, int isign, double huge *ans);
   int savgol(double huge *c, long int np, int nl, int nr, int ld, int m);

  if ((Filter = (double *)malloc(Spect->ipow2*sizeof(double)))==NULL)
   {
   WriteMsg(__FILE__,__LINE__,"Filter cannot be allocated");
   return(-1);
   }
  if ((Resp = (double *)malloc(2*Spect->ipow2*sizeof(double)))==NULL)
   {
   fpurge((void huge **)&Filter);
   WriteMsg(__FILE__,__LINE__,"Resp cannot be allocated");
   return(-1);
   }

  //***** remove a straight line from the data  *****
  intcp = *data;
  slope = (data[Spect->nop-1] - intcp) / (double)(Spect->nop-1);
  for (i=0 ; i<Spect->nop ; i++) dest[i]=data[i]-(intcp+slope*(double)i);
  for (i=Spect->nop ; i<Spect->ipow2 ; i++) dest[i] = 0.0;

  //***** calculate the coefficients of the filter *****
  if (savgol(Filter-1,Spect->ipow2,Neigh,Neigh,1,POrder)<0)
   {
   fpurge((void huge **)&Resp);
   fpurge((void huge **)&Filter);
   return(-1);
   }

  //***** apply the filter to the data *****
  if (convlv(dest-1,Spect->ipow2,Filter-1,2*Neigh+1,1,Resp-1)<0)
   {
   fpurge((void huge **)&Resp);
   fpurge((void huge **)&Filter);
   return(-1);
   }
  fpurge((void huge **)&Filter);

  //***** transfert the data to the final storage and re-add the background *****
  //for (i=0 ; i<Spect->nop ; i++) SavGolData[i]=Resp[i]+(intcp+slope*(double)i);
  for (i=0 ; i<Spect->nop ; i++) dest[i]=(Resp[i]+slope)/Spect->scan_speed;
  fpurge((void huge **)&Resp);
  return(0);
}

