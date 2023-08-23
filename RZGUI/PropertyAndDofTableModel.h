#ifndef PROPERTYANDDOFTABLEMODEL_H
#define PROPERTYANDDOFTABLEMODEL_H

#include <QAbstractTableModel>
#include <list>
#include <string>

namespace RZ {
  class GenericCompositeModel;
}

class PropertyAndDofTableModel : public QAbstractTableModel
{
  Q_OBJECT

  RZ::GenericCompositeModel *m_model = nullptr; // Borrowed
  std::vector<std::string> m_dofs;
  std::vector<std::string> m_params;

public:
  static QString asScientific(qreal);
  static QString toSuperIndex(QString const &);

  explicit PropertyAndDofTableModel(RZ::GenericCompositeModel *model, QObject *parent = nullptr);

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

private:
};

#endif // PROPERTYANDDOFTABLEMODEL_H
