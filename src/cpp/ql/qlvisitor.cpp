#include "internal/qlvisitor.hpp"
#include "exceptions.hpp"
#include "bnfc/Parser.H"


#include <boost/tokenizer.hpp>
#include <string>
#include <utility>

struct OrdTokenizer{
  bool operator() (
		   std::string::const_iterator& next,
		   std::string::const_iterator& end,
		   std::pair<std::string,std::string> &p)
  {
    bool has_key = false;
    std::string keyword, direction;
    while(next != end)if(*next == ' ')next++;else break;
    while(next != end){
      if(*next == ' ')
	{
	  has_key = true;
	  break;
	}
      else
	{
	  keyword.push_back(*next);
	  next++;
	}
    }
    while(next != end)if(*next == ' ')next++;else break;
    while(next != end){
      if(*next == ',')
	{
	  next++;		
	  break;
	}
      else
	{
	  direction.push_back(*next);
	  next++;
	}
    }	
    p.first = keyword;
    p.second = direction;	
    return has_key;
  }
  void reset(){}
};

std::string QLVisitor::parse_ord(const std::string &expression)
{
  std::string clause;

  if(expression == "")
    return clause;

  clause = " ORDER BY  ";
  boost::tokenizer<OrdTokenizer,
		   std::string::const_iterator,
		   std::pair<std::string,std::string>
		   >
    tok(expression);
  for(boost::tokenizer<
	OrdTokenizer,
	std::string::const_iterator,
	std::pair<std::string,std::string>
	>::iterator ord = tok.begin(); 
      ord != tok.end();
      ++ord)
    {
      info_->get_keyword_info(base_type_,(*ord).first); //we just need to verify if exists
      if((*ord).second == "asc" || (*ord).second == "ascending")
	clause.append((*ord).first+" asc, ");
      else if((*ord).second == "desc" || (*ord).second == "descending" || (*ord).second == "des")
	clause.append((*ord).first+" desc, ");
      else
	throw das::bad_ordering_clause();
    }
  clause[clause.size()-2]=' '; // delete the last coma
  return clause;
}

void  QLVisitor::visitConstInt(ConstInt *p)
{
  Integer x = p->integer_;
  char  c = (char)  x;
  short s = (short) x;
  int   i = (int)   x;

  if(x == c)
    stack.top().type_=t_byte;
  else if(x == s)
    stack.top().type_=t_short;
  else if(x == i)
    stack.top().type_=t_int;
  else
    stack.top().type_=t_long;

#ifdef VDBG
  std::cout << "long long ("<< x <<") coerced to " << type_to_string(stack.top().type_) << std::endl;
#endif

  std::stringstream ss;
  ss << x;
  stack.top().direct_code_.append(ss.str());
}

void  QLVisitor::visitConstFloat(ConstFloat *p)
{
  Double x = p->double_;
  float f = (float) x;
  if(x == f)
    stack.top().type_=t_float;
  else
    stack.top().type_=t_double;

#ifdef VDBG
  std::cout << "double ("<< x <<") coerced to " << type_to_string(stack.top().type_) << std::endl;
#endif

  std::stringstream ss;
  ss << x;
  stack.top().direct_code_.append(ss.str());
}

void QLVisitor::visitStr(String x)
{
  stack.top().type_=t_string;
  stack.top().direct_code_.append(x);
}

void QLVisitor::visitListName(ListName *p)
{
  stack.top().assoc_tables_.insert(stack.top().current_type_); //base type
  try
    {
      for(unsigned int i = 0; i < p->size()-1; ++i)
	{
	  const AssociationInfo& info = info_->
	    get_association_info(stack.top().current_type_,  static_cast<Keyword*>(p->at(i))->ident_);
	  gen_join_code(info);
	  stack.top().assoc_tables_.insert(info.association_table);
	  stack.top().assoc_tables_.insert(info.association_type);
	  stack.top().current_type_ = info.association_type;
	}
    }
  catch(std::out_of_range &e)
    {
      throw das::association_not_present();
    }
  p->at(p->size()-1)->accept(this);
}

void QLVisitor::visitKeyword(Keyword *p)
{
  try
    {
      stack.top().type_ = string_to_type(info_->get_keyword_info( stack.top().current_type_, p->ident_).type);
      //      stack.top().type_strict_ = true;
    }
  catch(std::out_of_range &e)
    {
      throw das::keyword_not_present();
    }
  
  if(stack.top().join_code_ != "")
    {
      stack.top().nested_code_.append(stack.top().current_type_);
      stack.top().nested_code_.append(".");
      stack.top().nested_code_.append(p->ident_);
    }
  else
    {
      stack.top().direct_code_.append(stack.top().current_type_);
      stack.top().direct_code_.append(".");
      stack.top().direct_code_.append(p->ident_);     
    }
}

