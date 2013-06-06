#include "qlvisitor.hpp"
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
    stack.top().type_=t_char;
  else if(x == s)
    stack.top().type_=t_short;
  else if(x == i)
    stack.top().type_=t_int;
  else
    stack.top().type_=t_long;

  std::stringstream ss;
  ss << x;
  stack.top().code_.append(ss.str());
}

void  QLVisitor::visitConstFloat(ConstFloat *p)
{
  Double x = p->double_;
  float f = (float) x;
  if(x == f)
    stack.top().type_=t_float;
  else
    stack.top().type_=t_double;

  std::stringstream ss;
  ss << x;
  stack.top().code_.append(ss.str());
}

void QLVisitor::visitStr(String x)
{
  stack.top().type_=t_string;
  stack.top().code_.append(x);
}

void QLVisitor::visitListName(ListName *p)
{
  try
    {
      for(unsigned int i = 0; i < p->size()-1; ++i)
	{
	  const std::string &new_type =  info_->get_association_type(stack.top().current_type_,  static_cast<Keyword*>(p->at(i))->ident_);
	  stack.top().current_type_ = new_type;
	  // TODO add association table to env;
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
    }
  catch(std::out_of_range &e)
    {
      throw das::keyword_not_present();
    }
  stack.top().code_.append(p->ident_);
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
  stack.top().code_.append("("+e1.code_+op+e2.code_+")");
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

  stack.top().code_.append("("+e1.code_);
  stack.top().code_.append(compop_to_sql(p->compop_));
  stack.top().code_.append(e2.code_+")");
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

  stack.top().code_.append("("+p->str_);
  stack.top().code_.append(compop_to_sql(p->compop_));
  stack.top().code_.append(e.code_+ ")");
}

void QLVisitor::visitCompExpCS(CompExpCS *p)
{
  stack.top().type_ = t_bool; //we shouldn't neeed this

  Env e(stack.top());
  stack.push(e);
  p->exp_->accept(this);

  e = stack.top();
  stack.pop();

  if(e.type_ != t_string)
    throw das::bad_type();
  stack.top().code_.append("(" + e.code_);
  stack.top().code_.append(compop_to_sql(p->compop_));
  stack.top().code_.append(p->str_ +")");

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

  stack.top().code_.append(p->boolconst_);
  stack.top().code_.append(compop_to_sql(p->compop_));
  stack.top().code_.append(e.code_);
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

  stack.top().code_.append(e.code_);
  stack.top().code_.append(compop_to_sql(p->compop_));
  stack.top().code_.append(p->boolconst_);
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

  std::string::iterator end = p->str_.end();
  stack.top().code_.append(e.code_ +" LIKE ");
  stack.top().code_.append(p->str_.begin(),--end);
  stack.top().code_.append("%' ");
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

  std::string::iterator start = p->str_.begin();
  stack.top().code_.append(e.code_ +" LIKE '%");
  stack.top().code_.append(++start,p->str_.end());
  stack.top().code_.append(" ");
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

  stack.top().code_.append("("+e1.code_+" AND "+e2.code_+")");
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

  stack.top().code_.append("("+e1.code_+" OR "+e2.code_+")");
}

void QLVisitor::visitBoolExpNot(BoolExpNot *p)
{
  Env e(stack.top());
  stack.push(e);
  p->boolexp_->accept(this);
  e = stack.top();
  stack.pop();

  stack.top().code_.append("NOT ("+e.code_+")");
}

type_e QLVisitor::common(const Env& e1,const Env& e2)
{
  if(e1.type_ == e2.type_)
    return e1.type_;
  if(e1.type_ <= t_double && e2.type_ <= t_double)
    return (e1.type_ > e2.type_)?e1.type_:e2.type_;

  throw das::non_compatible_types();
}

const char* QLVisitor::compop_to_sql(const std::string &op)
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

type_e QLVisitor::string_to_type(const std::string &t)
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

std::string QLVisitor::type_to_string(type_e t)
{
  switch(t)
    {
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
      std::string s = stack.top().code_;
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
