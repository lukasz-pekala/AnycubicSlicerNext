#include "FarmGroupPanelObj.hpp"

#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/font.h>
#include <wx/dcgraph.h>
#include <string>
#include <stdio.h>
#include "slic3r/GUI/wxExtensions.hpp"
#include "slic3r/GUI/MainFrame.hpp"
#include <wx/richtooltip.h>
#include "slic3r/GUI/Plater.hpp"
#include <plugins_sdk/event/detail/program_color.hxx>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "ACSpinInput.hpp"
#include "AmsInfoDialog.hpp"

#ifdef __WXOSX__
#define wxOSX true
#else
#define wxOSX false
#endif




FarmGroupPanelDialog::FarmGroupPanelDialog(
    wxWindow* parent, const FramGroupListInfo& framGroupList, int stateIndex, int groupIndex, int typeIndex, bool showLanIndex)
    : Slic3r::GUI::DPIDialog(parent,
                wxID_ANY,
                _L("Choose more batch printers"),
                wxDefaultPosition,
                wxDefaultSize,
#ifdef __APPLE__
                wxSTAY_ON_TOP |
#endif // __APPLE__

                    wxCAPTION | wxCLOSE_BOX,
                "Send Task List panel Dialog")
    , m_showLanIndex(showLanIndex)
{

          init(framGroupList, stateIndex, groupIndex, typeIndex);
}


void FarmGroupPanelDialog::init(const FramGroupListInfo& framGroupList, int stateIndex, int groupIndex, int typeIndex)
{

    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << "FarmGroupPanelDialog init ";
   
    this->SetBackgroundColour(COLOR_Neutral_01);

    m_mainSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* btn_sizer = new wxBoxSizer(wxVERTICAL);


    wxStaticText* showText = new wxStaticText(this, wxID_ANY, _L("Choose more batch printers"), wxDefaultPosition, wxDefaultSize,
                                             wxST_ELLIPSIZE_END);
    showText->Wrap(-1);
    showText->SetForegroundColour(COLOR_PRINTER_TEXT);
    showText->SetFont(Label::Head_14);

    FramGroupListInfo newGroupList_select;
    FramGroupListInfo newGroupList_not_select;

    for (const FramGroupInfo& itemInfo : framGroupList.g_framGroupList) {
        if (itemInfo.g_isSelect) {
            newGroupList_select.g_framGroupList.push_back(itemInfo);
        } else {
            newGroupList_not_select.g_framGroupList.push_back(itemInfo);
        }
    }
    for (int i = 0; i < newGroupList_not_select.g_framGroupList.size(); i++) {
        newGroupList_select.g_framGroupList.push_back(newGroupList_not_select.g_framGroupList[i]);
    }

    m_farmGroupPanel = new FarmGroupPanel(this, newGroupList_select, 15, stateIndex, groupIndex, typeIndex, 1, m_showLanIndex);

    wxPanel* btnPanel =  CreateBtnPanel();

    btn_sizer->Add(btnPanel, 0, wxEXPAND);

    m_mainSizer->AddSpacer(FromDIP(20));
    m_mainSizer->Add(showText, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(24));
    m_mainSizer->AddSpacer(FromDIP(8));
    m_mainSizer->Add(m_farmGroupPanel, 1, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(24));
    m_mainSizer->AddSpacer(FromDIP(14));
    m_mainSizer->Add(btn_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(24));
    m_mainSizer->AddSpacer(FromDIP(20));
    btn_sizer->Layout();

    SetSizerAndFit(m_mainSizer);
    m_mainSizer->Layout();
    Layout();

#ifdef __APPLE__
    SetSize(GetWindowSize());
#else
    this->SetMinSize(GetWindowSize());
    SetSize(GetWindowSize());
#endif
    Layout();

    wxPoint parentPoint;
    wxPoint nowPoint;
    GetParent()->GetPosition(&parentPoint.x, &nowPoint.y);
    
    int screenwidth = wxSystemSettings::GetMetric(wxSYS_SCREEN_X, NULL);
    nowPoint.x      = parentPoint.x + GetWindowSize().x;
    if (GetParent()->GetSize().x +  parentPoint.x + GetWindowSize().x > screenwidth) {
        nowPoint.x = parentPoint.x -  GetWindowSize().x;
    }
    SetPosition(nowPoint);
    Slic3r::GUI::wxGetApp().UpdateDlgDarkUI(this);
}


wxPanel* FarmGroupPanelDialog::CreateBtnPanel() 
{
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << "FarmGroupPanelDialog CreateBtnPanel ";
    wxPanel* btn_panel = new wxPanel(this);
    btn_panel->SetBackgroundColour(COLOR_Neutral_01);
    wxBoxSizer* infoSizer = new wxBoxSizer(wxHORIZONTAL);

    m_cancelBtn = new Button(btn_panel, _L("Cancel"));
    m_cancelBtn->SetMinSize(wxSize(120, 36));
    m_cancelBtn->SetCornerRadius(6);
    m_cancelBtn->SetFont(Label::Head_14);
    m_cancelBtn->Bind(wxEVT_BUTTON, &FarmGroupPanelDialog::OnButtonCancelEvent, this, m_cancelBtn->GetId());

    m_confirmBtn = new Button(btn_panel, _L("Confirm"));
    m_confirmBtn->SetMinSize(wxSize(120, 36));
    m_confirmBtn->SetCornerRadius(6);
    m_confirmBtn->SetFont(Label::Head_14);
    m_confirmBtn->Bind(wxEVT_BUTTON, &FarmGroupPanelDialog::OnButtonConfirmEvent, this, m_confirmBtn->GetId());

    SetButtonStyle(m_cancelBtn, false, wxSize(FromDIP(74), FromDIP(24)), FromDIP(12));
    SetButtonStyle(m_confirmBtn, true, wxSize(FromDIP(100), FromDIP(24)), FromDIP(12));

    

    infoSizer->Add(0, 0, 1, wxEXPAND);
    infoSizer->Add(m_cancelBtn);
    infoSizer->Add(m_confirmBtn, 0, wxLEFT, FromDIP(16));

    btn_panel->SetSizer(infoSizer);
    infoSizer->Layout();

    return btn_panel;
    


}

