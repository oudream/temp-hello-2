# CT 框架系統設計文檔 (V4.0)

> **文件版本**: 4.0
> **更新日期**: 2025年9月11日
> **設計目標**: 建立一個權責分明、高度內聚、低耦合、易於測試和擴展的工業CT軟體框架。該框架旨在有效管理 VTK 庫的複雜性，並通過接口定義一個清晰的模組化 UI 結構，為上層應用功能提供一個穩定的底層支持。

## 1. 總覽 (Overview)

本框架的核心是將 VTK 的功能進行精細的**分層**和**解耦**。我們不直接在業務邏輯中使用 VTK，而是將其隔離在一個獨立的層級中，通過抽象接口與核心業務邏輯進行通信。UI 層同樣採用接口驅動的模組化設計，確保各功能單元可以被獨立開發和管理。

### 1.1. 核心設計原則

* **分層架構 (Layered Architecture)**：將系統明確劃分為三個核心層級：**操作對象框架 (Model)**、**視覺化表示層 (Representation)** 和 **界面框架 (UI)**。
* **單向數據流 (Unidirectional Data Flow)**：用戶交互通過 `Command` 從 UI 流向 Model；Model 狀態變更通過 `Event` 從 Model 通知 UI。
* **接口隔離與依賴反轉 (Interface Segregation & Dependency Inversion)**：Model 不依賴具體 VTK 實現，UI 外殼不依賴具體 Page/Panel 實現，兩者均依賴於抽象接口。
* **分層測試策略 (Tiered Testing Strategy)**：採用單元測試和集成測試，確保業務邏輯的正確性和與 VTK 集成的正確性。

