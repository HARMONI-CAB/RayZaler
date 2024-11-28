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

#include <ParserContext.h>
#include "parser.h"
#include <ctype.h>
#include <algorithm>
#include <libgen.h>
#include <unistd.h>
#include <fcntl.h>
#include <Logger.h>

int
yylex(RZ::ParserContext *ctx)
{
  return ctx->lex();
}

void
yyerror(RZ::ParserContext *ctx, const char *msg)
{
  ctx->error(msg);
}

using namespace RZ;

ParserError::ParserError() : std::runtime_error("No error")
{

}

//
// Character and columns represented in the error message differ from those
// in the ParserError. In particular:
// - m_line: represents the line index (starting by zero) of the error.
// - m_char: represents the number of consumed characters in the current line
// before triggering the error (starting by zero).
//
// Since errors in a given line can only occur right after the first character
// of the line was consumed, m_char will always be at least 1 (hence the offending
// character is at m_char - 1)
//
// Since people expect lines and columns to start from 1, the error message must
// state that the failing line is m_line + 1. On the other hand, since the
// offending character is m_char - 1, we must report m_char - 1 + 1 = m_char
//

ParserError::ParserError(
  std::string const &file,
  int line,
  int col,
  std::string const &msg) : 
  std::runtime_error(file + ":" + std::to_string(line + 1) + ":" + std::to_string(col) + ": " + msg)
{
  m_file = file;
  m_line = line;
  m_char = col;
  m_msg  = msg;
}

bool
ParserContext::isOperatorChar(int c)
{
  return std::string("+-/*^();,{}!<=>").find(c) != std::string::npos;
}

bool
ParserContext::isIdStartChar(int c)
{
  return isalpha(c) || c == '_' || c == '.';
}

bool
ParserContext::isIdChar(int c)
{
  return isalnum(c) || c == '_' || c == '.';
}

bool
ParserContext::isTerminator(int c) const
{
  if (m_buf.size() == 0)
    return false;

  // Check for numbers
  if (looksLikeNumber(m_buf)) {
    c = tolower(c);
    if (isdigit(c))
      return false;

    if (m_buf.find('e') == std::string::npos) {
      // No exponent: exponent declaration and decimal point allowed
      if (c == 'e')
        return false;
      
      if (c == '.' && m_buf.find('.') == std::string::npos)
        return false;
    } else {
      // Exponent part.
      if (m_buf[m_buf.size() - 1] == 'e') {
        // Signs and numbers are allowed right after the exponent
        if (isdigit(c) || c == '+' || c == '-')
          return false;
        
      } else {
        // Here, only numbers are allowed
        return !isdigit(c);
      }
    }

    return true;
  } else if (isIdStartChar(m_buf[0])) {
    return !isIdChar(c);
  } else if (isOperatorChar(m_buf[0])) {
    if (m_buf.size() == 1 && c == '=') {
      const char *compStarts = "!<=>";

      // If the first character is either one of the "comparison operator
      // starts", it is NOT a terminator.
      return strchr(compStarts, m_buf[0]) == nullptr;
    }

    return true;
  }

  return true;
}

bool
ParserContext::isValidStartChar(int c)
{
  return isOperatorChar(c) || isalnum(c) || c == '.' || c == '"';
}

int
ParserContext::getChar()
{
  if (m_havePrevious) {
    m_havePrevious = false;
    return m_saved;
  }

  m_last = read();
  if (m_newLine) {
    m_newLine = false;
    m_char = 0;
    ++m_line;
  }

  if (m_last == '\n') {
    m_newLine = true;
    m_commentFound = false;
  } 

  ++m_char;

  return m_last;
}

void
ParserContext::registerVariable(ParserAssignExpr const &expr)
{
  m_recipe->pushVariable(expr.first, expr.second);
}

bool
ParserContext::returnChar()
{
  if (m_havePrevious)
    return false;

  m_saved = m_last;
  m_havePrevious = true;

  return true;
}

