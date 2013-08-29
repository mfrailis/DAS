#include "transaction.hpp"
#include "tpl/database.hpp"

using namespace das;

Transaction::Transaction(const shared_ptr<TransactionBundle> &tb) : tb_(tb){}

void
Transaction::commit() {
    if (!tb_ || !tb_->transaction_)
        throw das::invalid_transaction();
    
    tb_->flush_data();   //may modify data references
    tb_->flush_session();
    
    for(TransactionBundle::data_list_type::iterator it = tb_->data_list_.begin();
            it != tb_->data_list_.end(); ++it)
        (*it)->commit();
    
    tb_->session_.reset();    
    tb_->data_list_.clear();
    tb_->transaction_->commit();
    tb_->transaction_.reset();
    tb_.reset();
}

void
Transaction::rollback() {
    if (!tb_ || !tb_->transaction_)
        throw das::invalid_transaction();

   
    for(TransactionBundle::data_list_type::iterator it = tb_->data_list_.begin();
            it != tb_->data_list_.end(); ++it)
        (*it)->rollback();

    tb_->session_.reset();    
    tb_->data_list_.clear();
    tb_->transaction_->rollback();
    tb_->transaction_.reset();
    tb_.reset();
}