void FarmGroupPanelDialog::OnButtonCancelEvent(wxCommandEvent& event) 
{
    this->EndModal(wxID_NO);

}


void FarmGroupPanelDialog::OnButtonConfirmEvent(wxCommandEvent& event) 
{
    std::vector<FramGroupInfo> infoList    = m_farmGroupPanel->GetShowSelectSubListInfo();
    m_selectPrinterMap.clear();
    if (infoList.size() > 0) {
        for (const FramGroupInfo& obj: infoList) {
            
            m_selectPrinterMap[obj.g_deviceID] = obj.g_isSelect;
            
        }
    }


    this->EndModal(wxID_YES);

}



void FarmGroupPanelDialog::SetButtonStyle(Button* btn, bool isfull, wxSize btnSize, int radius)
{
    btn->SetBtnStyleColor(isfull ? 0 : 2);

    btn->SetFont(Label::Body_13);
    btn->SetCornerRadius(radius);
    btn->SetMinSize(btnSize);
}

void FarmGroupPanelDialog::msw_rescale()
{
    wxSize _size = GetWindowSize();
    this->SetMinSize(_size);

    Fit();

    this->Layout();

    Refresh();
}

wxSize FarmGroupPanelDialog::GetWindowSize()
{
    int winSize_W =
#ifdef __APPLE__
        FromDIP(562)
#else
        FromDIP(592)
#endif // __APPLE__

        ;

    int winSize_H = FromDIP(780);

    wxDisplay display;
    wxRect    screenGeometry = display.GetGeometry();
    int       screenHeight   = screenGeometry.GetHeight() - FromDIP(40);
    if (winSize_H > screenHeight) {
        winSize_H = screenHeight;
    }

    return wxSize(winSize_W, winSize_H);
}


FarmGroupPanel::FarmGroupPanel(
    wxWindow* parent, const FramGroupListInfo& framGroupList,int indexMaxNum, int stateIndex, int groupIndex, int typeIndex, int showNum, bool showLanIndex)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
    , m_framGroupList(framGroupList)
    , m_indexMaxNum(indexMaxNum)
    , m_showNum(showNum)
    , m_farmStateSelect(stateIndex)
    , m_farmGroupSelect(groupIndex)
    , m_farmTypeSelect(typeIndex)
    , m_showLanIndex(showLanIndex)
{ 

    init(); 
    BindEvent();
}


void FarmGroupPanel::SetNewListInfo(const FramGroupListInfo& framGroupList) 
{

    m_framGroupList = framGroupList;

}

void FarmGroupPanel::BindEvent() 
{
    this->Bind(EVT_FARM_GROUP_SELECT_PRINTER_FINISH_EVENT, [this](wxPluginEvent& event) {
        bool     isCheck  = event.GetId() == 1;
        if (m_checkObjMap.size() > 0) {
            m_titleCheckBox->SetValue(isCheck);
            for (auto& [key, value] : m_checkObjMap) {
                wxString  name = key;
                CheckBox* cb   = value;
                if (cb && cb->IsShown() && cb->GetValue() != isCheck) {
                    cb->SetValue(isCheck);
                }
            }
        }
        

    });
    
    

}

wxString FarmGroupPanel::GetStrFromList(const wxArrayString& infoList, int index)
{
    wxString resultStr = wxEmptyString;
    if (infoList.size() > 0 && infoList.size() > index) {
        resultStr = infoList[index];
    }
    

    return resultStr;
}


void FarmGroupPanel::SetAllFramGroupListInfo(bool check) 
{
    for (FramGroupInfo& obj : m_showSelectSubList) {
        wxPluginEvent evt(EVT_FARM_GROUP_SELECT_PRINTER_EVENT);
        evt.SetString(obj.g_deviceID);
        evt.SetInt(check ? 1 : 0);
        wxPostEvent(this, evt);
    }

}


void FarmGroupPanel::SetAllSelctState(bool isCheck)
{
    if (m_checkObjMap.size() > 0) {
        m_titleCheckBox->SetValue(isCheck);
        for (auto& [key, value] : m_checkObjMap) {
            wxString  name = key;
            CheckBox* cb   = value;
            if (cb && cb->IsShown() && cb->GetValue() != isCheck) {
                cb->SetValue(isCheck);
            }
            wxPluginEvent evt_send(EVT_FARM_GROUP_SELECT_PRINTER_EVENT);
            evt_send.SetString(name);
            evt_send.SetInt(isCheck ? 1 : 0);
            if (m_showNum == m_indexMaxNum) {
                wxPostEvent(wxGetTopLevelParent(this), evt_send);
            } else {
                wxPostEvent(this, evt_send); 
            }
        }
    }
    if (m_showNum == m_indexMaxNum) {
        wxPluginEvent evt_selectAll(EVT_FARM_GROUPLIST_SELECT_ALL_EVENT);
        evt_selectAll.SetInt(isCheck ? 1 : 0);
        wxPostEvent(wxGetTopLevelParent(this), evt_selectAll);
    } else {
        SetAllFramGroupListInfo(isCheck);
    }
    

}

