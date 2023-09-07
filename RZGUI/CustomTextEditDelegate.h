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
