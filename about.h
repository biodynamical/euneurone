/****************************************************************************
** Form interface generated from reading ui file 'about.ui'
**
** Created: Sun Dec 21 17:47:49 2003
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.1   edited Nov 21 17:40 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef ABOUTFORM_H
#define ABOUTFORM_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qdialog.h>
#include <qgroupbox.h>
#include <qvbox.h>
#include <qhbox.h>

class QLabel;
class QPushButton;
class QTabWidget;
class QTextEdit;
class QWidget;

class AboutForm : public QDialog
{
    Q_OBJECT

public:
    AboutForm( QWidget* parent = 0, const char* name = 0, bool modal = TRUE, WFlags fl = 0 );
    ~AboutForm();

    //QGridLayout *pGrid;
    QVBox *pVBox;
    QHBox *pHBox;
    QPushButton* closeButton;
    QTabWidget* tabWidget;
    QGroupBox* tab;
    QLabel* pixmapLabel;
    QLabel* titleLabel;
    QLabel* textLabel4;
    QLabel* textLabel5;
    QLabel* textLabel3;
    QLabel* textLabel2;
    QLabel* textLabel1;
    QLabel* textLabel6;
    QLabel* textLabel7;
    QLabel* urlLabel;
    QLabel* warrentyLabel;
    QTextEdit* textEdit;

protected:
    void mousePressEvent(QMouseEvent* e);
    
protected slots:
    virtual void languageChange();
    virtual void clickClose();
private:
    QPixmap image;

};

#endif // ABOUTFORM_H