void FarmGroupPanel::init() 
{ 
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << "FarmGroupPanel init ";
    this->SetBackgroundColour(COLOR_Neutral_01);
    if (m_showNum == m_indexMaxNum) {
        m_isSelectAll = false;
    }
    m_mainSizer = new wxBoxSizer(wxVERTICAL);


    m_printerNameMaxWidth = FromDIP(113);
    m_lanPrinterNameMaxWidth = FromDIP(70);
    m_stateMaxWidth       = FromDIP(70);
    m_groupNumMaxWidth    = FromDIP(70);
    m_modelNameMaxWidth   = FromDIP(70);
    m_amsMaxWidth         = FromDIP(118);


    for (const FramGroupInfo& itemInfo : m_framGroupList.g_framGroupList) 
    {
        wxString state = itemInfo.g_state;
        wxString g_modelName = itemInfo.g_modelName;
        wxString g_groupNum  = itemInfo.g_groupNum;
        wxString g_printerType = itemInfo.g_linkeType;

        if (m_printerStateList.Index(state, false) == wxNOT_FOUND) {
            m_printerStateList.Add(state);
        }

        if (g_groupNum.size() > 0 && m_printerGroupInfoList.Index(g_groupNum, false) == wxNOT_FOUND) {
            m_printerGroupInfoList.Add(g_groupNum);
        }

        if (m_printerModelNameList.Index(g_modelName, false) == wxNOT_FOUND) {
            m_printerModelNameList.Add(g_modelName);
        }

        if (m_printerTypeList.Index(g_printerType, false) == wxNOT_FOUND) {
            m_printerTypeList.Add(g_printerType);
        }
    }
    m_printerStateList.Insert(_L("ALL"),0);
    m_printerGroupInfoList.Insert(_L("ALL"), 0);
    m_printerTypeList.Insert(_L("ALL"), 0);



    m_showPrinterType  = GetStrFromList(m_printerTypeList, m_farmTypeSelect);
    m_showStateStr     = GetStrFromList(m_printerStateList, m_farmStateSelect);
    m_showModelNameStr = GetStrFromList(m_printerModelNameList,0);
    m_showNumStr       = GetStrFromList(m_printerGroupInfoList, m_farmGroupSelect);

    if (m_framGroupList.g_framGroupList.size() > 0) {
        for (int i = (m_nowPageNum - 1) * m_pageShowMaxNum; i < m_framGroupList.g_framGroupList.size(); i++) {
            const FramGroupInfo& info = m_framGroupList.g_framGroupList[i];
            if ((info.g_state == m_showStateStr || m_farmStateSelect == 0) && (info.g_groupNum == m_showNumStr || m_farmGroupSelect == 0) &&
                (info.g_linkeType == m_showPrinterType || m_farmTypeSelect == 0)) {
                m_showSelectSubList.push_back(info);
                if (m_showNum == 1 && m_isSelectAll && !info.g_isSelect) {
                    m_isSelectAll = false;
                }
            }
        }
    } else {
        m_isSelectAll = false;
    }

    m_printerSum = m_showSelectSubList.size();

    m_titlePanel   = CreateTitlePanel();
    m_contentPanel = CreateContentPanel();
    m_mainSizer->Add(m_titlePanel, 0, wxEXPAND);
    m_mainSizer->AddSpacer(FromDIP(8));
    m_mainSizer->Add(m_contentPanel, 0, wxEXPAND);

    if (m_showNum != m_indexMaxNum) {
        m_changePagePanel = CreateChangePagePanel();
        m_mainSizer->Add(0, 0, 1, wxEXPAND, 5);
        m_mainSizer->AddSpacer(FromDIP(8));
        m_mainSizer->Add(m_changePagePanel, 0, wxEXPAND);
    }

    this->SetSizer(m_mainSizer);
    m_mainSizer->Layout();

    CheckPageOpEvent();
    Bind(EVT_FARM_GROUP_SELECT_PRINTER_EVENT, [this](wxPluginEvent& evt) {
        wxString deviceID = evt.GetString();
        bool     check      = evt.GetInt() == 1;
        for (FramGroupInfo& obj : m_showSelectSubList) {
            if (obj.g_deviceID == deviceID) {
                obj.g_isSelect = check;
                break;
            }
        }

    });

}

