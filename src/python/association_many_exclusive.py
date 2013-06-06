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
    }// if bundle.expired()
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
  if(is_new())
  {
/*    for('''+pub_type+'''::iterator i='''+association.name+'''_new.begin();i!='''+association.name+'''_new.end();i++)
    {
      if(!(*i)->is_new())
      {
#ifdef VDBG
        std::cout << "DAS error0017: object needs to be persisted before been setted by already persistent objects" << std::endl;
#endif      
        throw das::not_in_managed_context();
      }
    }
*/
    '''+pub_type+''' current_vec = '''+association.name+'''(); //no loading from database implied because this a new object
    for('''+pub_type+'''::iterator i= current_vec.begin(); i!=current_vec.end();i++)
    {
      (*i)->'''+class_name+'''_'''+association.name+'''_.reset();
      (*i)->is_dirty_ = true;
    }
  }
  else //if is_new()
  {
    '''+pub_type+''' current_vec = '''+association.name+'''();
    das::tpl::DbBundle bundle = bundle_.lock();
    shared_ptr<odb::database> db = bundle.db();
    shared_ptr<odb::session> session = bundle.session();
    if(bundle.valid())
    {
      //check new association compatibility
       for ('''+pub_type+'''::iterator i = '''+association.name+'''_new.begin(); i != '''+association.name+'''_new.end(); ++i)
       {
         if(*i) // some pointers may be null
         {
           if(!(*i)->is_new())
           {
             shared_ptr<odb::database> db_n = (*i)->bundle_.db();
             shared_ptr<odb::session> session_n = (*i)->bundle_.session();
             if(!(*i)->bundle_.expired() && 
                ( db_n != db ||
                  session_n != session ||
                  (*i)->bundle_.alias() != bundle_.alias()))
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

      // perform old association  decoupling
      for('''+pub_type+'''::iterator i= current_vec.begin(); i!=current_vec.end();i++)
      {
        (*i)->'''+class_name+'''_'''+association.name+'''_.reset();
        (*i)->is_dirty_ = true;
      }

    }
    else //(if bundle.valid())
    {
      for('''+pub_type+'''::iterator i= current_vec.begin(); i!=current_vec.end();i++)
      {
        if(!(*i)->is_new())
        {
 #ifdef VDBG
          std::cout << "DAS error0010: trying to set new association in a detached object with non new objects" << std::endl;
#endif
          throw das::not_in_managed_context();
        }
      }

       for ('''+pub_type+'''::iterator i = '''+association.name+'''_new.begin(); i != '''+association.name+'''_new.end(); ++i)
       {
         if(*i) // some pointers may be null
         {
           if(!(*i)->is_new())
           {
 #ifdef VDBG
             std::cout << "DAS error0011: trying to set new association in a detached object with non new objects" << std::endl;
#endif
             throw das::not_in_managed_context();
           }
         }
       }
      // perform old association  decoupling
      for('''+pub_type+'''::iterator i= current_vec.begin(); i!=current_vec.end();i++)
      {
        (*i)->'''+class_name+'''_'''+association.name+'''_.reset();
        (*i)->is_dirty_ = true;
      }
    }
// not new
  }
  '''+association.name+'''_.clear();
  shared_ptr<'''+class_name+'''> self = self_.lock();
  for ('''+pub_type+'''::iterator i = '''+association.name+'''_new.begin(); i != '''+association.name+'''_new.end(); ++i){
    if(*i) // some pointers may be null
    {
      (*i)->'''+class_name+'''_'''+association.name+'''_ = self;
      (*i)->is_dirty_ = true;
    }// should we throw an exception if any pointer is null?
    '''+association.name+'''_.push_back(*i);
  }
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
