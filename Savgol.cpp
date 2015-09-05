#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include "Savgol.h"
#include "Utils.h"
#include "convlv.h"

/*=============================================================================*/
/*=============================================================================*/
void lubksb(double **a,int n,int *indx,double *b)
{
	int i,ii=0,ip,j;
	double sum;

	for (i=1;i<=n;i++)
	{
		ip=indx[i];
		sum=b[ip];
		b[ip]=b[i];
		if (ii)
			for (j=ii;j<=i-1;j++) sum -= a[i][j]*b[j];
		else if (sum) ii=i;
		b[i]=sum;
	}
	for (i=n;i>=1;i--)
	{
		sum=b[i];
		for (j=i+1;j<=n;j++) sum -= a[i][j]*b[j];
		b[i]=sum/a[i][i];
	}
}

#define TINY 1.0e-20;

/*=============================================================================*/
/*=============================================================================*/
int ludcmp(double **a, int n, int *indx,double *d)
{
	int i,imax=0,j,k;
	double big,dum,sum,temp;
	double *vv;
	static char SavGolDervCalc[]="ludcmp";

	vv=(double*)malloc((n+1)*sizeof(double))-1;
	*d=1.0;
	for (i=1;i<=n;i++)
	{
		big=0.0;
		for (j=1;j<=n;j++)
			if ((temp=fabs(a[i][j])) > big) big=temp;
		if (big == 0.0)
		{
			WriteMsg(SavGolDervCalc,__LINE__,"E0064: Singular matrix in routine LUDCMP");
			return(-1);
		}
		vv[i]=1.0/big;
	}
	for (j=1;j<=n;j++)
	{
		for (i=1;i<j;i++)
		{
			sum=a[i][j];
			for (k=1;k<i;k++) sum -= a[i][k]*a[k][j];
			a[i][j]=sum;
		}
		big=0.0;
		for (i=j;i<=n;i++)
		{
			sum=a[i][j];
			for (k=1;k<j;k++) sum -= a[i][k]*a[k][j];
			a[i][j]=sum;
			if ( (dum=vv[i]*fabs(sum)) >= big)
			{
				big=dum;
				imax=i;
			}
		}
		if (j != imax)
		{
			for (k=1;k<=n;k++)
			{
				dum=a[imax][k];
				a[imax][k]=a[j][k];
				a[j][k]=dum;
			}
			*d = -(*d);
			vv[imax]=vv[j];
		}
		indx[j]=imax;
		if (a[j][j] == 0.0) a[j][j]=TINY;
		if (j != n)
		{
			dum=1.0/(a[j][j]);
			for (i=j+1;i<=n;i++) a[i][j] *= dum;
		}
	}
	//free_vector(vv,1,n);
	free((void *)(vv+1));
	return(0);
}
#undef TINY

/*=============================================================================*/
/*!
  Smooth or differentiate the data

  \param c pointer to the data to smooth. Upon success, the buffer contains the
  smoothed data.
  \param np number of point to smooth
  \param nl number of neighbour to the left of the point
  \param nr number of neighbour to the right of the point
  \param ld 0 for smoothing; 1 for derivative
  \param m order of the smoothing polynomial (m=2 or m=4)

  \return 0 if ok; -1 if error
 */
/*=============================================================================*/
int savgol(double *c, long int np, int nl, int nr, int ld, int m)
{
	long kk;
	int j,mm;
	int *indx,ipj,k,imj;
	double d,fac,sum,**a,*b;
	static char SavGolDervCalc[]="savgol";

	if (np<nl+nr+1 || nl<0 || nr<0 || ld>m || nl+nr<m)
	{
		WriteMsg(SavGolDervCalc,__LINE__,"E0065: Bad args in savgol");
		return(-1);
	}
	indx=(int *)malloc((m+2)*sizeof(int))-1;
	a=(double **)malloc((m+2)*sizeof(double))-1;
	for (j=1 ; j<=m+1 ; j++) a[j]=(double *)malloc((m+2)*sizeof(double))-1;
	b=(double *)malloc((m+2)*sizeof(double))-1;
	for (ipj=0 ; ipj<=(m<<1) ; ipj++)
	{
		sum=(ipj ? 0.0 : 1.0);
		for (k=1 ; k<=nr ; k++) sum+=pow((double)k,(double)ipj);
		for (k=1 ; k<=nl ; k++) sum+=pow((double)-k,(double)ipj);
		mm=std::min(ipj,2*m-ipj);
		for (imj=-mm ; imj<=mm ; imj+=2) a[1+(ipj+imj)/2][1+(ipj-imj)/2]=sum;
	}
	ludcmp(a,m+1,indx,&d);
	for (j=1 ; j<=m+1 ; j++) b[j]=0.0;
	b[ld+1]=1.0;
	lubksb(a,m+1,indx,b);
	for (kk=1 ; kk<=np ; kk++) c[kk]=0.0;
	for (k=-nl ; k<=nr ; k++)
	{
		sum=b[1];
		fac=1.0;
		for (mm=1 ; mm<=m ; mm++) sum+=b[mm+1]*(fac*=k);
		kk=((np-(long)k)%np)+1;
		c[kk]=sum;
	}
	free((void *)(b+1));
	for (j=1 ; j<=m+1 ; j++) free((void*)(a[j]+1));
	free((void *)(a+1));
	free((void *)(indx+1));
	return(0);
}

