/****************************************************************************
** Form interface generated from reading ui file 'graphform.ui'
**
** Created: Mon Aug 18 21:14:29 2003
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.1   edited Nov 21 17:40 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GRAPHFORM_H
#define GRAPHFORM_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;

class GraphForm : public QDialog
{
    Q_OBJECT

public:
    GraphForm( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~GraphForm();


protected:

protected slots:
    virtual void languageChange();
};

#endif // GRAPHFORM_H
