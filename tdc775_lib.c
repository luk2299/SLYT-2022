// adc792 -> tdc775 , V792N -> V775N , adc -> tdc
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h> 
#include <time.h> 
#include "CAENVMElib.h"
#include "CAENVMEtypes.h" 
#include "CAENVMEoslib.h"
#include "tdc775_lib.h"
#include <iostream>

using namespace std;

/*-------------------------------------------------------------*/

unsigned short init_tdc775(int32_t BHandle) {

  int status=1;
  unsigned long address;
  unsigned long  DataLong;
  //  int nZS = 1;
  int caenst;

  /* QDC Reset */
  address = V775N_ADDRESS + 0x1000;
  //  status = vme_read_dt(address, &DataLong, AD32, D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(status != 1) {
    printf("Error READING V775N firmware -> address=%lx \n", address);
    return status;
  }
  else {
    if(tdc775_debug) printf("V775N firmware is version:  %lx \n", DataLong);
  }
  
  //Bit set register
  address = V775N_ADDRESS + 0x1006;
  DataLong = 0x80; //Issue a software reset. To be cleared with bit clear register access
  //  status = vme_write_dt(address,&DataLong, AD32, D16);
  //status = vme_read_dt(address, &DataLong, AD32, D16);
  caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  caenst = CAENVME_ReadCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(status != 1) {
    printf("Bit Set Register read: %li\n", DataLong);
  }

  //Control Register: enable BLK_end
  address = V775N_ADDRESS + 0x1010;
  DataLong = 0x4; //Sets bit 2 to 1 [enable blkend]
  //  status = vme_write_dt(address,&DataLong, AD32, D16);
  //status = vme_read_dt(address, &DataLong, AD32, D16);
  caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  caenst = CAENVME_ReadCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(status != 1) {
    printf("Bit Set Register read: %li\n", DataLong);
  }

  //Bit clear register
  address = V775N_ADDRESS + 0x1008;
  DataLong = 0x80; //Release the software reset. 
  //  status = vme_write_dt(address,&DataLong, AD32, D16);
  //status = vme_read_dt(address, &DataLong, AD32, D16);
  caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  caenst = CAENVME_ReadCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(status != 1) {
    printf("Bit Clear Register read: %li\n", DataLong);
  }

  //Bit set register 2 according to header file values

  address = V775N_ADDRESS + 0x1032;
  caenst = CAENVME_ReadCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  if(tdc775_debug) printf("Register 2 of TDC V775N before writing is at %lx", DataLong);
  DataLong = 0x5880 + tdc775_OVFSUP + tdc775_ZROSUP + tdc775_STASTOP;
    caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(status != 1) {
    printf("ERROR setting register 2 as %lx", DataLong);
  }
  caenst = CAENVME_ReadCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  if(tdc775_debug) printf("Register 2 of TDC V775N after writing is at %lx", DataLong);




