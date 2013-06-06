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
    {
      associated.push_back(i->get_eager());
    }
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
        for('''+priv_type+'''::iterator i = '''+association.name+'''_.begin(); i != '''+association.name+'''_.end(); ++i)
          associated.push_back(i->load());
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
          associated.push_back(i->load());
        }
      }
      catch(odb::not_in_transaction &e)
      {
        throw das::not_in_managed_context();
      }
    }// if bundle.valid()
  }// if ix_new()   
  return associated;
}''']
###############################################################################################################################################



###############################################################################################################################################
def setter(association, pub_type, priv_type, class_name):
  return ['''
void
'''+class_name+"::"+association.name + " ("+pub_type+" &"+association.name+'''_new)
{
  '''+pub_type+''' current_vec;
  if(is_new())
  {
    current_vec = '''+association.name+'''(); //no loading from database implied because this is a new object
  }
  else //if is_new()
  {
    current_vec = '''+association.name+'''();
    das::tpl::DbBundle bundle = bundle_.lock();
    const shared_ptr<odb::database> &db = bundle.db();
    const shared_ptr<odb::session> &session = bundle.session();
    if(bundle.valid())
    {
      //check new association compatibility
      for ('''+pub_type+'''::iterator i = '''+association.name+'''_new.begin(); i != '''+association.name+'''_new.end(); ++i)
      {
        if(*i) // some pointers may be null
        {
          if(!(*i)->is_new())
          {
            das::tpl::DbBundle new_bundle = (*i)->bundle_.lock();
            if((new_bundle.valid() && new_bundle != bundle) ||
               (new_bundle.alias() != bundle.alias()))
            {
              throw das::wrong_database();
            }
          }
        }// should we throw an exception if any pointer is null?
      }
     
      // add old association to the cache if is not new
      for('''+pub_type+'''::iterator i= current_vec.begin(); i!=current_vec.end();i++)
      {
        if(!(*i)->is_new())
        {
          bundle.attach<'''+association.atype+'''>(*i);
        }
      }

    }
    else //(if bundle.valid())
    {
      for('''+pub_type+'''::iterator i= current_vec.begin(); i!=current_vec.end();i++)
      {
        if(!(*i)->is_new())
        {
          //WARNING: linear search
          '''+pub_type+'''::iterator found = std::find('''+association.name+'''_new.begin(),'''+association.name+'''_new.end(), *i);
          if(found == '''+association.name+'''_new.end())
          {
 #ifdef VDBG
            std::cout << "DAS error0010: trying to release associated object in a non managed context" << std::endl;
#endif
            throw das::not_in_managed_context();
          }
        }
      }

    }
  }

  // perform old association  decoupling
  for('''+pub_type+'''::iterator i= current_vec.begin(); i!=current_vec.end();i++)
  {
    (*i)->'''+class_name+'''_'''+association.name+'''_.reset();
    (*i)->is_dirty_ = true;
  }

  '''+association.name+'''_.clear();
  shared_ptr<'''+class_name+'''> self = self_.lock();

  for ('''+pub_type+'''::iterator i = '''+association.name+'''_new.begin(); i != '''+association.name+'''_new.end(); ++i){
    if(*i) // some pointers may be null
    {
      (*i)->'''+class_name+'''_'''+association.name+'''_ = self;
      (*i)->is_dirty_ = true;
      '''+association.name+'''_.push_back(*i);
    }// should we throw an exception if any pointer is null?
  }
}''']
###############################################################################################################################################



###############################################################################################################################################
def update(association, priv_type):
    return '''
  //das::tpl::DbBundle bundle = bundle_.lock();
  for('''+priv_type+'''::iterator i = '''+association.name+'''_.begin(); i != '''+association.name+'''_.end(); ++i)
  {
    shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = (*i).get_eager();
    if('''+association.name+'''_temp)
    {
      if('''+association.name+'''_temp->is_new())
      {
        bundle.persist<'''+association.atype+'''> ('''+association.name+'''_temp);
      }
      // call update anyways because of the nested associated objects
      '''+association.name+'''_temp->update();
    }
  }
'''
###############################################################################################################################################



###############################################################################################################################################
def persist(association, priv_type):
    return '''  for('''+priv_type+'''::iterator i = '''+association.name+'''_.begin(); i != '''+association.name+'''_.end(); ++i)
  {
    shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = (*i).get_eager();
    db.persist<'''+association.atype+'''> ('''+association.name+'''_temp);
  }
'''