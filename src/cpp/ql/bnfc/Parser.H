#ifndef PARSER_HEADER_FILE
#define PARSER_HEADER_FILE

#include<vector>
#include<string>


class BoolExp;
class StrExp;
class CompExp;
class Exp;
class NumConst;
class Name;
class ListName;

typedef union
{
  long long int_;
  char char_;
  double double_;
  char* string_;
  BoolExp* boolexp_;
  StrExp* strexp_;
  CompExp* compexp_;
  Exp* exp_;
  NumConst* numconst_;
  Name* name_;
  ListName* listname_;
} YYSTYPE;

BoolExp* pBoolExp(FILE *inp);
BoolExp* pBoolExp(const char *str);



#define _ERROR_ 258
#define _SYMB_0 259
#define _SYMB_1 260
#define _SYMB_2 261
#define _SYMB_3 262
#define _SYMB_4 263
#define _SYMB_5 264
#define _SYMB_6 265
#define _SYMB_7 266
#define _SYMB_8 267
#define _SYMB_9 268
#define _SYMB_10 269
#define _SYMB_11 270
#define _SYMB_12 271
#define _SYMB_13 272
#define _SYMB_14 273
#define _INTEGER_ 274
#define _DOUBLE_ 275
#define _IDENT_ 276

extern YYSTYPE yylval;

#endif
