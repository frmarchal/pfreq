//---------------------------------------------------------------------------
#ifndef SavGolH
#define SavGolH
//---------------------------------------------------------------------------
int SavGolDervCalc(double *Data,double **Derv,int NPoint,int POrder,int Neigh);
int SavGolSmooth(double *data,double *dest,int NPoint,int POrder,int Neigh);
#endif