### 1.2. 架構圖
┌───────────────────────────────────────────────┐
│               界面框架 (UI Framework)           │
│ ┌────────────────┐   ┌──────────────────────┐ │
│ │  UI Managers   │   │  UI 接口與實現         │ │
│ │(Page/Panel)    │   │ ┌──────┐ ┌───────┐ │ │
│ └──────┬─────────┘   │ │ UPageBase│ │UPanelBase │ │ │
│        │             │ └─┬────┘ └─┬─────┘ │ │
│ (管理） │             │   │      │       │ │
│        ▼             │   ▼      ▼       │ │
│ ┌────────────────┐   │ ┌──────┐ ┌───────┐ │ │
│ │ MctMainWindow     │   │ │3DPage│ │PropPnl│ │ │
│ │ (外殼)         │   │ └──────┘ └───────┘ │ │
│ └────────────────┘   └──────────────────────┘ │
└──────────┬───────────────────────┬────────────┘
│ (發送 Command)        │ (訂閱 Event)
▼                       ▲
┌───────────────────────────────────────────────┐
│              Command / Event Bus              │
└──────────┬───────────────────────┬────────────┘
│ (操作 Model)          │ (來自 Model)
▼                       ▲
┌───────────────────────────────────────────────┐
│            操作對象框架 (Model Framework)       │
│  (MScene, MVolume)  ──────►  (IVolumeRep)     │
└──────────────────────┬────────────────────────┘
│ (接口由...實現)
▼
┌───────────────────────────────────────────────┐
│           視覺化表示層 (Representation Layer)   │
│          (VtkVolumeRep - 封裝所有VTK邏輯)     │
└───────────────────────────────────────────────┘

---

## 2. 框架詳解 (Framework Breakdown)

### 2.1. 操作對象框架 (Model Framework)

這是應用的**核心**，是所有狀態的「唯一真實來源 (Single Source of Truth)」。
* **職責**：管理應用的核心數據和狀態、定義和執行業務邏輯、處理命令與發布事件。
* **關鍵組件**：`MScene`, `MObject` 派生類, `MCommandBus`, `MEventHub`。
* **設計要點**：**完全不含 VTK 代碼**，通過持有 `IRepresentation` 接口指針與視覺化層解耦。

### 2.2. 視覺化表示層 (Representation Layer)

這是**抽象模型**與**具體渲染**之間的橋樑。
* **職責**：將 Model 層的抽象數據和狀態**轉換**為具體的 VTK 對象，封裝所有 VTK 複雜邏輯。
* **關鍵組件**：`IRepresentation` 接口, `VtkRepresentation` 實現類。
* **設計要點**：**VTK 隔離區**，所有 VTK 相關操作嚴格限制在此層。

### 2.3. 界面框架 (UI Framework)

這是用戶直接與之交互的**終端**，採用高度模組化的接口設計。

* **核心思想**：主窗口 (MctMainWindow) 是一個「空殼」，它只知道如何管理 `UPageBase` 和 `UPanelBase` 接口，而不知道任何具體的業務頁面。所有功能單元都是可註冊、可發現的獨立模塊。

* **關鍵組件**:
    * **UI 管理器 (UI Managers)**
        * `UPageManager`：負責註冊、實例化和切換中央工作區的所有 `UPageBase`。URibbon 菜單的點擊最終會委託給 `UPageManager` 來顯示對應的頁面。
        * `UPanelManager`：負責註冊、實例化、停靠、浮動和顯隱所有 `UPanelBase`。
    * **頁面接口 (UPageBase)**：定義了中央工作區功能頁面的標準。
        * **接口定義 (`UPageBase`)**:
            ```cpp
            class UPageBase {
            public:
                virtual ~UPageBase() = default;
                virtual const char* getName() const = 0; // 用於註冊和識別
                virtual QWidget* getWidget() = 0;         // 獲取頁面的UI實體
                virtual void onEnter() = 0;               // 進入頁面時的回調
                virtual void onLeave() =  0;              // 離開頁面時的回調
            };
            ```
        * **具體實現**: `UProjectPage`, `ReconstructionPage`, `ThreeDHostPage` 等。
    * **面板接口 (UPanelBase)**：定義了可停靠功能面板的標準。
        * **接口定義 (`UPanelBase`)**:
            ```cpp
            class UPanelBase {
            public:
                virtual ~UPanelBase() = default;
                virtual const char* getName() const = 0;
                virtual QDockWidget* getDockWidget() = 0; // 獲取面板的UI實體
                virtual void updateContext(MObject* activeObject) = 0; // 根據上下文更新內容
            };
            ```
        * **具體實現**: `PropertiesPanel`, `RenderPanel`, `LogPanel` 等。
    * **視圖接口 (UViewBase)**：定義了 3D 頁面內各個渲染視窗的標準。
        * **接口定義 (`UViewBase`)**:
            ```cpp
            class UViewBase {
            public:
                virtual ~UViewBase() = default;
                virtual vtkRenderer* getRenderer() = 0; // 每個視圖管理自己的渲染器
                virtual void setupInteractor(vtkRenderWindowInteractor* interactor) = 0; // 設置交互樣式
                virtual void handleEvent(const MEvent& event) = 0; // 獨立響應模型事件
                virtual void render() = 0; // 觸發自身渲染
            };
            ```
        * **具體實現**: `AxialView`, `SagittalView`, `CoronalView`, `ThreeDView`。它們都由 `ThreeDHostPage` 持有和管理。

---

## 3. 交互流程範例：用戶修改體素窗寬窗位

1.  **UI 層 (UPanelBase)**：用戶在 `RenderPanel` (實現了`UPanelBase`) 中拖動窗寬窗位滑塊。`RenderPanel` 的回調函數被觸發，創建一個 `CmdSetWindowLevel` 命令，其中包含目標體素的 `MId` 和新值 `{2000, 500}`。隨後調用 `_commandBus->submit(cmd)`。
2.  **Model 層 (命令執行)**：`MCommandBus` 執行命令，找到對應的 `MVolume` 對象，調用 `volume->setWindowLevel(2000, 500)`。
3.  **Model 層 (委託)**：`MVolume` 調用 `_representation->setWindowLevel(2000, 500)`。
4.  **Representation 層**：`VtkVolumeRepresentation` 執行對 `vtkVolumeProperty` 的具體操作。
5.  **Model 層 (事件發布)**：命令執行完畢，`MEventHub` 發布 `EvtPropertyChanged` 事件。
6.  **UI 層 (UPageBase/UViewBase 響應)**：`ThreeDHostPage` (實現了 `UPageBase`) 及其持有的所有 `UViewBase` 實現（`AxialView`, `ThreeDView` 等）都訂閱了此事件。它們的 `handleEvent` 方法被觸發，各自調用 `render()` 方法來請求 `vtkRenderWindow` 重新繪製自己的視圖。

---

## 4. 測試策略 (Testing Strategy)

本設計支持一個全面的、分層的測試策略。

### 4.1. 單元測試 (Unit Testing)

* **目標**：測試 Model 層的**所有**業務邏輯。
* **方法**：為 `IVolumeRepresentation` 創建一個輕量級的 `MockVolumeRepresentation`。在測試 `MVolume` 時，注入這個 Mock 對象。測試代碼只需驗證 Mock 對象的方法是否被正確調用，狀態是否符合預期即可。
* **優點**：不依賴 VTK，運行速度極快，可以集成到持續集成(CI)流程中，確保核心邏輯的穩定性。

### 4.2. 集成測試 (Integration Testing)

* **目標**：測試 **Model 層**與**Representation 層**之間的協作是否正常。
* **方法**：編寫獨立的測試用例，在測試中同時創建 `MVolume` 和**真實的** `VtkVolumeRepresentation`。測試代碼調用 `MVolume` 的方法後，需要獲取 `VtkVolumeRepresentation` 內部真實的 VTK 對象，並使用 VTK 的 API 來驗證其狀態是否正確。
* **優點**：確保了我們的代碼能正確驅動 VTK，為系統的正確性提供了最終保障。

---

