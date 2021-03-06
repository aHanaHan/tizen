#include <FApp.h>
#include <FBaseLog.h>
#include "TizenTodoMainForm.h"

using namespace Tizen::App;
using namespace Tizen::Base;
using namespace Tizen::Base::Collection;
using namespace Tizen::Io;
using namespace Tizen::Ui;
using namespace Tizen::Ui::Controls;
using namespace Tizen::Ui::Scenes;
using namespace Tizen::Graphics;
using namespace Tizen::Base::Utility;

TizenTodoMainForm::TizenTodoMainForm(void) :
		__pTodosList(null) {
}

TizenTodoMainForm::~TizenTodoMainForm(void) {
}

bool TizenTodoMainForm::Initialize(void) {
	Construct(L"IDF_FORM");
	return true;
}

result TizenTodoMainForm::OnInitializing(void) {
	result r = E_SUCCESS;

	// get a button via resource ID
	Tizen::Ui::Controls::Button *pButtonOk = static_cast<Button*>(GetControl(L"IDC_BUTTON_OK"));
	if (pButtonOk != null) {
		pButtonOk->SetActionId(ID_BUTTON_CREATE);
		pButtonOk->AddActionEventListener(*this);
	}

	// create button
	Tizen::Ui::Controls::Button *pButton1 = static_cast<Button*>(GetControl(L"IDC_BUTTON1"));
	if (pButtonOk != null) {
		pButton1->SetActionId(ID_BUTTON_DONE);
		pButton1->AddActionEventListener(*this);
	}

	// create list view
	__pListView = static_cast<ListView*>(GetControl(L"IDC_LISTVIEW"));
	__pListView->SetItemProvider(*this);
	__pListView->AddListViewItemEventListener(*this);
	__pTodosList = new (std::nothrow) ArrayList();

	// read todo works from file
	String strFileContents;
	String filePath = App::GetInstance()->GetAppDataPath() + L"todolist.txt";

	__pDataIO = new (std::nothrow) TizenTodoDataIO();
	__pDataIO->ReadFile(filePath, strFileContents);

	StringTokenizer st(strFileContents, String(L"\n"));
	String token;
	while(st.HasMoreTokens())
	{
		st.GetNextToken(token);
		__pTodosList->Add(new (std::nothrow) String(token));
	}

	//create popup
	CreatePopup();
	CreatePopup2();

	// setup back event listener
	SetFormBackEventListener(this);

	return r;
}

result TizenTodoMainForm::OnTerminating(void) {

	result r = E_SUCCESS;
	String filePath = App::GetInstance()->GetAppDataPath() + L"todolist.txt";

	String strFileContents;

	for(int i=0; i<__pTodosList->GetCount(); i++){
		String* todo = static_cast<String*>(__pTodosList->GetAt(i));
		strFileContents.Append(todo->GetPointer());
		strFileContents.Append("\n");
	}

	__pDataIO->WriteFile(filePath, strFileContents);

	return r;
}

/*
 * Create Popup
 * When clicking "New" button, this popup appears.
 * User can input todo work.
 * This popup has text edit and Ok/Cancel buttons.
 */
void TizenTodoMainForm::CreatePopup(void) {
	// create popup
	__pPopup = new (std::nothrow) Popup();
	__pPopup->Construct(true, Dimension(600, 450));
	__pPopup->SetTitleText(L"Input to do");

	Rectangle rect;
	rect = __pPopup->GetClientAreaBounds();

	// create label
	__pEditField = new (std::nothrow) EditField();
	__pEditField->Construct(Rectangle(50, 50, 450, 100));
	__pPopup->AddControl(*__pEditField);

	// create "ok" button
	Button *pButtonOK = new (std::nothrow) Button();
	pButtonOK->Construct(Rectangle(50, 230, 150, 74), L"OK");
	pButtonOK->SetActionId(ID_BUTTON_POPUP_OK);
	pButtonOK->AddActionEventListener(*this);
	__pPopup->AddControl(*pButtonOK);

	// create "cancel" button
	Button *pButtonClose = new (std::nothrow) Button();
	pButtonClose->Construct(Rectangle(350, 230, 150, 74), L"Cancel");
	pButtonClose->SetActionId(ID_BUTTON_POPUP_CANCEL);
	pButtonClose->AddActionEventListener(*this);
	__pPopup->AddControl(*pButtonClose);

}

/*
 * Create Popup
 * When clicking list item in list view, this popup appears.
 * User can delete todo work or move todo to done screen.
 * This popup has text view and Done/Delete/Cancel buttons.
 */
void TizenTodoMainForm::CreatePopup2(void) {
	// create popup
	__pPopup2 = new (std::nothrow) Popup();
	__pPopup2->Construct(true, Dimension(700, 450));
	__pPopup2->SetTitleText(L"Finish or delete the work ?");

	Rectangle rect;
	rect = __pPopup2->GetClientAreaBounds();

	// create label
	__pLabel = new (std::nothrow) Label();
	__pLabel->Construct(Rectangle(50, 50, 500, 100),L"");
	__pPopup2->AddControl(*__pLabel);

	// create "Done" button
	Button *pButtonDone = new (std::nothrow) Button();
	pButtonDone->Construct(Rectangle(50, 230, 150, 74), L"Finish");
	pButtonDone->SetActionId(ID_BUTTON_POPUP2_DONE);
	pButtonDone->AddActionEventListener(*this);
	__pPopup2->AddControl(*pButtonDone);

	// create "Delete" button
	Button *pButtonDelete = new (std::nothrow) Button();
	pButtonDelete->Construct(Rectangle(250, 230, 150, 74), L"Delete");
	pButtonDelete->SetActionId(ID_BUTTON_POPUP2_DELETE);
	pButtonDelete->AddActionEventListener(*this);
	__pPopup2->AddControl(*pButtonDelete);

	// create "Cancel" button
	Button *pButtonCancel = new (std::nothrow) Button();
	pButtonCancel->Construct(Rectangle(450, 230, 150, 74), L"Cancel");
	pButtonCancel->SetActionId(ID_BUTTON_POPUP2_CANCEL);
	pButtonCancel->AddActionEventListener(*this);
	__pPopup2->AddControl(*pButtonCancel);
}

