#ifndef SIMPLEEXPRESSIONEVALUATOR_H
#define SIMPLEEXPRESSIONEVALUATOR_H

#include <Vector.h>
#include <map>

class SimpleExpressionEvaluatorImpl;

typedef std::map<std::string, RZ::Real *> SimpleExpressionDict;

class SimpleExpressionEvaluator {
    SimpleExpressionEvaluatorImpl *p_impl = nullptr;

  public:
    SimpleExpressionEvaluator(SimpleExpressionDict const &);
    virtual ~SimpleExpressionEvaluator();

    bool compile(std::string const &);
    RZ::Real evaluate();
    std::string getLastParserError() const;
};


#endif // SIMPLEEXPRESSIONEVALUATOR_H
