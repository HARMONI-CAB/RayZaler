#include "RZMHighLighter.h"
#include <QTextDocument>
#include "GUIHelpers.h"

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
  int line = currentBlock().firstLineNumber();
  m_highlighting = true;

  if (m_errLine < 0)
   setFormat(0, text.size(), m_formats["background"]);


  for (const HighlightingRule &rule : std::as_const(m_rules)) {
    QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);

    while (matchIterator.hasNext()) {
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
  }

  if (line == m_errLine)
    setFormat(0, text.size(), m_formats["error"]);

  m_highlighting = false;
}

void
RZMHighLighter::highlightError(int line)
{
  if (m_errLine != line) {
    if (!m_highlighting) {
      m_errLine = line;
      rehighlight();
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

  fmt = QTextCharFormat();
  fmt.setBackground(QColor(255, 127, 127));
  defineFormat("error", fmt);

  fmt = QTextCharFormat();
  fmt.setBackground(QColor(255, 255, 255));
  defineFormat("background", fmt);

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
