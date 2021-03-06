#ifndef CUSTOMSTTVIEW_H
#define CUSTOMSTTVIEW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QTreeWidget>
#include "colorlisteditor.h"
#include "sarftagtype.h"

class QGroupBox;

class CustomSTTView : public QMainWindow
{
    Q_OBJECT
public:
    explicit CustomSTTView(QWidget *parent = 0);
    //bool showWindow;

signals:

private slots:
    void btnSelectAll_clicked();
    void btnUnselectAll_clicked();
    void btnAdd_clicked();
    void btnRemove_clicked();
    void btnSave_clicked();
    void btnLoad_clicked();
    //void btnCancel_clicked();
    void btnClose_clicked();
    void btnSelect_clicked();
    void btnUnselect_clicked();
    void cbSynEnable_checked(bool isChecked);
    void editPattern_changed(QString text);
    void cbTagType_changed(QString text);
    void fgcolor_changed(QString color);
    void bgcolor_changed(QString color);
    void font_changed(QString fontSize);
    void tagName_changed(QString name);
    void tagName_Edited(QString name);
    //void underline_clicked(bool underline);
    void bold_clicked(bool bold);
    void italic_clicked(bool italic);
    void desc_edited();
    void selectedTags_clicked(QTreeWidgetItem* item,int i);
    void closeEvent(QCloseEvent *event);

private:
    QPushButton *btnSelectAll;
    QPushButton *btnUnselectAll;
    QPushButton *btnAdd;
    QPushButton *btnSave;
    QPushButton *btnLoad;
    QPushButton *btnCancel;
    QPushButton *btnClose;
    QPushButton *btnSelect;
    QPushButton *btnUnselect;
    QPushButton *btnRemove;

    QLabel *lblPattern;
    QLabel *lblFeatures;
    QLabel *lblTagName;
    QLabel *lblDescription;
    QLabel *lblFGColor;
    QLabel *lblBGColor;
    QLabel *lblFont;
    QLabel *lblStep;
    //QLabel *lblItalic;
    //QLabel *lblBold;
    //QLabel *lblUnderline;

    QGroupBox * gbSyn;
    QSpinBox * sbSynStep;
    QCheckBox * cbSyn;

    QLineEdit *editPattern;
    QTextEdit *editDescription;

    ColorListEditor * colorfgcolor;
    ColorListEditor * colorbgcolor;

    QComboBox *cbTagType;
    QComboBox *cbTagName;
    QComboBox *cbfont;
    QComboBox *cbContain;

    //QCheckBox *cbunderline;
    QCheckBox *cbBold;
    QCheckBox *cbItalic;
    QCheckBox *cbCaseSensetive;

    QListWidget *listPossibleTags;
    QTreeWidget *listSelectedTags;

    QStringList listStems;
    QStringList listStemPOS;
    QStringList listPrefixGloss;
    QStringList listStemGloss;
    QStringList listSuffixGloss;
    QStringList listPrefix;
    QStringList listPrefixPOS;
    QStringList listSuffix;
    QStringList listSuffixPOS;
    QStringList listCategory;
    QStringList listCategoryId;

    QVector<TagType*> *sttVector;
    QString field;
    bool dirty;
    QString tagtypePath;
    void disconnect_Signals();
    void connect_Signals();
};

#endif // CUSTOMSTTVIEW_H
