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
  std::vector<OMTreeItem *> children;
  int relRow = -1;

  union {
    RZ::ReferenceFrame    *frame;
    RZ::Element           *element;
    RZ::OpticalElement    *opticalElement;
    RZ::Detector          *detector;
    const RZ::OpticalPath *path;
  };

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
    return 2;
  }

  QString data(int col);
};

class OMTreeModel : public QAbstractItemModel
{
  Q_OBJECT
    QMap<QString, QPixmap>  m_icons;
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
};

#endif // OMTREEMODEL_H
