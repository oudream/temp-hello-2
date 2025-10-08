#ifndef CX_CT_X2_FOUR_VIEW_WIDGET_H
#define CX_CT_X2_FOUR_VIEW_WIDGET_H


#include <QWidget>
#include <QLayout>
#include "view_frame.h"
#include "util.h"

class FourViewWidget : public QWidget {
Q_OBJECT
public:
    explicit FourViewWidget(QWidget *parent=nullptr);

    void setLayoutMode(FourViewLayout mode);
    FourViewLayout layoutMode() const { return m_mode; }

    ViewFrame* viewAxial()    const { return m_axial; }
    ViewFrame* viewSagittal() const { return m_sagittal; }
    ViewFrame* viewCoronal()  const { return m_coronal; }
    ViewFrame* view3D()       const { return m_view3D; }

private:
    FourViewLayout m_mode = FourViewLayout::Grid2x2;
    ViewFrame *m_axial = nullptr;
    ViewFrame *m_sagittal = nullptr;
    ViewFrame *m_coronal = nullptr;
    ViewFrame *m_view3D = nullptr;
    QLayout *m_currentLayout = nullptr;

    void rebuild();
};


#endif //CX_CT_X2_FOUR_VIEW_WIDGET_H