void TizenTodoMainForm::OnActionPerformed(
		const Tizen::Ui::Control& source, int actionId) {

	SceneManager* pSceneManager = SceneManager::GetInstance();
	AppAssert(pSceneManager);

	String todo;
	ListView lv;

	switch (actionId) {
	case ID_BUTTON_CREATE:
		ShowPopup(__pPopup);
		break;

	// go to the "done" scene.
	case ID_BUTTON_DONE:
		pSceneManager->GoForward(SceneTransitionId(L"ID_SCNT_2"));
		break;

	// add a todo to todo list.
	case ID_BUTTON_POPUP_OK:
		todo = __pEditField->GetText();
		__pTodosList->Add(new String(todo.GetPointer()));
		__pListView->UpdateList();
		HidePopup(__pPopup);
		break;

	// cancel to add new todo.
	case ID_BUTTON_POPUP_CANCEL:
		HidePopup(__pPopup);
		break;

	// move a todo to done list.
	case ID_BUTTON_POPUP2_DONE:
		__pDataIO->selectedDoneWork = (static_cast<String*>(__pTodosList->GetAt(selectIndex)))->GetPointer();
		HidePopup(__pPopup2);
		__pTodosList->RemoveAt(selectIndex);
		__pListView->UpdateList();
		pSceneManager->GoForward(SceneTransitionId(L"ID_SCNT_2"));
		break;

	// delete selected todo.
	case ID_BUTTON_POPUP2_DELETE:
		__pTodosList->RemoveAt(selectIndex);
		__pListView->UpdateList();
		HidePopup(__pPopup2);
		break;

	// cancel to delete or move todo.
	case ID_BUTTON_POPUP2_CANCEL:
		HidePopup(__pPopup2);
		break;

	default:
		break;
	}
}

/*
 *  When user press back button, go to the before screen.
 */
void TizenTodoMainForm::OnFormBackRequested(
		Tizen::Ui::Controls::Form& source) {
	UiApp* pApp = UiApp::GetInstance();
	AppAssert(pApp);
	pApp->Terminate();
}

void TizenTodoMainForm::OnSceneActivatedN(
		const Tizen::Ui::Scenes::SceneId& previousSceneId,
		const Tizen::Ui::Scenes::SceneId& currentSceneId,
		Tizen::Base::Collection::IList* pArgs) {
	// TODO:
	// Add your scene activate code here
}

void TizenTodoMainForm::OnSceneDeactivated(
		const Tizen::Ui::Scenes::SceneId& currentSceneId,
		const Tizen::Ui::Scenes::SceneId& nextSceneId) {
	// TODO:
	// Add your scene deactivate code here
}

void TizenTodoMainForm::ShowPopup(Popup* __pPopup) {
	__pPopup->SetShowState(true);
	__pPopup->Show();
}

void TizenTodoMainForm::HidePopup(Popup* __pPopup) {
	__pPopup->SetShowState(false);
	Invalidate(true);
}
//
// IListViewItemEventListener
void TizenTodoMainForm::OnListViewContextItemStateChanged(
		Tizen::Ui::Controls::ListView &listView, int index, int elementId,
		Tizen::Ui::Controls::ListContextItemStatus state) {

}

void TizenTodoMainForm::OnListViewItemStateChanged(
		Tizen::Ui::Controls::ListView &listView, int index, int elementId,
		Tizen::Ui::Controls::ListItemStatus status) {

	if (status == LIST_ITEM_STATUS_SELECTED) {
		selectIndex = index;
		String* todo = static_cast<String*>(__pTodosList->GetAt(selectIndex));
		__pLabel->SetText(todo->GetPointer());
		ShowPopup(__pPopup2);
	}
}

void TizenTodoMainForm::OnListViewItemSwept(
		Tizen::Ui::Controls::ListView &listView, int index,
		Tizen::Ui::Controls::SweepDirection direction) {
}

Tizen::Ui::Controls::ListItemBase* TizenTodoMainForm::CreateItem(
		int index, int itemWidth) {
	SimpleItem* pItem = new SimpleItem();
	AppAssert(pItem);

	String* todo = static_cast<String*>(__pTodosList->GetAt(index));

	pItem->Construct(Dimension(itemWidth, 50), LIST_ANNEX_STYLE_NORMAL);

	pItem->SetElement(*todo);

	return pItem;
}

bool TizenTodoMainForm::DeleteItem(int index,
		Tizen::Ui::Controls::ListItemBase* pItem, int itemWidth) {
	delete pItem;
	pItem = null;
	return true;
}

int TizenTodoMainForm::GetItemCount(void) {
	if (__pTodosList) {
		return __pTodosList->GetCount();
	} else {
		return 0;
	}
}
