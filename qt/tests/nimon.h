/*
 * Net Integrator Monitor
 */
#ifndef __NIMON_H
#define __NIMON_H

#include "wvlogrcv.h"
#include "wvqtstreamclone.h"
#include "wvstring.h"
#include "wvtcp.h"
#include "wvstreamlist.h"

#include <qmainwindow.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qscrollview.h>
#include <qlineedit.h>
#include <qtextbrowser.h>
#include <qtable.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <krun.h>
#include <klocale.h>
#include <kstddirs.h>

class NIStatusView;
class NIStatusTableView;
class NIAdminView;
class HttpReader;
class StatusUpdater;

class NIMonitorWindow : public QMainWindow
{
    Q_OBJECT
public:
    NIMonitorWindow(WvStreamList* l);
    virtual ~NIMonitorWindow();

private slots:
    void handleConfigureRequest(const QString& hostname);
    virtual void helpIndex(); 
    virtual void helpContents();
    virtual void helpAbout();   

private:
    WvStreamList* l;
    NIStatusView* statusView;
    NIStatusTableView* tableView;
    KRun* webBrowser;
    QMenuBar *menubar;
    QPopupMenu *fileMenu;
    QPopupMenu *helpMenu;
    QAction* helpContentsAction;
    QAction* helpIndexAction;
    QAction* helpAboutAction;
    QPushButton* configureButton;
};

enum NIStatus {
    NIS_UNKNOWN = 0, NIS_OK, NIS_PROBLEM, NIS_DOWN, NIS_COUNT
};

class NIStatusView : public QGroupBox
{
    Q_OBJECT
    
    WvStreamList* l;
    int refreshInterval;
    //reference to a NIStatusTableView
    NIStatusTableView* tableView;
    QPushButton* addButton;
    QPushButton* configureButton;
    QPushButton* removeButton;

public:
    NIStatusView(QWidget* parent, WvStreamList* l);
    virtual ~NIStatusView();
    void setTable(NIStatusTableView* table);

private slots:
   void promptAddMachine();
   void configureMachine();
   void removeMachine();
    
signals:    
    void configureRequest(const QString& hostname);

};

class NIStatusTableView : public QGroupBox
{
    Q_OBJECT
    
    static const QColor colors[NIS_COUNT];
    QTable* table;
    WvStreamList* l;
    
    public:
    	NIStatusTableView(QWidget* parent, WvStreamList* l);
    	virtual ~NIStatusTableView(); 
	void addMachine(const QString& hostname);
	QString promptAddMachine();
	friend class NIStatusView;

    public slots:
        void refreshAllStatus();
        void refreshStatus(const QString& hostname);
        void setStatus(const QString& hostname, NIStatus status);

    private slots:
       void handleContextMenuRequest(int row, int col, const QPoint& pos);
       void handleDoubleClick(int row, int col, int button, const QPoint& pos);

    signals:
       void configureRequest(const QString& hostname);

    private:
       int findMachineRow(const QString& hostname);
       void removeRows();
       void configureRows();
};

/* Quick hack to get callback on connection failure */
class SmartWvTCPConn : public WvTCPConn
{
public:
    inline SmartWvTCPConn(WvStringParm _hostname, __u16 _port = 0) :
        WvTCPConn(_hostname, _port) { }
    virtual ~SmartWvTCPConn() { callback(); }
};

class StatusUpdater : public QObject
{
    Q_OBJECT
    static const int CONNECTION_TIMEOUT;
    WvStreamList* l;
    QString hostname;
    WvTCPConn* conn;
public:
    StatusUpdater(WvStreamList* l, const QString& hostname);
    virtual ~StatusUpdater();
    void start();

private:
    void callback(WvStream&, void*);

signals:
    void status(const QString& hostname, NIStatus status);
};



#endif //__NIMON_H
