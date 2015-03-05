/* mainwindow.cpp
 * --------------
 * Implementation of the main GUI window for the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	this->setGeometry(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	this->setWindowTitle("Meaview: Channel view");
	initSettings();
	initCtrlWindow();
	initMenuBar();
	//initInfoWidget();
	//initRecordingControlsWidget();
	//initToolBar();
	initStatusBar();
	initPlotGroup();
}

MainWindow::~MainWindow() {
}

void MainWindow::initSettings() {
	settings.setSaveDir(DEFAULT_SAVE_DIR);
	settings.setSaveFilename(DEFAULT_SAVE_FILENAME);
	settings.setChannelView(DEFAULT_VIEW);
	settings.setExperimentLength(DEFAULT_EXPERIMENT_LENGTH);
	settings.setDisplayScale(DEFAULT_DISPLAY_SCALE);
	settings.setRefreshInterval(DISPLAY_REFRESH_INTERVAL);
	settings.setPlotColor(DEFAULT_PLOT_COLOR);
	settings.setAutoscale(false);
	settings.setOnlineAnalysisLength(DEFAULT_ONLINE_ANALYSIS_LENGTH);
	settings.setJump(AIB_BLOCK_SIZE);
}

void MainWindow::initCtrlWindow() {
	this->ctrlWindow = new CtrlWindow(this);
	this->ctrlWindow->show();
}

void MainWindow::initMenuBar() {
	this->menubar = new QMenuBar(0);

	/* File menu */
	this->fileMenu = new QMenu(tr("&File"));

	/* About menu item */
	//QMenu *aboutMenu = new QMenu("About Meaview");
	//this->menubar->addMenu(aboutMenu);

	/* New recording menu item */
	QAction *newRecordingAction = new QAction(tr("&New"), this->fileMenu);
	newRecordingAction->setShortcut(QKeySequence("Ctrl+N"));
	connect(newRecordingAction, SIGNAL(triggered()), this, SLOT(openNewRecording()));
	this->fileMenu->addAction(newRecordingAction);

	/* Load recording for replay */
	QAction *loadRecordingAction = new QAction(tr("&Open"), this->fileMenu);
	loadRecordingAction->setShortcut(QKeySequence("Ctrl+O"));
	connect(loadRecordingAction, SIGNAL(triggered()), this, SLOT(loadRecording()));
	this->fileMenu->addAction(loadRecordingAction);

	/* Windows menu */
	this->windowsMenu = new QMenu(tr("&Windows"));
	QAction *showMainWindow = new QAction(tr("&Channel view"), this->windowsMenu);
	showMainWindow->setShortcut(QKeySequence("Ctrl+0"));
	showMainWindow->setCheckable(true);
	showMainWindow->setChecked(true);
	connect(showMainWindow, SIGNAL(triggered()), this, SLOT(toggleVisible()));
	this->windowsMenu->addAction(showMainWindow);

	QAction *showControlsWindow = new QAction(tr("Control window"), this->windowsMenu);
	showControlsWindow->setShortcut(QKeySequence("Ctrl+1"));
	showControlsWindow->setCheckable(true);
	showControlsWindow->setChecked(true);
	connect(showControlsWindow, SIGNAL(triggered()), this->ctrlWindow, SLOT(toggleVisible()));
	this->windowsMenu->addAction(showControlsWindow);

	/* eventually same for online analysis and channel inspector */

	/* Add menus to bar and bar to MainWindow */
	this->menubar->addMenu(this->fileMenu);
	this->menubar->addMenu(this->windowsMenu);
	this->setMenuBar(this->menubar);
}

