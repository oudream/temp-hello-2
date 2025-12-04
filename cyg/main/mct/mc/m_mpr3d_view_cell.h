#ifndef CX_CT_X2_M_MPR3D_VIEW_CELL_H
#define CX_CT_X2_M_MPR3D_VIEW_CELL_H


#include "m_mpr_view_cell.h"


class Mpr3DViewCell : public MprViewCell
{
Q_OBJECT
public:
    explicit Mpr3DViewCell(EAxis axis, QObject *parent = nullptr);

};



#endif //CX_CT_X2_M_MPR3D_VIEW_CELL_H
