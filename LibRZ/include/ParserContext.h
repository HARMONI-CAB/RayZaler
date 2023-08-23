#ifndef _PARSER_CONTEXT_H
#define _PARSER_CONTEXT_H

#include <Vector.h>
#include <list>
#include <string>
#include <variant>
#include <Recipe.h>

namespace RZ {
  class ParserContext;
}

int  yylex(RZ::ParserContext *ctx);
void yyerror(RZ::ParserContext *ctx, const char *msg);
int  yyparse(RZ::ParserContext *ctx);

namespace RZ {
  struct ParserDOFDecl {
    std::string name;
    std::string min_expr;
    std::string max_expr;
    std::string assign_expr;
  };

  typedef std::pair<std::string, std::string> ParserAssignExpr;
  typedef std::list<ParserAssignExpr> ParserAssignList;

  struct UndefinedValueType {};
  
  typedef std::variant<
    UndefinedValueType, 
    std::string,
    ParserDOFDecl,
    std::list<std::string>,
    ParserAssignExpr,
    ParserAssignList,
    RecipeContextType> BaseValueType;

  class ValueType : public BaseValueType {
      using BaseValueType::BaseValueType;

    public:
      static inline ValueType
      undefined()
      {
        return ValueType();
      }

      inline bool
      isUndefined() const
      {
        return index() == 0;
      }

      template<typename T>
      inline operator T() const
      {
        return std::get<T>(*this);
      }

      template<typename T>
      inline T &value()
      {
        return std::get<T>(*this);
      }

      inline std::string &
      str()
      {
        return value<std::string>();
      }

      inline std::list<std::string> &
      strList()
      {
        return value<std::list<std::string>>();
      }
  };


  enum ParserState {
    SEARCHING,
    READING_IDENTIFIER,
    READING_NUMBER,
    READING_OPERATOR
  };

  class ParserContext {
      std::string m_file = "<no file>";
      Recipe *m_rootRecipe = nullptr;
      Recipe *m_recipe = nullptr;
      std::string m_buf;
      std::string m_lastToken;
      int m_line = 0;
      int m_char = 0;
      int m_tokLine = 0;
      int m_tokChar = 0;
      bool m_commentFound = false;

      bool m_havePrevious = false;
      int m_saved = '\0';
      int m_last = '\0';

      std::list<ValueType> m_values;
      
      bool isTerminator(int) const;
      static bool isOperatorChar(int);
      static bool isIdStartChar(int);
      static bool isIdChar(int);
      static bool isValidStartChar(int);
      static void parseDOFDecl(ParserDOFDecl const &, Real &, Real &, Real &);
      static bool looksLikeNumber(std::string const &);

      int  getChar();
      bool returnChar();

      friend int  ::yylex(ParserContext *ctx);
      friend void ::yyerror(ParserContext *ctx, const char *msg);
      friend int  ::yyparse(RZ::ParserContext *ctx);

      
    protected:
      void setFile(std::string const &);
      int  tokenType() const;
      std::string token() const;
      int  lex();
      void error(const char *msg);

      void registerParameter(ParserDOFDecl const &);
      void registerDOF(ParserDOFDecl const &);
      void registerPath(std::string const &name, std::list<std::string> const &);
      void pushFrame(RecipeContextType, std::string const &name, ParserAssignList const & );
      void pushOnPort(std::string const &name, std::string const &port);
      void pushPort(std::string const &port);
      void pushElementDefinition(std::string const &);
      void popElementDefinition();
      void popFrame();
      void defineElement(std::string const &name, std::string const &factory, ParserAssignList const & = ParserAssignList());
      void debugParamList(ParserAssignList const &);

      template<class T> 
      ValueType &value()
      {
        m_values.push_back(T());
        return m_values.back();
      }

      ValueType &
      dofDecl(
        std::string const &name,
        std::string const &min = "",
        std::string const &max = "")
      {
        auto &decl = value<ParserDOFDecl>();
        decl.value<ParserDOFDecl>().name = name;
        decl.value<ParserDOFDecl>().min_expr = min;
        decl.value<ParserDOFDecl>().max_expr = max;
        return decl;
      }

      ValueType &
      pathList(std::string const &first)
      {
        auto &val = value<std::list<std::string>>();
        val.value<std::list<std::string>>().push_back(first);
        return val;
      }

      ValueType &
      contextType(RecipeContextType ctxType)
      {
        auto &val = value<RecipeContextType>();
        val = ctxType;
        return val;
      }

      ValueType &
      assignExpr(std::string const &param, std::string const &expr)
      {
        auto &val = value<ParserAssignExpr>();
        val.value<ParserAssignExpr>().first  = param;
        val.value<ParserAssignExpr>().second = expr;
        return val;
      }

      ValueType &
      assignExprList(ParserAssignExpr const &expr)
      {
        auto &val = value<ParserAssignList>();
        val.value<ParserAssignList>().push_back(expr);
        return val;
      }

      ValueType &
      assignExprList(std::string const &param, std::string const &expr)
      {
        auto &val = value<ParserAssignList>();
        val.value<ParserAssignList>().push_back(ParserAssignExpr(param, expr));
        return val;
      }

    public:
      ParserContext(Recipe *recipe);
      bool parse();
      virtual int read() = 0;

  };
}

#endif // _PARSER_CONTEXT_H