void QLVisitor::visitBinExp(Exp *exp_1, Exp *exp_2,const std::string &op)
{
  Env e1(stack.top());
  Env e2(stack.top());
  stack.push(e1);
  exp_1->accept(this);

  e1 = stack.top();
  stack.pop();
  stack.push(e2);
  if(e1.type_ > t_double)
    throw das::bad_type();

  exp_2->accept(this);
  e2 = stack.top();
  stack.pop();

  if(e2.type_ > t_double)
    throw das::bad_type();

  stack.top().type_ = common(e1,e2);
  stack.top().join_code_ = e1.join_code_;
  
  if(e1.join_code_ != "" && e2.join_code_ != "")
    stack.top().join_code_.append(" AND ");

  stack.top().join_code_.append(e2.join_code_);
  stack.top().assoc_tables_.insert(e1.assoc_tables_.begin(),e1.assoc_tables_.end());
  stack.top().assoc_tables_.insert(e2.assoc_tables_.begin(),e2.assoc_tables_.end());
   
  if(e1.join_code_ != "" || e2.join_code_ != "")
    {
      if(e1.join_code_ != "")
	stack.top().nested_code_.append("("+e1.nested_code_+op);
      else
	stack.top().nested_code_.append("("+e1.direct_code_+op);
      
      if(e2.join_code_ != "")
	stack.top().nested_code_.append(e2.nested_code_+")");
      else
	stack.top().nested_code_.append(e2.direct_code_+")");
    }
  else
    stack.top().direct_code_.append("("+e1.direct_code_+op+e2.direct_code_+")");
}



void QLVisitor::visitCompExpAA(CompExpAA *p)
{
  stack.top().type_ = t_bool; //we shouldn't neeed this

  Env e1(stack.top());
  Env e2(stack.top());
  stack.push(e1);
  p->exp_1->accept(this);

  e1 = stack.top();
  stack.pop();

  stack.push(e2);
  p->exp_2->accept(this);
  e2 = stack.top();
  stack.pop();

  type_e t = common (e1,e2);

  if(p->compop_ != "==" && p->compop_ != "!=" && t == t_bool)
    throw das::bad_type();
  /*
  if(e1.type_strict_ && e1.type_ < t)
    throw das::bad_type();

  if(e2.type_strict_ && e2.type_ < t)
    throw das::bad_type();    
  */
  stack.top().join_code_ = e1.join_code_;
  
  if(e1.join_code_ != "" && e2.join_code_ != "")
    stack.top().join_code_.append(" AND ");

  stack.top().join_code_.append(e2.join_code_);
  stack.top().assoc_tables_.insert(e1.assoc_tables_.begin(),e1.assoc_tables_.end());
  stack.top().assoc_tables_.insert(e2.assoc_tables_.begin(),e2.assoc_tables_.end());
  
  
  if(e1.join_code_ != "" || e2.join_code_ != "")
    {
      if(e1.join_code_ != "")
	stack.top().nested_code_.append("("+e1.nested_code_);
      else
	stack.top().nested_code_.append("("+e1.direct_code_);
      
      stack.top().nested_code_.append(compop_to_sql(p->compop_));
      
      if(e2.join_code_ != "")
	stack.top().nested_code_.append(e2.nested_code_+")");
      else
	stack.top().nested_code_.append(e2.direct_code_+")");      
    }
  else
    {
      stack.top().direct_code_.append("("+e1.direct_code_);
      stack.top().direct_code_.append(compop_to_sql(p->compop_));
      stack.top().direct_code_.append(e2.direct_code_+")");
    }
}

void QLVisitor::visitCompExpSC(CompExpSC *p)
{
  stack.top().type_ = t_bool; //we shouldn't neeed this

  Env e(stack.top());
  stack.push(e);
  p->exp_->accept(this);

  e = stack.top();
  stack.pop();

  if(e.type_ != t_string)
    throw das::bad_type();

  stack.top().join_code_ = e.join_code_;
  stack.top().assoc_tables_.insert(e.assoc_tables_.begin(),e.assoc_tables_.end());
  
  if(e.join_code_ != "")
    {
      stack.top().nested_code_.append("("+p->str_);
      stack.top().nested_code_.append(compop_to_sql(p->compop_));
      stack.top().nested_code_.append(e.nested_code_+ ")");      
    }
  else
    {
      stack.top().direct_code_.append("("+p->str_);
      stack.top().direct_code_.append(compop_to_sql(p->compop_));
      stack.top().direct_code_.append(e.direct_code_+ ")");
    }
}

