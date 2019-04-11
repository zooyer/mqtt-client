#include "tabledelegate.h"
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QAbstractItemView>

QosDelegate::QosDelegate(QWidget *parent) : QItemDelegate(parent)
{

}

QWidget *QosDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QComboBox *comboBox = new QComboBox(parent);
    QLineEdit *lineEdit = new QLineEdit(comboBox);

    lineEdit->setReadOnly(true);
    lineEdit->setAlignment(Qt::AlignCenter);

    comboBox->setLineEdit(lineEdit);
    comboBox->addItem("0");
    comboBox->addItem("1");
    comboBox->addItem("2");

    for (int i=0; i<comboBox->count(); i++) {
        QStandardItemModel *itemModel = static_cast<QStandardItemModel*>(comboBox->view()->model());
        itemModel->item(i)->setTextAlignment(Qt::AlignCenter);
    }

    return  comboBox;
}

void QosDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString text = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int tindex = comboBox->findText(text);
    comboBox->setCurrentIndex(tindex);
}

void QosDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast <QComboBox*>(editor);
    QString text = comboBox->currentText();
    model->setData(index, text, Qt::EditRole);
}

void QosDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

CheckedDelegate::CheckedDelegate(QWidget *parent) : QItemDelegate(parent)
{

}

QWidget *CheckedDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    QCheckBox *checkBox = new QCheckBox(parent);
    checkBox->setChecked(true);

    return checkBox;
}

void CheckedDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor);
    Q_UNUSED(index);
}

void CheckedDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QCheckBox *checkBox = static_cast <QCheckBox*>(editor);
    model->setData(index, checkBox->text(), Qt::EditRole);
}

void CheckedDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

TopicModel::TopicModel(QObject *parent) : QStandardItemModel(parent)
{

}

TopicModel::TopicModel(int row, int column, QObject *parent) : QStandardItemModel(row, column, parent)
{

}

bool TopicModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

//    if (role == Qt::CheckStateRole && index.column() == 0) {
//        m_checkd[index.row()] = (value == Qt::Checked ? Qt::Checked : Qt::Unchecked);
//        return true;
//    }

    return QStandardItemModel::setData(index, value, role);
}

QVariant TopicModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
            return QVariant();

//    switch (role) {
//    case Qt::TextAlignmentRole:
//        return Qt::AlignCenter;
//    case Qt::CheckStateRole:
//        if (index.column() == 0) {
//            if (m_checkd.contains(index.row()))
//                return m_checkd[index.row()] == Qt::Checked ? Qt::Checked : Qt::Unchecked;
//            return Qt::Unchecked;
//        }
//    default:
//        break;
//    }

    return QStandardItemModel::data(index, role);
}

Qt::ItemFlags TopicModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    if (index.column() == 0)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;

    return QStandardItemModel::flags(index);
}
