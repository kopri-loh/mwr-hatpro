#include  <signal.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <time.h>

#include  <math.h>

#include  <fcntl.h>
#include  <sys/types.h>
#include  <errno.h>
#include  <sys/stat.h>
#include  <unistd.h>

#define  MAXLEV  3000
#define  MAXREC  6000
#define  dbg  1

// created: Holger Deckelmann. Dezember 2011
// CHANGES: Holger Deckelmann, 1.11.2013 , HPC file format lesbar gemacht
//  3.2.21 Holger, MAXREC=6000, Speichermanagement :malloc 

  
void  swap  (  char  *p)  {
  char  *mem;
  

  *mem=*p;
  *p=*(p+2);
  *(p+2)=*mem;

  *mem=*(p+1);
  *(p+1)=*(p+3);
  *(p+3)=*mem;
}

int  main  (int  argc,  char  **argv)
{
    int  i,j,k,  bytesok,msec;
    int  *code,  *nrec,  *TRef,*Retrieval,*AltAnz;
    float  *Min,*Max,  *x;
    int  recsize,p0;
   
    int *ival, Altitude[MAXREC], Time[MAXREC];


  float Freq[MAXREC];

		//  float  Temp[MAXREC][MAXLEV];
  //float  aHUM[MAXREC][MAXLEV];
		//float  rHUM[MAXREC][MAXLEV];
		float *rHUM[MAXLEV];
		float *aHUM[MAXLEV];
		float *Temp[MAXLEV];
  float  IWV_LWP[MAXREC];
  unsigned  char  RF[MAXREC];
  float vz,elev[MAXREC],azim[MAXREC];
    int  ang;

    FILE  *f,  *log,  *of;
    char  outfile[512],ch,fn[128];
				//    unsigned  char  buf[1000000];
				unsigned char *buf;
				
				
   

    printf ("Radiometer V1.1 / 3.2.2021\n");

				for ( i=0;i<MAXLEV;i++) {
									rHUM[i]=(float *)malloc(MAXREC*sizeof(float));
									aHUM[i]=(float *)malloc(MAXREC*sizeof(float));
									Temp[i]=(float *)malloc(MAXREC*sizeof(float));
				}
				
				buf=(unsigned char*) malloc(1000000);
				
    *argv++;  argc--;
    if  (  argc<2  )  {
      printf  ("\n  radiometer      <input-file>  <output-file>  \n");
      strcpy(fn,"zip/stand_obs_110_111130_161440.TPC");
      strcpy(outfile,"out.dat");
      //                exit(1);
    }
    else  {
      strcpy(fn,  *argv++);
      strcpy(outfile,  *argv++);
    }
    printf  ("RADIOMETER  process  %s\n",fn);

    f=fopen(fn,  "rt");

    if ( !f ) {printf ("No File %s\n",fn);exit(1);}
    k=fread((void  *)&buf[0],  1,  1000000,  f);
      printf  ("      %d  bytes  read  \n",k);

     fclose(f);

    if  (  k<10)  exit(0);
    //    for  (  i=0;i<4;i++)  printf  ("buf[%d]=  %x\n",i,buf[i]);
    //  DOS  ->  UNIX  ??
    //for  (  i=0;i<7;i+=4  )  swap((char  *)&buf[i]);

    //     swap  ((char  *)&buf[0]);
    //    for  (  i=0;i<4;i++)  printf  ("buf[%d]=  %x\n",i,buf[i]);
    code=(int*)&buf[0];
 


    nrec=(int*)&buf[4];
    Min=(float*)&buf[8];
    Max=(float*)&buf[12];
    TRef=(int*)&buf[16];

    
    if  (  dbg  )  {
     printf  ("      %d  Recs\n      Min:          %f\n      Max:          %f\n      TRef:      %d",*nrec,*Min,*Max,*TRef);
    printf  ("      FILE  CODE        :%d\n",  *code);
    }

    if ( *nrec > MAXREC ) {
      printf ("es sind mehr als %d records im File (%d). Bitte Programmcode anpassen, MAXREC vergroessern!\n",MAXREC,*nrec);
      exit(1);
    }
     
    switch  (  *code  )  {
    case  671112496:  //IRT    
      AltAnz=(  int*)&buf[20];
      p0=24+4*  *AltAnz;

      break;
    case  780798065:    //  TPC
    case  459769847:    //  TPB
    case  117343673:    //  HPC  mit  RH
    case  4567:    //  LPR
      Retrieval=(int*)&buf[20];
      AltAnz=(  int*)&buf[24];
      p0=28+4*  *AltAnz;
      break;
    default:
      AltAnz=(  int*)&buf[24];
      
      break;
    }




    
    printf ("RD-START\n");
      
    switch  (  *code  )  {
    case  934501978:    //  LWP
    case  594811068:    //  IWV
      p0=24;
      recsize=13;
      for  (  i=0;i<*nrec;i++)  {
        ival=(int*)&buf[p0+recsize*i];
        Time[i]=(*ival)  %  86400;    
        RF[i]=buf[p0+recsize*i+4];
        x=(float  *)&buf[p0+recsize*i  +5  ];
        IWV_LWP[i]=*x;
        x=(float *)&buf[p0+recsize*i  +9  ];
        ang=(int)  *x;

        vz=1.0;
        if  (  ang<0)  {vz=-1.0;ang=-ang;}
        azim[i]=ang/1000;
        elev[i]=(*x*vz-azim[i]*1000.0)  *  vz;

      }

      of=fopen(outfile,"at");
      for  (  i=0;i<*nrec;i++)  {
        //        printf  (  "%f  %d  %f\n",(float)Time[i]/3600.0,Altitude[j],Temp[i][j]);
        fprintf  (of,  "%f  %f  %f %f\n",(float)Time[i]/3600.0,elev[i],azim[i],IWV_LWP[i]);
      }
      fclose(of);
      if ( dbg) printf ("FILE OUT OK, LWP,IWV\n");

      break;
      //--------------------------------------------------------------------------------------------------------------------
    case  671112496:  //IRT
						printf ("IRT erkannt\n");
						
      recsize=9+4* *AltAnz;
      for  (  i=0;i<*AltAnz;i++)  {
      x=(float  *)&buf[24+i*4];
      Freq[i]=*x;
      }

      for  (  i=0;i<*nrec;i++)  {
        ival=(int  *)&buf[p0+i*recsize];
        Time[i]=(*ival)  %  86400;    
        RF[i]=buf[p0+recsize*i+4];

        x=(float  *)&buf[p0+recsize*i  +5  +  *AltAnz*4];
        
        ang=(int)  *x;
        vz=1.0;
        if  (  ang<0)  {vz=-1.0;ang=-ang;}
        azim[i]=ang/1000;
        elev[i]=(*x*vz - azim[i]*1000.0)  *  vz;

								        if ( dbg) printf ("IRT %d/%d Time %f ang: %f El %f  Az %f\n",i,*nrec,(float)Time[i]/3600.0,*x,elev[i],azim[i] );



        for  (  j=0;j<*AltAnz;j++)  {
          x=(float  *)&buf[p0+recsize*i  +5  +  j*4];
          Temp[j][i]=*x;
        }  
      }

      of=fopen(outfile,"at");

      for  (  j=0;j<*AltAnz;j++)  {
      for  (  i=0;i<*nrec;i++)  {
								//                printf  (  "%f  %d  %f\n",(float)Time[i]/3600.0,Altitude[j],Temp[i][j]);
        fprintf  (of,  "%f  %f %f  %f %f\n",(float)Time[i]/3600.0,Freq[j],elev[i],azim[i],Temp[j][i]);
      }
      }
      fclose(of);
 
      break;

    case  780798065:    //  TPC
    case  459769847:    //  TPB
    case  4567:    //  LPR

    for  (  i=0;i<  *AltAnz;i++)  {
      ival=(int  *)&buf[28+i*4];
      Altitude[i]=*ival;
      //      printf  ("ALT[%d]:    %d\n",i,*ival);
    }

      printf  ("      Levels            :  %d\n",*AltAnz);
    recsize=5+4*  *AltAnz;
    for  (  i=0;i<*nrec;i++)  {
      ival=(int*)&buf[p0+recsize*i];
      Time[i]=(*ival)  %  86400;    

      RF[i]=buf[p0+recsize*i+4];
      for  (  j=0;j<*AltAnz;j++)  {
        x=(float  *)&buf[p0+recsize*i  +5  +  j*4];
          Temp[i][j]=*x;
         if  (  *code  ==  4567)  ;
          //         Temp[i][j]*=    1000000.0;
          else
            Temp[i][j]-=    273.15;
      }
    }

    of=fopen(outfile,"at");
    for  (  i=0;i<*nrec;i++)  
      for  (  j=0;j<*AltAnz;j++)  {
        //        printf  (  "%f  %d  %f\n",(float)Time[i]/3600.0,Altitude[j],Temp[i][j]);
        fprintf  (of,  "%f  %d  %f\n",(float)Time[i]/3600.0,Altitude[j],Temp[i][j]);
      }
    fclose(of);


    strcat  (outfile,  ".rain");
    of=fopen(outfile,"at");
    for  (  i=0;i<*nrec;i++)  
        fprintf  (of,  "%f  %d  %d\n",(float)Time[i]/3600.0,RF[i]&1,  (RF[i]&6)>>2);
    fclose(of);

      break;

    case  117343673:    //  HPC  mit  RH

      printf ("HPC mit RH. Altitude-Anzahl: %d\n",*AltAnz);

    for  (  i=0;i<  *AltAnz;i++)  {
      ival=(unsigned int *)&buf[28+i*4];
      Altitude[i]=*ival;
    }

      recsize=5+4*  *AltAnz;    //  erstmal

    for  (  i=0;i<*nrec;i++)  {

      ival=(int*)&buf[p0+recsize*i];

      Time[i]=(*ival)  %  86400;    

      RF[i]=buf[p0+recsize*i+4];
      for  (  j=0;j<*AltAnz;j++)  {

        x=(float  *)&buf[p0+recsize*i  +5  +  j*4];
        aHUM[j][i]=*x  ;
      }
    }


    //  p0  setzen  auf  bereich  hinter  abs  feuchte:
    p0=p0+recsize*  *nrec+2*4  ;  //  RHmin,  RHmax
    
    //  recsize  bleibt  5+level*4
    for  (  i=0;i<*nrec;i++)  {
      ival=(int*)&buf[p0+recsize*i];
      Time[i]=(*ival)  %  86400;    


      RF[i]=buf[p0+recsize*i+4];
      for  (  j=0;j<*AltAnz;j++)  {
        x=(float  *)&buf[p0+recsize*i  +5  +  j*4];
        rHUM[j][i]=*x  ;
      }
    }


    of=fopen(outfile,"at");
    for  (  i=0;i<*nrec;i++)  
      for  (  j=0;j<*AltAnz;j++)  {
        //        printf  (  "%f  %d  %f\n",(float)Time[i]/3600.0,Altitude[j],Temp[i][j]);
        fprintf  (of,  "%f  %4d  %12.6f  %12.6f  %d  %d\n",(float)Time[i]/3600.0,Altitude[j],rHUM[j][i],aHUM[j][i],RF[i]&1,  (RF[i]&6)>>2);
      }
    fclose(of);



    break;



    default:
      printf  ("UNKOWN  FILE\n");
      exit(0);
    }

    exit(0);

}
//------------------------------------------------------
