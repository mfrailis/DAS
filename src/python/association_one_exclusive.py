def getter(association, pub_type, priv_type, class_name):
  return ['''
'''+pub_type+'''  
'''+class_name+"::"+association.name +''' ()
{
  odb::transaction *transaction;

  '''+pub_type+''' associated;
  if(is_new())
  {
    // previously setted pointer on this transient object or null
    associated = '''+association.name+'''_.get_eager();
  }
  else
  {
    das::tpl::DbBundle bundle = bundle_.lock();
    const shared_ptr<odb::database> &db = bundle.db();
    const shared_ptr<odb::session> &session = bundle.session();
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
  shared_ptr<'''+association.atype+'''> current;
  
  if(is_new())
  {
    current = '''+association.name +'''_.get_eager();
  }
  else //if is_new()
  {
    shared_ptr<'''+association.atype+'''> current = '''+association.name +'''();
    das::tpl::DbBundle bundle = bundle_.lock();
    const shared_ptr<odb::database> &db = bundle.db();
    const shared_ptr<odb::session> &session = bundle.session();
    if(bundle.valid())
    {

      //check new association compatibility
      if(!'''+association.name+'''_new->is_new())
      {
        das::tpl::DbBundle new_bundle = '''+association.name+'''_new->bundle_.lock();
        if((new_bundle.valid() && new_bundle != bundle) ||
           (new_bundle.alias() != bundle.alias()))
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
      }
    }
    else //(if bundle.valid())
    {
      if(current && !current->is_new())
      {
#ifdef VDBG
        std::cout << "DAS error0007: trying to set new association in a detached object with non new objects associated" << std::endl;
#endif
        throw das::not_in_managed_context();
      }
    }
  }

  if(current == '''+association.name+'''_new)
    return;

  if(current)
  {
    // perform old association  decoupling
    current->'''+class_name+'''_'''+association.name+'''_.reset();
    current->is_dirty_ = true;
  }
  '''+association.name+'''_new->'''+class_name+'''_'''+association.name+'''_ = self_.lock();
  '''+association.name+'''_new->is_dirty_ = true;

  '''+association.name+'''_ = '''+association.name+'''_new;
}''']
###############################################################################################################################################


###############################################################################################################################################
def update(association, priv_type):
    return '''
  shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = '''+association.name+'''_.get_eager();
  if('''+association.name+'''_temp)
  {
    if('''+association.name+'''_temp->is_new())
    {
      //das::tpl::DbBundle bundle = bundle_.lock();
      bundle.persist<'''+association.atype+'''> ('''+association.name+'''_temp);
    }
    // call update anyways because of the nested associated objects
    '''+association.name+'''_temp->update();
  }
''' 
###############################################################################################################################################


###############################################################################################################################################
def persist(association, priv_type):
    return '''
  shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = '''+association.name+'''_.get_eager();
  if('''+association.name+'''_temp) // the association may not be setted
    db.persist<'''+association.atype+'''> ('''+association.name+'''_temp);
'''