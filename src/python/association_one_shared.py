def getter(association, pub_type, priv_type, class_name):
  return ['''
'''+pub_type+'''
'''+class_name+"::"+association.name + ''' ()
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
    }// if bundle.valid()
  }// if is_new() 
  return associated;
}''']
###############################################################################################################################################


###############################################################################################################################################
def setter(association, pub_type, priv_type, class_name):
  return ['''
void
'''+class_name+"::"+association.name + " ("+pub_type+" &"+association.name+'''_new)
{
  '''+association.name+"_ = "+association.name+'''_new;
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
