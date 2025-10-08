#ifndef UI_ADAPTERS_U_COMMAND_HELPERS_H
#define UI_ADAPTERS_U_COMMAND_HELPERS_H

#include <QString>
#include <QVariantMap>

/// 将 UI 的 cmd 字符串规范化为 Cmd* 类型名或路由键（符合“统一规范 4.1”）  :contentReference[oaicite:19]{index=19}
inline QString normalizeCmd(const QString &cmd)
{
    // 示例：camera.dolly -> CmdDollyCamera；knife.move -> CmdMoveKnifePlane
    if (cmd == "camera.dolly") return "CmdDollyCamera";
    if (cmd == "camera.orbit") return "CmdOrbitCamera";
    if (cmd == "camera.reset") return "CmdResetCamera";
    if (cmd == "knife.move") return "CmdMoveKnifePlane";
    if (cmd == "knife.rotate") return "CmdMoveKnifePlane"; // 旋转同一命令不同参数
    if (cmd == "knife.set_thickness") return "CmdSetKnifeThickness";
    if (cmd == "knife.set_lock_axis") return "CmdSetKnifeLockaxis";
    if (cmd == "render.set_mode") return "CmdSetTransferFunction";
    if (cmd == "render.set_sampling") return "CmdSetTransferFunction";
    if (cmd == "render.set_lut") return "CmdSetTransferFunction";
    if (cmd == "render.set_window") return "CmdSetWindowLevel";
    if (cmd == "render.set_level") return "CmdSetWindowLevel";
    if (cmd == "measure.create_line") return "CmdCreateMeasure";
    if (cmd == "measure.create_angle")return "CmdCreateMeasure";
    if (cmd == "measure.delete_selected") return "CmdDeleteMeasure";
    // ...补充其他映射...
    return cmd;
}

#endif // UI_ADAPTERS_U_COMMAND_HELPERS_H
