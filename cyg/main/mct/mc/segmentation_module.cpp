#include "segmentation_module.h"
#include "segmentation_handler.h"
#include "base/app_ids.h"
#include "u_ribbon_block.h"
#include <QAction>
#include <QIcon>


#include "u_ribbon_builder.h"

URibbonBlock SegmentationModule::createModule(QObject* parent, QMap<QString,QAction*>& actions, IURibbonHandler* handler) const {
    RibbonBuilder<SegmentationHandler> rb(parent, actions, handler);
    const QString gMethod = RB_GROUP(rb, AppIds::kGroup_Segment_Method);

    RB_MAKE(rb, AppIds::kBlock_Segmentation, AppIds::kAction_Seg_Thresh, AppIds::kText_Seg_Thresh, SegmentationHandler, onSegThreshold, gMethod);
    RB_MAKE(rb, AppIds::kBlock_Segmentation, AppIds::kAction_Seg_Grow,   AppIds::kText_Seg_Grow,   SegmentationHandler, onSegRegionGrow,gMethod);
    RB_MAKE(rb, AppIds::kBlock_Segmentation, AppIds::kAction_Seg_AI,     AppIds::kText_Seg_AI,     SegmentationHandler, onSegAI,        gMethod);

    return rb.buildBlock(QObject::tr(AppIds::kBlock_Segmentation_Cn), AppIds::ERibbonBlock::Segmentation);
}

IURibbonHandler* SegmentationModule::createHandler(QObject* parent) const {
    return new SegmentationHandler(parent);
}
