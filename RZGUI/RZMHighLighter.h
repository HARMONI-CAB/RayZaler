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