bool
ParserContext::lexString(int c)
{
  bool tokenCompleted = false;

  if (c == EOF) {
    throw std::runtime_error("Unexpected end-of-file when reading string");
  } else {
    if (m_escaped) {
      switch (c) {
        case 'n':
          c = '\n';
          break;
        
        case 'b':
          c = '\b';
          break;
        
        case 't':
          c = '\t';
          break;
        
        case 'v':
          c = '\v';
          break;
        
        case 'r':
          c = '\r';
          break;

        case '"':
          c = '"';
          break;

        default:
          throw std::runtime_error("Unknown escape sequence `\\" + std::to_string(c) + "`");
      }

      m_escaped = false;
    } else {
      if (c == '\\')
        m_escaped = true;
      else if (c == '"')
        tokenCompleted = true;
    }
  }

  if (!tokenCompleted && !m_escaped)
    m_buf.push_back(c);

  return tokenCompleted;
}

bool
ParserContext::lexNonString(int c)
{
  bool tokenCompleted = false;

  if (c == EOF) {
    tokenCompleted = true;
  } else {
    tokenCompleted = isTerminator(c);
    if (tokenCompleted)
      returnChar();
  }

  if (!tokenCompleted)
    m_buf.push_back(c);

  return tokenCompleted;
}

enum ParserLexingMode {
  
};

int
ParserContext::lex()
{
  bool tokenCompleted = false;
  bool stringMode     = false;

  m_buf.clear();
  
  m_tokLine = m_line;
  m_tokChar = m_char;

  do {
    int c = getChar();

    if (!stringMode) {
      if (c == '#')
        m_commentFound = true;

      if (m_commentFound)
        continue;
      
      if (m_buf.size() == 0) {
        if (c == EOF)
          return YYEOF;

        // First character
        if (isspace(c))
          continue;

        if (!isValidStartChar(c))
          return YYUNDEF;
        
        if (c == '"') {
          stringMode = true;
          m_escaped  = false;
        } else {
          m_buf.push_back(c);
        }
      } else {
        tokenCompleted = lexNonString(c);
      }
    } else {
      tokenCompleted = lexString(c);
    }
  } while (!tokenCompleted);

  m_lastToken = m_buf;
  yylval = token();
  m_buf.clear();

  if (stringMode) {
    return STRING;
  } else {
    if (m_lastToken == "true")
      m_lastToken = "1";
    else if (m_lastToken == "false")
      m_lastToken = "0";
    return tokenType();
  }
}

void
ParserContext::setFile(std::string const &file)
{
  m_file = file;
}

bool
ParserContext::looksLikeNumber(std::string const &string)
{
  if (isdigit(string[0]))
    return true;

  if (string.size() > 1 && string[0] == '.' && isdigit(string[1]))
    return true;

  return false;
}

int
ParserContext::tokenType() const
{  
  if (looksLikeNumber(m_lastToken)) {
    return NUM;
  } else if (m_lastToken[0] == '"') {
    return STRING;
  } else if (isIdStartChar(m_lastToken[0])) {
    // Identify keyword
    if (m_lastToken == "rotate")
      return ROTATE_KEYWORD;
    else if (m_lastToken == "translate")
      return TRANSLATE_KEYWORD;
    else if (m_lastToken == "path")
      return PATH_KEYWORD;
    else if (m_lastToken == "to")
      return TO_KEYWORD;
    else if (m_lastToken == "parameter")
      return PARAM_KEYWORD;
    else if (m_lastToken == "dof")
      return DOF_KEYWORD;
    else if (m_lastToken == "on")
      return ON_KEYWORD;
    else if (m_lastToken == "of")
      return OF_KEYWORD;
    else if (m_lastToken == "element")
      return ELEMENT_KEYWORD;
    else if (m_lastToken == "port")
      return PORT_KEYWORD;
    else if (m_lastToken == "import")
      return IMPORT_KEYWORD;
    else if (m_lastToken == "script")
      return SCRIPT_KEYWORD;
    else if (m_lastToken == "var")
      return VAR_KEYWORD;
    else
      return IDENTIFIER;
  } else if (isOperatorChar(m_lastToken[0])) {
    if (m_lastToken == "=="
          || m_lastToken == "!="
          || m_lastToken == ">="
          || m_lastToken == "<="
          || m_lastToken == "<"
          || m_lastToken == ">")
      return COMP_OP;
    
    return m_lastToken[0]; // Self-identifying token
  }

  return YYUNDEF;
}

