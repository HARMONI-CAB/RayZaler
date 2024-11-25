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

#include "CustomTextEditDelegate.h"
#include <QLineEdit>

CustomTextEditDelegate::CustomTextEditDelegate(QObject *parent)
: QItemDelegate(parent)
{

}

QWidget *
CustomTextEditDelegate::createEditor(
    QWidget *parent,
    const QStyleOptionViewItem &,
    const QModelIndex &) const
{
  QLineEdit *editor = new QLineEdit(parent);

  editor->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  return editor;
}

void
CustomTextEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  QString value = index.model()->data(index, Qt::EditRole).toString();
  QLineEdit *txtEdit = static_cast<QLineEdit *>(editor);

  txtEdit->setText(value);
}

void
CustomTextEditDelegate::setModelData(
    QWidget *editor,
    QAbstractItemModel *model,
    const QModelIndex &index) const
{
  // store edited model data to model
  QLineEdit *txtEdit = static_cast<QLineEdit *>(editor);
  QString value = txtEdit->text();

  model->setData(index, value, Qt::EditRole);
}

void
CustomTextEditDelegate::updateEditorGeometry(
    QWidget *editor,
    const QStyleOptionViewItem &option,
    const QModelIndex &) const
{
  editor->setGeometry(option.rect);
}
