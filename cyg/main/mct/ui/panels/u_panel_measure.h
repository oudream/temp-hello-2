#ifndef UI_U_PANEL_MEASURE_H
#define UI_U_PANEL_MEASURE_H

#include "u_panel_base.h"
#include <QPointer>

class QListWidget;

class QPushButton;

/**
 * @brief PanelMeasure - 测量面板（3D页-测量）
 * 作用：新增/编辑/删除测量项，列表展示
 */
class UPanelMeasure : public UPanelBase
{
Q_OBJECT
public:
    explicit UPanelMeasure(QWidget *parent = 0);

    virtual ~UPanelMeasure();

    virtual const char *getName() const;

public slots:

    virtual void updateContext(MObject *activeObject);

private slots:

    void onNewLine();

    void onNewAngle();

    void onDelete();

private:
    QWidget *_root;
    QPointer<QListWidget> _list;
    QPointer<QPushButton> _btnNewLine;
    QPointer<QPushButton> _btnNewAngle;
    QPointer<QPushButton> _btnDelete;

    void buildUi();
};

#endif // UI_U_PANEL_MEASURE_H
