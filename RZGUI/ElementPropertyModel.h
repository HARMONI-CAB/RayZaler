#ifndef ELEMENTPROPERTYMODEL_H
#define ELEMENTPROPERTYMODEL_H

#include <QAbstractTableModel>
#include <list>
#include <string>
#include <QColor>
#include <map>
#include <Element.h>

struct ElementPropertyModelEntry {
  QString           name;
  bool              customColor = false;
  bool              editable = false;
  QColor            color;
  RZ::PropertyValue value;
  bool              isProperty = false;
};

class ElementPropertyModel : public QAbstractTableModel
{
  Q_OBJECT

  RZ::Element *m_element = nullptr;

  std::list<ElementPropertyModelEntry>               m_allocation;
  std::vector<ElementPropertyModelEntry *>           m_properties;
  std::map<std::string, ElementPropertyModelEntry *> m_nameToProp;

  ElementPropertyModelEntry *pushEntry(QString name, RZ::PropertyValue const &val, QColor color = QColor());

public:
  explicit ElementPropertyModel(QObject *parent = nullptr);

  void setElement(RZ::Element *element);
  RZ::Element *element() const;

  // Header:
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

  // Basic functionality:
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  // Editable:
  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole) override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

signals:
  void propertyChanged(QString);

private:
};

#endif // ELEMENTPROPERTYMODEL_H
