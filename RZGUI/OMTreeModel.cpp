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
        return displayText;

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

QPixmap &
OMTreeModel::getIcon(QString const &name)
{
  if (!m_icons.contains(name))
    m_icons[name] = QPixmap(":/ommodel/icons/" + name + ".svg").scaled(QSize(16, 16), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  return m_icons[name];
}

void
OMTreeModel::assignItemIcon(OMTreeItem *item)
{
  std::string factory;

  switch (item->type) {
    case OM_TREE_ITEM_TYPE_CATEGORY:
      if (item->displayText == "Reference frames")
        item->icon = &getIcon("reference-frame");
      else if (item->displayText == "Elements (all)")
        item->icon = &getIcon("elements");
      else if (item->displayText == "Optical Elements")
        item->icon = &getIcon("optical-elements");
      else if (item->displayText == "Detectors")
        item->icon = &getIcon("detector");
      else if (item->displayText == "Paths")
        item->icon = &getIcon("paths");
      else
        item->icon = &getIcon("elements");
      break;

    case OM_TREE_ITEM_TYPE_FRAME:
      switch (item->frame->typeId()) {
        case RZ_REF_FRAME_WORLD_ID:
          item->icon = &getIcon("world");
          break;

        case RZ_REF_FRAME_ROTATION_ID:
          item->icon = &getIcon("rotated");
          break;

        case RZ_REF_FRAME_TRANSLATION_ID:
          item->icon = &getIcon("translated");
          break;

        case RZ_REF_FRAME_TRIPOD_ID:
          item->icon = &getIcon("tripod");
          break;
      }
      break;

    case OM_TREE_ITEM_TYPE_ELEMENT:
    case OM_TREE_ITEM_TYPE_OPTICAL_ELEMENT:
    case OM_TREE_ITEM_TYPE_DETECTOR:
      factory = item->element->factory()->name();

      if (factory == "ApertureStop")
        item->icon = &getIcon("aperture-stop");
      else if (factory == "ApertureStop")
        item->icon = &getIcon("aperture-stop");
      else if (factory == "Detector")
        item->icon = &getIcon("detector");
      else if (factory == "BlockElement")
        item->icon = &getIcon("block-element");
      else if (factory == "TubeElement")
        item->icon = &getIcon("tube-element");
      else if (factory == "RodElement")
        item->icon = &getIcon("rod-element");
      else if (factory == "StlMesh")
        item->icon = &getIcon("stl-mesh");
      else if (factory == "LensletArray")
        item->icon = &getIcon("mla");
      else if (item->element->nestedModel() != nullptr)
        item->icon = &getIcon("composite-element");
      else
        item->icon = &getIcon("elements");
      break;

    default:
      item->icon = nullptr;
  }
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
OMTreeModel::populateSubModel(OMTreeItem *root, RZ::OMModel *model)
{
  auto elementList = model->elements();
  if (!elementList.empty()) {
    auto elements = allocItem(OM_TREE_ITEM_TYPE_CATEGORY, root, "Elements (all)");
    assignItemIcon(elements);
    for (auto &p : elementList) {
      auto element = model->lookupElement(p);
      if (element != nullptr) {
        auto item = allocItem(OM_TREE_ITEM_TYPE_ELEMENT, elements);
        item->element = element;
        assignItemIcon(item);

        auto nested = element->nestedModel();
        if (nested != nullptr)
          populateSubModel(item, nested);
      }
    }
  }

  auto frameList = model->frames();
  if (!frameList.empty()) {
    auto frames = allocItem(OM_TREE_ITEM_TYPE_CATEGORY, root, "Reference frames");
    assignItemIcon(frames);
    for (auto &p : frameList) {
      auto frame = model->lookupReferenceFrame(p);
      if (frame != nullptr) {
        auto item = allocItem(OM_TREE_ITEM_TYPE_FRAME, frames, p.c_str());
        item->frame = frame;
        assignItemIcon(item);
      }
    }
  }

  auto optElementList = model->opticalElements();
  if (!optElementList.empty()) {
    auto opticalElements = allocItem(OM_TREE_ITEM_TYPE_CATEGORY, root, "Optical Elements");
    assignItemIcon(opticalElements);
    for (auto &p : optElementList) {
      auto opticalElement = model->lookupOpticalElement(p);
      if (opticalElement != nullptr) {
        auto item = allocItem(OM_TREE_ITEM_TYPE_OPTICAL_ELEMENT, opticalElements);
        item->opticalElement = opticalElement;
        assignItemIcon(item);
        auto nested = opticalElement->nestedModel();
        if (nested != nullptr)
          populateSubModel(item, nested);
      }
    }
  }

  auto detList = model->detectors();
  if (!detList.empty()) {
    auto detectors = allocItem(OM_TREE_ITEM_TYPE_CATEGORY, root, "Detectors");
    assignItemIcon(detectors);
    for (auto &p : detList) {
      auto detector = model->lookupDetector(p);
      if (detector != nullptr) {
        auto item = allocItem(OM_TREE_ITEM_TYPE_DETECTOR, detectors);
        item->detector = detector;
        assignItemIcon(item);
      }
    }
  }

  auto pathList = model->opticalPaths();
  if (!pathList.empty()) {
    auto paths = allocItem(OM_TREE_ITEM_TYPE_CATEGORY, root, "Paths");
    assignItemIcon(paths);
    for (auto &p : pathList) {
      auto path = model->lookupOpticalPath(p);
      if (path != nullptr) {
        auto item = allocItem(OM_TREE_ITEM_TYPE_OPTICAL_PATH, paths);
        item->path = path;
        item->displayText = QString::fromStdString(p);
        if (p.size() == 0)
          item->displayText = "(Default path)";
        assignItemIcon(item);
      }
    }
  }
}

void
OMTreeModel::setModel(RZ::OMModel *model)
{
  if (m_model != model) {
    m_model = model;
    beginResetModel();
    clearModel();

    auto root = allocItem(OM_TREE_ITEM_TYPE_ROOT);

    if (model != nullptr)
      populateSubModel(root, m_model);

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

  OMTreeItem *item = itemFromIndex(index);
  if (item == nullptr)
    return QVariant();

  switch (role) {
    case Qt::DisplayRole:
      return item->data(index.column());

    case Qt::DecorationRole:
      if (index.column() == 0)
        if (item->icon != nullptr)
          return *item->icon;
  }

  return QVariant();
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
      else if (section == 1)
        return "Type";
    }

    return QVariant();
}