void QLVisitor::visitCompExpCS(CompExpCS *p)
{
  stack.top().type_ = t_bool; //we shouldn't neeed this

  Env e(stack.top());
  stack.push(e);
  p->exp_->accept(this);

  e = stack.top();
  stack.pop();

  stack.top().join_code_ = e.join_code_;
  stack.top().assoc_tables_.insert(e.assoc_tables_.begin(),e.assoc_tables_.end());
  
  if(e.type_ != t_string)
    throw das::bad_type();
  
  if(e.join_code_ != "")
    {
      stack.top().nested_code_.append("(" + e.nested_code_);
      stack.top().nested_code_.append(compop_to_sql(p->compop_));
      stack.top().nested_code_.append(p->str_ +")");     
    }
  else
    {
      stack.top().direct_code_.append("(" + e.direct_code_);
      stack.top().direct_code_.append(compop_to_sql(p->compop_));
      stack.top().direct_code_.append(p->str_ +")");     
    }


}

void QLVisitor::visitCompExpBA(CompExpBA *p)
{
  stack.top().type_ = t_bool; //we shouldn't neeed this

  Env e(stack.top());
  stack.push(e);
  p->exp_->accept(this);

  e = stack.top();
  stack.pop();

  if(e.type_ != t_bool)
    throw das::bad_type();

  if(p->compop_ != "==" && p->compop_ != "!=")
    throw das::bad_type();

  stack.top().join_code_ = e.join_code_;
  stack.top().assoc_tables_.insert(e.assoc_tables_.begin(),e.assoc_tables_.end());
  
  if(e.join_code_ != "")
    {
      stack.top().nested_code_.append(e.nested_code_);
      stack.top().nested_code_.append(compop_to_sql(p->compop_));       
      stack.top().nested_code_.append(p->boolconst_);
    }
  else
    {
      stack.top().direct_code_.append(e.direct_code_);
      stack.top().direct_code_.append(compop_to_sql(p->compop_));       
      stack.top().direct_code_.append(p->boolconst_);
    }
}

void QLVisitor::visitCompExpAB(CompExpAB *p)
{
  stack.top().type_ = t_bool; //we shouldn't neeed this

  Env e(stack.top());
  stack.push(e);
  p->exp_->accept(this);

  e = stack.top();
  stack.pop();

  if(e.type_ != t_bool)
    throw das::bad_type();

  if(p->compop_ != "==" && p->compop_ != "!=")
    throw das::bad_type();

  stack.top().join_code_ = e.join_code_;
  stack.top().assoc_tables_.insert(e.assoc_tables_.begin(),e.assoc_tables_.end());
  
  if(e.join_code_ != "")
    {
      stack.top().nested_code_.append(e.nested_code_);
      stack.top().nested_code_.append(compop_to_sql(p->compop_));       
      stack.top().nested_code_.append(p->boolconst_);
    }
  else
    {
      stack.top().direct_code_.append(e.direct_code_);
      stack.top().direct_code_.append(compop_to_sql(p->compop_));       
      stack.top().direct_code_.append(p->boolconst_);
    }
}


void QLVisitor::visitStartsWith(StartsWith *p)
{
  stack.top().type_ = t_bool; //we shouldn't neeed this

  Env e(stack.top());
  stack.push(e);
  p->listname_->accept(this);

  e = stack.top();
  stack.pop();

  if(e.type_ != t_string)
    throw das::bad_type();

  stack.top().join_code_ = e.join_code_;
  stack.top().assoc_tables_.insert(e.assoc_tables_.begin(),e.assoc_tables_.end());
  
  std::string::iterator end = p->str_.end();
  if(e.join_code_ != "")
    {
      stack.top().nested_code_.append(e.nested_code_ +" LIKE ");
      stack.top().nested_code_.append(p->str_.begin(),--end);
      stack.top().nested_code_.append("%' ");
    }
  else
    {
      stack.top().direct_code_.append(e.direct_code_ +" LIKE ");
      stack.top().direct_code_.append(p->str_.begin(),--end);
      stack.top().direct_code_.append("%' ");
    }
}

