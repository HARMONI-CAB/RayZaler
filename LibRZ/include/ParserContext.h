//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//

#ifndef _PARSER_CONTEXT_H
#define _PARSER_CONTEXT_H

#include <Vector.h>
#include <list>
#include <string>
#include <variant>
#include <Recipe.h>
#include <stdexcept>

#define PARSER_CONTEXT_MAX_RECURSION 20

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
  
  class ParserError : public std::runtime_error {
    std::string m_msg;
    std::string m_file;
    int m_line = 0;
    int m_char = 0;

  public:
    ParserError();
    ParserError(std::string const &, int, int, std::string const &);

    inline int
    line() const
    {
      return m_line;
    }

    inline int
    col() const
    {
      return m_char;
    }

    inline std::string const &
    message() const
    {
      return m_msg;
    }

    inline std::string const &
    file() const
    {
      return m_file;
    }
  };

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
    READING_OPERATOR,
    READING_STRING
  };

  class ParserContext {
      std::string m_file = "<no file>";
      ParserContext *m_parentContext = nullptr;
      Recipe *m_rootRecipe = nullptr;
      Recipe *m_recipe = nullptr;
      std::string m_buf;
      std::string m_lastToken;
      std::list<std::set<std::string>> m_includeOnceContexts;
      std::set<std::string>  m_includeOnce;
      std::list<std::string> m_searchPaths;

      int m_recursion = 0;
      int m_line = 0;
      int m_char = 0;
      bool m_newLine = false;
      int m_tokLine = 0;
      int m_tokChar = 0;
      bool m_commentFound = false;
      bool m_escaped = false;
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
      bool lexNonString(int c);
      bool lexString(int c);
      std::string resolvePath(std::string const &path);
      friend int  ::yylex(ParserContext *ctx);
      friend void ::yyerror(ParserContext *ctx, const char *msg);
      friend int  ::yyparse(RZ::ParserContext *ctx);

      void pushIncludeOnce();
      bool popIncludeOnce();
      
    protected:
      int  tokenType() const;
      std::string token() const;
      int  lex();
      void error(const char *msg);

      void import(std::string const &);
      void script(std::string const &);
      void registerParameter(ParserDOFDecl const &);
      void registerDOF(ParserDOFDecl const &);
      void registerVariable(ParserAssignExpr const &);
      void registerPath(std::string const &name, std::list<std::string> const &);
      void pushFrame(RecipeContextType, std::string const &name, ParserAssignList const & );
      void pushOnPort(std::string const &name, std::string const &port);
      void pushPort(std::string const &port);
      void pushElementDefinition(std::string const &);
      void popElementDefinition();
      void popFrame();
      void defineElement(std::string const &name, std::string const &factory, ParserAssignList const & = ParserAssignList());
      void debugParamList(ParserAssignList const &);
      bool alreadyImported(std::string const &path) const;
      void addImportOnce(std::string const &path);

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
      assignString(std::string const &param, std::string const &str)
      {
        auto &val = value<ParserAssignExpr>();
        val.value<ParserAssignExpr>().first  = param;
        val.value<ParserAssignExpr>().second = "\"" + str + "\"";
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

      ValueType &
      assignStringList(std::string const &param, std::string const &str)
      {
        auto &val = value<ParserAssignList>();
        val.value<ParserAssignList>().push_back(
          ParserAssignExpr(param, '"' + str + '"'));
        return val;
      }

    public:
      ParserContext(Recipe *recipe, int recursion = 0);
      ParserContext(ParserContext *parent, int recursion);

      inline Recipe *
      recipe() const
      {
        return m_recipe;
      }

      inline Recipe *
      rootRecipe() const
      {
        return m_rootRecipe;
      }
      
      void setFile(std::string const &);
      bool parse();
      void addSearchPath(std::string const &path);
      void inheritSearchPaths(ParserContext const *);
      virtual int read() = 0;

  };

  class FileParserContext : public RZ::ParserContext {
      using ParserContext::ParserContext;
      FILE *m_fp = stdin;

    public:
      void setFile(FILE *fp, std::string const &name);
      virtual int read() override;
      virtual ~FileParserContext();
  };

  class StringParserContext : public RZ::ParserContext {
      using ParserContext::ParserContext;
      std::string m_contents;
      unsigned int m_ptr = 0;
      
    public:
      void setContents(std::string const &contents, std::string const &name);
      virtual int read() override;
      virtual ~StringParserContext();
  };
}

#endif // _PARSER_CONTEXT_H