wxPanel* FarmGroupPanel::CreateTitlePanel()
{
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << "FarmGroupPanel CreateTitle ";

    wxPanel* title_panel = new wxPanel(this);
    title_panel->SetBackgroundColour(COLOR_Neutral_03);
    title_panel->SetSize(FromDIP(wxSize(-1,28)));
    title_panel->SetMinSize(FromDIP(wxSize(-1, 28)));
    wxBoxSizer* titleSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* titleSizer_ace= new wxBoxSizer(wxHORIZONTAL);
    int         titleGap       = FromDIP(20);
    m_titleCheckBox = new CheckBox(title_panel);
    m_titleCheckBox->Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent& evt) {
        int isChecked = evt.GetInt();
        SetAllSelctState(isChecked==1);
        evt.Skip();
    });


    m_titleCheckBox->SetValue(m_isSelectAll);
    wxString btnIco     = "drop_down_white";
    int      maxWidth = FromDIP(1000);

    m_linkTypeComboBox = new ACCustomComboBox(title_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(m_lanPrinterNameMaxWidth, -1),
                                              0, nullptr, maxWidth, wxCB_READONLY);
    m_linkTypeComboBox->SetToolTip(m_showPrinterType);
    m_linkTypeComboBox->Set(m_printerTypeList);
    m_linkTypeComboBox->SetSelection(m_farmTypeSelect);
    m_linkTypeComboBox->SetStyleInfo(0);


    m_showSatateComboBox = new ACCustomComboBox(title_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(m_stateMaxWidth, -1), 0,
                                                nullptr, maxWidth, wxCB_READONLY);
    m_showSatateComboBox->SetToolTip(m_showStateStr);
    m_showSatateComboBox->Set(m_printerStateList);
    m_showSatateComboBox->SetSelection(m_farmStateSelect);
    m_showSatateComboBox->SetStyleInfo(0);

    m_showNumComboBox = new ACCustomComboBox(title_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(m_groupNumMaxWidth, -1), 0,
                                             nullptr, maxWidth, wxCB_READONLY);
    m_showNumComboBox->SetToolTip(m_showNumStr);
    m_showNumComboBox->Set(m_printerGroupInfoList);
    m_showNumComboBox->SetSelection(m_farmGroupSelect);
    m_showNumComboBox->SetStyleInfo(0);

    m_linkTypeComboBox->Bind(wxEVT_COMBOBOX, &FarmGroupPanel::OnButtonPopEvent, this, m_linkTypeComboBox->GetId());
    m_showSatateComboBox->Bind(wxEVT_COMBOBOX, &FarmGroupPanel::OnButtonPopEvent, this, m_showSatateComboBox->GetId());
    m_showNumComboBox->Bind(wxEVT_COMBOBOX, &FarmGroupPanel::OnButtonPopEvent, this, m_showNumComboBox->GetId());


    wxStaticText* modelNameText = new wxStaticText(title_panel, wxID_ANY, _L("Printer"), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);
    modelNameText->Wrap(-1);
    modelNameText->SetMaxSize(wxSize(m_modelNameMaxWidth, -1));
    modelNameText->SetMinSize(wxSize(m_modelNameMaxWidth, -1));
    modelNameText->SetSize(wxSize(m_modelNameMaxWidth, -1));
    modelNameText->SetForegroundColour(COLOR_FARM_TITLE_GRAY_TEXT);
    modelNameText->SetFont(Label::Body_13);


    wxString      titleNumStr = wxString::Format("%d", m_printerSum )+ _L("pcs");
    m_numText            = new wxStaticText(title_panel, wxID_ANY, titleNumStr, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);
    m_numText->Wrap(-1);
    m_numText->SetMaxSize(wxSize(m_printerNameMaxWidth, -1));
    m_numText->SetMinSize(wxSize(m_printerNameMaxWidth, -1));
    m_numText->SetSize(wxSize(m_printerNameMaxWidth, -1));
    m_numText->SetToolTip(titleNumStr);
    m_numText->SetForegroundColour(COLOR_Blue_10);
    m_numText->SetFont(Label::Body_13);

    wxStaticText* aceProText = new wxStaticText(title_panel, wxID_ANY, "ACE Pro", wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);
    aceProText->Wrap(-1);
    aceProText->SetMaxSize(wxSize(m_amsMaxWidth, -1));
    aceProText->SetMinSize(wxSize(m_amsMaxWidth, -1));
    aceProText->SetSize(wxSize(m_amsMaxWidth, -1));
    aceProText->SetForegroundColour(COLOR_FARM_TITLE_GRAY_TEXT);
    aceProText->SetFont(Label::Body_13);

    titleSizer_ace->Add(aceProText, 0, wxALIGN_CENTER|wxTOP,FromDIP(1));

    if (m_printerSum == 0)
        m_titleCheckBox->Enable(false);
    titleSizer->AddSpacer(FromDIP(14));
    titleSizer->Add(m_titleCheckBox, 0, wxALIGN_CENTER);
    titleSizer->AddSpacer(FromDIP(8));
    titleSizer->Add(m_numText, 0, wxALIGN_CENTER);
    titleSizer->AddSpacer(FromDIP(8));
    titleSizer->Add(m_linkTypeComboBox, 0, wxEXPAND | wxALIGN_CENTER);
    titleSizer->Add(m_showSatateComboBox, 0, wxEXPAND | wxALIGN_CENTER);
    titleSizer->AddSpacer(FromDIP(8));
    titleSizer->Add(m_showNumComboBox, 0, wxEXPAND | wxALIGN_CENTER);
    titleSizer->AddSpacer(FromDIP(20));
    titleSizer->Add(modelNameText, 0,  wxALIGN_CENTER);
    
    if (!m_showLanIndex) {
        titleSizer->Hide(m_linkTypeComboBox);
    } else {
        titleSizer->Hide(m_showSatateComboBox);
    }
    

    titleSizer->Add(0, 0, 1, wxEXPAND, 5);
    titleSizer->Add(titleSizer_ace, 0, wxEXPAND|wxRIGHT);

    title_panel->SetSizer(titleSizer);
    titleSizer->Layout();

    return title_panel;
    

}


wxPanel* FarmGroupPanel::CreatePage(wxWindow* win, const std::vector<FramGroupInfo>& infoList)
{
    wxPanel* page_panel = new wxPanel(win);
    page_panel->SetBackgroundColour(COLOR_Neutral_01);

    wxBoxSizer* pageSizer = new wxBoxSizer(wxVERTICAL);
    for (const FramGroupInfo& info : infoList) {


        pageSizer->Add(CreateObjItem(page_panel, info), 0, wxTOP | wxEXPAND, FromDIP(4));
    }


    page_panel->SetSizer(pageSizer);
    pageSizer->Layout();

    return page_panel;
}

