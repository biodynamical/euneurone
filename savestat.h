/****************************************************************************
** Form interface generated from reading ui file 'form1.ui'
**
** Created: Mon Jul 4 14:02:56 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**
****************************************************************************/

#ifndef SAVESTAT_H
#define SAVESTAT_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QTextEdit;
class QProgressBar;
class QPushButton;

class SaveStat : public QDialog
{
    Q_OBJECT

public:
    SaveStat( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SaveStat();
    void done();
    
    QTextEdit* textEdit;
    QProgressBar* progressBar;
    QPushButton* pushButton;
    bool bCancel;
    
signals:
	void cancelSave();

protected slots:
    virtual void languageChange();
    virtual void doit();

};

#endif // SAVESTAT_H