void
ParserContext::parseDOFDecl(ParserDOFDecl const &decl, Real &min, Real &max, Real &defVal)
{
  if (decl.min_expr.size() == 0)
    min = -INFINITY;
  else if (sscanf(decl.min_expr.c_str(), "%lg", &min) < 1)
    throw std::runtime_error("Invalid real literal `" + decl.min_expr + "'");
  
  if (decl.max_expr.size() == 0)
    max = +INFINITY;
  else if (sscanf(decl.max_expr.c_str(), "%lg", &max) < 1)
    throw std::runtime_error("Invalid real literal `" + decl.max_expr + "'");
  
  if (min > max)
    throw std::runtime_error("Lower bound of parameter/DOF (" + std::to_string(min) + ") is greater than its upper bound (" + std::to_string(max) + ")");
  
  if (decl.assign_expr.size() == 0)
    defVal = 0;
  else if (sscanf(decl.assign_expr.c_str(), "%lg", &defVal) < 1)
    throw std::runtime_error("Invalid real literal `" + decl.assign_expr + "'");
  else if (defVal < min || defVal > max)
    throw std::runtime_error("Default value out of bounds");

  if (defVal < min)
    defVal = min;

  if (defVal > max)
    defVal = max;
}

void
ParserContext::registerParameter(ParserDOFDecl const &decl)
{
  Real min, max, defVal;
  parseDOFDecl(decl, min, max, defVal);
  m_recipe->addParam(decl.name, defVal, min, max);
}

void
ParserContext::registerDOF(ParserDOFDecl const &decl)
{
  Real min, max, defVal;
  parseDOFDecl(decl, min, max, defVal);
  m_recipe->addDof(decl.name, defVal, min, max);
}

void
ParserContext::registerPath(std::string const &name, std::list<std::string> const &list)
{
  auto path = m_recipe->allocatePath(name);

  for (auto p : list)
    path->plug(p);
}

void
ParserContext::pushFrame(RecipeContextType type, std::string const &name, ParserAssignList const &list)
{
  if (type == RECIPE_CONTEXT_TYPE_ROTATION) {
    RecipeParamListProto proto;

    proto.pushParam("angle");
    proto.pushParam("ex", "0");
    proto.pushParam("ey", "0");
    proto.pushParam("ez", "1");

    for (auto p : list)
      proto.set(p.first, p.second);

    if (!proto.isSet("angle"))
      throw std::runtime_error("Undefined rotation angle");

    m_recipe->pushRotation(
      proto.values["angle"],
      proto.values["ex"],
      proto.values["ey"],
      proto.values["ez"],
      name);
  } else if (type == RECIPE_CONTEXT_TYPE_TRANSLATION) {
    RecipeParamListProto proto;

    proto.pushParam("dx", "0");
    proto.pushParam("dy", "0");
    proto.pushParam("dz", "0");

    for (auto p : list)
      proto.set(p.first, p.second);

    m_recipe->pushTranslation(
      proto.values["dx"],
      proto.values["dy"],
      proto.values["dz"],
      name);
  } else {
    throw std::runtime_error("Invalid frame type");
  }
}

void
ParserContext::pushOnPort(std::string const &name, std::string const &port)
{
  auto element = m_recipe->resolveElement(name);

  if (element == nullptr)
    throw std::runtime_error("Element `" + name + "' does not exist");

  m_recipe->pushPortContext(element, port);
}

std::string
ParserContext::resolvePath(std::string const &path)
{
  std::string absPath;

  // Relative filename
  if (path[0] != '/') {
    for (auto p : m_searchPaths) {
      absPath = p + "/" + path;
      if (access(absPath.c_str(), F_OK) != -1)
        return absPath;
    }

    return "";
  }

  return path;
}

void
ParserContext::script(std::string const &path)
{
  std::string absPath;

  absPath = resolvePath(path);
  if (absPath.empty())
    throw std::runtime_error("Cannot resolve relative script path `" + path + "'");
  
  m_recipe->addScript(absPath);
}

void
ParserContext::addImportOnce(std::string const &path)
{
  if (m_parentContext != nullptr) {
    m_parentContext->addImportOnce(path);
    return;
  }

  if (alreadyImported(path))
    return;
  
  m_includeOnce.insert(path);
}

bool
ParserContext::alreadyImported(std::string const &path) const
{
  if (m_parentContext != nullptr)
    return m_parentContext->alreadyImported(path);
  
  return m_includeOnce.find(path) != m_includeOnce.end();
}