wxPanel* FarmGroupPanel::CreateContentPanel() 
{
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << "FarmGroupPanel CreateContent ";

    wxPanel* content_panel = new wxPanel(this);
    content_panel->SetBackgroundColour(COLOR_Neutral_03);
    m_contentSizer = new wxBoxSizer(wxHORIZONTAL);

    std::vector<FramGroupInfo> now_subList;


    int sumSize = m_showSelectSubList.size();
    for (int i = (m_nowPageNum - 1) * m_pageShowMaxNum; i < sumSize; i++) {
        if ((m_showNum == m_indexMaxNum && i >= m_indexMaxNum) || i >= (m_nowPageNum * m_pageShowMaxNum)) {
            break;
        }
        const FramGroupInfo& info = m_showSelectSubList[i];
        now_subList.push_back(info);
    }

    m_sumPageNum       = int((sumSize + (m_pageShowMaxNum - 1)) / m_pageShowMaxNum);
    m_nowShowPagePanel = CreatePage(content_panel, now_subList);
    

    m_contentSizer->Add(m_nowShowPagePanel, 1, wxEXPAND);

    content_panel->SetSizer(m_contentSizer);
    m_contentSizer->Layout();

    return content_panel;

}



wxPanel* FarmGroupPanel::CreateChangePagePanel() 
{

    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << "FarmGroupPanel CreateChangePagePanel ";

    wxPanel* changePage_panel = new wxPanel(this);
    changePage_panel->SetBackgroundColour(COLOR_Neutral_01);
    changePage_panel->SetMinSize(FromDIP(wxSize(-1, 28)));
    wxBoxSizer* changePageSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* changePageSizer_info = new wxBoxSizer(wxHORIZONTAL);

    wxString leftIco     = GetLeftOrRightIcoName(true, false);
    wxString leftIco_dis = GetLeftOrRightIcoName(true, true);

    m_left_btn = new Button(changePage_panel, "", leftIco, 0, 14);
    SetBtnStyle(m_left_btn);
    m_left_btn->Bind(wxEVT_BUTTON, &FarmGroupPanel::OnButtonLeftlEvent, this);


    wxString rightIco     = GetLeftOrRightIcoName(false, false);
    wxString rightIco_dis = GetLeftOrRightIcoName(false, true);
    m_right_btn           = new Button(changePage_panel, "", rightIco,0,14);
    SetBtnStyle(m_right_btn);
    m_right_btn->Bind(wxEVT_BUTTON, &FarmGroupPanel::OnButtonRightEvent, this);

    //m_nowPageNum m_sumPageNum;

    wxString contentTextStr = wxString::Format("%d/%d", m_nowPageNum, m_sumPageNum);
    m_pageInfoContentText   = CreateItemTextObj(changePage_panel, contentTextStr, -1, COLOR_Neutral_13);


    changePageSizer_info->Add(m_left_btn);
    changePageSizer_info->AddSpacer(FromDIP(8));
    changePageSizer_info->Add(m_pageInfoContentText);
    changePageSizer_info->AddSpacer(FromDIP(8));
    changePageSizer_info->Add(m_right_btn);

    changePageSizer->Add(0, 0, 1, wxEXPAND, 5);
    changePageSizer->Add(changePageSizer_info, 1, wxEXPAND);
    changePageSizer->Add(0, 0, 1, wxEXPAND, 5);

    changePage_panel->SetSizer(changePageSizer);
    changePageSizer->Layout();

    return changePage_panel;

}

