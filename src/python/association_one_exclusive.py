def getter(association, pub_type, priv_type, class_name):
  return ['''
'''+pub_type+'''  
'''+class_name+"::"+association.name +''' ()
{
  odb::transaction *transaction;

  '''+pub_type+''' associated;
  if(is_new())
  {
    // returns previously setted pointer on this transient object
    associated = '''+association.name+'''_.get_eager();
  }
  else
  {
    das::tpl::DbBundle bundle = bundle_.lock();
    shared_ptr<odb::database> db = bundle.db();
    shared_ptr<odb::session> session = bundle.session();
    if(bundle.valid())
    {
      odb::session::current(*session);
      bool local_trans = !odb::transaction::has_current();
      if(local_trans)
      {
        transaction = new odb::transaction(db->begin());
      }
      else
      {
        transaction = &odb::transaction::current();
      }
      try
      {
        associated = '''+association.name+'''_.load();
      }
      catch(std::exception &e)
      {
        if(local_trans)
        {
          transaction->rollback();
          delete transaction;
        }
        throw;
      }
      if(local_trans)
      {
        transaction->commit();
        delete transaction;
      }
    }
    else
    {
      try
      {
        associated = '''+association.name+'''_.load();
      }
      catch(odb::not_in_transaction &e)
      {
        throw das::not_in_managed_context();
      }
    }// if bundle.expired()
  }// if if_new()
  return associated;
}''']
###############################################################################################################################################


###############################################################################################################################################
def setter(association, pub_type, priv_type, class_name):
  return ['''
void
'''+class_name+"::"+association.name + " ("+pub_type+" &"+association.name+'''_new)
{
  if(is_new())
  {
/*    if(!'''+association.name+'''_new->is_new())
    {
#ifdef VDBG
      std::cout << "DAS error0004: object needs to be persisted before get setted by already persistent objects" << std::endl;
#endif      
      throw das::not_in_managed_context();
    }
*/
    shared_ptr<'''+association.atype+'''> current = '''+association.name +'''_.get_eager();
    if(current)
    {
      current->'''+class_name+'''_'''+association.name+'''_.reset();
      current->is_dirty_ = true;
    }
  }
  else //if is_new()
  {
    shared_ptr<'''+association.atype+'''> current = '''+association.name +'''();
    das::tpl::DbBundle bundle = bundle_.lock();
    shared_ptr<odb::database> db = bundle.db();
    shared_ptr<odb::session> session = bundle.session();
    if(bundle.valid())
    {

      //check new association compatibility
      if(!'''+association.name+'''_new->is_new())
      {
        shared_ptr<odb::database> db_n = '''+association.name+'''_new->bundle_.db();
        shared_ptr<odb::session> session_n = '''+association.name+'''_new->bundle_.session();
        if(!'''+association.name+'''_new->bundle_.expired() && 
           ( db_n != db ||
             session_n != session ||
             '''+association.name+'''_new->bundle_.alias() != bundle_.alias()))
        {
          throw das::wrong_database();
        }
      }
      if(current)
      {
      // add old association to the cache if is not new
        if(!current->is_new())
        {
          bundle.attach<'''+association.atype+'''>(current);
        }

        // perform old association  decoupling
        current->'''+class_name+'''_'''+association.name+'''_.reset();
        current->is_dirty_ = true;
      }
    }
    else //(if bundle.valid())
    {
      if((!current || current->is_new()) && '''+association.name+'''_new->is_new())
      {
        if(current)
        {
          current->'''+class_name+'''_'''+association.name+'''_.reset();
          current->is_dirty_ = true; 
        }       
      }
      else
      {
#ifdef VDBG
        std::cout << "DAS error0007: trying to set new association in a detached object with non new objects" << std::endl;
#endif
        throw das::not_in_managed_context();
      }
    }
  }
  '''+association.name+'''_new->'''+class_name+'''_'''+association.name+'''_ = self_.lock();
  '''+association.name+'''_new->is_dirty_ = true;
}''']
###############################################################################################################################################


###############################################################################################################################################
def update(association, priv_type):
    return '''
  shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = '''+association.name+'''_.get_eager();
  if('''+association.name+'''_temp)
    '''+association.name+'''_temp->update();
''' 
###############################################################################################################################################


###############################################################################################################################################
def persist(association, priv_type):
    return '''
  shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = '''+association.name+'''_.get_eager();
  if('''+association.name+'''_temp) // the association may not be setted
    db->persist<'''+association.atype+'''> ('''+association.name+'''_temp);
'''
