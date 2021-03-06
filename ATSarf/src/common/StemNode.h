#ifndef STEMNODE_H
#define STEMNODE_H

#include <QString>
#include <QVector>
#include <QList>
#include "common.h"
#include "text_handling.h"
#include <QtAlgorithms>
#include <QVector>
#include <QList>
#include <QDataStream>

class StemNode {
    public:
        typedef QVector<long> CategoryVector;

        QString key;
        unsigned long long stem_id;
        CategoryVector category_ids;
    public:
        void add_info(long cat_id);
        bool exists(long cat_id) {
            QVector<long>::iterator i = qBinaryFind(category_ids.begin(), category_ids.end(), cat_id);
            return (i != category_ids.end());
        }
    public:
        typedef QVector<QString> RawDatasEntry;
        typedef QVector<RawDatasEntry> RawDatas;

        void add_info(long cat_id, QString &raw_data);
        void add_info(long cat_id, RawDatasEntry &raw_data_entry);
        bool exists(long cat_id, QString &raw_data) { //by reference just to reduce copy constructors
            QVector<long>::iterator i = qBinaryFind(category_ids.begin(), category_ids.end(), cat_id);

            if (i != category_ids.end()) {
                int index = i - category_ids.begin(); //maybe does not work for 64-bit machine

                for (int j = 0; j < raw_datas[index].size(); j++)
                    if (equal(raw_datas[index][j], raw_data)) {
                        return true;
                    }

                return false;
            } else {
                return false;
            }
        }
        bool get(long cat_id, RawDatasEntry &raw_data_entry) {
            QVector<long>::iterator i = qBinaryFind(category_ids.begin(), category_ids.end(), cat_id);

            if (i != category_ids.end()) {
                int index = i - category_ids.begin(); //maybe does not work for 64-bit machine
                raw_data_entry = this->raw_datas[index];
                return true;
            } else {
                return false;
            }
        }
        RawDatas raw_datas;
};


inline QDataStream &operator>>(QDataStream &in, long &l) {
    qint64 s;
    in >> s;
    l = s;
    return in;
}

inline QDataStream &operator<<(QDataStream &out, const long &l) {
    out << (qint64)l;
    return out;
}

inline QDataStream &operator>>(QDataStream &in, StemNode &node) {
    qulonglong stem_id;
    in >> stem_id >> node.category_ids;
    node.stem_id = stem_id;
    in >> node.raw_datas;
    return in;
}
inline QDataStream &operator<<(QDataStream &out, const StemNode &node) {
    out << (qulonglong)node.stem_id << node.category_ids;
    out << node.raw_datas;
    return out;
}
QDataStream &operator>>(QDataStream &in, StemNode *&node);
QDataStream &operator<<(QDataStream &out, const StemNode *&node);


typedef struct StemNode_info_ {
    long category_id;
    QString raw_data;
} StemNode_info;

class Search_StemNode {
    private:
        const StemNode *node;
        int cat_index;
        int rawdata_index;

    public:
        Search_StemNode() {
            //implement later
        }
        Search_StemNode(const StemNode *node) {
            setNode(node);
        }
        void setNode(const StemNode *node) {
            this->node = node;

            if (node == NULL) {
                throw "cannot search a null pointer";
            }

            cat_index = 0;
            rawdata_index = 0;
        }
        bool retrieve(long &category_id) {
            if (cat_index < node->category_ids.size()) {
                category_id = node->category_ids[cat_index];
                cat_index++;
                return true;
            } else {
                return false;
            }
        }
        bool retrieve(StemNode_info &info) {
            if (cat_index < node->category_ids.size()) {
                if (rawdata_index < node->raw_datas[cat_index].size()) {
                    info.category_id = node->category_ids[cat_index];
                    info.raw_data = node->raw_datas[cat_index][rawdata_index];
                    rawdata_index++;
                    return true;
                } else {
                    cat_index++;
                    rawdata_index = 0;
                    return retrieve(info);
                }
            } else {
                return false;
            }
        }
        bool retrieve(long &category_id, QVector<QString> &raw_datas) {
            if (cat_index < node->category_ids.size()) {
                category_id = node->category_ids[cat_index];
                //raw_datas.clear();
                raw_datas = node->raw_datas[cat_index];
                cat_index++;
                return true;
            } else {
                return false;
            }
        }
};

#endif
