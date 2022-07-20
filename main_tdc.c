//file nuovo

/* ---------------------------------------------------------------------------
     * ** main_tdc.c

     * Questo programma permette di comunicare con l'elettronica VME (V792, V1718)
     * della CAEN utilizzando le CaenVMELIB per inizializzare e acquisire i dati.
     *
     * Opzioni 
     * 
     * -q <filename> salva l'output del qdc in formato testuale nel file filename
     *  (obbligatorio)
     * -q <filename> salva l'output del tdc in formato testuale nel file filename
     *  (obbligatorio)
     *
     * -n <nevents> imposta il numero di eventi da acquisire
     * 
     * -p <IPED> imposta la corrente di piedistallo
     *
     * -T <time> imposta il tempo di acquisizione
     *
     * -r genera automaticamente il file di root con nome <filename>.root
     *
     * -v aumenta la verbosità del programma fornendo informazioni utili per i debug
     *  ATTENZIONE l'aumento della verbosità diminuisce la rate massima
     *  d'acquisizione non utilizzare -v durante la presa dati ma solo per
     *  effettuare debug
     *
     * il programma dovra' ricevere almeno o il numero di eventi o il tempo di
     * acquisizione se vengono forniti tutti e due il programma termina quando una
     * delle due condizioni viene raggiunta ignorando l'altra. Per arrestare il
     * programma manualmente premere Ctrl+c per deinizializzare correttamente
     * l'elettronica e salvare i dati acquisiti sul disco.
     *
     * Il presente programma e' rilasciato "as is" gli autori non si assumono
     * nessuna responsabilita' per aventuali malfunzionamenti o bug
     * 
     *NOTA: clear_veto fa in modo che il segnale di veto in uscita corrisponda allo zero NIM logico;
     *      set_veto fa in modo che il segnale di veto in uscita corrisponda all'1 NIM logico.
     *      Le funzioni clear_veto e set_veto restituiscono un valore int pari ad 1 se le rispettive operazioni sono andate a buon fine,
     *      altro in caso contrario.
     *
     * **
     * ** Author: Daniele Racanelli, Flavio Pisani, Paolo Cretaro
     * **Coauthor: Gaia Franciosini, Luca Ceccarelli, Pippo Franco, Alessandro Santoni
     * **questi hanno solo aggiunto qualche commento qui e li
     *
     * chi ha perso la testa per includere il tdc:
     * Nicola Alborè, Maria Adriana Sabia, Federica Troni



     * -------------------------------------------------------------------------*/
    #include <string.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <ctype.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <getopt.h>
    #include <sys/ioctl.h>
    #include <sys/time.h>
    #include <time.h>
    #include <signal.h>
    #include "my_vmeio.h"
    #include "my_vmeint.h"

    //Bridge!
    #include "vme_bridge.h"

    #include "main_daqpet.h"
    #include "v1718_lib.h"
    #include <fstream>
    #include <iostream>
    #include <vector>
    #include <string>
    #include <stdexcept>
    #include "adc792_lib.h"
    #include "tdcdd00_lib.h"


    // root
    #include "root.h"

    using namespace std;

    void print_usage(FILE* stream, char* name, int exit_status){
	    fprintf(stream, "Usage: %s -o filename  -n nevents -t time -p ped [-r/--root]\n\n"
			    "\t-o FILENAME\tfile di output in cui vengono scritti gli eventi del qdc e del tdc." 
			    "In particolare vengono creati due file rinominati FILENAME_qdc e FILENAME_tdc.\n"
			    "\t-n NEVENTS\timposta il numero di eventi da acquisire\n"
			    "\t-t TIME\t\timposta il tempo di acquisizione\n"
			    "\n\t\t\tdeve essere specificata almeno una di queste due opzioni, se si sceglie solo -n l'acquisizione viene fermata"
			    "con CTRL + C (miraccomando a non chiudere il terminale...polletti degli anni successivi)\n"
			    "\t\t\tse vengono specificate sia -t che -n il programma terminera' quando una delle due condizioni viene raggiunta per prima"
			    "\n\n"
			    "\t-r, --root\tgenera automaticamente i due file root chiamandoli FILENAME_qdc.root e FILENAME_tdc.root\n"
			    "\t-v\t\taumenta la verbosita' (e lentezza) del programma (da usare in caso di debug)\n"
			    "\t-p\t\timposta la corrente di piedistallo (default 95)\n",
			    name);
	    exit(exit_status);
    }

    void int_handler(int sig){
	    fprintf(stderr, "\nCaught signal %d\n", sig);
	    exit_signal = true;
    }

   int main(int argc, char** argv)
    {
	    /************************* PARSING DEI PARAMETRI **************************
	     */
	    int opt;
	    unsigned long EvMax = 0;
	    unsigned long TMax = 0;
	    unsigned long Pmax=IPED_VAL;
	    unsigned long time_curr = 0;
	    char* output_filename_basic = NULL;
	    char* output_filename = (char *)malloc(25); 
	    
      verbose = 0;
	    bool rootc = false;

	    struct option long_options[] = {
		    { "help", 0, NULL, 'h'},
		    { "verbose", 0, NULL, 'v'},
		    { "output", 1, NULL, 'o'},
		    { "nevents", 1, NULL, 'n'},
		    { "root", 0, NULL, 'r'},
		    { "time", 1, NULL, 't'},
		    { "ped", 0, NULL, 'p'},
		    { NULL, 0, NULL, 0}
	    };

	    do{
		    opt = getopt_long(argc, argv, "hvo:n:r:t:p:", long_options, NULL);
		    switch(opt){
			    case 'h':
				    print_usage(stdout, argv[0], 0);
				    break;
			    case 'v':
				    // questo permette di fare piu' livelli di verbosita' a seconda di quante -v vengono passate
				    ++verbose;
				    break;
			    case 'o':
				    output_filename_basic = optarg;
				    break;
			    case 'n':
				    EvMax = atoi(optarg);
				    break;
			    case 'r':
				    rootc = true;
				    break;
			    case 't':
				    TMax = atoi(optarg);
				    break;
			    case 'p':
				    Pmax = atoi(optarg);
				    break;
			    case -1:
				    break;
			    default:
				    print_usage(stderr, argv[0], EXIT_FAILURE);
		    }
	    } while(opt != -1);

	    if(output_filename_basic == NULL || (EvMax == 0 && TMax == 0)){
		    print_usage(stderr, argv[0], EXIT_FAILURE);
	    }
	    /********************************* FINE PARSING PARAMETRI *********************************/
	    printf("Selected ped=%lu \n", Pmax);	

	    int status_init,status;
	    int32_t BHandle(0);
	    vector<unsigned long> DataAdc(V792N_CHANNEL+2); //34
	    vector<unsigned long> DataTdc(Vdd00N_CHANNEL * 2 + 2);//18 x2

	    FILE* output_file; 


	    /* Bridge VME initialization */
	    status_init = bridge_init(BHandle);  
	    bridge_deinit(BHandle);
	    status_init = bridge_init(BHandle);

	    printf("\n\n VME initialization\n");
	    if (status_init != 1){


		    printf("VME Initialization error ... STOP!\n");
		    return(EXIT_FAILURE);
	    }

	    /* Modules initialization */
	    if(V1718)
	    {
		    printf("\n Bridge initialization and trigger vetoed\n");
		    status_init *= init_1718(BHandle);
		    status_init *= clear_veto_1718(BHandle);
		    /*we put veto signal to 0 because we want a 0 from the output of the Logic Unit: 
		      this operation stops the generation of the gate during the initialization.*/
		    status_init *= init_scaler_1718(BHandle) ;
		    status_init *= init_pulser_1718(BHandle) ;
		    status_init *= clearbusy_1718(BHandle);
		    if (status_init != 1) { return(EXIT_FAILURE); }

	    }
	    else {
		    printf("\n No TRIGGER module is present:: EXIT!\n");
		    return(EXIT_FAILURE);
	    }

	    /*ADC792 initialization*/

	    if(ADC792)
	      {
		printf("Initialization of ADC792\n");
		status_init *= init_adc792(BHandle,Pmax);
		if (status_init!=1)
		  {
		    printf("Error in adc792 initialization....STOP! \n");
		    return(EXIT_FAILURE);
		  }
	      }

	    /*tdc DD00 initialization*/

	    if(TDCdd00)
	      {
		printf("Initialization of TDCDD00\n");
		status_init *= init_tdcdd00(BHandle);
		if (status_init!=1)
		  {
		    printf("Error in TDCDD00 initialization....STOP! \n");
		    return(EXIT_FAILURE);
		  }
	      }

	    //	 status_read *= read_tdcdd00_simple(BHandle, pDataTdc);


	/*-------------- HANDLING INTERRUPT SIGNAL ------------------*/

	    struct sigaction sigIntHandler;
	    sigIntHandler.sa_handler = int_handler;
	    sigemptyset(&sigIntHandler.sa_mask);
	    sigIntHandler.sa_flags = 0;
	    sigaction(SIGINT, &sigIntHandler, NULL);



	/*------------- ACQUISITION------------------*/

	    if(access(output_filename_basic, F_OK) == 0){
		    fprintf(stderr, "Warning %s already exists overwite <Y/n>:", output_filename_basic);
		    if(getchar() == 'n'){
			    return EXIT_SUCCESS;
		    }
	    }
	    strcpy(output_filename,output_filename_basic);
	    strcat(output_filename,"_qdc");
	    if((output_file = fopen(output_filename,"w")) == NULL){
		    perror(output_filename);
	    }

      /*---------------Setting the output file----------------*/
      /*QDC output file*/

	    fprintf(output_file,"#Ev.\tEv.Buffer\t");
	    for(int i=0;i<V792N_CHANNEL/2;i++){
		    fprintf(output_file,"CH %d\t", i);
		    fprintf(output_file,"CH %d\t", i + V792N_CHANNEL/2);
	    };

      /*TDC output file*/
	    /* COMMENTED OUT IN 2022 to attempt a simpler printout of the output
	    for(int i=0;i<Vdd00N_CHANNEL/2;i++){
		    fprintf(output_file,"TCH %d\t", i);
		    fprintf(output_file,"TCH %d\t", i + Vdd00N_CHANNEL/2);
	    }	
	    */
	    for(int i=0;i<Vdd00N_CHANNEL;i++){
		    fprintf(output_file,"TCH %d\t", i);
            }
            fprintf(output_file, "\n");

	    /*------------------------------------------------------*/

	    /* resetting adc buffer and releasing veto ACQUIRING EVENTS BEYOND THIS POINT*/
	    data_reset_adc792(BHandle);
	    data_reset_tdcdd00(BHandle);
	    /*We put the veto signal at 1 in order to have coincidence in the Logic Unit. This operation permits the gate generation. */
	    if(set_veto_1718(BHandle) != 1){
		    fprintf(stderr, "ERROR veto not released aborting\n");
		    return EXIT_FAILURE;
	    }
	    fprintf(stderr, "Veto released starting acquisition\n");

      /* acquisition starts */
	    exit_signal = false;
	    bool trigger;
	    struct timespec start, end;

	    /*---------STARTING ACQUISITION TIME--------------*/
	    clock_gettime(CLOCK_MONOTONIC, &start);
	    unsigned long Event = 0, EvNum = 0,EvNumTdc =0;
	    int counter =0;
      /*---------MAIN ACQUISITION CYCLE--------------*/
	    while((((EvMax != 0) && (EvNum < EvMax)) || ((TMax != 0) && (time_curr < TMax)))
			    && (exit_signal==false)){
		    status=1;
		    trigger = false;
		    
		    /*---------WAITING FOR TRIGGER--------------*/
		    while (trigger==false && status==1 && exit_signal == false){
			    status = trigger_interrupt(BHandle, &trigger);
		    }
		    if(verbose>0){
			    printf("TRIGGER GONE\n");
		    }  
		    /*We put the veto to 0 to start the reading-writing operations*/
		    if(clear_veto_1718(BHandle) != 1){      
			    fprintf(stderr, "ERROR VETO NOT RELEASED\n");
			    return EXIT_FAILURE;
		    }
		    /*---------READING THE WHOLE BUFFER--------------*/
		    DataAdc = read_block_adc792(BHandle, 0, status);	   
	  	    DataTdc = read_block_tdcdd00(BHandle, status);
		    	   
		     if(status!=1){
			    fprintf(stderr, "ERRORE!!!! %d\n", status);
		    }

		    // vedere pagina 45 del qdc per la conformazione del buffer
		    int valid, validtdc, valid_datum;
		    valid = (DataAdc[0] >> 24) & 0x7; //Bit di controllo 26-24 
		    validtdc = (DataTdc[0] >> 24) & 0x7; //Bit di controllo 26-24 
		    int tdc_data[32][Vdd00N_CHANNEL]; // 32 is the buffer size ;)
		    int tdc_chan[32][Vdd00N_CHANNEL];
		    int qdc_data[32][V792N_CHANNEL];
		    int k=0, j=0;
		    /*-------UNPACKING DATA FROM BUFFER AND DISCARTING NON VALID EVENTS--------*/

		    // we store the content to be print to file into qdc_data,
		    // which therefore must be initialized for each of the events
		    // we read
		    // the default value for non-valid or non-read channels is -999
		    for(int ievt =0; ievt < 32; ievt++) {
		            for(int i =0; i < V792N_CHANNEL; i++) {
		        	    qdc_data[ievt][i]=-999;
		            }

		            for(int i =0; i < Vdd00N_CHANNEL; i++) { //check the max value of i in for
		                    tdc_data[ievt][i]=-999;
		                    tdc_chan[ievt][i]=-999;
		            }
		    }

		    int n_qdc_events = 0;
		    while( valid == 2 ){ // events with control bit 010

			    /*
			if( k>=34){
	                      for(int i=1;i<Vdd00N_CHANNEL+1;i++){    
				      fprintf(output_file,"-999\t");									        
	      		      }
			      
	                 }
			   */
			    EvNum =DataAdc[k+V792N_CHANNEL+1] & 0xffffff; //Bit 0-24
			    for(int i=1;i<V792N_CHANNEL+1;i++){ //from1 to 32 channels
				  //fprintf(output_file,"%lu\t",DataAdc[k+i] & 0xfff);//Bit 0-11 (dato)
				    qdc_data[k % (V792N_CHANNEL+2)][i-1] = DataAdc[k+i] & 0xfff;
			    }
	  //if k = 0 , 0+33+2 = 35 
			    //printf("\n k = %d\n",k);
			
			    //printf("k= %d \n",k);
			    k += V792N_CHANNEL+2;
			    counter++;
			    valid = (DataAdc[k] >> 24) & 0x7; //Bit di controllo 26-24
			    n_qdc_events++;
		    } // loop over QDC events
	
  			    
		      int n_tdc_events = 0;
		      while( validtdc==2 ) {

//		      EvNumTdc =DataTdc[j+Vdd00N_CHANNEL+1] & 0xffffff; //x2
		            const int n_channels = (DataTdc[j] >> 8) & 0x3f;
			    if(verbose>0) {
				    if (n_tdc_events == 0) printf("TDC event %d in the buffer", n_tdc_events);
				    printf("number of channels %lu ",(DataTdc[j] >> 8) & 0x3f);	   
			    }

			    bool has_read = false;
			    for(int i=1;i<n_channels+1;i++){ // note they may be unordered!
				valid_datum = (DataTdc[j+i] >> 24) & 0x7;
				long unsigned int this_chan = (DataTdc[j+i] >> 17) & 0xf;
				if(verbose>0) {
				    printf("\nposition: %d ch: %lu  valid: %lu underthres: %lu overflow: %lu  counts: %lu",j, this_chan,
				    		(DataTdc[j+i] >> 14) & 0x1 ,(DataTdc[j+i] >> 13) & 0x1,
				    		(DataTdc[j+i] >> 12) & 0x1, (DataTdc[j+i] & 0xfff ));
				}
				if( valid_datum == 0) {     
				       // fprintf(output_file,"%lu\t",DataTdc[j+i] & 0xfff);
					tdc_data[n_tdc_events][i-1] = DataTdc[j+i] & 0xfff;
					tdc_chan[n_tdc_events][i-1] = (DataTdc[j+i] >> 17) & 0xf;
					has_read = true;
					if (verbose > 0) printf(" dato valido preso! canale %d",tdc_chan[n_tdc_events][i-1]);

				}
				//else if( valid_datum == 0x4 || valid_datum == 0x6) {
			//		fprintf(output_file,"-111\t");
		          //       }
			    	
			    }
			    if(verbose>0) {
			    printf("\n");
			    }
			    //if k = 0 , 0+16+2 = 18
			    //printf("\n j = %d\n",j);
			    // j += Vdd00N_CHANNEL+2;
			    
			    //printf("j= %d \n",j);
			    if (((DataTdc[j+n_channels+1] >> 24) & 0x7) != 0x4) {
				    cout << "A DISASTER HAPPENED" << endl;
				    throw std::runtime_error("End of block not found");
			    }
			    j += Vdd00N_CHANNEL+2;
			    
			    if (has_read == true) {
			           // printf("qui non entro\n");
				    n_tdc_events++; // increment only if we read something
			         
			    }
			    validtdc = (DataTdc[j] >> 24) & 0x7;
			    //printf("valid dopo run è %d\n",validtdc);
				//for(int y = 0; y < 18; y++){
			       // printf("DataTdc[%d] = %lu \n",y, DataTdc[y]);
			       // }
			//	printf("*--------------------------------------------------------------------------------*\n");

		       } // loop over TDC event

  //                                      for (int j = 0; j< Vdd00N_CHANNEL; j++){
//						printf("tdc_chan[0][%d] is %d\n",j,tdc_chan[0][j]);												                    }

		      //int m = 0, n = 0;	
		      for (int ievt = 0; ievt < max(n_tdc_events, n_qdc_events); ievt++) {
			      
		  	      fprintf(output_file, "%lu\t%lu\t\t",Event, EvNum);
			      for (int i = 0; i < V792N_CHANNEL; i++) {
		
			        if (ievt < n_qdc_events) fprintf(output_file,"%d\t", qdc_data[ievt][i]);
				else fprintf(output_file,"%d\t", -999);
		
			      }
			      for (int i = 0; i < Vdd00N_CHANNEL; i++) { // column of output file (cf. L283) 
				 int what_to_print = -999; // default value for empty TDC

				 if (ievt < n_tdc_events) { // check we arent out of the TDC buffer
			            for (int ientry = 0; ientry < Vdd00N_CHANNEL; ientry++) { // loop over content of TDC buffer for this event
				       if (tdc_chan[ievt][ientry] == i) { // note that the LHS is -999 when the value was not filled in this event
				         if (what_to_print != -999) {
					    printf("A DISASTER HAPPENED\n");
					    throw std::runtime_error("more than one TDC entry for the same channel");
				         } else {
				           what_to_print = tdc_data[ievt][ientry];
					 }
				       }
			            } // loop over content of TDC buffer
			         } // event exists also in the TDC

			         fprintf(output_file, "%d\t", what_to_print);
		              } // loop over TDC columns of the output file

			      /* COMMENTED OUT IN 2022 TO ATTEMPT A NON-WRONG VERSION OF THE SAME IDEA
			      for (int i = 0; i < Vdd00N_CHANNEL; i++) { //loop sulle colonne, i fissata = colonna fissata
			      
				if (i % 2 == 0) m = (int) i/2;
				else m = 8 + (int) (i-1)/2; // m rappresenta un modo per passare da 0-1-2-3... a 0-8-1-9-2....
					
					if (ievt < n_tdc_events) { //controllo
				   				
						if ( tdc_chan[ievt][n] == m) { //dunque se l'evento si riferisce allo stesso canale della colonna
					
							fprintf(output_file,"%d\t", tdc_data[ievt][n]);
							//printf("canale che scrivo sul file: %d\n",tdc_chan[ievt][n]); //debug
							n++;
				}// diosanto ma perchè non mi scrivi mai un cazzo, scusate l'emozione del momento 
				        	
				  		else fprintf(output_file,"%d\t",-999); //se l'evento si riferisce ad un canale diverso
					}

			        	else fprintf(output_file,"%d\t",-999); //se l'evento nel buffer è vuoto   			
			        
			      }
			      */
		
			      	      
		         	fprintf(output_file,"\n");
		      }
		    
		    Event++;
		      
		    
	/*-----WRITING DATA TO DISK------------*/
		    fflush(output_file);
		    if(verbose>0){
			    printf("ev num %lu\n", EvNum);
		    }
	
		    //if(verbose>0){
			  //  printf("ev num %lu\n", EvNumTdc);
		    //}
    		    /*After the reading-writing operations, we are ready for another event, so we put veto signal to 1 */
		    if(set_veto_1718(BHandle) != 1){
			    fprintf(stderr, "ERROR VETO NOT RELEASED.\n");
			    return EXIT_FAILURE;
		    }

		    /*----UPDATING ACQUISITION TIME--------------*/
		    clock_gettime(CLOCK_MONOTONIC, &end);
		    time_curr = end.tv_sec - start.tv_sec;

	    }
	    /*------------CALCULATING TOTAL ACQUISITION TIME------------------*/
	    clock_gettime(CLOCK_MONOTONIC, &end);
	    /*------------CLEAR VETO SIGNAL--------------------*/
	    /*After the acquisition cycle we put veto signal to 0 */
	    status_init *= clear_veto_1718(BHandle);
	    printf("Done\n");
	    printf("\eventi scartati tdc %lu\n",counter-EvNum-1);
	    /*------------CALCULATING NUMBER OF EVENTS AND ACQUISITION RATE--------------*/
	    unsigned adc_scaler;
	    /*
	     * FIXME: senza questo sleep la read_scaler_adc792 ritorna un numero a caso
	     * forse si tratta di un problema di timing (il qdc non e' pronto a rispondere)?
	     */
	    sleep(1);
	    adc_scaler = read_scaler_adc792(BHandle);
	    double time_elapsed = end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec)*1e-9;
	    printf("\nTime: %lf s\tRate: %lf Hz\n", time_elapsed, (adc_scaler+1)/time_elapsed);

	    fprintf(output_file, "\n# Time: %lf s\tRate: %lf Hz", time_elapsed, (adc_scaler+1)/time_elapsed);
      fclose(output_file);

	    /* VME deinitialization */
	    if(bridge_deinit(BHandle)==1){
		    printf("\n VME and modules deinitialization completed \n\n");
	    }

	    /*-------------GENERATING ROOT FILE-----------------*/
	    if(rootc){
		    printf("Generating ROOT file\n");
		    string root_filename = output_filename;
		    root_filename += ".root";
		    ASCII2TTree(output_filename,root_filename.c_str(), V792N_CHANNEL, Vdd00N_CHANNEL);
	    }
	free(output_filename);
	    return(EXIT_SUCCESS);
    }
