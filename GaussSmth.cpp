#include <stdlib.h>
#include <math.h>
#include "GaussSmth.h"
#include "Utils.h"

/*=============================================================================*/
/*!
  Smooth the data using a gaussian smooth algorithm.
 */
/*=============================================================================*/
int CalcGaussSmooth(double *Data,double XFreq,double **Smooth,int NPoints,double Width,int Neigh)
{
	long i,j;
	double Num,Den,a,XCoef;
	double *ExpTbl;

	//***** initialize the parameter *****
	Purge(*Smooth);
	*Smooth=(double *)malloc(NPoints*sizeof(double));
	if (*Smooth==NULL)
	{
		WriteMsg(__FILE__,__LINE__,"GaussData cannot be allocated");
		return(-1);
	}
	if (Neigh>NPoints) Neigh=NPoints;
	ExpTbl=(double *)malloc(Neigh*sizeof(double));
	if (ExpTbl==NULL)
	{
		Purge(*Smooth);
		WriteMsg(__FILE__,__LINE__,"GaussData cannot be allocated");
		return(-1);
	}
	XCoef=1./Width/XFreq;
	for (i=0 ; i<Neigh ; i++)
	{
		a=(double)i*XCoef;
		ExpTbl[i]=exp(-a*a);
	}

	//***** transfert the data to the final storage and re-add the background *****
	for (i=0 ; i<NPoints ; i++)
	{
		Num=Data[i];
		Den=1.;
		for (j=1 ; j<=Neigh ; j++)
		{
			if (i-j>=0)
			{
				Num+=ExpTbl[j]*Data[i-j];
				Den+=ExpTbl[j];
			}
			if (i+j<NPoints)
			{
				Num+=ExpTbl[j]*Data[i+j];
				Den+=ExpTbl[j];
			}
		}
		(*Smooth)[i]=Num/Den;
	}

	return(0);
}