/*   //Enable/disable zero suppression, overflow sup., common start-stop */
/*   if(nZS) { */
/*     address = V775N_ADDRESS + 0x1032; */
/*     //    DataLong = 0x18; //Disable Zero Suppression + disable overfl */
/*     DataLong = 0x218; //Disable Zero Suppression + disable overfl + com stop */
/*     //    status = vme_write_dt(address,&DataLong, AD32, D16); */
/*     caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16); */
/*     status *= (1-caenst);  */
/*     if(status != 1) { */
/*       printf("Could not disable ZS: %li\n", DataLong); */
/*     } */
/*   } else { */
/*     address = V775N_ADDRESS + 0x1032; */
/*     //    DataLong = 0x88; //Enable Zero Suppression + disable overfl */
/*     DataLong = 0x88; //Enable Zero Suppression + disable overfl + com stop */
/*     //  status = vme_write_dt(address,&DataLong, AD32, D16); */
/*     caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16); */
/*     status *= (1-caenst);  */
/*     if(status != 1) { */
/*       printf("Could not disable ZS: %li\n", DataLong); */
/*     } */
/*   } */

  //Set the thresholds.
  for(int i=0; i<16; i++) {
    address = V775N_ADDRESS + 0x1080 +4*i;
    if(tdc775_debug) printf(
	   "Address of the thr register %i : address = %lx\n",i,address);
    DataLong = tdc775_THR; //Threshold
    //status = vme_write_dt(address, &DataLong, AD32, D16);
    caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
    status *= (1-caenst); 
    if(tdc775_debug) printf("Threshold set: %li\n", DataLong);
    if(status != 1) {
      printf("Threshold register read: %li\n", DataLong);
    }
  }
  
  //Set the LSB value according to tdc775_LSB in header file
  address = V775N_ADDRESS + 0x1060;
  DataLong = tdc775_LSB; 
  //  status = vme_read_dt(address, &DataLong, AD32, D16);
  caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  caenst = CAENVME_ReadCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(tdc775_debug) printf("LSB register read: %li\n", DataLong);
  if(status != 1) {
    printf("LSB register read: %li\n", DataLong);
  }
  return status;
 }


/*------------------------------------------------------------------*/

vector<int> read_tdc775(int32_t BHandle, short int* pstatus)
{
  /*
    reading of the V775N 
    returns vector with output
  */
  vector<int> tdc_val; tdc_val.clear(); tdc_val.resize(V775N_CHANNEL);
  vector<int> outD;
  int caenst;
  unsigned long address, data, dt_type;
  unsigned long tdc775_rdy, tdc775_busy, evtnum;
  unsigned long full,empty, evc_lsb, evc_msb, tdc_value, tdc_chan;
  unsigned int ncha;
  int status = 1; 

  /*
    check if the fifo has something inside: use status register 1
   */  
  address = V775N_ADDRESS + tdc775_shift.statusreg1;
  //status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status = (1-caenst); 

  if(tdc775_debug) printf("ST (str1) :: %i, %lx, %lx \n",status,address,data);
  tdc775_rdy = data & tdc775_bitmask.rdy;
  tdc775_busy = data & tdc775_bitmask.busy;

  //Trigger status
  address = V775N_ADDRESS + 0x1020;
  //status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(tdc775_debug) printf("ST (trg1) :: %i, %lx, %lx \n",status,address,data);


  //Event counter register
  address = V775N_ADDRESS + 0x1024;
  //  status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  evc_lsb = data;
  address = V775N_ADDRESS + 0x1026;
  // status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  evc_msb = data & 0xFF;
  
  if(tdc775_rdy == 1 && tdc775_busy == 0) {
    /*
      Data Ready and board not busy
    */
    
    //Read the Event Header
    address = V775N_ADDRESS;
    //status = vme_read_dt(address,&data,AD32,D32);
    caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
    status *= (1-caenst); 
    if(tdc775_debug) printf("Data Header :: %i, %lx, %lx \n",status,address,data);
    ncha =  data>>8 & 0x3F;
    if(tdc775_debug) cout<<"Going to Read "<<ncha<<" channels!"<<endl;
    full = 0; empty = 0;
    while(!full && !empty) {

      //Read MEB for each channel and get the TDC value
      address = V775N_ADDRESS;
      //status = vme_read_dt(address,&data,AD32,D32);
      caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
      status *= (1-caenst); 
      if(tdc775_debug) printf("Reading and got:: %i %lx %lx\n", status, data, address);

      dt_type = data>>24 & 0x7;
      if(tdc775_debug) printf("typ:: %lx\n", dt_type);

      if(!(dt_type & 0x7)) {
	tdc_value = data & 0xFFF;
	tdc_chan = data>>17 & 0xF;
	if(tdc775_debug) cout<<tdc_value<<" "<<tdc_chan<<" "<<endl;
	tdc_val[tdc_chan] = tdc_value;
	if(data>>12 & 0x1) cout<<" Overflow, my dear !! "<<tdc_value<<endl;
      } else if(dt_type & 0x4) {
	//EOB
	evtnum = data & 0xFFFFFF;
	if(tdc775_debug) cout<<"EvtNum "<<evtnum<<endl;
      } else if(dt_type & 0x2) {
	//Header
	if(tdc775_debug) cout<<" ERROR:: THIS SHOULD NOT HAPPEN!!!"<<endl;
      }

      //Check the status register to check the MEB status
      address = V775N_ADDRESS + tdc775_shift.statusreg2;
      //status = vme_read_dt(address,&data,AD32,D16);
      caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
      status *= (1-caenst); 
  
      full = data &  tdc775_bitmask.full;
      empty = data &  tdc775_bitmask.empty;
      if(full) {
	cout<<"MEB Full:: "<<full<<" !!!"<<endl;
      }
    }

    //Write event in output vector
    for(int iC = 0; iC<(int)tdc_val.size(); iC++) {
      outD.push_back(tdc_val[iC]);
    }
  }
  *pstatus = (short int) status;
  return outD;
}

