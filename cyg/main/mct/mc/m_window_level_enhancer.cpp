#include "m_window_level_enhancer.h"
#include "m_mpr2d_view.h"

void WindowLevelEnhancer::setWindowLevel(double win, double lev)
{
    _win = win;
    _lev = lev; emit windowLevelChanged(_win, _lev);
}

void WindowLevelEnhancer::applyTo(Mpr2DView &v) const
{ v.setWL(_win, _lev); }

void WindowLevelEnhancer::presetBone()
{ setWindowLevel(2000, 300); }

void WindowLevelEnhancer::presetSoft()
{ setWindowLevel(400, 40); }

void WindowLevelEnhancer::presetMetal()
{ setWindowLevel(4000, 800); }