/*=============================================================================*/
/*!
  Calculate the derivative of a curve using the Savitsky and Golay
  algorithm.
 */
/*=============================================================================*/
int SavGolDervCalc(double *Data,double **Derv,int NPoint,int POrder,int Neigh)
{
	double *Filter, *Resp, *Temp, intcp, slope;
	long int i,ipow2;

	//***** initialize the parameter *****
	if (*Derv)
	{
		free(*Derv);
		*Derv=NULL;
	}
	*Derv=(double *)malloc(NPoint*sizeof(double));
	if (*Derv==NULL)
	{
		WriteMsg(__FILE__,__LINE__,"Derivative cannot be allocated");
		return(-1);
	}


	for (ipow2=8 ; ipow2<NPoint ; ipow2<<=1);
	ipow2<<=1;

	if ((Filter = (double *)calloc(ipow2, sizeof(double)))==NULL)
	{
		WriteMsg(__FILE__,__LINE__,"E0066: Filter cannot be allocated");
		return(-1);
	}
	if ((Resp = (double *)calloc(2*ipow2, sizeof(double)))==NULL)
	{
		free(Filter);
		WriteMsg(__FILE__,__LINE__,"E0067: Resp cannot be allocated");
		return(-1);
	}
	if ((Temp = (double *)calloc(2*ipow2, sizeof(double)))==NULL)
	{
		free(Resp);
		free(Filter);
		WriteMsg(__FILE__,__LINE__,"E0068: Temp cannot be allocated");
		return(-1);
	}

	//***** remove a straight line from the data  *****
	intcp = *Data;
	slope = (Data[NPoint-1] - intcp) / (double)(NPoint-1);
	for (i=0 ; i<NPoint ; i++) Temp[i]=Data[i]-(intcp+slope*(double)i);
	for (i=NPoint ; i<ipow2 ; i++) Temp[i] = 0.0;

	//***** calculate the coefficients of the filter *****
	if (savgol(Filter-1,ipow2,Neigh,Neigh,1,POrder)<0)
	{
		free(Temp);
		free(Resp);
		free(Filter);
		return(-1);
	}

	//***** apply the filter to the data *****
	if (convlv(Temp-1,ipow2,Filter-1,2*Neigh+1,1,Resp-1)<0)
	{
		free(Temp);
		free(Resp);
		free(Filter);
		return(-1);
	}
	free(Filter);
	free(Temp);

	//***** transfert the data to the final storage and re-add the background *****
	//for (i=0 ; i<Spect->nop ; i++) SavGolData[i]=Resp[i]+(intcp+slope*(double)i);
	for (i=0 ; i<NPoint ; i++) (*Derv)[i]=(Resp[i]+slope);
	free(Resp);
	return(0);
}

/*=============================================================================*/
/*!
  Description: Smooth a curve using the Savitsky and Golay algorithm.
 */
/*=============================================================================*/
int SavGolSmooth(double *data,double *dest,int NPoint,int POrder,int Neigh)
{
	double *Filter, *Resp, *Temp, intcp, slope;
	long int i,ipow2;

	for (ipow2=8 ; ipow2<NPoint ; ipow2<<=1);
	ipow2<<=1;

	if ((Filter = (double *)calloc(ipow2, sizeof(double)))==NULL)
	{
		WriteMsg("SavGolDervCalc",__LINE__,"E0066: Filter cannot be allocated");
		return(-1);
	}
	if ((Resp = (double *)calloc(2*ipow2, sizeof(double)))==NULL)
	{
		free(Filter);
		WriteMsg("SavGolDervCalc",__LINE__,"R0067: Resp cannot be allocated");
		return(-1);
	}
	if ((Temp = (double *)calloc(2*ipow2, sizeof(double)))==NULL)
	{
		free(Resp);
		free(Filter);
		WriteMsg("SavGolDervCalc",__LINE__,"E0068: Temp cannot be allocated");
		return(-1);
	}

	//***** remove a straight line from the data  *****
	intcp = *data;
	slope = (data[NPoint-1] - intcp) / (double)(NPoint-1);
	for (i=0 ; i<NPoint ; i++) Temp[i]=data[i]-(intcp+slope*(double)i);
	for (i=NPoint ; i<ipow2 ; i++) Temp[i] = 0.0;

	//***** calculate the coefficients of the filter *****
	if (savgol(Filter-1,ipow2,Neigh,Neigh,0,POrder)<0)
	{
		free(Temp);
		free(Resp);
		free(Filter);
		return(-1);
	}

	//***** apply the filter to the data *****
	if (convlv(Temp-1,ipow2,Filter-1,2*Neigh+1,1,Resp-1)<0)
	{
		free(Temp);
		free(Resp);
		free(Filter);
		return(-1);
	}
	free(Filter);
	free(Temp);

	//***** transfert the data to the final storage and re-add the background *****
	//for (i=0 ; i<Spect->nop ; i++) SavGolData[i]=Resp[i]+(intcp+slope*(double)i);
	for (i=0 ; i<NPoint ; i++) dest[i]=(Resp[i]+slope*(double)i+intcp);
	free(Resp);
	return(0);
}
