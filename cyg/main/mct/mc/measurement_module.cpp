#include "measurement_module.h"

#include "measurement_handler.h"
#include "base/app_ids.h"
#include "u_ribbon_block.h"

#include <QAction>
#include <QIcon>


#include "u_ribbon_builder.h"

URibbonBlock MeasurementModule::createModule(QObject* parent, QMap<QString,QAction*>& actions, IURibbonHandler* handler) const {
    RibbonBuilder<MeasurementHandler> rb(parent, actions, handler);
    const QString gTools = RB_GROUP(rb, AppIds::kGroup_Measure_Tools);

    RB_MAKE(rb, AppIds::kBlock_Measurement, AppIds::kAction_Measure_Line,   AppIds::kText_Measure_Line,   MeasurementHandler, onMeasureLine,   gTools);
    RB_MAKE(rb, AppIds::kBlock_Measurement, AppIds::kAction_Measure_Angle,  AppIds::kText_Measure_Angle,  MeasurementHandler, onMeasureAngle,  gTools);
    RB_MAKE(rb, AppIds::kBlock_Measurement, AppIds::kAction_Measure_Circle, AppIds::kText_Measure_Circle, MeasurementHandler, onMeasureCircle, gTools);
    RB_MAKE(rb, AppIds::kBlock_Measurement, AppIds::kAction_Measure_Area,   AppIds::kText_Measure_Area,   MeasurementHandler, onMeasureArea,   gTools);

    return rb.buildBlock(QObject::tr(AppIds::kBlock_Measurement_Cn), AppIds::ERibbonBlock::Measurement);
}

IURibbonHandler *MeasurementModule::createHandler(QObject *parent) const
{
    return new MeasurementHandler(parent);
}
