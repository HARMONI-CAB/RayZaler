#ifndef RZMHIGHLIGHTER_H
#define RZMHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QMap>

class QTextDocument;

class RZMHighLighter : public QSyntaxHighlighter
{
  Q_OBJECT


  struct HighlightingRule {
    QRegularExpression pattern;
    QString            formatName;
    QTextCharFormat    format;
  };

  QList<HighlightingRule>        m_rules;
  QMap<QString, QTextCharFormat> m_formats;
  int                            m_errLine = -1;
  bool                           m_clearingErrors = false;
  bool                           m_highlighting = false;
  void addRule(QString const &what, QString const &regex);

protected:
    virtual void highlightBlock(const QString &text) override;

public:
    void highlightError(int line);
    void defineFormat(QString const &, QTextCharFormat const &);
    void rebuildRules();
    QList<QString> formats() const;

    RZMHighLighter(QTextDocument *parent = nullptr);
};

#endif // RZMHIGHLIGHTER_H
