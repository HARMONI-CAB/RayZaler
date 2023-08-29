#ifndef PROPERTYANDDOFEXPRMODEL_H
#define PROPERTYANDDOFEXPRMODEL_H

#include <QAbstractTableModel>
#include <map>
#include <string>

namespace RZ {
  class  GenericCompositeModel;
  struct GenericModelParam;
}

struct PropertyState {
  int globalIndex;
  std::string name;
  std::string defaultExpr;
  std::string expr;
  bool failed   = false;
  bool modified = false;
  bool isDof    = false;
  RZ::GenericModelParam *paramPtr = nullptr;
};

class PropertyAndDofExprModel : public QAbstractTableModel
{
  Q_OBJECT

  RZ::GenericCompositeModel *m_model = nullptr; // Borrowed

  std::list<PropertyState> m_allProperties;

  std::map<std::string, PropertyState *> m_params;
  std::map<std::string, PropertyState *> m_dofs;
  std::vector<PropertyState *>           m_propVec;

  PropertyState *allocProperty();

public:
  void resetDofs();
  bool dofEdited(std::string const &) const;
  std::string dof(std::string const &) const;
  std::string param(std::string const &) const;

  void setDofFailed(std::string const &, bool failed = true);
  void setParamFailed(std::string const &, bool failed = true);

  explicit PropertyAndDofExprModel(RZ::GenericCompositeModel *model, QObject *parent = nullptr);

  void setModel(RZ::GenericCompositeModel *model);

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
  void paramChanged(QString, QString);
  void dofChanged(QString, QString);

private:
};
#endif // PROPERTYANDDOFEXPRMODEL_H
