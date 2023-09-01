#include "OMTreeModel.h"
#include <OMModel.h>

QString
OMTreeItem::data(int col)
{
  if (col == 0) {
    switch (type) {
      case OM_TREE_ITEM_TYPE_ROOT:
        return "Optomechanical model";

      case OM_TREE_ITEM_TYPE_CATEGORY:
        return displayText;

      case OM_TREE_ITEM_TYPE_FRAME:
        return QString::fromStdString(frame->name());

      case OM_TREE_ITEM_TYPE_ELEMENT:
        if (element->name().size() == 0)
          return "(Anonymous)";
        return QString::fromStdString(element->name());

      case OM_TREE_ITEM_TYPE_OPTICAL_ELEMENT:
        if (opticalElement->name().size() == 0)
          return "(Anonymous)";
        return QString::fromStdString(opticalElement->name());

      case OM_TREE_ITEM_TYPE_DETECTOR:
        return QString::fromStdString(detector->name());

      case OM_TREE_ITEM_TYPE_OPTICAL_PATH:
        return displayText;
    }
  } else if (col == 1) {
    switch (type) {
      case OM_TREE_ITEM_TYPE_OPTICAL_ELEMENT:
      case OM_TREE_ITEM_TYPE_ELEMENT:
      case OM_TREE_ITEM_TYPE_DETECTOR:
        return QString::fromStdString(element->factory()->name());

      default:
        return "";
    }
  }

  return "???";
}

OMTreeModel::OMTreeModel(QObject *parent)
  : QAbstractItemModel{parent}
{

}

OMTreeModel::~OMTreeModel()
{
  clearModel();
}

void
OMTreeModel::clearModel()
{
  m_root = nullptr;

  for (auto p : m_itemAlloc)
    delete p;

  m_itemAlloc.clear();
}


OMTreeItem *
OMTreeModel::allocItem(OMTreeItemType type, OMTreeItem *parent, const char *text)
{
  OMTreeItem *item = new OMTreeItem();

  item->type = type;
  if (text != nullptr)
    item->displayText = text;
  item->parent = parent;

  if (parent != nullptr) {
    item->relRow = parent->childCount();
    parent->children.push_back(item);
  }

  m_itemAlloc.push_back(item);

  if (type == OM_TREE_ITEM_TYPE_ROOT)
    m_root = item;

  return item;
}

void
OMTreeModel::setModel(RZ::OMModel *model)
{
  if (m_model != model) {
    m_model = model;
    beginResetModel();
    clearModel();

    auto root      = allocItem(OM_TREE_ITEM_TYPE_ROOT);
    auto frames    = allocItem(OM_TREE_ITEM_TYPE_CATEGORY, root, "Reference frames");
    auto elements  = allocItem(OM_TREE_ITEM_TYPE_CATEGORY, root, "Elements (all)");
    auto opticalElements = allocItem(OM_TREE_ITEM_TYPE_CATEGORY, root, "Optical Elements");
    auto detectors = allocItem(OM_TREE_ITEM_TYPE_CATEGORY, root, "Detectors");
    auto paths     = allocItem(OM_TREE_ITEM_TYPE_CATEGORY, root, "Paths");

    if (m_model != nullptr) {
      for (auto p : m_model->frames()) {
        auto frame = m_model->lookupReferenceFrame(p);
        if (frame != nullptr) {
          auto item = allocItem(OM_TREE_ITEM_TYPE_FRAME, frames);
          item->frame = frame;
        }
      }

      for (auto p : m_model->elements()) {
        auto element = m_model->lookupElement(p);
        if (element != nullptr) {
          auto item = allocItem(OM_TREE_ITEM_TYPE_ELEMENT, elements);
          item->element = element;
        }
      }

      for (auto p : m_model->opticalElements()) {
        auto opticalElement = m_model->lookupOpticalElement(p);
        if (opticalElement != nullptr) {
          auto item = allocItem(OM_TREE_ITEM_TYPE_OPTICAL_ELEMENT, opticalElements);
          item->opticalElement = opticalElement;
        }
      }

      for (auto p : m_model->detectors()) {
        auto detector = m_model->lookupDetector(p);
        if (detector != nullptr) {
          auto item = allocItem(OM_TREE_ITEM_TYPE_DETECTOR, detectors);
          item->detector = detector;
        }
      }

      for (auto p : m_model->opticalPaths()) {
        auto path = m_model->lookupOpticalPath(p);
        if (path != nullptr) {
          auto item = allocItem(OM_TREE_ITEM_TYPE_OPTICAL_PATH, paths);
          item->path = path;
          item->displayText = QString::fromStdString(p);
          if (p.size() == 0)
            item->displayText = "(Default path)";
        }
      }
    }

    endResetModel();
  }
}

QModelIndex
OMTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (m_root == nullptr || !hasIndex(row, column, parent))
        return QModelIndex();

    OMTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = m_root;
    else
        parentItem = static_cast<OMTreeItem*>(parent.internalPointer());

    OMTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex OMTreeModel::parent(const QModelIndex &index) const
{
    if (m_root == nullptr || !index.isValid())
        return QModelIndex();

    OMTreeItem *childItem = static_cast<OMTreeItem*>(index.internalPointer());
    OMTreeItem *parentItem = childItem->parentItem();

    if (parentItem == m_root)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int OMTreeModel::rowCount(const QModelIndex &parent) const
{
    OMTreeItem *parentItem;
    if (m_root == nullptr || parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_root;
    else
        parentItem = static_cast<OMTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int OMTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<OMTreeItem*>(parent.internalPointer())->columnCount();
    if (m_root != nullptr)
      return m_root->columnCount();

    return 0;
}

OMTreeItem *
OMTreeModel::itemFromIndex(const QModelIndex &index) const
{
  if (!index.isValid())
    return nullptr;

  OMTreeItem *item = static_cast<OMTreeItem*>(index.internalPointer());

  return item;
}

QVariant OMTreeModel::data(const QModelIndex &index, int role) const
{
    if (m_root == nullptr)
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    OMTreeItem *item = itemFromIndex(index);

    if (item == nullptr)
      return QVariant();

    return item->data(index.column());
}

Qt::ItemFlags OMTreeModel::flags(const QModelIndex &index) const
{
    if (m_root == nullptr || !index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant OMTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
  if (m_root == nullptr)
    return QVariant();

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      if (section == 0)
        return "Name";
      else
        return "Type";
    }

    return QVariant();
}