void MainWindow::initInfoWidget() {
	infoWidget = new QDockWidget("Info:", this);
	infoWidgetLayout = new QGridLayout(this);

	timeGroup = new QGroupBox();
	timeLabel = new QLabel("Time:");
	timeLine = new QLineEdit("");
	timeLine->setMaxLength(5);
	timeValidator = new QIntValidator(0, MAX_EXPERIMENT_LENGTH);
	timeLine->setValidator(timeValidator);
	timeLine->setReadOnly(true);
	infoWidgetLayout->addWidget(timeLabel, 0, 0);
	infoWidgetLayout->addWidget(timeLine, 0, 1);

	fileLabel = new QLabel("File:");
	fileLine = new QLineEdit(this->settings.getSaveFilename());
	fileLine->setReadOnly(true);
	infoWidgetLayout->addWidget(fileLabel, 1, 0);
	infoWidgetLayout->addWidget(fileLine, 1, 1);
	timeGroup->setLayout(infoWidgetLayout);
	infoWidget->setWidget(timeGroup);
	this->addDockWidget(Qt::TopDockWidgetArea, infoWidget);
}

void MainWindow::initToolBar() {
	toolbar = new QToolBar();

	/* Group showing current time and total time */
	timeGroup = new QGroupBox();
	timeLabel = new QLabel("Time:");
	timeLine = new QLineEdit("");
	timeLine->setMaxLength(5);
	timeValidator = new QIntValidator(0, MAX_EXPERIMENT_LENGTH);
	timeLine->setValidator(timeValidator);
	timeLine->setReadOnly(true);
	timeLayout = new QHBoxLayout();
	timeLayout->addWidget(timeLabel);
	timeLayout->addWidget(timeLine);
	timeGroup->setLayout(timeLayout);
	toolbar->addWidget(timeGroup);
	
	toolbar->addSeparator();

	/* Button to start/stop recording */
	startButton = new QPushButton("Start");
	startButton->setEnabled(false);
	startButton->setShortcut(QKeySequence("Ctrl+Enter"));
	//connect(this->startButton, SIGNAL(triggered()), this, SLOT(startPlayback()));
	toolbar->addWidget(this->startButton);

	settingsButton = new QPushButton("Settings");
	connect(this->settingsButton, SIGNAL(clicked()), this, SLOT(openSettings()));
	toolbar->addWidget(settingsButton);

	toolbar->addSeparator();

	autoscale = false;
	scaleGroup = new QGroupBox();
	scaleLayout = new QHBoxLayout();
	scaleLabel = new QLabel("Scale:");
	scaleBox = new QComboBox();
	for (auto &s: DISPLAY_SCALES)
		scaleBox->addItem(QString::number(s), QVariant(s));
	scaleBox->setCurrentIndex(scaleBox->findData(QVariant(DEFAULT_DISPLAY_SCALE)));
	connect(scaleBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setScale(int)));
	autoscaleCheckBox = new QCheckBox("Autoscale");
	autoscaleCheckBox->setTristate(false);
	autoscaleCheckBox->setCheckState(Qt::Unchecked);
	connect(autoscaleCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setAutoscale(int)));
	scaleLayout->addWidget(scaleLabel);
	scaleLayout->addWidget(autoscaleCheckBox);
	scaleLayout->addWidget(scaleBox);
	scaleGroup->setLayout(scaleLayout);
	toolbar->addWidget(scaleGroup);

	addToolBar(toolbar);
}

void MainWindow::openSettings() {
	SettingsWindow w(this);
	w.exec();
}

void MainWindow::initStatusBar() {
	this->statusBar = new QStatusBar();
	statusLabel = new QLabel("Ready");
	this->statusBar->addWidget(statusLabel);
	this->setStatusBar(this->statusBar);
}

void MainWindow::initPlotGroup() {
	channelPlotGroup = new QWidget(this);
	channelLayout = new QGridLayout();
	channelPlots.resize(NUM_CHANNELS);
	for (int c = 0; c < NUM_CHANNELS; c++) {
		QPair<int, int> pos = this->settings.getChannelView().at(c);
		channelPlots.at(c) = new ChannelPlot(c, pos);
		channelLayout->addWidget(channelPlots.at(c), pos.first, pos.second, 1, 1);
		//connect(channelPlots.at(c), SIGNAL(mouseDoubleClick(QMouseEvent *)),
				//this, SLOT(openSingleChannel()));
	}
	channelPlotGroup->setLayout(channelLayout);
	this->setCentralWidget(channelPlotGroup);

	/* X-values for all plots */
	QVector<double> tmpX(SAMPLE_RATE * (DISPLAY_REFRESH_INTERVAL / 1000));
	double start = 0.0;
	iota(tmpX.begin(), tmpX.end(), start);
	this->PLOT_X = tmpX;
}