wxPanel* FarmGroupPanel::CreateObjItem(wxWindow* win,const FramGroupInfo& info)
{
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << "FarmGroupPanel CreateObjItem ";

    wxPanel* item_panel = new wxPanel(win);
    item_panel->SetBackgroundColour(COLOR_Neutral_01);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* itemSizer = new wxBoxSizer(wxHORIZONTAL);

    
    CheckBox* itemCheckBox = new CheckBox(item_panel);
    

    
    itemCheckBox->Bind(wxEVT_TOGGLEBUTTON, [this,info](wxCommandEvent& evt) {
       
        int            isChecked = evt.GetInt();
        wxPluginEvent evt_send(EVT_FARM_GROUP_SELECT_PRINTER_EVENT);
        evt_send.SetString(info.g_deviceID);
        evt_send.SetInt(isChecked);
        if (m_showNum == m_indexMaxNum) {
            wxPostEvent(wxGetTopLevelParent(this), evt_send);
        } else {
            wxPostEvent(this, evt_send);
        }

        SetAllCheckState();
        evt.Skip();
    });
    


    itemCheckBox->SetValue(info.g_isSelect);
    m_checkObjMap[info.g_deviceID] = itemCheckBox;
    
    wxStaticText* showPrinterNameText = CreateItemTextObj(item_panel, info.g_printerName,m_printerNameMaxWidth,COLOR_Neutral_13);
    wxStaticText* showStateText     = CreateItemTextObj(item_panel, info.g_state, m_stateMaxWidth, COLOR_Neutral_13);
    wxStaticText* showgroupNumText  = CreateItemTextObj(item_panel, info.g_groupNum, m_groupNumMaxWidth, COLOR_Neutral_13);
    wxStaticText* showModelNameText = CreateItemTextObj(item_panel, info.g_modelName.AfterFirst(' '), m_modelNameMaxWidth, COLOR_Neutral_13);

    wxPanel* lanPanel = CreateShowLanIcoPanel(item_panel, info.g_linkeType != "LAN");

    wxPanel* m_amsBoxPanel_empty = new wxPanel(item_panel);
    m_amsBoxPanel_empty->SetMaxSize(wxSize(m_amsMaxWidth, FromDIP(28)));
    m_amsBoxPanel_empty->SetMinSize(wxSize(m_amsMaxWidth, FromDIP(28)));
    m_amsBoxPanel_empty->SetSize(wxSize(m_amsMaxWidth, FromDIP(28)));
    m_amsBoxPanel_empty->SetBackgroundColour(COLOR_Neutral_01);

    Button* showText_link = new Button(m_amsBoxPanel_empty, _L("Disconnected"));

    StateColor report_bg;
    showText_link->SetCanFocus(false);
    showText_link->SetPaddingSize(wxSize(0, 0));
    report_bg.append(COLOR_Neutral_01, StateColor::Normal);
    showText_link->SetBackgroundColor(report_bg);
    showText_link->SetBorderWidth(0);
    showText_link->SetTextColor(COLOR_Remote_gray);
    showText_link->SetFont(Label::Body_13);

    wxBoxSizer* empty_printer_sizer = new wxBoxSizer(wxHORIZONTAL);
    empty_printer_sizer->Add(showText_link, 0, wxALIGN_CENTER_VERTICAL);
    m_amsBoxPanel_empty->SetSizer(empty_printer_sizer);
    empty_printer_sizer->Layout();






    
    wxPanel* amsPanel = new AmsInfoPanel(item_panel, info.g_slotInfoList, true, true, false);
    amsPanel->SetMaxSize(wxSize(m_amsMaxWidth, FromDIP(28)));
    amsPanel->SetMinSize(wxSize(m_amsMaxWidth, FromDIP(28)));
    amsPanel->SetSize(wxSize(m_amsMaxWidth, FromDIP(28)));

    itemSizer->AddSpacer(FromDIP(14));
    itemSizer->Add(itemCheckBox,0,wxEXPAND);
    itemSizer->AddSpacer(FromDIP(8));
    itemSizer->Add(showPrinterNameText, 0,  wxALIGN_CENTER);
    itemSizer->AddSpacer(FromDIP(8));
    itemSizer->Add(lanPanel, 0, wxALIGN_CENTER);
    if (m_showLanIndex) {
        itemSizer->AddSpacer(FromDIP(15));
    }
    itemSizer->Add(showStateText, 0,  wxALIGN_CENTER);
    itemSizer->AddSpacer(FromDIP(8));
    itemSizer->Add(showgroupNumText, 0, wxALIGN_CENTER);                                          
    itemSizer->AddSpacer(FromDIP(8));
    itemSizer->Add(showModelNameText, 0, wxALIGN_CENTER);
    itemSizer->AddSpacer(FromDIP(8));
    itemSizer->Add(m_amsBoxPanel_empty, 1, wxEXPAND | wxLEFT, FromDIP(12));
    itemSizer->Add(amsPanel, 0, wxEXPAND);
    if (info.g_slotInfoList.size() > 0 ) {
        m_amsBoxPanel_empty->Hide();
    } else {
        amsPanel->Hide();
    }
    if (!m_showLanIndex) {
        itemSizer->Hide(lanPanel);
    } else {
        itemSizer->Hide(showStateText);
    }
    //itemSizer->AddSpacer(FromDIP(8));

    wxPanel* bottomLine = new wxPanel(item_panel);
    bottomLine->SetSize(-1, 1);
    bottomLine->SetBackgroundColour(COLOR_Ams_dialog_borderColor);



    mainSizer->Add(itemSizer, 0, wxEXPAND);
    mainSizer->AddSpacer(FromDIP(4));
    mainSizer->Add(bottomLine,0,wxEXPAND);


    item_panel->SetSizer(mainSizer);
    mainSizer->Layout();

    return item_panel;

}



wxPanel* FarmGroupPanel::CreateShowLanIcoPanel(wxWindow* parent, bool isEmpty)
{
    wxPanel* panel = new wxPanel(parent);
    panel->SetBackgroundColour(COLOR_Blue_02);
    wxSize panelSize = parent->FromDIP(wxSize(55, 19));
#ifdef __APPLE__
    panel->SetSize(panelSize);
#else
    panel->SetMinSize(panelSize);
    panel->SetSize(panelSize);
#endif

    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

    wxString icoStr     = "icon_lan_info";
    wxString ShowLANStr = isEmpty ? "WAN " : "LAN ";

    Button* lanIcoShow = new Button(panel, "", icoStr, 0, 16);
    lanIcoShow->SetPaddingSize(wxSize(0, 0));
    lanIcoShow->SetBackgroundColor(wxColour(0, 0, 0, 0));
    lanIcoShow->SetBorderWidth(0);

    wxStaticText* lanInfo = new wxStaticText(panel, wxID_ANY, ShowLANStr, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);
    lanInfo->Wrap(-1);
    lanInfo->SetForegroundColour(COLOR_Blue_10);
    lanInfo->SetFont(Label::Body_12);

    mainSizer->AddSpacer(parent->FromDIP(4));
    mainSizer->Add(lanIcoShow);
    mainSizer->AddSpacer(parent->FromDIP(2));
    mainSizer->Add(lanInfo, 0, wxALIGN_CENTER);
    mainSizer->Add(0, 0, 1, wxEXPAND);

    panel->SetSizer(mainSizer);
    mainSizer->Layout();
    return panel;
}


