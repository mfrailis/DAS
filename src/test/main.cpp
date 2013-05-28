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

int main(int argc, char * argv[])
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
    if(argc != 3 && argc != 1){
        cout << "usage: " << argv[0] << " \"query to execute\" \"ordering\"" << endl;
        cout << "or " << argv[0] << endl;
        return 1;
    }
        
    shared_ptr<D::Database> db = D::Database::create("benchmark");
    if( argc == 1)
    for(int i=0; i < 5; i++)
    {
        std::stringstream tm_n;
        tm_n << "TypeMain " << i;
        shared_ptr<TypeMain> tm = TypeMain::create(tm_n.str());
        
        TypeMain::many_ex_vector mev;
        TypeMain::many_sh_vector msv;
        for(int j=0; j < 2; j++)
        { 
            std::stringstream  me_n;
            me_n << "manyEx "<<i<<" "<<j;
            shared_ptr<manyEx> me = manyEx::create(me_n.str());
            mev.push_back(me);
            
            std::stringstream  ms_n;
            ms_n << "manySh " << i << " " <<j;
            shared_ptr<manySh> ms = manySh::create(ms_n.str());
            msv.push_back(ms);
            
            manyEx::nested_many_ex_vector nmev;
            manySh::nested_many_sh_vector nmsv;
            for(int k=0; k < 3; k++)
            {
                std::stringstream a3_n;
                a3_n << "assoc3 "<< i<<" "<<j<<" "<<k;
                shared_ptr<assoc3> a3 = assoc3::create(a3_n.str());
                nmev.push_back(a3);
                
                std::stringstream  a4_n;
                a4_n << "assoc4 " << i<<" "<<j<<" "<<k;
                shared_ptr<assoc4> a4 = assoc4::create(a4_n.str());
                nmsv.push_back(a4);
            }
            ms->nested_many_sh(nmsv);
            me->nested_many_ex(nmev);
        }
        std::stringstream os_n;
        os_n<< "oneSh "<< i;
        shared_ptr<oneSh> os = oneSh::create(os_n.str());
        
        std::stringstream oe_n;
        oe_n << "oneEx " <<i;
        shared_ptr<oneEx> oe = oneEx::create(oe_n.str());
        
        std::stringstream a1_n;
        a1_n << "assoc1 "<< i;
        shared_ptr<assoc1> a1 = assoc1::create(a1_n.str());
        oe->nested_one_ex(a1);
            
        std::stringstream a2_n;
        a2_n << "assoc2 "<< i;
        shared_ptr<assoc2> a2 = assoc2::create(a2_n.str());
        os->nested_one_sh(a2);
        
        tm->one_sh(os);
        tm->one_ex(oe);
        tm->many_sh(msv);
        tm->many_ex(mev);

	D::Transaction t (db->begin ());       
        db->persist<TypeMain>(tm);
	t.commit();
    }
    
    typedef odb::result<TypeMain> result;

    if(argc == 3 ){
      D::Transaction t (db->begin ());
      result r (db->query<TypeMain> (argv[1],argv[2]));
      cout << "result:" << endl;
      for (result::iterator i (r.begin ()); i != r.end (); ++i)
      {
          cout << (*i).name() << endl;
      }
      t.commit ();
    }
    return 0;
}