/*-------------------------------------------------------------*/


vector<int> readFasttdc775(int32_t BHandle, short int* pstatus)
{
  /*
    Implements Block Transfer Readout of V775N 
    returns vector with output
  */
  int nbytes_tran = 0;
  vector<int> outD; outD.clear();
  int status = 1, caenst=0;
  unsigned long data,address;
  unsigned long dataV[34]; int wr;
  unsigned long tdc775_rdy, tdc775_busy;
  unsigned int ncha, idV;
  struct timeval tv;
  time_t curt, curt2, tdiff, tdiff1, curt3;
  /*
    check if the fifo has something inside: use status register 1
  */  
  
  if(tdc775_debug) {
    gettimeofday(&tv, NULL); 
    curt3=tv.tv_usec;
  }

  address = V775N_ADDRESS + tdc775_shift.statusreg1;
  //status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(tdc775_debug) printf("ST (str1) :: %i, %lx, %lx \n",status,address,data);
  tdc775_rdy = data & tdc775_bitmask.rdy;
  tdc775_busy = data & tdc775_bitmask.busy;

  //Event counter register
  if(tdc775_debug) {
    gettimeofday(&tv, NULL); 
    curt=tv.tv_usec;
  }

  if(tdc775_rdy == 1 && tdc775_busy == 0) {
    /*
      Data Ready and board not busy
    */

    //Read the Event Header
    address = V775N_ADDRESS;
    //status = vme_read_dt(address,&data,AD32,D32);
    caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
    status *= (1-caenst); 
    //I put the header in
    outD.push_back((int)data);

    if(tdc775_debug) printf("Data Header :: %i, %lx, %lx \n",status,address,data);
    ncha =  data>>8 & 0x3F;
    if(tdc775_debug) cout<<"Going to Read "<<ncha<<" channels!"<<endl;

    //Vector reset
    idV = 0; while(idV<34) { dataV[idV] = 0; idV++; }
    wr = (ncha+1)*4;
    //    printf("numchann %d \n",ncha);

    //status *= vme_read_blk(address,dataV,wr,AD32,D32);    
    caenst = CAENVME_BLTReadCycle(BHandle,address,dataV,wr,
				  cvA32_U_DATA,cvD32,&nbytes_tran);
    status *= (1-caenst); 

    //Vector dump into output
    idV = 0; while(idV<ncha+1) { outD.push_back((int)dataV[idV]); idV++;  }
  }

  if(tdc775_debug) {
    gettimeofday(&tv, NULL); 
    curt2=tv.tv_usec;
    tdiff = (curt2-curt);
    tdiff1 = (curt-curt3);
    cout<<"Just BLT "<<curt<<" "<<curt2<<" "<<tdiff<<" "<<tdiff1<<endl;
  }
  *pstatus = (short int) status; 
  return outD;
}
/*-------------------------------------------------------------*/


