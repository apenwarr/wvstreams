#pragma once


// MyEdit

class MyEdit : public CRichEditCtrl
{
	DECLARE_DYNAMIC(MyEdit)

public:
	MyEdit();
	virtual ~MyEdit();

protected:
	DECLARE_MESSAGE_MAP()
};


