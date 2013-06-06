#include <iostream>

#include "tpl/Database.hpp"
#include "ddl/types.hpp"

using namespace std;
namespace D  = das::tpl;

void print(const shared_ptr<lfiHkDaeSlowVoltage>& hk)
{
  cout << "Run ID: " << hk->runId() << endl
       << "  Start Time: " << hk->startTime() << endl
       << "  End Time  : " << hk->endTime() << endl
       << "  APID      : " << hk->APID() << endl;
}


void print(const testLog& log)
{
  cout << "Run ID: " << log.runId() << endl
       << "  Start Time: " << log.startTime() << endl
       << "  End Time  : " << log.endTime() << endl
       << "  log       : " << log.log() << endl;
}

void print(const shared_ptr<testLogImage> &img)
{
  cout << "Image name: " << img->name() << endl
       << "  naxis1 : " << img->naxis1() << endl
       << "  naxis2 : " << img->naxis2() << endl
       << "  format : " << img->format() << endl;
}

int main()
{
    shared_ptr<D::Database> db = D::Database::create("local");

    shared_ptr<lfiHkDaeSlowVoltage> hk (new lfiHkDaeSlowVoltage("LfiDaeSlowVoltage_TOI_0001"));

    hk->runId("FUNC_0001");
    hk->startTime(107361255356137);
    hk->endTime(107367005705966);
    hk->APID(1538);
    hk->type(3);
    hk->subtype(25);
    hk->PI1_val(1);
    hk->PI2_val(0);

    db->persist (*hk);

    shared_ptr<testLog> log (new testLog("TestLog_FUNC_0001"));
    log->runId("FUNC_0001");
    log->startTime(107361255356137);
    log->endTime(107367005705966);
    log->log("A first example of ddl mapping into the DAS");

    shared_ptr<testLogImage> logImage(new testLogImage("IMG_001_FUNC_0001"));
    logImage->naxis1(25);
    logImage->naxis2(50);
    logImage->format("png");

    testLog::images_vector v = log->images();
    v.push_back(logImage);

    log->images(v);

    db->persist (*log);

    {
      typedef odb::result<testLog> result;


      odb::transaction t (db->begin ());

      result r (db->query<testLog> ("runId == 'FUNC_0001'","version descending"));

      for (result::iterator i (r.begin ()); i != r.end (); ++i)
      {
        testLog::images_vector images = i->images();
        for (testLog::images_vector::iterator j(images.begin());j != images.end(); ++j)
            print(*j);
        print (*i);
      }

      t.commit ();
    }


    return 0;
}

