#ifndef TABLEDELEGATE_H
#define TABLEDELEGATE_H

#include <QItemDelegate>
#include <QStandardItemModel>

class QosDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit QosDelegate(QWidget *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void  setEditorData(QWidget *editor, const QModelIndex &index) const;
    void  setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void  updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:

public slots:
};

class CheckedDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit CheckedDelegate(QWidget *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void  setEditorData(QWidget *editor, const QModelIndex &index) const;
    void  setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void  updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:

public slots:
};

class TopicModel : public QStandardItemModel
{
    Q_OBJECT
public:
    TopicModel(QObject *parent = nullptr);
    TopicModel(int row, int column, QObject *parent = nullptr);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    QMap<int, Qt::CheckState> m_checkd;
};


#endif // TABLEDELEGATE_H
