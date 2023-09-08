#include "PropertyAndDofExprModel.h"
#include <GenericCompositeModel.h>
#include <Recipe.h>
#include <QPalette>
#include <QFont>
#include <GUIHelpers.h>

PropertyAndDofExprModel::PropertyAndDofExprModel(
    RZ::GenericCompositeModel *model,
    QObject *parent)
  : QAbstractTableModel(parent)
{
  setModel(model);
}

PropertyState *
PropertyAndDofExprModel::allocProperty()
{
  m_allProperties.push_back(PropertyState());

  auto &last = m_allProperties.back();

  last.globalIndex = static_cast<int>(m_propVec.size());
  m_propVec.push_back(&last);

  return &last;
}

void
PropertyAndDofExprModel::setModel(RZ::GenericCompositeModel *model)
{
  if (model != m_model) {
    beginResetModel();

    m_model = model;

    m_dofs.clear();
    m_params.clear();
    m_propVec.clear();
    m_allProperties.clear();

    if (m_model != nullptr) {
      for (auto p : m_model->params()) {
        auto param = m_model->lookupParam(p);
        if (param != nullptr) {
          auto prop = allocProperty();
          prop->name        = p;
          prop->defaultExpr = std::to_string(param->value);
          prop->expr        = prop->defaultExpr;
          prop->paramPtr    = param;
          m_params[p]       = prop;
        }
      }

      for (auto p : m_model->dofs()) {
        auto dof = m_model->lookupDof(p);
        if (dof != nullptr) {
          auto prop = allocProperty();
          prop->name        = p;
          prop->defaultExpr = std::to_string(dof->value);
          prop->expr        = prop->defaultExpr;
          prop->paramPtr    = dof;
          prop->isDof       = true;
          m_dofs[p]         = prop;
        }
      }
    }

    endResetModel();
  }
}

std::string
PropertyAndDofExprModel::dof(std::string const &name) const
{
  auto dof = m_dofs.find(name);
  if (dof == m_dofs.end())
    return "";

  return dof->second->expr;
}

void
PropertyAndDofExprModel::setDof(
    std::string const &name,
    std::string const &value,
    bool setEdited)
{
  auto dof = m_dofs.find(name);

  if (dof != m_dofs.end()) {
    if (setEdited) {
      m_dofs[name]->expr = value;
      m_dofs[name]->modified = true;
    } else {
      m_dofs[name]->expr = value;
      m_dofs[name]->defaultExpr = value;
      m_dofs[name]->modified = false;
    }
  }
}

std::string
PropertyAndDofExprModel::param(std::string const &name) const
{
  auto param = m_params.find(name);
  if (param == m_params.end())
    return "";

  return param->second->expr;
}

void
PropertyAndDofExprModel::setDofFailed(std::string const &name, bool failed)
{
  auto dof = m_dofs.find(name);
  if (dof == m_dofs.end())
    return;

  dof->second->failed = true;
  dataChanged(
        index(dof->second->globalIndex, 0),
        index(dof->second->globalIndex, 3), {Qt::BackgroundRole});
}

void
PropertyAndDofExprModel::setParamFailed(std::string const &name, bool failed)
{
  auto param = m_params.find(name);
  if (param == m_params.end())
    return;

  param->second->failed = true;
  dataChanged(
        index(param->second->globalIndex, 0),
        index(param->second->globalIndex, 3), {Qt::BackgroundRole});
}

QVariant PropertyAndDofExprModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  // FIXME: Implement me!
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    QString headers[] = {"Name", "Min", "Max", "Value"};

    if (section < 0 || section >= 4)
      return QVariant();

    return headers[section];
  }

  return QVariant();
}

bool PropertyAndDofExprModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
  if (value != headerData(section, orientation, role)) {
    // FIXME: Implement me!
    emit headerDataChanged(orientation, section, section);
    return true;
  }
  return false;
}


int PropertyAndDofExprModel::rowCount(const QModelIndex &) const
{
  if (m_model == nullptr)
    return 0;

  return static_cast<int>(m_propVec.size());
}

int PropertyAndDofExprModel::columnCount(const QModelIndex &) const
{
  return 4;
}

QVariant PropertyAndDofExprModel::data(const QModelIndex &index, int role) const
{
  int row, col;

  PropertyState *prop = nullptr;

  if (!index.isValid() || m_model == nullptr)
    return QVariant();

  row = index.row();
  col = index.column();

  if (row >= 0 && row < rowCount())
    prop = m_propVec[static_cast<size_t>(row)];

  if (prop == nullptr)
    return QVariant();

  switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
      switch (col) {
        case 0:
          return QString::fromStdString(prop->name);

        case 1:
          return asScientific(prop->paramPtr->description->min);

        case 2:
          return asScientific(prop->paramPtr->description->max);

        case 3:
          return QString::fromStdString(prop->expr);
      }
      break;

    case Qt::FontRole:
      if (prop->isDof && col == 0) {
        QFont font;
        font.setBold(true);
        return font;
      } else if (prop->modified && col == 3) {
        QFont font;
        font.setItalic(true);
        return font;
      }
      break;

    case Qt::BackgroundRole:
      if (prop->failed)
        return QColor(0xff, 0xbf, 0xbf);
      else if (!prop->isDof)
        return QPalette().color(QPalette::ColorRole::ToolTipBase);
      break;

    case Qt::TextAlignmentRole:
      if (col == 0)
        return Qt::AlignLeft;
      else
        return Qt::AlignRight;
  }

  return QVariant();
}

void
PropertyAndDofExprModel::resetDofs()
{
  for (auto &p : m_dofs) {
    p.second->modified = false;
    p.second->expr     = p.second->defaultExpr;
  }

  if (m_propVec.size() > 0)
    emit dataChanged(
          index(0, 0),
          index(static_cast<int>(m_propVec.size() - 1), 3));
}

bool
PropertyAndDofExprModel::dofEdited(std::string const &name) const
{
  auto dof = m_dofs.find(name);
  if (dof == m_dofs.end())
    return false;

  return dof->second->modified;
}

bool PropertyAndDofExprModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid() || m_model == nullptr)
    return false;

  if (role == Qt::EditRole) {
    int row, col;
    row = index.row();
    col = index.column();

    if (col == 3 && row >= 0 && row < rowCount()) {
      auto newExpr = value.value<QString>().toStdString();
      std::string asStr = newExpr;

      auto prop = m_propVec[static_cast<size_t>(row)];

      prop->expr     = asStr;
      prop->failed   = false;
      prop->modified = asStr != prop->defaultExpr;

      if (prop->isDof)
        emit dofChanged(
              QString::fromStdString(prop->name),
              QString::fromStdString(newExpr));
      else
        emit paramChanged(
              QString::fromStdString(prop->name),
              QString::fromStdString(newExpr));

      emit dataChanged(
            index,
            index,
            {Qt::DisplayRole, Qt::FontRole, Qt::BackgroundRole});
      return true;
    }
  }

  return false;
}

Qt::ItemFlags PropertyAndDofExprModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = Qt::NoItemFlags;
  int row = index.row();

  if (row >= 0 && row < rowCount()) {
    auto prop = m_propVec[static_cast<size_t>(row)];
    if (prop->isDof && index.column() == 3)
      flags |= Qt::ItemIsEditable;
  }

  return QAbstractItemModel::flags(index) | flags;
}