void
ParserContext::import(std::string const &path)
{
  FileParserContext *nestedContext = nullptr;
  std::string absPath, dirName;
  std::string error;
  FILE *fp = nullptr;

  if (m_recursion >= PARSER_CONTEXT_MAX_RECURSION)
    throw std::runtime_error("Too many nested imports");
  
  absPath = resolvePath(path);
  if (absPath.empty())
    throw std::runtime_error("Cannot resolve relative import `" + path + "'");

  if (alreadyImported(absPath))
    return;

  fp = fopen(absPath.c_str(), "r");
  if (fp == nullptr) {
    throw std::runtime_error(
      "Cannot open import file `" 
      + absPath 
      + "': " 
      + strerror(errno));
  }

  std::string absPathCopy = absPath;

  dirName = dirname(absPathCopy.data());

  nestedContext = new FileParserContext(this, m_recursion + 1);
  nestedContext->addSearchPath(dirName);
  nestedContext->inheritSearchPaths(this);
  nestedContext->setFile(fp, path);

  try {
    addImportOnce(absPath);
    nestedContext->parse();
  } catch (std::runtime_error const &e) {
    error = "In file import:\n" + std::string(e.what());
  }

  if (nestedContext != nullptr)
    delete nestedContext;

  if (error.size() > 0)
    throw std::runtime_error(error);
}

void
ParserContext::popFrame()
{
  m_recipe->pop();
}

void
ParserContext::pushPort(std::string const &port)
{
  m_recipe->addPort(port);
}

void
ParserContext::pushElementDefinition(std::string const &name)
{
  Recipe *recipe = m_recipe->makeCustomElement(name);

  if (recipe == nullptr)
    throw std::runtime_error("Element class `" + name + "' redefined");

  m_recipe = recipe;
}

void
ParserContext::popElementDefinition()
{
  if (m_rootRecipe == m_recipe)
    throw std::runtime_error("Internal error: attempting to leave root recipe!");

  m_recipe = m_recipe->parent();
}

void
ParserContext::defineElement(std::string const &name, std::string const &factory, ParserAssignList const &list)
{
  auto step = m_recipe->addElement(name, factory);

  for (auto p : list)
    step->set(p.first, p.second);
}

void
ParserContext::debugParamList(ParserAssignList const &list)
{
  for (auto p : list) {
    printf("  %s = %s\n", p.first.c_str(), p.second.c_str());
  }
}

std::string
ParserContext::token() const
{
  return m_lastToken;
}

void
ParserContext::error(const char *msg)
{
  throw std::runtime_error(msg);
}

ParserContext::ParserContext(Recipe *recipe, int recursion) :
  m_rootRecipe(recipe)
{
  setlocale(LC_NUMERIC, "C");
  m_recipe    = m_rootRecipe;
  m_recursion = recursion;
}

ParserContext::ParserContext(ParserContext *prev, int recursion) :
  ParserContext(prev->m_recipe, recursion)
{
  m_parentContext = prev;
}

bool
ParserContext::parse()
{
  try {
    yyparse(this);
  } catch (std::runtime_error const &e) {
    throw ParserError(m_file, m_tokLine, m_tokChar, e.what());
  }

  return true;
}

void
ParserContext::addSearchPath(std::string const &path)
{
  if (std::find(m_searchPaths.begin(), m_searchPaths.end(), path) 
    == m_searchPaths.end())
      m_searchPaths.push_back(path);

  m_recipe->pushSearchPath(path);
}

void
ParserContext::inheritSearchPaths(ParserContext const *context)
{
  for (auto p : context->m_searchPaths)
    addSearchPath(p);
}

//////////////////////////// File parser context ///////////////////////////////
void
FileParserContext::setFile(FILE *fp, std::string const &name)
{
  m_fp = fp;
  ParserContext::setFile(name);
}

int
FileParserContext::read()
{
  return fgetc(m_fp);
}

FileParserContext::~FileParserContext()
{
  if (m_fp != nullptr && m_fp != stdin)
    fclose(m_fp);
}

//////////////////////////// String parser context /////////////////////////////
void
StringParserContext::setContents(std::string const &contents, std::string const &name)
{
  m_contents = contents;
  m_ptr = 0;

  ParserContext::setFile(name);
}

int
StringParserContext::read()
{
  if (m_ptr < m_contents.size())
    return m_contents[m_ptr++];

  return -1;
}

StringParserContext::~StringParserContext()
{
  
}