void QLVisitor::visitEndsWith(EndsWith *p)
{
  stack.top().type_ = t_bool; //we shouldn't neeed this

  Env e(stack.top());
  stack.push(e);
  p->listname_->accept(this);

  e = stack.top();
  stack.pop();

  if(e.type_ != t_string)
    throw das::bad_type();

  stack.top().join_code_ = e.join_code_;
  stack.top().assoc_tables_.insert(e.assoc_tables_.begin(),e.assoc_tables_.end());
  
  std::string::iterator start = p->str_.begin();
  if(e.join_code_ != "")
    {
      stack.top().nested_code_.append(e.nested_code_ +" LIKE '%");
      stack.top().nested_code_.append(++start,p->str_.end());
      stack.top().nested_code_.append(" ");  
    }
  else
    {
      stack.top().direct_code_.append(e.direct_code_ +" LIKE '%");
      stack.top().direct_code_.append(++start,p->str_.end());
      stack.top().direct_code_.append(" ");
    }
}

void QLVisitor::visitBoolExpAnd(BoolExpAnd *p)
{
  Env e1(stack.top());
  Env e2(stack.top());
  stack.push(e1);
  p->boolexp_1->accept(this);
  e1 = stack.top();
  stack.pop();

  stack.push(e2);
  p->boolexp_2->accept(this);
  e2 = stack.top();
  stack.pop();
  
  if(e1.join_code_ != "" || e2.join_code_ != "")
    {
      if(e1.join_code_ != "" && e2.join_code_ != "")
	{
          stack.top().join_code_ = e1.join_code_;
          
          if(e1.join_code_ != "" && e2.join_code_ != "")
	    stack.top().join_code_.append(" AND ");
          
          stack.top().join_code_.append(e2.join_code_);
          stack.top().assoc_tables_.insert(e1.assoc_tables_.begin(),e1.assoc_tables_.end());
          stack.top().assoc_tables_.insert(e2.assoc_tables_.begin(),e2.assoc_tables_.end());
 
          stack.top().nested_code_.append("("+e1.nested_code_+" AND "+e2.nested_code_+")");
	}
      else if(e1.join_code_ != "")
	{
          stack.top().direct_code_.append("( ");
          gen_nested(e1);
          stack.top().direct_code_.append(" AND ");
          stack.top().direct_code_.append(e2.direct_code_+")");
	}
      else
	{
	  stack.top().direct_code_.append("("+e1.direct_code_+" AND ");
	  gen_nested(e2);
	  stack.top().direct_code_.append(")"); 
         
	}
    }
  else
    stack.top().direct_code_.append("("+e1.direct_code_+" AND "+e2.direct_code_+")");
}

void QLVisitor::visitBoolExpOr(BoolExpOr *p)
{
  Env e1(stack.top());
  Env e2(stack.top());
  stack.push(e1);
  p->boolexp_1->accept(this);
  e1 = stack.top();
  stack.pop();

  stack.push(e2);
  p->boolexp_2->accept(this);
  e2 = stack.top();
  stack.pop();

  if(e1.join_code_ != "" || e2.join_code_ != "")
    {
      if(e1.join_code_ != "" && e2.join_code_ != "")
	{
          stack.top().join_code_ = e1.join_code_;
          
          if(e1.join_code_ != "" && e2.join_code_ != "")
	    stack.top().join_code_.append(" AND ");
          
          stack.top().join_code_.append(e2.join_code_);
          stack.top().assoc_tables_.insert(e1.assoc_tables_.begin(),e1.assoc_tables_.end());
          stack.top().assoc_tables_.insert(e2.assoc_tables_.begin(),e2.assoc_tables_.end());
 
          stack.top().nested_code_.append("("+e1.nested_code_+" OR "+e2.nested_code_+")");
	}
      else if(e1.join_code_ != "")
	{
          stack.top().direct_code_.append("( ");
          gen_nested(e1);
          stack.top().direct_code_.append(" OR ");
          stack.top().direct_code_.append(e2.direct_code_+")");
	}
      else
	{
	  stack.top().direct_code_.append("("+e1.direct_code_+" OR ");
	  gen_nested(e2);
	  stack.top().direct_code_.append(")"); 
         
	}
    }
  else
    stack.top().direct_code_.append("("+e1.direct_code_+" OR "+e2.direct_code_+")");
}

void QLVisitor::visitBoolExpNot(BoolExpNot *p)
{
  Env e(stack.top());
  stack.push(e);
  p->boolexp_->accept(this);
  e = stack.top();
  stack.pop();
     
  stack.top().join_code_ = e.join_code_;
  stack.top().assoc_tables_.insert(e.assoc_tables_.begin(),e.assoc_tables_.end());
  
  if(e.join_code_ != "")
    stack.top().nested_code_.append("NOT ("+e.nested_code_+")");
  else
    stack.top().direct_code_.append("NOT ("+e.direct_code_+")");
}

