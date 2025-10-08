# Create a comprehensive Markdown spec for unified norms and rules
spec_md = r"""# CT 框架统一规范与规格（完整版 · 可执行落地）

> 适用范围：本规范覆盖 **UI 框架（ui/**）**、**操作对象框架（model/**）**、以及 **两框架间**）**。  
> 目标：保证跨模块协作、一致的代码风格、可维护性与可扩展性。**所有新代码必须符合本规范**。

---

## 1. 基本编码约定

1. **头文件保护**：一律使用包含卫士宏（`#ifndef / #define / #endif`），**不使用** `#pragma once`。
2. **类型与参数**：
    - 结构体/大对象统一使用 `const T&` 输入；输出使用 `T&`；小标量按值传参；尽量**不返回临时对象**。
    - 指针使用 nullptr，字符空使用std::string()，空结尾使用 '\0' (而不是 0 字面值)。
    - 允许在**UI 面向接口**中直接暴露 VTK 结构（如 `vtkImageData*`、`vtkRenderer*`、`vtkSmartPointer<T>` 等）。
3. **禁止项**：公共接口**不使用** `auto`、可变参数模板、晦涩元编程；不要用宏做逻辑；不随意用异常作为流程控制。
4. **命名风格**：
    - 命名空间：自己写代码，除库以外，不建议使用命名空间。
    - 类/结构体：`MVolume`、`UViewport3D`、`CmdMoveKnifePlane`、`EvtSliceReady`。
    - 函数：动词开头的 `camelCase`（例：`setWindowLevel`、`ensureVtkImageReady`）。
    - 成员：下划线前缀 `_fieldName`；常量 (字符 `CS_Name`, 数字 `CD_`)；枚举 `PascalCaseEnum`；全局或静态 (字符 `GS_Name`, 数字 `GD_`)。
5. **const用法**：函数不修改对象数据都加const；传参有值尽量使用 const T&；传参可能无值尽量使用 const T*；。
6. **异常**：不使用异常，使用返回值，（bool 成功与否）（std::string 空表示成功，不为空是错误内容）(int >=0修改或添加个数；<0错误码），业务型函数使用字符，算法型使用整形或bool
7. **空值语义**：可空返回均使用 **裸指针**（`T*`），使用方必须 `p != 0` 检查；不返回悬空引用。
8. **对象归属**：自定义对象归属层次，不能超过4层深
9. **C++标准库**：尽量只使用C++11及经典C++语法，使用代码易读易懂
10. **文档与注释**：
    - **类头注释**（`///` 或 `/** ... */`）必须说明：**作用、职责边界、核心原理/数据流**。
    - **函数注释**必须写：**参数、返回值、前置条件/后置条件、副作用、线程安全性**。
    - **成员变量注释**简述用途与单位；与 UI 绑定的变量标注“UI 线程使用”。
    - 推荐 Doxygen 风格，示例见第 8 节。
11. **单位/坐标**：长度 mm、角度 degree；右手系；矩阵**行主序**；3D 统一用齐次矩阵 `Mat4`/`MTransform`。

---

## 2. 所有权与生命周期

1. **集中所有权**：`MScene` 内部持有所有 `M*` 对象的唯一所有权（实现细节可用 `std::unique_ptr<T>`）。
2. **对外访问**：UI 与命令通过 **引用或裸指针** 访问对象；不传递所有权。
3. **删除策略（墓碑）**：`EraseObject()` **不立刻 `delete`**，仅标记对象 `_alive=false`；对象地址不复用，回收在场景析构/离线整理时统一进行。
4. **有效性检查**： **裸指针** 校验 `p!=0`。
5. **跨线程对象**：`M*` 对象**不跨线程共享写入**。自己把握生命周期，耗时任务在桥接层或执行器完成后，通过事件回到主线程更新。

---

## 3. 线程模型与并发

1. **主线程写**：命令执行、对象状态修改、事件发布在主线程进行（除渲染/切片计算）。
2. **异步任务**：重切片/纹理上传可在工作线程执行；完成后回到主线程发布 `EvtSliceReady`。
3. **版本/丢弃**：所有异步结果携带 `cacheVersion`；UI/桥接收到后必须与 `MSliceSet` 当前版本比对，过期**丢弃**。
4. **锁与等待**：核心层避免加锁；必要时在桥接层内局部使用细粒度锁（不向外暴露）。

---

## 4. 命令/事件体系（行为约束）

### 4.1 命令（Command）
- **命令抽象**：命令有序列化，反序列化接口，登记时使用序列化，撤销/重做在对上一个状态数据基础（使用反序列化）再重做一篇，大变化时记录状态数据。
- **命名**：`Cmd + 动词 + 对象`，如 `CmdMoveKnifePlane`、`CmdSetWindowLevel`。
- **职责**：**最轻量结构参数**，记录命令列表，**不做**重切片/上传等重计算。
- **撤销/重做**：命令保存 `old/new` 状态；`MCommandBus` 负责 `Submit/Undo/Redo`、分组、合并拖拽。

### 4.2 事件（Event）
- **命名**：`Evt + 对象 + Changed` / `EvtSliceInvalidated` / `EvtSliceReady`。
- **负载**：`{ pointer, versionAfter, flags }` + 业务字段（如 `viewIndex`、`textureHandle`）。
- **发布者**：
    - `MScene` 在命令完成后发布对象同步事件：`TransformChanged`、`PropertyChanged`、`ObjectAdded/Removed`、`SliceInvalidated`、`CameraChanged`等；
    - 桥接执行器在任务完成后发布 `EvtSliceReady`。

---

## 5. 面向 UI 的 VTK 暴露策略

1. **可暴露类型**（示例，不限于）：`vtkImageData*`、`vtkRenderer*`、`vtkRenderWindow*`、`vtkCamera*`、`vtkProp*`、`vtkSmartPointer<T>`。
2. **暴露原则**：
    - UI 仅用来**读取/渲染/交互**；**不**在 UI 内修改核心数据；状态改动仍通过 `Cmd*`。
    - 若返回 `vtkSmartPointer<T>`，统一以 **`const vtkSmartPointer<T>&`** 只读形式；可空则返回 `T*`。
    - 跨帧缓存由桥接层维护，UI 不持久保留 VTK 原始指针。
3. **桥接层职责**：订阅事件 → 构建或更新 `vtkActor/vtkVolume/vtkImageReslice` 等 → 驱动渲染。

---

## 6. 关系/上下文/变更收集

- **MRelationIndex**：提供只读查询（如 `SliceSetsForKnife/Style`、`LinkedCameras`），**不做重计算**。
- **MEditingContext**：保存当前激活对象、选择集、联动开关（相机/窗宽窗位等）。
- **MChangeTracker**：对象在 `set*` 中调用 `markDirty(bits)`；命令结束后由 `CommandBus` 统一从 `ChangeTracker` 读取并发布事件。

---

