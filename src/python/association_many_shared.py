def getter(association, pub_type, priv_type, class_name):
  return ['''
'''+class_name+'::'+pub_type+'''
'''+class_name+"::"+association.name +''' ()
{
  '''+pub_type+''' associated;
  if(is_new())
  {
    // returns previously setted pointers on this transient object
    for('''+priv_type+'''::iterator i = '''+association.name+'''_.begin(); i != '''+association.name+'''_.end(); ++i)
      associated.push_back(i->get_eager());
  }
  else
  {
    shared_ptr<odb::session> s = bundle_.lock_session(false);
    if(s)
    {
      odb::session::current(*s);
    }
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
    return '''
  for('''+priv_type+'''::iterator i = '''+association.name+'''_.begin(); i != '''+association.name+'''_.end(); ++i)
  {
    shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = (*i).get_eager();
    if('''+association.name+'''_temp)
    {
      if('''+association.name+'''_temp->is_new())
      {
        tb.persist<'''+association.atype+'''> ('''+association.name+'''_temp);
      }
      // call update anyways because of the nested associated objects
      '''+association.name+'''_temp->update(tb);
    }
  }
'''
###############################################################################################################################################



###############################################################################################################################################
#persist_associated_pre()
def persist(association, priv_type):
    return '''  for('''+priv_type+'''::iterator i = '''+association.name+'''_.begin(); i != '''+association.name+'''_.end(); ++i)
  {
    shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = (*i).get_eager();
    tb.persist<'''+association.atype+'''> ('''+association.name+'''_temp);
  }
'''
###############################################################################################################################################



###############################################################################################################################################
def attach(association, priv_type):
    return '''  for('''+priv_type+'''::iterator i = ptr->'''+association.name+'''_.begin(); i != ptr->'''+association.name+'''_.end(); ++i)
  {
    shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = (*i).get_eager();
    if('''+association.name+'''_temp && !'''+association.name+'''_temp->is_new()) 
      '''+association.atype+'''::attach('''+association.name+'''_temp,bundle);
  }
'''