inline
type_e
QLVisitor::common(const Env& e1,const Env& e2)
{
  if(e1.type_ == e2.type_)
    return e1.type_;
  if(e1.type_ <= t_double && e2.type_ <= t_double)
    return (e1.type_ > e2.type_)?e1.type_:e2.type_;

  throw das::non_compatible_types();
}

inline
const char*
QLVisitor::compop_to_sql(const std::string &op)
{
  if(op == "==")
    {
      return "=";
    }
  else if(op == "!=")
    {
      return "<>";
    }
  else
    {
      return op.c_str();
    }

}

inline
type_e
QLVisitor::string_to_type(const std::string &t)
{
  if(t == "byte")
    return t_byte;
  if(t == "char")
    return t_char;
  if(t == "int16")
    return t_short;
  if(t == "int32")
    return t_int;
  if(t == "int64")
    return t_long;
  if(t == "float32")
    return t_float;
  if(t == "float64")
    return t_double;
  else if(t == "boolean")
    return t_bool;
  else if(t == "string")
    return t_string;
  else if(t == "text")
    return t_text;

  throw das::unknown_kewyword_type();
}

inline
std::string
QLVisitor::type_to_string(type_e t)
{
  switch(t)
    {
    case t_char:
      return "char";
    case t_byte:
      return "byte";
    case t_short:
      return "int16";
    case t_int:
      return "int32";
    case t_long:
      return "int64";
    case t_float:
      return "float32";
    case t_double:
      return "float64";
    case t_bool:
      return "boolean";
    case t_string:
      return "string";
    default:
      return "unknown";
    }
}

std::string QLVisitor::parse_exp(const std::string &expression) 
//TODO: implement it properly
{
  BoolExp *parse_tree = pBoolExp(expression.c_str());
  if(parse_tree)
    {
      Env start;
      start.current_type_ = base_type_;
      stack.push(start);
      parse_tree->accept(this);
      gen_nested(stack.top());
      std::string s = stack.top().direct_code_;
      stack.pop();
      delete parse_tree;
      return s;
    }
  else
    {
      delete parse_tree;
      throw das::incomplete_statement();
    }
}

inline
void
QLVisitor::gen_join_code(const AssociationInfo &info)
{
  Env &top = stack.top();
  if(top.join_code_ != "")
    top.join_code_.append(" AND ");

  if(info.association_key != "" && info.object_key != "") // table of associations
    {
      top.join_code_.append(top.current_type_);
      top.join_code_.append(".das_id = ");
      top.join_code_.append(info.association_table);
      top.join_code_.append(".");
      top.join_code_.append(info.object_key);
      top.join_code_.append(" AND ");
      top.join_code_.append(info.association_type);
      top.join_code_.append(".das_id = ");
      top.join_code_.append(info.association_table);
      top.join_code_.append(".");
      top.join_code_.append(info.association_key);
    }
  else
    {
      if(info.association_key == "") // foreign key on associated type
	{
	  top.join_code_.append(top.current_type_);
	  top.join_code_.append(".das_id = ");
	  top.join_code_.append(info.association_table);
	  top.join_code_.append(".");
	  top.join_code_.append(info.object_key);	  
	}
      else  // foreign key on base type
	{
	  top.join_code_.append(info.association_type);
	  top.join_code_.append(".das_id = ");
	  top.join_code_.append(info.association_table);
	  top.join_code_.append(".");
	  top.join_code_.append(info.association_key);	  
	}
    }
}

inline
void
QLVisitor::gen_nested(Env& e)
{
  if(e.join_code_ == "")
    return;
    
  Env &top = stack.top();
  e.assoc_tables_.insert(top.current_type_);
  top.direct_code_.append(top.current_type_+".das_id IN (\n");
  top.direct_code_.append("SELECT "+top.current_type_+".das_id\n");
  top.direct_code_.append("FROM ");
  for (
       std::set<std::string>::iterator i = e.assoc_tables_.begin();
       i != e.assoc_tables_.end();
       ++i)
    {
      top.direct_code_.append(" "+*i+",");  
    }
  top.direct_code_[top.direct_code_.size()-1]='\n';
  top.direct_code_.append("WHERE ");
  top.direct_code_.append(e.join_code_);
  top.direct_code_.append(" AND\n(");
  top.direct_code_.append(e.nested_code_);
  top.direct_code_.append(")\n)\n");
}