wxPanel* FarmGroupPanel::CrateAmsColorPanel(wxWindow* win,const std::vector<AmsSlotObjInfo>& slotInfoList)
{
    wxPanel* amsBoxPanel = new wxPanel(win);
    return amsBoxPanel;
    /*amsBoxPanel->SetBackgroundColour(COLOR_Neutral_01);

    wxSize panelSize(FromDIP(104), FromDIP(28));
    wxBoxSizer* color_sizer = new wxBoxSizer(wxHORIZONTAL);

    bool isOne = slotInfoList.size() <= 4;

    std::vector<AmsSlotObjInfo> nowAmsSlotInfoList_1;
    std::vector<AmsSlotObjInfo> nowAmsSlotInfoList_2;
    for (int i = 0; i < slotInfoList.size(); i++) {
        if (i < 4) {
            nowAmsSlotInfoList_1.push_back(slotInfoList[i]);
        } else {
            nowAmsSlotInfoList_2.push_back(slotInfoList[i]);
        }
    }
    RemoteSlotColorPanel*colorPanel_1 = new RemoteSlotColorPanel(amsBoxPanel, nowAmsSlotInfoList_1, panelSize);

    RemoteSlotColorPanel*colorPanel_2 = new RemoteSlotColorPanel(amsBoxPanel, nowAmsSlotInfoList_2, panelSize);

    color_sizer->Add(colorPanel_1);
    color_sizer->Add(colorPanel_2, 0, wxLEFT, FromDIP(6));
    if (isOne)
        colorPanel_2->Show(false);

    amsBoxPanel->SetSizer(color_sizer);
    color_sizer->Layout();
    return amsBoxPanel;*/
}

wxString FarmGroupPanel::GetLeftOrRightIcoName(bool isLeft,bool isDisable) 
{
    wxString icoStr;

    bool isDrak = wxGetApp().dark_mode();
    if (isLeft) {
        icoStr = isDisable ? isDrak ? "farm_group_more_left_disable_icon_drak" : "farm_group_more_left_disable_icon" :
                 isDrak    ? "farm_group_more_left_icon_drak" : "farm_group_more_left_icon";
        
    } else {
        icoStr = isDisable ? isDrak ? "farm_group_more_right_disable_icon_drak" : "farm_group_more_right_disable_icon" :
                 isDrak    ? "farm_group_more_right_icon_drak" : "farm_group_more_right_icon";
    }

    return icoStr;
}

void FarmGroupPanel::CheckPageOpEvent() 
{
    if (m_left_btn) {

        wxString contentTextStr = wxString::Format("%d/%d", m_nowPageNum, m_sumPageNum);
        m_pageInfoContentText->SetLabel(contentTextStr);


        wxString leftIco = GetLeftOrRightIcoName(true,false);
        wxString leftIco_dis = GetLeftOrRightIcoName(true, true);

        wxString rightIco     = GetLeftOrRightIcoName(false, false);
        wxString rightIco_dis = GetLeftOrRightIcoName(false, true);


        if (!m_left_btn->IsEnabled()) {
            m_left_btn->Enable(true);
            m_left_btn->SetIcon(leftIco);
        }
        if (!m_right_btn->IsEnabled()) {
            m_right_btn->Enable(true);
            m_right_btn->SetIcon(rightIco);
        }

        if ((m_nowPageNum == 1 && m_sumPageNum == 1) || m_sumPageNum == 0) {
            m_left_btn->Enable(false);
            m_left_btn->SetIcon(leftIco_dis);
            m_right_btn->Enable(false);
            m_right_btn->SetIcon(rightIco_dis);
            return;
        }
        if (m_nowPageNum == 1) {
            m_left_btn->Enable(false);
            m_left_btn->SetIcon(leftIco_dis);
            return;
        }
        if (m_nowPageNum == m_sumPageNum) {
            m_right_btn->Enable(false);
            m_right_btn->SetIcon(rightIco_dis);
            return;
        }
    }

}

wxStaticText* FarmGroupPanel::CreateItemTextObj(wxWindow* parent, const wxString& showStr, int maxWidth, wxColour textColor)
{

    wxStaticText* showText = new wxStaticText(parent, wxID_ANY, showStr, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);
    showText->SetMaxSize(wxSize(maxWidth, -1));
    showText->SetMinSize(wxSize(maxWidth, -1));
    showText->SetSize(wxSize(maxWidth, -1));
    showText->SetToolTip(showStr);
    showText->SetForegroundColour(textColor);
    showText->SetFont(Label::Body_14);


    return showText;
}



void FarmGroupPanel::SetSelectPrinterCheck(const wxString& id, bool check, bool isTitleCheck, bool fromDialog)
{

    if (m_checkObjMap.size() > 0) {
        for (auto& [key, value] : m_checkObjMap) {
            wxString  name = key;
            CheckBox* cb   = value;
            if (id == name && cb && cb->IsShown() && cb->GetValue() != check) {
                cb->SetValue(check);
                break;
            }
        }
        SetAllCheckState(isTitleCheck);
    }


}


bool FarmGroupPanel::NowListAllSelect() 
{
    bool isSelectAll = true;
    for (int i = 0; i < m_showSelectSubList.size(); i++) {
        const FramGroupInfo& info = m_showSelectSubList[i];
        if (!info.g_isSelect) {
            isSelectAll = false;
        }
    }
    return isSelectAll;
}

