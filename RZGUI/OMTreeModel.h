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

#ifndef OMTREEMODEL_H
#define OMTREEMODEL_H

#include <QAbstractItemModel>
#include <QMap>
#include <QPixmap>

namespace RZ {
  class OMModel;
  class ReferenceFrame;
  class Element;
  class OpticalElement;
  class Detector;
  class OpticalPath;
}

enum OMTreeItemType {
  OM_TREE_ITEM_TYPE_ROOT,
  OM_TREE_ITEM_TYPE_CATEGORY,
  OM_TREE_ITEM_TYPE_FRAME,
  OM_TREE_ITEM_TYPE_ELEMENT,
  OM_TREE_ITEM_TYPE_OPTICAL_ELEMENT,
  OM_TREE_ITEM_TYPE_DETECTOR,
  OM_TREE_ITEM_TYPE_OPTICAL_PATH,
};

struct OMTreeItem {
  OMTreeItemType type;

  QString displayText;
  OMTreeItem *parent = nullptr;
  QPixmap *icon = nullptr;
  QPixmap disabledIcon;
  
  std::vector<OMTreeItem *> children;
  int relRow = -1;

  union {
    RZ::ReferenceFrame    *frame;
    RZ::Element           *element;
    RZ::OpticalElement    *opticalElement;
    RZ::Detector          *detector;
    const RZ::OpticalPath *path;
  };

  bool
  isElement() const
  {
    return type == OM_TREE_ITEM_TYPE_ELEMENT
    || type == OM_TREE_ITEM_TYPE_OPTICAL_ELEMENT
    || type == OM_TREE_ITEM_TYPE_DETECTOR;
  }

  OMTreeItem *
  child(int ndx)
  {
    if (ndx < 0 || ndx >= children.size())
      return nullptr;

    return children[ndx];
  }

  int
  row() const
  {
    return relRow;
  }

  OMTreeItem *
  parentItem()
  {
    return parent;
  }

  int
  childCount() const
  {
    return static_cast<int>(children.size());
  }

  int
  columnCount() const
  {
    return 1;
  }

  QString data(int col);
};

class OMTreeModel : public QAbstractItemModel
{
  Q_OBJECT
    QMap<QString, QPixmap>  m_icons;
    QMap<const RZ::Element *, std::list<QModelIndex>> m_elements;
    std::list<OMTreeItem *> m_itemAlloc;
    
    OMTreeItem *m_root     = nullptr;
    RZ::OMModel *m_model   = nullptr;

    OMTreeItem *allocItem(OMTreeItemType type, OMTreeItem *parent = nullptr, const char *displayText= nullptr);
    void populateSubModel(OMTreeItem *root, RZ::OMModel *model);

    QPixmap &getIcon(QString const &name);
    void assignItemIcon(OMTreeItem *);

  public:
    void clearModel();
    void setModel(RZ::OMModel *model);
    explicit OMTreeModel(QObject *parent = nullptr);
    ~OMTreeModel();

    OMTreeItem *itemFromIndex(const QModelIndex &index) const;

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    
    void notifyIndexChanged(const QModelIndex &);
};

#endif // OMTREEMODEL_H
