// file      : inverse/driver.cxx
// copyright : not copyrighted - public domain

#include <memory>   // std::auto_ptr
#include <iostream>
#include <ctime>

#include <odb/database.hxx>
#include <odb/session.hxx>
#include <odb/transaction.hxx>

#include "database.hxx" // create_database

#include "ddl_instance.hxx"
#include "ddl_instance-odb.hxx"

using namespace std;
using namespace odb::core;

using boost::posix_time::microsec_clock;


void print(const LfiDaeSlowVoltage& hk)
{
  cout << "Run ID: " << hk.runId() << endl
       << "  Start Time: " << hk.startTime() << endl
       << "  End Time  : " << hk.endTime() << endl 
       << "  APID      : " << hk.apid() << endl;
}


void print(const TestLog &log)
{
  cout << "Run ID: " << log.runId() << endl
       << "  Start Time: " << log.startTime() << endl
       << "  End Time  : " << log.endTime() << endl 
       << "  log       : " << log.log() << endl;
}

void print(const TestLogImage &img)
{
  cout << "Image name: " << img.name() << endl
       << "  naxis1 : " << img.naxis1() << endl
       << "  naxis2 : " << img.naxis2() << endl 
       << "  format : " << img.format() << endl;
}

int
main (int argc, char* argv[])
{
  using tr1::shared_ptr;

  try
  {
    auto_ptr<database> db (create_database (argc, argv));

    // Create a few persistent objects.
    //
    {
      // Creating and inserting an LfiDaeSlowVoltage timeline
      //
      {
        shared_ptr<LfiDaeSlowVoltage> hk (new LfiDaeSlowVoltage("LfiDaeSlowVoltage_TOI_0001", 
								0));
	
	hk->dbUserId("mfrailis");
	hk->creationDate(microsec_clock::universal_time());
     
	hk->runId("FUNC_0001");
	hk->startTime(107361255356137);
	hk->endTime(107367005705966);
	hk->apid(1538);
	hk->type(3);
	hk->subtype(25);
	hk->pi1Val(1);
	hk->pi2Val(0);

	shared_ptr<TestLog> testLog (new TestLog("TestLog_FUNC_0001", 0));
	testLog->runId("FUNC_0001");
	testLog->startTime(107361255356137);
	testLog->endTime(107367005705966);
	testLog->log("A first example of ddl mapping into the DAS");

	shared_ptr<TestLogImage> logImage(new TestLogImage("IMG_001_FUNC_0001", 0));
	logImage->naxis1(25);
	logImage->naxis2(50);
	logImage->format("png");

	testLog->images().push_back(logImage);
				      
        transaction t (db->begin ());

        db->persist (hk);
	db->persist (logImage);
	db->persist (testLog);

        t.commit ();
      }

    }


    // Find and load the inserted timeline
    //
    {
      typedef odb::query<TestLog> query;
      typedef odb::result<TestLog> result;

      session s;
      transaction t (db->begin ());

      result r (db->query<TestLog> (query::runId == "FUNC_0001"));

      for (result::iterator i (r.begin ()); i != r.end (); ++i)
      {
	std::vector<lazy_shared_ptr<TestLogImage> >& images = i->images();
	for (std::vector<lazy_shared_ptr<TestLogImage> >::iterator j(images.begin()); 
	     j != images.end(); ++j)
	  print(*(*j));
	print (*i);
      }

      t.commit ();
    }


  }
  catch (const odb::exception& e)
  {
    cerr << e.what () << endl;
    return 1;
  }
}
