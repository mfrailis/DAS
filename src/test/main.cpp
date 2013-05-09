#include <iostream>

#include "tpl/Database.hpp"
#include "ddl_types.hpp"

using namespace std;
namespace D  = das::tpl;
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

  return 0;
}

