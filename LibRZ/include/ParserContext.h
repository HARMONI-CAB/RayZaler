#ifndef _PARSER_CONTEXT_H
#define _PARSER_CONTEXT_H

#include <Vector.h>

namespace RZ {
  class ParserContext;
}

int  yylex(RZ::ParserContext *ctx);
void yyerror(RZ::ParserContext *ctx, const char *msg);

namespace RZ {
  enum TokenType {
    TT_ROTATE_KEYWORD,
    TT_TRANSLATE_KEYWORD,
    TT_PATH_KEYWORD,
    TT_TO_KEYWORD,
    TT_PARAM_KEYWORD,
    TT_DOF_KEYWORD
  };

  enum ValueType {
    VT_REAL,
    VT_STRING
  };

  struct ValueStruct {
    ValueType type;
    union {
      RZ::Real asReal;
      char *asString;
    };
  };

  class ParserContext;

  class ParserContext {
    protected:
      int  lex();
      void error(const char *msg);

    public:
      friend int  ::yylex(ParserContext *ctx);
      friend void ::yyerror(ParserContext *ctx, const char *msg);
  };
}

#endif // _PARSER_CONTEXT_H
