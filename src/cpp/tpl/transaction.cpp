#include "tpl/transaction.hpp"
#include "tpl/database.hpp"

using namespace das::tpl;

Transaction::Transaction(const DbBundle &b, const shared_ptr<odb::session> &s, shared_ptr<odb::transaction> t)
: b_(b), session_(s), transaction_(t) {
}

void
Transaction::commit() {
    if (!transaction_)
        throw das::invalid_transaction();

    b_.flush_session();
    session_.reset();
    transaction_->commit();
    transaction_.reset();
}

void
inline
Transaction::rollback() {
    if (!transaction_)
        throw das::invalid_transaction();

    session_.reset();
    transaction_->rollback();
    transaction_.reset();
}