unsigned short read_tdc775_simple(int32_t BHandle, int *pDataTdc)
{

  /*
    reading of the V775N returns pointer to vector with output
  */
  int tdc_val[V775N_CHANNEL]={0};
  int status=0, caenst=0;
  unsigned long address, data, dt_type;
  unsigned long tdc775_rdy, tdc775_busy, evtnum;
  unsigned long full,empty, evc_lsb, evc_msb, tdc_value, tdc_chan;
  unsigned int ncha;
  status = 1; 

  /*
    check if the fifo has something inside: use status register 1
   */  
  address = V775N_ADDRESS + tdc775_shift.statusreg1;
  //status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status = (1-caenst); 

  if(tdc775_debug) printf("ST (str1) :: %i, %lx, %lx \n",status,address,data);
  tdc775_rdy = data & tdc775_bitmask.rdy;
  tdc775_busy = data & tdc775_bitmask.busy;

  //Trigger status
  address = V775N_ADDRESS + 0x1020;
  //status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(tdc775_debug) printf("ST (trg1) :: %i, %lx, %lx \n",status,address,data);


  //Event counter register
  address = V775N_ADDRESS + 0x1024;
  //  status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  evc_lsb = data;
  address = V775N_ADDRESS + 0x1026;
  // status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  evc_msb = data & 0xFF;
  
  if(tdc775_rdy == 1 && tdc775_busy == 0) {
    /*
      Data Ready and board not busy
    */
    
    //Read the Event Header
    address = V775N_ADDRESS;
    //status = vme_read_dt(address,&data,AD32,D32);
    caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
    status *= (1-caenst); 
    if(tdc775_debug) printf("Data Header :: %i, %lx, %lx \n",status,address,data);
    ncha =  data>>8 & 0x3F;
    if(tdc775_debug) cout<<"Going to Read "<<ncha<<" channels!"<<endl;
    full = 0; empty = 0;
    while(!full && !empty) {

      //Read MEB for each channel and get the TDC value
      address = V775N_ADDRESS;
      //status = vme_read_dt(address,&data,AD32,D32);
      caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
      status *= (1-caenst); 
      if(tdc775_debug) printf("Reading and got:: %i %lx %lx\n", status, data, address);

      dt_type = data>>24 & 0x7;
      if(tdc775_debug) printf("typ:: %lx\n", dt_type);

      if(!(dt_type & 0x7)) {
	tdc_value = data & 0xFFF;
	tdc_chan = data>>17 & 0xF;
	if(tdc775_debug) cout<<tdc_value<<" "<<tdc_chan<<" "<<endl;
	tdc_val[tdc_chan] = tdc_value;
	if(data>>12 & 0x1) cout<<"TDC Overflow, my dear !! "<<tdc_value<<endl;
      } else if(dt_type & 0x4) {
	//EOB
	evtnum = data & 0xFFFFFF;
	if(tdc775_debug) cout<<"EvtNum "<<evtnum<<endl;
      } else if(dt_type & 0x2) {
	//Header
	if(tdc775_debug) cout<<" ERROR:: THIS SHOULD NOT HAPPEN!!!"<<endl;
      }

      //Check the status register to check the MEB status
      address = V775N_ADDRESS + tdc775_shift.statusreg2;
      //status = vme_read_dt(address,&data,AD32,D16);
      caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
      status *= (1-caenst); 
  
      full = data &  tdc775_bitmask.full;
      empty = data &  tdc775_bitmask.empty;
      if(full) {
	cout<<"MEB Full:: "<<full<<" !!!"<<endl;
      }
    }

    //Write event in output vector
    for(int iC = 0; iC<V775N_CHANNEL; iC++) {
      *(pDataTdc+iC)=tdc_val[iC];
    }
  }
  
  return status;
}
