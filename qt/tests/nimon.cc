/*
 * Net Integrator Monitor
 */
#include "nimon.moc"

#include <qtimer.h>
#include <qpopupmenu.h>
#include <qinputdialog.h>
#include <qsizepolicy.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtable.h>
#include <qlayout.h>
#include <qframe.h>
#include <qmessagebox.h>
#include <qsplitter.h>
#include <qvbox.h>
#include <qcolor.h>
#include <qtextbrowser.h>
#include <kled.h>
#include <kurl.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kuniqueapplication.h>

int main(int argc, char *argv[])
{
    KAboutData about("nimon", "Network Integrator Monitor", "v0.01");
    KCmdLineArgs::init(argc, argv, & about);

#if 1
    KUniqueApplication::addCmdLineOptions();
    if (! KUniqueApplication::start()) {
        fputs("The program is already running...\n", stdout);
        exit(0);
    }
    KUniqueApplication app;
#else
    KApplication app;
#endif

    // setup WvStreams
    WvStreamList *l = new WvStreamList();
    WvQtStreamClone qtclone(l, 1000);
    WvLogConsole(2, WvLog::Info);
    
    // run the Qt event loop
    NIMonitorWindow* window = new NIMonitorWindow(l);
    app.setMainWidget(window);
    int result = app.exec();
    
    // finish up any remaining WvStreams i/o
    // not strictly necessary in this program, but proves that it works
    qtclone.qt_detach();
    while (! l->isempty()) {
        if (qtclone.select(1000))
            qtclone.callback();
    }

    return result;
}

/***** NIMonitorWindow *****/

NIMonitorWindow::NIMonitorWindow(WvStreamList* l) :
    QMainWindow(), l(l)
{
    setCaption("Net Integrator Monitor");
    QPixmap exitIcon(locate("icon","hicolor/16x16/actions/exit.png"));

    helpContentsAction = new QAction( this, "helpContentsAction" );
    helpContentsAction->setText( trUtf8( "Contents" ) );
    helpContentsAction->setMenuText( trUtf8( "&Contents..." ) );
    helpContentsAction->setAccel( 0 );
    helpIndexAction = new QAction( this, "helpIndexAction" );
    helpIndexAction->setText( trUtf8( "Index" ) );
    helpIndexAction->setMenuText( trUtf8( "&Index..." ) );
    helpIndexAction->setAccel( 0 );
    helpAboutAction = new QAction( this, "helpAboutAction" );
    helpAboutAction->setText( trUtf8( "About" ) );
    helpAboutAction->setMenuText( trUtf8( "&About..." ) );
    helpAboutAction->setAccel( 0 );

    // menubar
    menubar = new QMenuBar( this, "menubar" );

    fileMenu = new QPopupMenu;
    fileMenu->insertItem(exitIcon,i18n("&Quit"),qApp,SLOT(quit()),CTRL+Key_Q);

    menubar->insertItem( trUtf8( "&File" ), fileMenu );

    helpMenu = new QPopupMenu( this );
    helpContentsAction->addTo( helpMenu );
    helpIndexAction->addTo( helpMenu );
    helpMenu->insertSeparator();
    helpAboutAction->addTo( helpMenu );
    menubar->insertItem( trUtf8( "&Help" ), helpMenu );

    // signals and slots connections
    connect( helpIndexAction, SIGNAL( activated() ), this, SLOT( helpIndex() ) );
    connect( helpContentsAction, SIGNAL( activated() ), this, SLOT( helpContents() ) );
    connect( helpAboutAction, SIGNAL( activated() ), this, SLOT( helpAbout() ) );
    

    QFrame* frame = new QFrame(this);
    frame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setCentralWidget(frame);
    QVBoxLayout* layout = new QVBoxLayout(frame);
    layout->setAutoAdd(true);

    QSplitter* splitter = new QSplitter(frame);
    splitter->setOrientation(Vertical);

    statusView = new NIStatusView(splitter, l);


    tableView = new NIStatusTableView(splitter, l);

    //set the table for the statusView
    statusView->setTable(tableView);

    QObject::connect(statusView, SIGNAL(configureRequest(const QString&)),
      this, SLOT(handleConfigureRequest(const QString&)));

    QObject::connect(tableView, SIGNAL(configureRequest(const QString&)),
      this, SLOT(handleConfigureRequest(const QString&)));
    
    show();
    resize(500,300);
}


