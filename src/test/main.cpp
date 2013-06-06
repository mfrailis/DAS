#include <iostream>
#include <sstream>
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
{/*
    shared_ptr<D::Database> db = D::Database::create("local");

    shared_ptr<lfiHkDaeSlowVoltage> hk = lfiHkDaeSlowVoltage::create("LfiDaeSlowVoltage_TOI_0001");

    hk->runId("FUNC_0001");
    hk->startTime(107361255356137);
    hk->endTime(107367005705966);
    hk->APID(1538);
    hk->type(3);
    hk->subtype(25);
    hk->PI1_val(1);
    hk->PI2_val(0);

    db->persist<lfiHkDaeSlowVoltage>(hk);

    shared_ptr<testLog> log = testLog::create("TestLog_FUNC_0001");
    log->runId("FUNC_0001");
    log->startTime(107361255356137);
    log->endTime(107367005705966);
    log->log("A first example of ddl mapping into the DAS");

    shared_ptr<testLogImage> logImage = testLogImage::create("IMG_001_FUNC_0001");
    logImage->naxis1(25);
    logImage->naxis2(50);
    logImage->format("png");

    testLog::images_vector v = log->images();
    v.push_back(logImage);

    log->images(v);

    db->persist<testLog>(log);

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
    */
    odb::session s;
    
    shared_ptr<D::Database> db = D::Database::create("benchmark");
    
    shared_ptr<TypeMain> tm = TypeMain::create("main1");
    TypeMain::many_ex_vector exass = tm->many_ex();
    TypeMain::many_sh_vector shass = tm->many_sh();
    
    for(int i=0; i< 5; i++)
    {
        std::stringstream se,ss;
        se << "ass_EX_prima" << i;
        shared_ptr<manyEx> exp = manyEx::create(se.str());
        exass.push_back(exp);
        
        ss << "ass_SH_prima" << i;
        shared_ptr<manySh> shp = manySh::create(ss.str());
        shass.push_back(shp);        
    }
    tm->many_ex(exass);
    tm->many_sh(shass);
    
//    TypeMain::many_sh_vector shass_temp = tm->many_sh();
    TypeMain::many_ex_vector exass_temp = tm->many_ex();
    
    db->persist<TypeMain>(tm);
 
    TypeMain::many_ex_vector exass2;
    TypeMain::many_sh_vector shass2;   
    for(int i=0; i< 5; i++)
    {
        std::stringstream ss,se;
        se << "ass_EX_seconda" << i;
        shared_ptr<manyEx> exp = manyEx::create(se.str());
        exass2.push_back(exp);
        db->persist<manyEx>(exp);
        
        ss << "ass_SH_seconda" << i;               
        shared_ptr<manySh> shp = manySh::create(ss.str());
        shass2.push_back(shp);
        db->persist<manySh>(shp);      
    }   
    tm->many_ex(exass2);
    tm->many_sh(shass2);
    
    /*db->update<TypeMain>(*tm,false);
    
    for(TypeMain::many_ex_vector::iterator i = exass2.begin(); i != exass2.end(); ++i)
      db->update<manyEx>(*i,true);       
  
    for(TypeMain::many_ex_vector::iterator i = exass.begin(); i != exass.end(); ++i)
        db->update<manyEx>(*i,true);
    */  
    
    return 0;
}

