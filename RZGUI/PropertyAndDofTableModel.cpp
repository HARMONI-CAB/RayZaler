#include "PropertyAndDofTableModel.h"
#include <GenericCompositeModel.h>
#include <Recipe.h>
#include <QPalette>
#include <QFont>

PropertyAndDofTableModel::PropertyAndDofTableModel(
    RZ::GenericCompositeModel *model,
    QObject *parent)
  : QAbstractTableModel(parent)
{
  setModel(model);
}

void
PropertyAndDofTableModel::setModel(RZ::GenericCompositeModel *model)
{
  if (model != m_model) {
    beginResetModel();

    m_model = model;
    m_params.clear();
    m_dofs.clear();

    if (m_model != nullptr) {
      for (auto p : m_model->params())
        m_params.push_back(p);

      for (auto p : m_model->dofs())
        m_dofs.push_back(p);
    }

    endResetModel();
  }
}

QString
PropertyAndDofTableModel::toSuperIndex(QString const &string)
{
  QString result = string;

  return result
        .replace(QString("0"), QString("⁰"))
        .replace(QString("1"), QString("¹"))
        .replace(QString("2"), QString("²"))
        .replace(QString("3"), QString("³"))
        .replace(QString("4"), QString("⁴"))
        .replace(QString("5"), QString("⁵"))
        .replace(QString("6"), QString("⁶"))
        .replace(QString("7"), QString("⁷"))
        .replace(QString("8"), QString("⁸"))
        .replace(QString("9"), QString("⁹"))
        .replace(QString("+"), QString("⁺"))
        .replace(QString("-"), QString("⁻"));
}

QString
PropertyAndDofTableModel::asScientific(qreal value)
{
  qreal inAbs = fabs(value);
  qreal exponent = floor(log10(inAbs));
  bool  isInf = std::isinf(value);
  bool  haveExponent = std::isfinite(exponent);
  QString result = "NaN";

  if (!isInf) {
    qreal mag = 1, mantissa;
    int iExponent;
    if (haveExponent) {
      iExponent = static_cast<int>(exponent);
      if (iExponent >= 0 && iExponent < 3)
        iExponent = 0;
      haveExponent = iExponent != 0;
    } else {
      iExponent = 0;
    }

    mag = pow(10., iExponent);
    mantissa = value / mag;

    result = QString::number(mantissa);

    if (haveExponent) {
       if (result == "1")
         result = "";
       else
         result += "×";
       result += "10" + toSuperIndex(QString::number(exponent));
    }
  } else {
    result = value < 0 ? "-∞" : "∞";
  }

  return result;
}

QVariant PropertyAndDofTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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

bool PropertyAndDofTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
  if (value != headerData(section, orientation, role)) {
    // FIXME: Implement me!
    emit headerDataChanged(orientation, section, section);
    return true;
  }
  return false;
}


int PropertyAndDofTableModel::rowCount(const QModelIndex &) const
{
  return static_cast<int>(m_params.size() + m_dofs.size());
}

int PropertyAndDofTableModel::columnCount(const QModelIndex &) const
{
  return 4;
}

QVariant PropertyAndDofTableModel::data(const QModelIndex &index, int role) const
{
  int row, col;
  RZ::GenericModelParam *param = nullptr;
  bool isDof = false;
  std::string name;

  if (!index.isValid() || m_model == nullptr)
    return QVariant();

  row = index.row();
  col = index.column();

  if (row >= 0) {
    size_t srow = static_cast<size_t>(row);
    size_t dofOff = m_params.size();

    if (srow < m_params.size()) {
      name  = m_params[srow];
      param = m_model->lookupParam(name);
    } else if (srow - dofOff < m_dofs.size()) {
      name  = m_dofs[srow - m_params.size()];
      param = m_model->lookupDof(name);
      isDof = true;
    }
  }

  if (param == nullptr)
    return QVariant();

  switch (role) {
    case Qt::DisplayRole:
      switch (col) {
        case 0:
          return QString::fromStdString(name);

        case 1:
          return asScientific(param->description->min);

        case 2:
          return asScientific(param->description->max);

        case 3:
          return QString::number(param->value);
      }
      break;

    case Qt::FontRole:
      if (isDof && col == 0) {
        QFont font;
        font.setBold(true);
        return font;
      }
      break;

    case Qt::BackgroundRole:
      if (!isDof)
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

bool PropertyAndDofTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid() || m_model == nullptr)
    return false;

  if (role == Qt::EditRole) {
    int row, col;
    row = index.row();
    col = index.column();

    if (data(index, role) != value && col == 3) {
      RZ::Real asReal = value.value<qreal>();
      if (row >= 0) {
        size_t srow = static_cast<size_t>(row);
        size_t dofOff = m_params.size();
        bool changed = false;

        if (srow < m_params.size())
          changed = m_model->setParam(m_params[srow], asReal);
        else if (srow - dofOff < m_dofs.size())
          changed = m_model->setDof(m_dofs[srow - dofOff], asReal);

        if (changed) {
          emit dataChanged(index, index, {role});
          return true;
        }
      }
    }
  }

  return false;
}

Qt::ItemFlags PropertyAndDofTableModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = Qt::NoItemFlags;

  if (index.column() == 3)
    flags |= Qt::ItemIsEditable;

  return QAbstractItemModel::flags(index) | flags;
}