NIMonitorWindow::~NIMonitorWindow()
{
  delete webBrowser;
}
 
void NIMonitorWindow::helpIndex()
{
    qWarning( "NIMonitorWindow::helpIndex(): Not implemented yet!" );
}
 
void NIMonitorWindow::helpContents()
{
    qWarning( "NIMonitorWindow::helpContents(): Not implemented yet!" );
}
 
void NIMonitorWindow::helpAbout()
{
    qWarning( "NIMonitorWindow::helpAbout(): Not implemented yet!" );
}


/** new handleConfigureRequest method that launches the default webbrowser when 
    a configureRequest is made.
*/

void NIMonitorWindow::handleConfigureRequest(const QString& hostname)
{        
  QString url("https://");
  url += hostname;
  url += ":8043/";

  webBrowser = new KRun(KURL(url), 0, false, false);
}

/***** NIStatusView *****/

NIStatusView::NIStatusView(QWidget* parent, WvStreamList* l) :
    QGroupBox(3, Qt::Horizontal, parent), l(l),
    refreshInterval(30000)
{
    setTitle("Net Integrator Status");

    addButton = new QPushButton(this);
    addButton->setText("&Add Machine");

    configureButton = new QPushButton(this);
    configureButton->setText("&Configure Machine"); 

    removeButton = new QPushButton(this);
    removeButton->setText("&Remove Machine");


    QTimer* timer = new QTimer(this);

    QObject::connect(addButton, SIGNAL(clicked()),
		     this, SLOT(promptAddMachine()));

    QObject::connect(configureButton, SIGNAL(clicked()),
      this, SLOT(configureMachine()));

    QObject::connect(removeButton, SIGNAL(clicked()),
      this, SLOT(removeMachine()));
        
    timer->start(refreshInterval);
}

void NIStatusView::setTable(NIStatusTableView* table)
{
    tableView = table;
}


void NIStatusView::promptAddMachine()
{   
     QString hostname = tableView->promptAddMachine();
     if (hostname.length() != 0) tableView->addMachine(hostname);
}

void NIStatusView::configureMachine()
{
    tableView->configureRows();
}

void NIStatusView::removeMachine()
{
    tableView->removeRows();
}


NIStatusView::~NIStatusView()
{
}

/******* NIStatusTableView*********/

const QColor NIStatusTableView::colors[NIS_COUNT] =
    { Qt::black, Qt::green, Qt::yellow, Qt::red };

NIStatusTableView::NIStatusTableView(QWidget* parent, WvStreamList* l) :
  QGroupBox(1, Qt::Horizontal, parent), l(l)
{
    table = new QTable(0, 3, this);
    table->setSelectionMode(QTable::MultiRow);
    table->setSorting(false);
    table->setColumnReadOnly(0,true);
    table->setColumnReadOnly(1, true);
    table->setColumnReadOnly(2, false);
    
    QHeader* header = table->horizontalHeader();
    header->setLabel(0, "Hostname");
    header->setLabel(1, "Status");
    header->setLabel(2, "Update");

    table->verticalHeader()->hide();
    table->setLeftMargin(0);

    
    QObject::connect(table, SIGNAL(contextMenuRequested(int, int, const QPoint&)),
        this, SLOT(handleContextMenuRequest(int, int, const QPoint&)));
    QObject::connect(table, SIGNAL(doubleClicked(int, int, int, const QPoint&)),
        this, SLOT(handleDoubleClick(int, int, int, const QPoint&)));
}

void NIStatusTableView::setStatus(const QString& hostname, NIStatus status)
{
    int index = findMachineRow(hostname);
    if (index == -1) return;
    KLed* led = static_cast<KLed*>(table->cellWidget(index, 1));
    led->setColor(colors[status]);
}

void NIStatusTableView::handleContextMenuRequest(
    int row, int col, const QPoint& pos)
{
    QPopupMenu* menu = new QPopupMenu(this);
    menu->insertItem("&Add Machine", 1);
    int sel = table->currentRow();
    if (sel >= 0) {
        menu->insertSeparator();
        menu->insertItem("&Configure Machine", 3);
        menu->insertItem("&Remove Machine", 2);
    }
    
    int item = menu->exec(pos);
    delete menu;

    switch (item) {
        case 1: { // add machine
            QString hostname = promptAddMachine();
            if (hostname.length() != 0) addMachine(hostname);
            break;
        }
        case 2: { // remove machine
	    removeRows();
            break;
        }
        case 3: { // configure machine

	    configureRows();
            break;
        }
    }
}

