#include <QSize>
#include "ArgumentsModel.h"

namespace rstools {
namespace batch {
namespace util {

ArgumentsModel::ArgumentsModel(RSJob* job, QObject *parent) : QAbstractTableModel(parent)
{
    this->job = job;
}

ArgumentsModel::~ArgumentsModel()
{}

int ArgumentsModel::rowCount(const QModelIndex & /*parent*/) const
{
   return job->getArguments().size() + 1;
}

int ArgumentsModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 2;
}

QVariant ArgumentsModel::data(const QModelIndex &index, int role) const
{    
    if (role == Qt::DisplayRole || role == Qt::EditRole ) {

        vector<rsArgument*> args = job->getArguments();
        
        if ( index.row() >= (int)args.size() ) {
            return QVariant();
        }
        
        rsArgument* arg = args[index.row()];
        
        switch ( index.column() ) {
            case 0:
                return QString(arg->key);
            case 1:
                return QString(arg->value);
        }
    }
    return QVariant();
}

bool ArgumentsModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole) {
        QString result = value.toString();
        QByteArray result2 = result.toLatin1();
        char *v = rsString(result2.data());
        bool insertedRow = false;
        vector<rsArgument*> args = job->getArguments();
        
        if ( index.row() >= (int)args.size() ) {
            
            //beginInsertRows(index, 0, 1);
            
            rsArgument* arg = (rsArgument*)rsMalloc(sizeof(rsArgument));
            arg->key = rsString("<empty>");
            arg->value = rsString("");
            job->addArgument(arg);

            insertedRow = true;
                
            //endInsertRows();
        }
        
        args = job->getArguments();
        rsArgument* arg = args[index.row()];
        
        if ( index.column() == 0 ) {
            arg->key = v;
        } else {
            arg->value = v;
        }
                
        if ( insertedRow ) {
            beginResetModel();
            endResetModel();
        } else {
            emit editCompleted(result);    
        }
    }
    return true;
}

QVariant ArgumentsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:
                    return QString("Key");
                case 1:
                    return QString("Value");
            }
        }
     }
     return QVariant();
}

Qt::ItemFlags ArgumentsModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}

}}} // namespace rstools::batch::util
