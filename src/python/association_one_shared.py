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
    shared_ptr<odb::session> s = bundle_.lock_session(false);
    if(s)
    {
      odb::session::current(*s);
    }
    try
    {
      associated = '''+association.name+'''_.load();
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
#persist_associated_pre()
def persist(association, priv_type):
    return '''
  shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = '''+association.name+'''_.get_eager();
  if('''+association.name+'''_temp) // the association may not be setted
    db.persist<'''+association.atype+'''> ('''+association.name+'''_temp);

'''
###############################################################################################################################################


###############################################################################################################################################
def attach(association, priv_type):
  return '''
  shared_ptr<'''+association.atype+'''> '''+association.name+'''_temp = ptr->'''+association.name+'''_.get_eager();
  if('''+association.name+'''_temp && !'''+association.name+'''_temp->is_new()) // the association may not be setted
    '''+association.atype+'''::attach('''+association.name+'''_temp,bundle);

'''
