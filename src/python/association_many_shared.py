def getter(association, pub_type, priv_type, class_name):
  return ['''
'''+class_name+'::'+pub_type+'''
'''+class_name+"::"+association.name +''' ()
{
  odb::transaction *transaction;

  '''+pub_type+''' associated;
  if(is_new())
  {
    // returns previously setted pointers on this transient object
    for('''+priv_type+'''::iterator i = '''+association.name+'''_.begin(); i != '''+association.name+'''_.end(); ++i)
      associated.push_back(i->get_eager());
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
        for('''+priv_type+'''::iterator i = '''+association.name+'''_.begin(); i != '''+association.name+'''_.end(); ++i)
          associated.push_back(i->load()); // load only if not already in cache
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
        for('''+priv_type+'''::iterator i = '''+association.name+'''_.begin(); i != '''+association.name+'''_.end(); ++i)
        {
          shared_ptr<'''+association.atype+'''> sp = i->load();
          if(!sp)
          {
#ifdef VDBG
            std::cout << "DAS error0003: association weak pointer expired" << std::endl;
#endif
            throw das::not_in_managed_context();
          }
          associated.push_back(sp);
        }
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
'''+class_name+"::"+association.name+" ("+pub_type+" &"+association.name+'''_new)
{
  '''+association.name+'''_.clear();
  for('''+pub_type+'''::const_iterator i = '''+association.name+'''_new.begin(); i != '''+association.name+'''_new.end(); ++i)
      '''+association.name+'''_.push_back(*i);
}''']
###############################################################################################################################################



###############################################################################################################################################
def update(association, priv_type):
    return '''  for('''+priv_type+'''::iterator i = '''+association.name+'''_.begin(); i != '''+association.name+'''_.end(); ++i)
  {
    shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = (*i).get_eager();
    if('''+association.name+'''_temp)
      '''+association.name+'''_temp->update();
  }
'''
###############################################################################################################################################



###############################################################################################################################################
def persist(association, priv_type):
    return '''  for('''+priv_type+'''::iterator i = '''+association.name+'''_.begin(); i != '''+association.name+'''_.end(); ++i)
  {
    shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = (*i).get_eager();
    db->persist<'''+association.atype+'''> ('''+association.name+'''_temp);
  }
'''
