/***************************************************************************
                          CIWS_FITS.h  -  description
                             -------------------
    
    copyright            : (C) 2013 by Vito Conforti
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


#ifndef CIWSProcessor_H_HEADER_INCLUDED_C3772E73
#define CIWSProcessor_H_HEADER_INCLUDED_C3772E73
// DAS include
#include <vector> 
#include <das/tpl/database.hpp>
#include <das/transaction.hpp>
#include <das/ddl/types.hpp>

#include "Processor.h"

#include <string>
#include "stdint.h"
#include <stdio.h>
#include <stdlib.h>

/** 
* @file CIWSProcessor.h 
* @brief this header file contains the declaration of CIWSProcessor Class.  
* 	This class has all method for read each packets and forward it to CIWS_FITS object. 
*
* @author Vito Conforti
*
* @date 10/01/2013
*/


// define of block that is the source data field. 
typedef  struct
{   
	int pdm_id;   		// pdm identifier
	int trpdm; 		// Triggered PDM
	int highGain[64];	// high gain for each of 64 pixels
	int lowGain[64];	// low gain for each of 64 pixels
} BLOCK;


// define of structure of a packet. 
typedef struct
{
    int header[5]; // packet header 
    int time_tag[8]; // yy, mm, dd, hh, mm, ss, gps, nanosec. 
    int event_counter; 
    int flag[8]; // last dfh row: fibst, fibmod, lid, scan, trigdac, rgb, pktmode, npdm
    BLOCK* block;   /* pointer at block  */

} PACKET;






/*
*
* This class is aimed at the reading  each packet and store the fields in memory. 	
*
*/
class CIWSProcessor : public Processor
{
    public:
        
	// constructor
        CIWSProcessor();  

        bool loadConfiguration(int argc, char** argv) throw(PacketException*);
        //##Documentation
        //## Gets a string that describes the version of current processor
        virtual char* processorVersion() { return "2.1.0 (CIWS Mode 2 Submode 1 )"; };

	//##Documentation
        //## Gets a string that describes the version of current processor
        virtual char* processorAuthor() { return "Vito Conforti"; };

	char* processorDescriptor() { return "IASF Bologna CIWS Processor 9 PDM w/o temp w/o tresh.";};
	
	char* processorID() { 					
	  return "ICDIssue1_2";
	};

	void databaseSave();

	//performance
	int getNumPackets();
	int getNumBlocks();



    protected:

/*
I seguenti metodi devono essere obbligatoriamente implementati perchè sono virtual nella classe genitore processore. 
*/


        virtual void createMemoryStructure();

        virtual bool setValue();

        /** Inizializza un array di stringhe che contiene i valori da passare all'inizializzazione del file FITS */
        virtual char** initCharValueForOutput_init();

        /** Inizializza un array di interi che contiene i valori da passare all'inizializzazione del file FITS */
        virtual int* initIntValueForOutput_init();

        /** Inizializza un array di stringhe che contiene i valori da passare alla chiusura del file FITS */
        virtual char** initCharValueForOutput_close();

        /** Inizializza un array di interi che contiene i valori da passare alla chiusura del file FITS */
        virtual int* initIntValueForOutput_close();
				
	// assign the name at the output file
	//virtual char* createOutputFileNameBase();





         private:
        int apid;

	int n_blocchi_passati;

	//performance variable
	int numPackets;
	int numBlocks;


	// the name of the output file
	string outputFilename; 	


	// data required for the header FITS
	int telescope_id;  // read in the first packet
	char* filename; // assigned as parameter at the program
	uint8_t type; // read in the first packet
	uint8_t subtype; // read in the first packet
	int* tstart; // event time tag of the first packet; 
	int tstartms; // time start millisecond; 
	int* tstop;  // event time tag of the last packet; 
	int tstopms; // time stop millisecond

	int sequence_number; // contatore di file con lo stesso run_id

	int sourceSequenceCounter; 
	string mc_config; 


	string inputPath; // memorizza il nome del file di input se acquisisce via file

	// das object
	shared_ptr<s21> das;
};
#endif                        
