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