void FarmGroupPanel::SetAllCheckState(bool check, bool fromDialog)
{

    if (m_checkObjMap.size() > 0) {
        bool isCheck = true;
        for (auto& [key, value] : m_checkObjMap) 
        {
            wxString  name = key;
            CheckBox* cb   = value;
            if (cb && cb->IsShown() && !cb->GetValue()) {
                isCheck = false;
                break;
            }
        
        }

        m_titleCheckBox->SetValue(isCheck);
        if (m_showNum == m_indexMaxNum && check) {
            m_titleCheckBox->SetValue(false);
        }
        if (m_showNum == m_indexMaxNum && !fromDialog && isCheck && m_showSelectSubList.size() != m_checkObjMap.size()) {
            m_titleCheckBox->SetValue(false);
        }
        
    }


}

void FarmGroupPanel::RefreshContentEvent() 
{
    m_checkObjMap.clear();
    wxWindowUpdateLocker noUpdates(this);
    wxPanel* oldpanel = m_contentPanel;
    m_contentPanel = CreateContentPanel();
    wxGetApp().UpdateDarkUIWin(m_contentPanel);
    m_mainSizer->Replace(oldpanel, m_contentPanel);
    oldpanel->Destroy();
    m_mainSizer->Layout();
    this->Layout();
    
    CheckPageOpEvent();
}


void FarmGroupPanel::RefreshContentFromSelect(int stateIndex, int groupIndex, int typeIndex)
{

    if (m_farmStateSelect == stateIndex && m_farmGroupSelect == groupIndex && m_farmTypeSelect == groupIndex)
        return;
    m_farmStateSelect = stateIndex;
    m_farmGroupSelect = groupIndex;
    m_farmTypeSelect  = typeIndex;

    m_showSatateComboBox->SetSelection(m_farmStateSelect);
    m_showNumComboBox->SetSelection(m_farmGroupSelect);
    m_linkTypeComboBox->SetSelection(m_farmTypeSelect);

    m_showStateStr    = GetStrFromList(m_printerStateList, m_farmStateSelect);
    m_showSatateComboBox->SetToolTip(m_showStateStr);
    m_showNumStr      = GetStrFromList(m_printerGroupInfoList, m_farmGroupSelect);
    m_showNumComboBox->SetToolTip(m_showNumStr);

    m_showPrinterType = GetStrFromList(m_printerTypeList, m_farmTypeSelect);
    m_linkTypeComboBox->SetToolTip(m_showPrinterType);

    m_nowPageNum = 1;
    m_showSelectSubList.clear();
    //m_checkObjMap.clear();
    for (int i = (m_nowPageNum - 1) * m_pageShowMaxNum; i < m_framGroupList.g_framGroupList.size(); i++) {
        const FramGroupInfo& info = m_framGroupList.g_framGroupList[i];
        if ((info.g_state == m_showStateStr || m_farmStateSelect == 0) && (info.g_groupNum == m_showNumStr || m_farmGroupSelect == 0) &&
            (info.g_linkeType == m_showPrinterType || m_farmTypeSelect == 0)) {
            m_showSelectSubList.push_back(info);
        }
    }

    m_printerSum         = m_showSelectSubList.size();
    wxString titleNumStr = wxString::Format("%d", m_printerSum) + _L("pcs");
    m_numText->SetLabel(titleNumStr);

    RefreshContentEvent();
    SetAllCheckState();
    GetParent()->Layout();

}

void FarmGroupPanel::OnButtonPopEvent(wxCommandEvent& event)
{ 
    RefreshContentFromSelect(m_showSatateComboBox->GetSelection(), m_showNumComboBox->GetSelection(), m_linkTypeComboBox->GetSelection());
}

void FarmGroupPanel::OnButtonLeftlEvent(wxCommandEvent& event) 
{ 
    m_nowPageNum--;
    RefreshContentEvent();
}

void FarmGroupPanel::OnButtonRightEvent(wxCommandEvent& event) 
{

    m_nowPageNum++;
    RefreshContentEvent();
}

void FarmGroupPanel::SetComboBoxStyle(ACCustomComboBox* comboBox)
{


    StateColor background_color = StateColor(std::make_pair(COLOR_Neutral_03, (int) StateColor::Disabled),std::make_pair(COLOR_Neutral_03, (int) StateColor::Normal));
    StateColor text_color       = StateColor(std::make_pair(COLOR_FARM_TITLE_GRAY_TEXT, (int) StateColor::Disabled),std::make_pair(COLOR_FARM_TITLE_GRAY_TEXT, (int) StateColor::Normal));
    StateColor borderColor      = StateColor(std::make_pair(COLOR_Neutral_03, (int) StateColor::Disabled),std::make_pair(COLOR_Neutral_03, (int) StateColor::Normal));
    
    comboBox->SetBackgroundColor(background_color);
    comboBox->SetBorderColor(borderColor);
    comboBox->SetLabelColor(text_color);
    comboBox->SetBorderWidth(0);
    comboBox->GetDropDown().SetMinSize(FromDIP(wxSize(120,-1)));
    comboBox->GetDropDown().SetSize(FromDIP(wxSize(120, -1)));
    comboBox->GetDropDown().SetMaxSize(FromDIP(wxSize(120, -1)));
}

void FarmGroupPanel::SetBtnStyle(Button* btn)
{
    btn->SetTextColor(COLOR_FARM_TITLE_GRAY_TEXT);
    btn->SetPaddingSize(wxSize(0, 0));
    btn->SetBackgroundColor(wxColour(0, 0, 0, 0));
    btn->SetBorderWidth(0);
    btn->SetFont(Label::Body_13);
}