void MainWindow::openNewRecording() {
	NewRecordingWindow *w = new NewRecordingWindow(this);
	int ret;

	while (true) {
		ret = w->exec();
		if (ret == QDialog::Rejected)
			return;
		if ((ret = w->validateChoices()) != 0)
			w->close();
		else
			break;
	}

	/* Validated file name */
	QString filename = w->getFullFilename();
	qDebug() << "File: " << filename << endl;

	/* Make a file */
	//this->recording = new LiveRecording(filename, w->getTime());
}

void MainWindow::loadRecording() {
	this->statusLabel->setText("Loading recording");
	QString filename = QFileDialog::getOpenFileName(
			this, tr("Load recording"),
			DEFAULT_SAVE_DIR, tr("Recordings (*.bin)"));
	if (filename.isNull())
		return;
	
	/* Open the recording */
	this->recording = new PlaybackRecording(filename);
	this->initPlaybackRecording();
	this->statusLabel->setText("Ready");
}

void MainWindow::initPlaybackRecording() {
	this->ctrlWindow->startPauseButton->setEnabled(true);
	this->ctrlWindow->timeLine->setText("0");
	this->ctrlWindow->timeLine->setReadOnly(false); // Until play back started
	connect(this->ctrlWindow->startPauseButton, SIGNAL(clicked()), this, SLOT(togglePlayback()));
	this->playbackTimer = new QTimer();
	this->playbackTimer->setInterval(this->settings.getRefreshInterval());
	//connect(this->playbackTimer, SIGNAL(timeout()), this, SLOT(plotNextDataBlock()));
	connect(this->playbackTimer, SIGNAL(timeout()), this, SLOT(plot()));
}

void MainWindow::setScale(int s) {
	this->settings.setDisplayScale(DISPLAY_SCALES[s]);
	this->scaleBox->setCurrentIndex(s);
}

void MainWindow::togglePlayback() {
	if (this->isPlaying) {
		this->playbackTimer->stop();
		this->ctrlWindow->startPauseButton->setText("Start");
		this->ctrlWindow->startPauseButton->setStyleSheet("QPushButton {color : black}");
	} else {
		this->playbackTimer->start();
		this->ctrlWindow->startPauseButton->setText("Stop");
		this->ctrlWindow->startPauseButton->setStyleSheet("QPushButton {color : rgb(178, 51, 51)}");
	}
	this->isPlaying = !this->isPlaying;
}

void MainWindow::plot() {
	QtConcurrent::run(this, &MainWindow::plotNextDataBlock);
}

void MainWindow::plotNextDataBlock() {
	//QFuture<QVector<QVector<int16_t> > > dataFuture = QtConcurrent::run(
			//this->recording, &PlaybackRecording::getNextDataBlock);
	QPen pen = this->settings.getPlotPen();
	double scale = this->settings.getDisplayScale();
	QVector<QVector<int16_t> > data = this->recording->getNextDataBlock();
	for (auto i = 0; i < NUM_CHANNELS; i++) {
		QVector<double> tmp(AIB_BLOCK_SIZE, 0);
		for (auto j = 0; j < AIB_BLOCK_SIZE; j++)
			tmp[j] = data.at(i).at(j);
		this->channelPlots.at(i)->graph(0)->setData(PLOT_X, tmp);
		if (this->ctrlWindow->autoscale) {
			this->channelPlots.at(i)->yAxis->rescale();
		} else {
			this->channelPlots.at(i)->yAxis->setRange(-(scale * NEG_DISPLAY_RANGE),
					(scale * POS_DISPLAY_RANGE));
		}
		this->channelPlots.at(i)->graph(0)->setPen(pen);
		this->channelPlots.at(i)->replot();
	}
}

void MainWindow::setAutoscale(int state) {
	bool set = state == Qt::Checked;
	autoscale = set;
	this->autoscaleCheckBox->setChecked(set);
	this->scaleBox->setEnabled(!set);
	this->settings.setAutoscale(set);
}

void MainWindow::toggleVisible() {
	this->setVisible(!this->isVisible());
}
