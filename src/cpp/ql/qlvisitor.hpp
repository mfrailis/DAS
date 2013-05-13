#ifndef QLVISITOR_HPP
#define QLISITOR_HPP
#include "bnfc/Absyn.H"
#include "ddl/info.hpp"
#include <stack>
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

class Env{// TODO: implement operator= as a swap
public:
  type_e type_;
  std::string code_;
  Env& operator= (Env& rhs)
  {
      type_ = rhs.type_;
      swap(code_,rhs.code_);
      return *this;
  }
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
  inline std::string type_to_string(type_e t);
  inline type_e string_to_type(const std::string &t);
  inline type_e common(const Env& e1,const Env& e2);
  inline const char* compop_to_sql(const std::string &op);
  void visitBinExp(Exp *e1, Exp *e2, const std::string &op);
  void visitStrFun(ListName *listname);

  DdlInfo *info_;
  std::string base_type_;
  std::stack<Env> stack;
};

#endif