void NIStatusTableView::handleDoubleClick(int row, int col, int button, const QPoint& pos)
{
    if (row >= 0 && col >= 0) {
        QString hostname = table->text(row, 0);
        emit configureRequest(hostname);
    }
}


void NIStatusTableView::addMachine(const QString& hostname)
{
    int index = findMachineRow(hostname);
    if (index != -1) return;
    int row = 0;
    table->insertRows(row, 1);
    
    KLed* led = new KLed(table);
    led->setState(KLed::On);
    table->setText(row, 0, hostname);
    table->setCellWidget(row, 1, led);

    QCheckTableItem* chkUpdate = new QCheckTableItem(table, ""); 
    table->setItem(row, 2, chkUpdate);

    setStatus(hostname, NIS_UNKNOWN);
    refreshStatus(hostname);

}

int NIStatusTableView::findMachineRow(const QString& hostname)
{
    for (int row = 0; row < table->numRows(); ++row) {
        if (table->text(row, 0) == hostname) return row;
    }
    return -1;
}


QString NIStatusTableView::promptAddMachine()
{
    bool ok = false;
    QString hostname = QInputDialog::getText(
        "Add Machine",
        "Please enter the hostname or IP address of a Net Integrator",
        QLineEdit::Normal, QString::null, & ok, this);
    if (! ok) return "";
    return hostname;
}


void NIStatusTableView::refreshAllStatus()
{
     for (int i = 0; i < table->numRows(); ++i) {
         QString hostname = table->text(i, 0);
         refreshStatus(hostname);
     }
}


void NIStatusTableView::refreshStatus(const QString& hostname)
{
     StatusUpdater* updater = new StatusUpdater(l, hostname);
     QObject::connect(updater, SIGNAL(status(const QString&, NIStatus)),
         this, SLOT(setStatus(const QString&, NIStatus)));
     updater->start();
     // note: not a leak
}

void NIStatusTableView::removeRows()
{
    QMemArray<int> rows;
    int arrCounter = 0;

    //this is necessary since the selection method of the QTable API does not 
    //work properly
    for(int i = 0; i < table->numRows(); i++){
        if(table->isRowSelected(i)){
	    rows.resize(arrCounter + 1);
	    rows[arrCounter] = i;
	    arrCounter++;
	}
    }

    table->removeRows(rows);
}

void NIStatusTableView::configureRows()
{
    int currentRow = 0;
    while(currentRow < table->numRows()){
        if(table->isRowSelected(currentRow)){
	    QString hostname = table->text(currentRow, 0);
	    emit configureRequest(hostname);
        }
       currentRow = currentRow + 1;
    } 
}


NIStatusTableView::~NIStatusTableView()
{
}


/***** StatusUpdater *****/

const int StatusUpdater::CONNECTION_TIMEOUT = 10000;

StatusUpdater::StatusUpdater(WvStreamList* l, const QString& hostname) :
    l(l), hostname(hostname), conn(NULL)
{
}

StatusUpdater::~StatusUpdater()
{
}

void StatusUpdater::start()
{
    // open connection to Net Integrator status monitor port
    conn = new SmartWvTCPConn(hostname.latin1(), 3932);
    conn->force_select(true, false, false);
    conn->setcallback(wvcallback(WvStreamCallback, *this,
        StatusUpdater::callback), NULL);
    conn->alarm(CONNECTION_TIMEOUT);
    l->append(conn, true);
}

void StatusUpdater::callback(WvStream&, void*)
{
    NIStatus statusCode = NIS_UNKNOWN;
    if (! conn->alarm_was_ticking && conn->isok()) {
        char bit;
        int len = conn->read(&bit, 1);
        if (len == 0) return; // no data yet
        if (len == 1) {
            if (bit == '1') statusCode = NIS_OK;
            else if (bit == '0') statusCode = NIS_PROBLEM;
        }
    } else {
        statusCode = NIS_DOWN;
    }
    // prevent further callbacks from occurring
    conn->drain();
    conn->setcallback(NULL, NULL);
    emit status(hostname, statusCode);
    delete this;
}


