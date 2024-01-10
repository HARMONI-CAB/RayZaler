#include <GUIHelpers.h>
#include "ElementPropertyModel.h"
#include <QFont>
#include <QPalette>

ElementPropertyModel::ElementPropertyModel(QObject *parent)
  : QAbstractTableModel(parent)
{
}

ElementPropertyModelEntry *
ElementPropertyModel::pushEntry(QString name, RZ::PropertyValue const &val, QColor color)
{
  ElementPropertyModelEntry entry;

  entry.name  = name;
  entry.value = val;

  if (color.isValid()) {
    entry.customColor = true;
    entry.color       = color;
  }

  m_allocation.push_back(entry);

  auto &it = m_allocation.back();

  m_properties.push_back(&it);

  return &it;
}

void
ElementPropertyModel::setElement(RZ::Element *element)
{
  if (element != m_element) {
    beginResetModel();

    m_element = element;
    m_properties.clear();
    m_allocation.clear();
    m_nameToProp.clear();

    if (m_element != nullptr) {
      pushEntry(
            "name",
            m_element->name(),
            QPalette().color(QPalette::ColorRole::ToolTipBase));

      pushEntry(
            "class",
            m_element->factory()->name(),
            QPalette().color(QPalette::ColorRole::ToolTipBase));

      for (auto &p : m_element->properties()) {
        auto entry = pushEntry(
              QString::fromStdString(p),
              m_element->get(p));
        entry->editable = p != "optical";
        entry->isProperty = true;
        m_nameToProp[p] = entry;
      }
    }

    endResetModel();
  }
}

RZ::Element *
ElementPropertyModel::element() const
{
  return m_element;
}

QVariant
ElementPropertyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  // FIXME: Implement me!
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    QString headers[] = {"Name", "Value"};

    if (section < 0 || section >= 2)
      return QVariant();

    return headers[section];
  }

  return QVariant();
}

bool
ElementPropertyModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
  if (value != headerData(section, orientation, role)) {
    // FIXME: Implement me!
    emit headerDataChanged(orientation, section, section);
    return true;
  }
  return false;
}


int
ElementPropertyModel::rowCount(const QModelIndex &) const
{
  return static_cast<int>(m_properties.size());
}

int
ElementPropertyModel::columnCount(const QModelIndex &) const
{
  return 2;
}

QVariant
ElementPropertyModel::data(const QModelIndex &index, int role) const
{
  int row, col;
  ElementPropertyModelEntry *entry = nullptr;
  std::string name;

  if (!index.isValid() || m_element == nullptr)
    return QVariant();

  row = index.row();
  col = index.column();

  if (row >= 0 && static_cast<size_t>(row) < m_properties.size())
    entry = m_properties[static_cast<size_t>(row)];

  if (entry == nullptr)
    return QVariant();

  name = entry->name.toStdString();

  // Leverage this to update element
  if (entry->isProperty)
    entry->value = m_element->get(name);

  switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
      switch (col) {
        case 0:
          return entry->name;

        case 1:
          switch (entry->value.type()) {
            case RZ::UndefinedValue:
              return "Undefined";

            case RZ::IntegerValue:
              return QString::number(std::get<int64_t>(entry->value));

            case RZ::RealValue:
              return asScientific(std::get<RZ::Real>(entry->value));

            case RZ::BooleanValue:
              return std::get<bool>(entry->value) ? "true" : "false";

            case RZ::StringValue:
              return QString::fromStdString(std::get<std::string>(entry->value));
          }

          return QVariant();
      }
      break;

    case Qt::FontRole:
      if (entry->editable) {
        QFont font;
        font.setBold(true);
        return font;
      }

      break;

    case Qt::BackgroundRole:
      if (entry->customColor)
        return entry->color;
      break;

    case Qt::TextAlignmentRole:
      if (col == 0)
        return Qt::AlignLeft;
      else
        return Qt::AlignRight;
  }

  return QVariant();
}

bool
ElementPropertyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid() || m_element == nullptr)
    return false;

  if (role == Qt::EditRole) {
    int row, col;
    row = index.row();
    col = index.column();

    if (data(index, role) != value && col == 1) {
      if (row >= 0 && static_cast<size_t>(row) < m_properties.size()) {
        size_t srow = static_cast<size_t>(row);
        bool changed = false;
        auto *entry = m_properties[srow];
        std::string name = entry->name.toStdString();

        if (entry->editable) {
          switch (entry->value.type()) {
            case RZ::IntegerValue:
              changed = m_element->set(name, value.value<int64_t>());
              break;

            case RZ::RealValue:
              changed = m_element->set(name, value.value<RZ::Real>());
              break;

            case RZ::BooleanValue:
              changed = m_element->set(name, value.value<bool>());
              break;

            case RZ::StringValue:
              changed = m_element->set(
                    name,
                    value.value<QString>().toStdString());
              break;

            default:
              break;
          }
        }

        if (changed) {
          emit dataChanged(index, index, {role});
          emit propertyChanged(entry->name);
          return true;
        }
      }
    }
  }

  return false;
}

Qt::ItemFlags
ElementPropertyModel::flags(const QModelIndex &index) const
{
  int row, col;
  ElementPropertyModelEntry *entry = nullptr;
  Qt::ItemFlags flags = Qt::NoItemFlags;

  row = index.row();
  col = index.column();

  if (row >= 0 && static_cast<size_t>(row) < m_properties.size())
    entry = m_properties[static_cast<size_t>(row)];

  if (entry != nullptr && col == 1 && entry->editable)
    flags |= Qt::ItemIsEditable;

  return QAbstractItemModel::flags(index) | flags;
}
