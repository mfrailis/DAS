/* Support Large File */
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE
/***************************************************************************//*
									    CIWS PROCESSOR Prototype
									    -------------------

									    Author               : Vito Conforti
									    email                : conforti@iasfbo.inaf.it
									     ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include "CIWSProcessor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include "stdint.h"




using namespace std; 

/*! \brief Constructor of Processor
 * \param void 
 * \return void
 */
CIWSProcessor::CIWSProcessor()
{    
	
  apid=0;
  n_blocchi_passati = 0;

  //performance
  numPackets = 0;
  numBlocks = 0;


  telescope_id = 0;  // read in the first packet
  filename = ""; // assigned as parameter at the program
  type = 0; // read in the first packet
  subtype = 0; // read in the first packet
  tstart = new int[6]; // event time tag of the first packet; 
  tstop = new int[6];  // event time tag of the last packet; 
  tstartms = 0; 
  tstopms = 0; 
  sourceSequenceCounter = 0; 
  mc_config =""; 

  sequence_number = 0; // contatore di file con lo stesso run_id

}


/*!
 *      \brief  take filename by a path
 *
 *	take a string of complet path  in input and return only the file name (all chars after the last / )
 */
string getFileNameByPath( string path ){
	
  string file_name = ""; 
  // find the last /
  unsigned found = path.find_last_of("\//");
  file_name = path.substr(found +1 );  

  // delete the extension and lv0 
  found = file_name.find_last_of("."); 
  file_name = file_name.erase(found); 

  // add the packet type and subtype to the filename before the _lv1 
  file_name += "_02_01"; 

  return file_name; 
}




/*! \brief Set the configuration 
 * \param all options passed when program start. 
 * \return ret  
 */
bool CIWSProcessor::loadConfiguration(int argc, char** argv) throw(PacketException*)
{

  inputPath = ""; 

  // retrieve the input file name 
  for (int i = 0; i < argc ; i++){
    string current = argv[i]; 
    if (current.compare("-s") == 0 ) {
      inputPath = argv[i+1]; 			
    }
  }


  // questo serve per scrivere il nome del file
  //outputFilename = getFileNameByPath(inputPath); 

	
  bool ret = Processor::loadConfiguration(argc, argv);
	
  return ret;


}


/*! \brief Allocate space in memory
  1. Creation of data structure that processor needed for store packets 
  2. Allocation of space needed for a packet; 
  3. Allocation of space to storage the maximum number of blocks. 
  4. \Postcondition: arrayDataOutput the pointer to allocated memory 

*/
void CIWSProcessor::createMemoryStructure(){

  /*
  // packet
  PACKET* packet = (PACKET*) new PACKET; 
	
  // blocks	
  packet->block = (BLOCK*) new BLOCK[p->dataField->sourceDataField->getMaxNumberOfBlock()];   

  cout << "CIWS - max number of blocks " <<       p->dataField->sourceDataField->getMaxNumberOfBlock() << endl;

  for(int i=0; i<p->dataField->sourceDataField->getMaxNumberOfBlock(); i++){
		
  packet->block[i].trpdm = (int) new int; 
  packet->block[i].pdm_id = (int) new int;		
		
  // initialize high gain
  for (int j = 0; j< 64; j++){	
  packet->block[i].highGain[j] =  (int) new int;	
  }		

  // initialize low gan 
  for (int j = 0; j< 64; j++){	
  packet->block[i].lowGain[j] =  (int) new int;	
  }

  }


  //2) importante, da non dimenticare (prima dell'invocazione di setValue() - occorre associare il pacchetto all'array (obbligatorio) 
  arrayDataOutput = (void*) packet;

  printf("PROCESSOR: createMemoryStructure ... done ");
  */
	
}

string convertInt(int number)
{
  stringstream ss;//create a stringstream
  ss << number;//add number to the stream
  return ss.str();//return a string with the contents of the stream
}


/*! 
 *	\brief Set value on DAS
 *
 */
