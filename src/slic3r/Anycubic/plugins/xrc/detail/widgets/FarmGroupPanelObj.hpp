#ifndef slic3r_GUI_Anycubic_RemotePrinting_FarmGroupPanelObj_hpp_
#define slic3r_GUI_Anycubic_RemotePrinting_FarmGroupPanelObj_hpp_

#include <wx/wx.h>
#include "slic3r/GUI/GUI.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/GUI_Utils.hpp"
#include "slic3r/GUI/BitmapCache.hpp"
#include "libslic3r/Utils.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"
#include "slic3r/GUI/Widgets/CheckBox.hpp"
#include "slic3r/GUI/Widgets/Button.hpp"
#include "slic3r/GUI/Widgets/ComboBox.hpp"
#include "RemotePrintingAmsDialog.hpp"
#include "PrinterFunctionCheckBox.hpp"
#include "RemoteSlotColorPanel.hpp"
#include "ACButton.hpp"
#include "ACCustomComboBox.hpp"
#include <plugins_sdk/event/detail/plugin_custom_event.hxx>

#include "custom_struct_manger.hxx"


class FarmGroupPanel : public wxPanel
{
public:
    FarmGroupPanel(wxWindow*                parent,
                   const FramGroupListInfo& framGroupList,
                   int                      indexMaxNum,
                   int                      stateIndex = 0,
                   int                      groupIndex = 0,
                   int                      typeIndex  = 0,
                   int                      showNum    = 3,
                   bool                     showLanIndex = false);
    ~FarmGroupPanel() = default;


    void OnButtonLeftlEvent(wxCommandEvent& event);
    void OnButtonRightEvent(wxCommandEvent& event);

    void OnButtonPopEvent(wxCommandEvent& event);
    void RefreshContentFromSelect(int stateIndex, int groupIndex,int typeIndex);

    std::map<wxString, CheckBox*> GetFarmInfoMap() { return m_checkObjMap; }
    void                          SetSelectPrinterCheck(const wxString& id, bool check, bool isTitleCheck,bool fromDialog=false);
    void                          SetAllCheckState(bool check = false, bool fromDialog = false);


    int GetFarmStateSelectIndex() { return m_farmStateSelect; }
    int GetFarmGroupSelectIndex() { return m_farmGroupSelect; }
    int GetLinkTypeSelectIndex() { return m_farmTypeSelect; }


    std::vector<FramGroupInfo> GetShowSelectSubListInfo() { return m_showSelectSubList; }
    wxPanel*                   CreateShowLanIcoPanel(wxWindow* parent, bool isEmpty = true);

private:
    bool m_showLanIndex{false};


    void init();
    void BindEvent();

    wxString GetStrFromList(const wxArrayString& infoList,int index);
    void SetAllSelctState(bool isCheck);
    void SetAllFramGroupListInfo(bool check);
    bool NowListAllSelect();

    void SetNewListInfo(const FramGroupListInfo& framGroupList);

    wxPanel* CreateTitlePanel();
    wxPanel* CreateContentPanel();
    wxPanel* CreateChangePagePanel();

    wxPanel* CreatePage(wxWindow* win, const std::vector<FramGroupInfo>& infoList);

    wxPanel* CreateObjItem(wxWindow* win,const FramGroupInfo& info);

    wxStaticText* CreateItemTextObj(wxWindow* parent, const wxString& showStr, int maxWidth, wxColour textColor);
    wxPanel*      CrateAmsColorPanel(wxWindow* win, const std::vector<AmsSlotObjInfo>& slotInfoList);

    wxString GetLeftOrRightIcoName(bool isLeft,bool isDisable);

    void CheckPageOpEvent();
    void RefreshContentEvent();

    void SetComboBoxStyle(ACCustomComboBox* comboBox);
    void SetBtnStyle(Button* btn);

    FramGroupListInfo m_framGroupList;

    int m_showNum;
    int m_pageShowMaxNum = 15;
    int m_printerSum;
    int m_indexMaxNum;

    int m_farmTypeSelect = 0;
    int m_farmStateSelect = 0;
    int m_farmGroupSelect = 0;

    int m_lanPrinterNameMaxWidth;
    int m_printerNameMaxWidth;
    int m_stateMaxWidth;
    int m_groupNumMaxWidth;
    int m_modelNameMaxWidth;
    int m_amsMaxWidth;

    int m_sumPageNum;
    int m_nowPageNum = 1;

    bool m_isSelectAll{true};

    wxArrayString m_printerTypeList;
    wxArrayString m_printerStateList;
    wxArrayString m_printerGroupInfoList;
    wxArrayString m_printerModelNameList;

    wxPanel* m_titlePanel{nullptr};
    wxPanel* m_contentPanel{nullptr};
    wxPanel* m_changePagePanel{nullptr};


    wxPanel* m_nowShowPagePanel{nullptr};
    

    std::vector<wxPanel*> m_itemPanelList;

    std::vector<wxPanel*> m_pagePanelList;

    CheckBox* m_titleCheckBox{nullptr};
    wxBoxSizer* m_contentSizer{nullptr};
    wxBoxSizer*   m_mainSizer{nullptr};
    wxStaticText* m_pageInfoContentText{nullptr};

    Button*   m_left_btn{nullptr};
    Button* m_right_btn{nullptr};

    ACCustomComboBox* m_showSatateComboBox{nullptr};
    ACCustomComboBox* m_showNumComboBox{nullptr};
    ACCustomComboBox* m_linkTypeComboBox{nullptr};
    wxStaticText*     m_numText {nullptr};

    std::map<wxString, CheckBox*> m_checkObjMap;


    wxString m_showPrinterType{wxEmptyString};
    wxString m_showStateStr{wxEmptyString};
    wxString m_showNumStr{wxEmptyString};
    wxString m_showModelNameStr{wxEmptyString};

    std::vector<FramGroupInfo> m_showSelectSubList;


};

class FarmGroupPanelDialog : public Slic3r::GUI::DPIDialog
{
public:
    FarmGroupPanelDialog(
        wxWindow* parent, const FramGroupListInfo& framGroupList, int stateIndex, int groupIndex, int typeIndex, bool showLanIndex = false);
    ~FarmGroupPanelDialog() = default;


    void   init(const FramGroupListInfo& framGroupList, int stateIndex, int groupIndex, int typeIndex);
    void msw_rescale();
    wxSize GetWindowSize();
    void   SetButtonStyle(Button* btn, bool isfull, wxSize btnSize, int radius);

    void OnButtonCancelEvent(wxCommandEvent& event);
    void OnButtonConfirmEvent(wxCommandEvent& event);
    FarmGroupPanel* GetFarmGroupPanel() { return m_farmGroupPanel; }

    std::map<wxString, bool> GetSelectPrinterMap() { return m_selectPrinterMap; }

private:
    bool     m_showLanIndex{false};
    wxPanel* CreateBtnPanel();

    wxBoxSizer* m_mainSizer{nullptr};

    Button* m_confirmBtn{nullptr};
    Button* m_cancelBtn{nullptr};
    bool    m_selectAll{false};
    FarmGroupPanel* m_farmGroupPanel{nullptr};
    std::map<wxString,bool>   m_selectPrinterMap;

protected:
    void on_dpi_changed(const wxRect& suggested_rect) override { msw_rescale(); }

};





#endif //!slic3r_GUI_Anycubic_RemotePrinting_FarmGroupPanelObj_hpp_
