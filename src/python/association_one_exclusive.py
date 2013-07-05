def getter(association, pub_type, priv_type, class_name):
  return ['''
'''+pub_type+'''  
'''+class_name+"::"+association.name +''' ()
{
  '''+pub_type+''' associated;
  if(is_new())
  {
    // previously setted pointer on this transient object or null
    associated = '''+association.name+'''_.get_eager();
  }
  else
  {
    shared_ptr<odb::session> s = bundle_.lock_session(false);
    if(s)
      odb::session::current(*s);
    try
    {
      associated = '''+association.name+'''_.load();
    }
    catch(odb::not_in_transaction &e)
    {
      throw das::not_in_managed_context();
    }
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
    das::tpl::DbBundle b = bundle_.lock();
    const shared_ptr<odb::database> &db = b.db();
    shared_ptr<odb::session> s = b.lock_session(false);
    if(b.valid())
    {

      //check new association compatibility
      if(!'''+association.name+'''_new->is_new())
      {
        das::tpl::DbBundle new_bundle = '''+association.name+'''_new->bundle_.lock();
        if((new_bundle.valid() && new_bundle != b) ||
           (new_bundle.alias() != b.alias()))
        {
          throw das::wrong_database();
        }
      }
      if(current)
      {
      // add old association to the cache if is not new
        if(!current->is_new())
        {
          b.attach<'''+association.atype+'''>(current);
        }
      }
    }
    else //(if b.valid())
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
#persist_associated_post()
def persist(association, priv_type):
    return '''
  shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = '''+association.name+'''_.get_eager();
  if('''+association.name+'''_temp){ // the association may not be setted
    if('''+association.name+'''_temp->is_new())
      db.persist<'''+association.atype+'''> ('''+association.name+'''_temp);
    else
      // the foreign key nedds to be updated with the new one from this object
      '''+association.name+'''_temp->is_dirty_ = true;
  }
'''
###############################################################################################################################################


###############################################################################################################################################
def attach(association, priv_type):
  return '''
  shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = ptr->'''+association.name+'''_.get_eager();
  if('''+association.name+'''_temp && !'''+association.name+'''_temp->is_new()){ // the association may not be setted
      '''+association.atype+'''::attach('''+association.name+'''_temp,bundle);
  }
'''
