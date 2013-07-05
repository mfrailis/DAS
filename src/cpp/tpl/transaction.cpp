#include "tpl/transaction.hpp"
#include "tpl/database.hpp"

using namespace das::tpl;

Transaction::Transaction(const DbBundle &b,const shared_ptr<odb::session> &s)
: b_(b), session_(s) {
    transaction_.reset(new odb::transaction(b_.db()->begin()));
}

void
Transaction::commit() {
    b_.flush_session();
    session_.reset();
    transaction_->commit();
}

void
inline
Transaction::rollback(){
    session_.reset();
    transaction_->rollback();
}
