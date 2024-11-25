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

#ifndef CUSTOMTEXTEDITDELEGATE_H
#define CUSTOMTEXTEDITDELEGATE_H

#include <QItemDelegate>
#include <QObject>

class CustomTextEditDelegate : public QItemDelegate
{
public:
  explicit CustomTextEditDelegate(QObject *parent = nullptr);

  virtual QWidget *createEditor(
      QWidget *parent,
      const QStyleOptionViewItem &option,
      const QModelIndex &index) const override;

  virtual void setEditorData(
      QWidget *editor,
      const QModelIndex &index) const override;

  virtual void setModelData(
      QWidget *editor,
      QAbstractItemModel *model,
      const QModelIndex &index) const override;

  virtual void updateEditorGeometry(
      QWidget *editor,
      const QStyleOptionViewItem &option,
      const QModelIndex &index) const override;

  CustomTextEditDelegate();
};

#endif // CUSTOMTEXTEDITDELEGATE_H
