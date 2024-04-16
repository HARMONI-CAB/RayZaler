#include "RZMHighLighter.h"

#define WORD(wrd) QStringLiteral("\\b" wrd "\\b")
void
RZMHighLighter::addRule(QString const &what, QString const &regex)
{
  HighlightingRule newRule;

  newRule.pattern     = QRegularExpression(regex);
  newRule.formatName  = what;
  newRule.format      = m_formats[what];

  m_rules.append(newRule);
}

void
RZMHighLighter::defineFormat(QString const &name, QTextCharFormat const &fmt)
{
  m_formats[name] = fmt;
}

void
RZMHighLighter::highlightBlock(const QString &text)
{
  for (const HighlightingRule &rule : std::as_const(m_rules)) {
    QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);

    while (matchIterator.hasNext()) {
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
  }
}

void
RZMHighLighter::rebuildRules()
{
  for (auto &p : m_rules)
    p.format = m_formats[p.formatName];
}

QList<QString>
RZMHighLighter::formats() const
{

}

RZMHighLighter::RZMHighLighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
  QTextCharFormat fmt;

  fmt.setFontWeight(QFont::Bold);
  fmt.setForeground(Qt::blue);
  defineFormat("keyword", fmt);

  fmt.setFontWeight(QFont::Normal);
  fmt.setForeground(Qt::blue);
  defineFormat("datatype", fmt);

  fmt.setFontWeight(QFont::Normal);
  fmt.setForeground(Qt::magenta);
  defineFormat("constant", fmt);

  fmt.setFontWeight(QFont::Normal);
  fmt.setForeground(QColor(0x7f, 0x7f, 0x7f));
  defineFormat("comment", fmt);

  fmt.setFontWeight(QFont::Bold);
  fmt.setForeground(Qt::black);
  defineFormat("identifier", fmt);

  fmt.setFontWeight(QFont::Bold);
  fmt.setForeground(Qt::red);
  defineFormat("string", fmt);

  addRule("constant", QStringLiteral("[+-]?\\.?(\\d+([.]\\d*)?([eE][+-]?\\d+)?|[.]\\d+([eE][+-]?\\d+)?)\\b"));
  addRule("identifier", WORD("[A-Za-z_][A-Za-z0-9_]*"));
  addRule("datatype", QStringLiteral("(?:\\h|^)[A-Za-z_][A-Za-z0-9_]*(?=\\h+[A-Za-z_][A-Za-z0-9_]*)\\b"));
  addRule("identifier", QStringLiteral("(?:on|of|path|to)\\h+[A-Za-z_][A-Za-z0-9_]*\\b"));


  addRule("keyword", WORD("rotate"));
  addRule("keyword", WORD("translate"));
  addRule("keyword", WORD("path"));
  addRule("keyword", WORD("to"));
  addRule("keyword", WORD("parameter"));
  addRule("keyword", WORD("dof"));
  addRule("keyword", WORD("on"));
  addRule("keyword", WORD("of"));
  addRule("keyword", WORD("element"));
  addRule("keyword", WORD("port"));
  addRule("keyword", WORD("import"));
  addRule("keyword", WORD("script"));

  addRule("string", QStringLiteral("\".*\""));
  addRule("comment", QStringLiteral("#[^\n]*"));
}
