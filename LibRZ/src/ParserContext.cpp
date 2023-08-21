#include <ParserContext.h>
#include "parser.h"
#include <ctype.h>

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

bool
ParserContext::isOperatorChar(int c)
{
  return std::string("+-/*^();,={}").find(c) != std::string::npos;
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
  if (isdigit(m_buf[0])) {
    c = tolower(c);

    if (isdigit(c))
      return false;

    if (m_buf.find('e') == std::string::npos) {
      if (c == 'e')
        return false;
      
      if (c == '.' && m_buf.find('.') == std::string::npos)
        return false;
    } else {
      if (m_buf[m_buf.size() - 1] == 'e') {
        if (!isdigit(c) && c != '+' && c != '-')
          return true;
      } else {
        return !isdigit(c);
      }
    }

    return true;
  } else if (isIdStartChar(m_buf[0])) {
    return !isIdChar(c);
  } else if (isOperatorChar(m_buf[0])) {
    return true;
  }

  return true;
}

bool
ParserContext::isValidStartChar(int c)
{
  return isOperatorChar(c) || isalnum(c);
}

int
ParserContext::getChar()
{
  if (m_havePrevious) {
    m_havePrevious = false;
    return m_saved;
  }

  m_last = read();
  if (m_last == '\n') {
    m_char = 0;
    ++m_line;
    m_commentFound = false;
  } else {
    ++m_char;
  }

  return m_last;
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

int
ParserContext::lex()
{
  bool tokenCompleted = false;

  m_buf.clear();
  
  m_tokLine = m_line;
  m_tokChar = m_char;

  do {
    int c = getChar();

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
      
      m_buf.push_back(c);
    } else {
      if (c == EOF) {
        tokenCompleted = true;
      } else {
        tokenCompleted = isTerminator(c);
        if (tokenCompleted)
          returnChar();
      }

      if (!tokenCompleted)
        m_buf.push_back(c);
    }
  } while (!tokenCompleted);

  m_lastToken = m_buf;
  yylval = token();
  m_buf.clear();

  return tokenType();
}

void
ParserContext::setFile(std::string const &file)
{
  m_file = file;
}

int
ParserContext::tokenType() const
{
  if (isdigit(m_lastToken[0])) {
    return NUM;
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
    else
      return IDENTIFIER;
  } else if (isOperatorChar(m_lastToken[0])) {
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
  
  if (decl.min_expr > decl.max_expr)
    throw std::runtime_error("Lower bound of parameter/DOF is greater than its upper bound");
  
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

ParserContext::ParserContext(Recipe *recipe) :
  m_rootRecipe(recipe)
{
  m_recipe = m_rootRecipe;
}

bool
ParserContext::parse()
{
  try {
    yyparse(this);
  } catch (std::runtime_error const &e) {
    throw std::runtime_error(
      m_file + ":" +
      std::to_string(m_tokLine + 1) + ":" +
      std::to_string(m_tokChar + 1) + ":" +
      e.what());
    
    return false;
  }

  return true;
}