bool CIWSProcessor::setValue(){


  int nblocks;                //Numero reale di event data blocks 

  // prelevo il pacchetto precedentemente inserito nell'arrayDataOutput dalla funzione createMemoryStructure	
  PACKET* packet = (PACKET*) arrayDataOutput;

  // alla creazione del primo pacchetto
  if(!das){
    int ssc = p->header->getFieldValue(3); 
    string ssc_s = convertInt(ssc);   
    das = s21::create(ssc_s, "ciws_prototype" ); 
  }	 
	
  uint8_t telescopeId = (uint8_t) p->header->getFieldValue(0); 
  das->dtelescopeid(telescopeId); 

  type =  (uint8_t) p->header->getFieldValue(1);  
  das->dtype(type);

  subtype =  (uint8_t) p->header->getFieldValue(2);
  das->dsubtype(subtype);
	
  das->dssc(ssc);

  int year = p->dataField->dataFieldHeader->getFieldValue(0); // year	
  int month = p->dataField->dataFieldHeader->getFieldValue(1); // month
  int day = p->dataField->dataFieldHeader->getFieldValue(2); // day

  // hour is composed by a 1 MSB and 4 LSB
  int hh_msb = p->dataField->dataFieldHeader->getFieldValue(3); // hours msb
  int hh_lsb = p->dataField->dataFieldHeader->getFieldValue(4); // hours lsb
  int hour = ( hh_msb * 16 ) + hh_lsb;   // 16 is 2 alla 4


  int minute = p->dataField->dataFieldHeader->getFieldValue(5); // minute
  int second = p->dataField->dataFieldHeader->getFieldValue(6); // seconds
  int gps_status = p->dataField->dataFieldHeader->getFieldValue(7); // gps status

  das->gpsStatus(gps_status);

  stringstream ss_date; 
  ss_date << year<< "-" << month << "-" << day <<"T" << hour << ":" << minute << ":" << second ;  
  string sdate = ss_date.str();
  das->datetime(sdate);
	
	
  // nanosecond is composed by 15 bit MSB and 16 bit LSB
  int nano_msb = p->dataField->dataFieldHeader->getFieldValue(8); // nanoseconds msb
  int nano_lsb = p->dataField->dataFieldHeader->getFieldValue(9); // nanoseconds lsb
  int nanosecond = (nano_msb * 65536) + nano_lsb;   // 65536 è 2 alla 16
	
  das->nanoseconds(nanosecond);
	


  int event_number = (uint32_t) p->dataField->dataFieldHeader->getFieldValue_4_14(10) ;      // event counter
  das->eventnumber(event_number);

	
  // configuration
  int fib_st = p->dataField->dataFieldHeader->getFieldValue(11); // fib st
  int fib_mode = p->dataField->dataFieldHeader->getFieldValue(12); // fib mode
  int lid = p->dataField->dataFieldHeader->getFieldValue(13); // lid
  int scan = p->dataField->dataFieldHeader->getFieldValue(14); // scan 
  int trig = p->dataField->dataFieldHeader->getFieldValue(15); // trig
  int rgb = p->dataField->dataFieldHeader->getFieldValue(16); // rgb 


  stringstream ss_config; 
  ss_config << fib_st << fib_mode << lid << scan << trig << rgb ;  
  das::Array<std::string> datetime(1);
  datetime(0) = ss_config.str();
  das->append_column("datetime",datetime);
  

  das::Array<int> flags(1);
  flags(0) = p->dataField->dataFieldHeader->getFieldValue(17); // pkt mode
  das->append_column("flags",flags);


  //  int numpdm = p->dataField->dataFieldHeader->getFieldValue(18); // npdm
  //das->npdm(numpdm);
	


  // set value of blocks
  nblocks = p->dataField->getNumberOfRealDataBlock();
  SDFBlockFixed* sdf = (SDFBlockFixed*) p->dataField->sourceDataField;
	
  //performance 
  numPackets++;
  numBlocks = numBlocks + nblocks;

  // create vector of pdms
  //s21::pdms_vector all_pdm = das->pdms(); 
	
  
  //das::Arrray<signed char> triggered_pdm(nblocks);
  for (int i = 0; i < nblocks; i++){
    stringstream hi_cname;
    stringstream lo_cname;
    
    hi_cname << "PDM" << i+1 << "_HighGain";
    lo_cname << "PDM" << i+1 << "_LowGain";
    
    das::Array<int> high_gains_array(64);
    das::Array<int> low_gains_array(64);
    
    SDFBFBlock* current_block = sdf->getBlock(i);		

    // the field 2 is the spare. 		

    triggered_pdm(i) = current_block->getFieldValue(1);


    int offset = 3; //  the primer are of pdm id

    hi_gains(i).resize(64);		
    for (int j = 0; j< 64; j++)						 		 
      hi_gains(i)(j) = current_block->getFieldValue( j + offset);
   

    offset += 64;

    lo_gains(i).resize(64);
    for (int j =0; j< 64; j++)					
      lo_gains(i)(j) =  current_block->getFieldValue(j+offset);
    
    das::ColumnArray<int>h_g(&high_gains_array , 1 , das::neverDeleteData);
    das::ColumnArray<int>l_g(&low_gains_array , 1 , das::neverDeleteData);
    das->append_column(hi_cname.str(), h_g);
    das->append_column(lo_cname.str(), l_g);   
  }

  //das->append_column("tr_pdm",triggered_pdm);

  // Needed by ProcessorLib
  nrowsFITS =  nblocks;
	
  n_blocchi_passati += nblocks;
		
  sourceSequenceCounter++; // incremento del source sequence counter --- dovrebbe coincidere con quello inserito nel pacchetto. 

  return true;
}

void CIWSProcessor::databaseSave(){
  
}


char** CIWSProcessor::initCharValueForOutput_init(){
  return 0;
}

int* CIWSProcessor::initIntValueForOutput_init(){
  das::DatabaseConfig::database("ciws_prototype").buffered_data(false); // 
  return 0;
}

char** CIWSProcessor::initCharValueForOutput_close(){
  return 0;
}

int* CIWSProcessor::initIntValueForOutput_close(){
  if(das){ // persist the das
    // create connection
    shared_ptr<das::tpl::Database> db = das::tpl::Database::create("ciws_prototype");

    // create transaction
    das::Transaction tr = db->begin(); 

    // write packet
    db->persist(das); 

    tr.commit();
  }
  return 0; 
}

//performance
int CIWSProcessor::getNumPackets(){
  return numPackets;
}

int CIWSProcessor::getNumBlocks(){
  return numBlocks;
}

