#ifndef CX_CT_X2_THREED_PAGE_H
#define CX_CT_X2_THREED_PAGE_H


#include <QWidget>
#include "four_view_widget.h"

class ThreeDPage : public QWidget {
Q_OBJECT
public:
    explicit ThreeDPage(QWidget *parent=nullptr);
    FourViewWidget* fourView() const { return m_four; }

private:
    FourViewWidget *m_four = nullptr;
};


#endif //CX_CT_X2_THREED_PAGE_H
