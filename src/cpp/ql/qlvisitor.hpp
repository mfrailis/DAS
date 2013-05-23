#ifndef QLVISITOR_HPP
#define QLISITOR_HPP
#include "bnfc/Absyn.H"
#include "ddl/info.hpp"
#include <stack>
#include <set>
#include <sstream>

enum type_e {
  t_byte   = 1,
  t_char   = 1,
  t_short  = 2,
  t_int    = 3,
  t_long   = 4,
  t_float  = 5,
  t_double = 6,
  t_bool   =16,
  t_string =32,
  t_text   =32
};
/*
class Env{// TODO: implement operator= as a swap
public:
  //  bool is_assoc_chain_;
  type_e type_;
  std::string code_;
  std::string current_type_;
  Env& operator= (Env& rhs)
  {
    //    is_assoc_chain_ = rhs.is_assoc_chain_;
    type_ = rhs.type_;
    current_type_ = rhs.current_type_;
    swap(code_,rhs.code_);
    return *this;
  }
  Env(const Env& rhs)
  {
    current_type_ = rhs.current_type_;
    //is_assoc_chain_ = rhs.assoc_chain_;
  }
  Env(){}
};
*/
class Env{
public:
  type_e type_;
  std::string direct_code_;
  std::string nested_code_;
  std::string join_code_;
  std::string current_type_;
  std::set<std::string> assoc_tables_;
  
  Env& operator= (Env& rhs)
  {
    type_ = rhs.type_;
    swap(direct_code_,rhs.direct_code_);
    swap(nested_code_,rhs.nested_code_);
    swap(join_code_,rhs.join_code_);
    swap(assoc_tables_,rhs.assoc_tables_);
    current_type_ = rhs.current_type_;

    return *this;
  }  
  Env(const Env& rhs)
  {
    current_type_ = rhs.current_type_;
    //is_assoc_chain_ = rhs.assoc_chain_;
  }
  Env(){}
};

class QLVisitor: public Visitor
{
public:
  virtual void visitBoolExp(BoolExp *p)    {};
  virtual void visitStrExp(StrExp *p)      {};
  virtual void visitCompExp(CompExp *p)    {};
  virtual void visitExp(Exp *p)            {};
  virtual void visitNumConst(NumConst *p)  {};
  virtual void visitName(Name *p)          {};

  virtual void visitBoolExpAnd(BoolExpAnd *p)  ;//done
  virtual void visitBoolExpOr(BoolExpOr *p)    ;//done
  virtual void visitBoolExpNot(BoolExpNot *p)  ;//done
  virtual void visitBoolCompExp(BoolCompExp *p){p->compexp_->accept(this);}
  virtual void visitBoolStrExp(BoolStrExp *p)  {p->strexp_->accept(this);}

  virtual void visitStartsWith(StartsWith *p) ;//done
  virtual void visitEndsWith(EndsWith *p)     ;//done

  virtual void visitCompExpAA(CompExpAA *p);//done
  virtual void visitCompExpSC(CompExpSC *p);//done
  virtual void visitCompExpCS(CompExpCS *p);//done
  virtual void visitCompExpBA(CompExpBA *p);//done
  virtual void visitCompExpAB(CompExpAB *p);//done

  virtual void visitExpSum(ExpSum *p){visitBinExp(p->exp_1,p->exp_2,"+");}
  virtual void visitExpSub(ExpSub *p){visitBinExp(p->exp_1,p->exp_2,"-");}
  virtual void visitExpMul(ExpMul *p){visitBinExp(p->exp_1,p->exp_2,"*");}
  virtual void visitExpDiv(ExpDiv *p){visitBinExp(p->exp_1,p->exp_2,"/");}

  virtual void visitExpConst(ExpConst *p)  {p->numconst_->accept(this);}
  virtual void visitExpName(ExpName *p)    {p->listname_->accept(this);}
  virtual void visitConstFloat(ConstFloat *p)   ; //done
  virtual void visitConstInt(ConstInt *p)       ;
  virtual void visitKeyword(Keyword *p)         ;//done
  virtual void visitListName(ListName *p)       ;//done


  virtual void visitInteger(Integer x)     {}
  virtual void visitChar(Char x)           {}
  virtual void visitDouble(Double x)       {}
  virtual void visitString(String x)       {}
  virtual void visitIdent(Ident x)         {}
  virtual void visitCompOp(CompOp x)       {}
  virtual void visitStr(Str x)              ;  //done
  virtual void visitBoolConst(BoolConst x) {}

  QLVisitor(const std::string &query_type, DdlInfo *info)
  {
    info_ = info;
    base_type_ = query_type;
  }


  std::string parse_exp(const std::string &expression);
  std::string parse_ord(const std::string &expression);
private:
  std::string type_to_string(type_e t);
  type_e string_to_type(const std::string &t);
  type_e common(const Env& e1,const Env& e2);
  const char* compop_to_sql(const std::string &op);
  void gen_join_code(const AssociationInfo& info);
  void gen_nested(Env& e);
  void merge_envs(Env& e1, Env& e2);
  void visitBinExp(Exp *e1, Exp *e2, const std::string &op);
  void visitStrFun(ListName *listname);

  DdlInfo *info_;
  std::string base_type_;
  std::stack<Env> stack;
};

#endif
