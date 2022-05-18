#include "mainwindow.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	srp[CanballModel] = new Canball();
	srp[NPlateModel] = new NPlate();
	srp[RayTraceCPUModel] = new RayTraceCPU();
	srp[AdvancedGPUModel] = new Render();

	model =  static_cast<SRPModel>(0);

	//Set initial properties from elements of the GUI.
	ui->cb_advance->hide();
	ui->label_InfoVis->hide();
	ui->label_InfoIcon->hide();
	ui->groupBox_4->hide();

	ui->gridGroupBox->setStyleSheet("border:none;");
	ui->pushButton->hide();
}

MainWindow::~MainWindow()
{
	delete ui;
	for(uint i=0; i< dataVisualizations.size();i++)
		if(dataVisualizations[i])
			delete dataVisualizations[i];
	for(uint i=0; i< glVisualizations.size();i++)
		if(glVisualizations[i])
			delete glVisualizations[i];
	for(uint i=0; i< comparisonVisualizations.size();i++)
		if(comparisonVisualizations[i])
			delete comparisonVisualizations[i];
}

void MainWindow::setVisualizations(std::vector<VisualizationWindow *>&vis)
{
	dataVisualizations = vis;
}

void MainWindow::on_bttn_loadOBJ_clicked()
{
	QString filename;
	QString path = QCoreApplication::applicationDirPath() + QString("/resources/models/");

	filename = QFileDialog::getOpenFileName(this,
											tr("Load OBJ"), path,
											tr(".obj files (*.obj)"));

	if (!filename.isNull()) {
		QString name = filename.mid(filename.lastIndexOf("/")+1);
		ui->text_OBJ->setText(name);

		nameObj = filename;
	}
}

void MainWindow::on_bttn_loadMAT_clicked()
{
	QString filename;

	QString path = QCoreApplication::applicationDirPath() + QString("/resources/models/");

	filename = QFileDialog::getOpenFileName(this,
											tr("Load MTL"), path,
											tr(".mtl files (*.mtl)"));

	if (!filename.isNull()) {
		QString name = filename.mid(filename.lastIndexOf("/")+1);
		ui->text_MAT->setText(name);

		nameMtl=filename;
	}
}


void MainWindow::bttn_loadSatelliteInfo_clicked()
{
	QString filename;

	filename = QFileDialog::getOpenFileName(this,
											tr("Load Satellite Information"), "./resources/models/",
											tr(".in files (*.in)"));

	if (!filename.isNull()) {
		QString name = filename.mid(filename.lastIndexOf("/")+1);
		//ui->text_satelliteInfo->setText(name);
		infoSatellite->setText(name);


		if (NPlate* nplate = dynamic_cast<NPlate*>(srp[NPlateModel]))
		{
			nplate->satelliteInfoFile = filename;
		}
	}
}

//Load the parameters that the used has set for the selected method.
void MainWindow::loadUserParameters(){

	char * nameObj, *nameMtl;
	std::string obj = this->nameObj.toStdString();
	std::string mat = this->nameMtl.toStdString();
	nameObj = (char*)obj.c_str();
	nameMtl = (char*)mat.c_str();

	if(obj.length()==0 && mat.length()==0){
		errorCode=-1;
		QMessageBox msgBox;
		msgBox.setText("The OBJ file AND the MAT file paths are missing.");
		msgBox.exec();
		return;
	}
	else if(obj.length()==0){
		errorCode=-1;
		QMessageBox msgBox;
		msgBox.setText("The OBJ file path is missing.");
		msgBox.exec();
		return;
	}
	else if(mat.length()==0){
		errorCode=-1;
		QMessageBox msgBox;
		msgBox.setText("The MAT file path is missing");
		msgBox.exec();
		return;
	}

	//Load the triangleMesh and materials from the files chosen.
	Object* satellite = new Object();
	int qq = satellite->loadOBJ(nameObj, nameMtl);
	if(qq == 1){

		errorCode=-1;
		QMessageBox msgBox;
		msgBox.setText("There was a problem loading the satellite information. Please check the OBJ and the MAT paths.");
		msgBox.exec();
		return;
	}
	srp[model]->setSatellite(satellite);

	//Load the initial properties of the selected method.

	int reflectiveType =Reflective;
	if(model == AdvancedGPUModel){
		if(ui->comboBox->currentIndex()==4){
			srp[model] = new RayTraceGPUTextures();
		}

		if(RayTraceGPU* ray=dynamic_cast<RayTraceGPU*>(srp[model])){
			int numRays = numSecondaryRays->value();
			ray->setNumSecondaryRays(numRays);

			int numDiffuse = numDiffuseRays->value();
			ray->setDiffuseRays(numDiffuse);
			ray->setReflectionType(reflectiveType);
		}

		srp[model]->setSatellite(satellite);
	}
	float az = ui->sb_az->value();
	float el = ui->sb_el->value();

	srp[model]->setAzimuthStep(az);
	srp[model]->setElevationStep(el);

	srp[model]->setMsat(ui->dsb_msat->value());

	if(model == CanballModel){
		Canball* c = dynamic_cast<Canball*>(srp[CanballModel]);
		c->ps =0.05;
		c->cr = cr->value();
		c->area = area->value();

	}
	else if(model == RayTraceCPUModel){
		RayTraceCPU * srpModel = dynamic_cast<RayTraceCPU*>(srp[model]);
		srpModel->cm = QVector3D();
		srpModel->nx =nx->value();
		srpModel->ny =ny->value();

		srpModel->setNumSecondaryRays(numSecondaryRays->value());

		srpModel->setNumDiffuseRays(numDiffuseRays->value());

		Reflectiveness r= Reflective;

		satellite->setReflectivenessInMaterials(r);
		satellite->setRefractiveIndexInMaterials(1);
	}
	else if(model == NPlateModel){
		NPlate * srpModel = dynamic_cast<NPlate*>(srp[model]);
		if(!srpModel->satelliteInfoFile.isEmpty())
			srpModel->loadSatelliteInfo();
		else if(srpModel->satelliteInfoFile.isEmpty()){
			errorCode=-1;
			QMessageBox msgBox;
			msgBox.setText("Load the information of the satellite (*.in) file.");
			msgBox.exec();
			return;
		}
	}
	errorCode=1;
}

//Creates a window with 4 3D viewers where the srp acceleration was computed
//considering the light coming from different points of a sphere.
void MainWindow::on_bttn_generateOutput_clicked()
{
	if(ui->comboBox->currentIndex()==0){
		QMessageBox msgBox;
		msgBox.setText("No method has been selected yet.");
		msgBox.exec();
		return;
	}

	loadUserParameters();

	if(errorCode<0)
		return;

	float az = ui->sb_az->value();
	float el = ui->sb_el->value();
	int NEL = 180/el+1;
	int NAZ = 360/az+1;
	Grid *results = new Grid(NAZ,NEL);

	AdvancedGPU* gpuMethod;
	GLWidget* glWidget;

	QWidget *window;

	if(gpuMethod=dynamic_cast<AdvancedGPU*>(srp[model]))
	{
		if(ui->cb_advance->currentIndex()==1){
			gpuMethod->showNothing();
		}else if(ui->cb_advance->currentIndex()==0){
			gpuMethod->showForces();
		}
	}
	QVBoxLayout * vLayout;

	if(!dynamic_cast<BasicSRP*>(srp[model])){
		window = new QWidget;
		vLayout = new QVBoxLayout;
	}
	if(gpuMethod=dynamic_cast<AdvancedGPU*>(srp[model])){
		QGLFormat glfModern=QGLFormat::defaultFormat();
		glfModern.setVersion(3,3);
		glfModern.setProfile(QGLFormat::CoreProfile);
		glfModern.setSampleBuffers(true);
		glfModern.setSwapInterval(0);
		glfModern.setDefaultFormat(glfModern);

		glWidget= new GLWidget(glfModern);
		glWidget->setGeometry(QRect(0, 0, 512, 512));
		glWidget->setMaximumSize(512, 512);
		glWidget->setMinimumSize(512, 512);

		vLayout->addWidget(glWidget);
	}
	if(!dynamic_cast<BasicSRP*>(srp[model])){
		QProgressBar* progressBar = new QProgressBar();
		progressBar->setMinimum(0);
		progressBar->setValue(0);
		progressBar->setMaximum(NEL*NAZ);

		srp[model]->setProgressBar(progressBar);
		vLayout->addWidget(progressBar);

		QPushButton *stop = new QPushButton;
		stop->setText("Stop");
		srp[model]->setStopExecution(false);
		QObject::connect(stop, &QPushButton::clicked, [this]() {
					srp[model]->setStopExecution(true);
				});

		vLayout->addWidget(stop);

		window->setLayout(vLayout);

		if(gpuMethod=dynamic_cast<AdvancedGPU*>(srp[model])){
			glWidget->setRayTraceGPU(gpuMethod);
			glWidget->setSatellite(gpuMethod->getSatellite());

			glWidget->setShowSatellite(false);
		}
			//glWidget->show();
		window->show();
	}

	if(gpuMethod=dynamic_cast<AdvancedGPU*>(srp[model])){

		glWidget->initializeBuffers();

		glWidget->updateGL();
	}

	srp[model]->computeSRP(results);

	if(!dynamic_cast<BasicSRP*>(srp[model]))
		window->close();

	if(srp[model]->getStopExecution()) return;

	QString nameResult = getNameFromModel(model);

	int idx = fieldResults[model].addResult(nameResult,results);

	QString offset;
	if(idx>0){
		offset= QString(" ")+ QString::number(idx);
	}
	QString particularNameResult = nameResult +offset;

	VisualizationWindow* visualization = new VisualizationWindow();
	visualization->setELstep(el);
	visualization->setAZstep(az);

	visualization->plotResults(results, particularNameResult);
	dataVisualizations.push_back(visualization);
}

//Load a window to visualize the satellite with particular SRP forces.
void MainWindow::on_bttn_visualizeSatellite_clicked()
{
	if(ui->comboBox->currentIndex()==0){
		QMessageBox msgBox;
		msgBox.setText("No method has been selected yet.");
		msgBox.exec();
		return;
	}

	loadUserParameters();

	if(errorCode<0)
		return;

	GLWindow * glVisualization = new GLWindow();
	glVisualization->setModel(srp[model]);
	glVisualization->setSatellite(srp[model]->getSatellite());
	glVisualization->show();
	glVisualization->drawSatellite();
	glVisualizations.push_back(glVisualization);
}

//Obtain the name of the model.
QString MainWindow::getNameFromModel(SRPModel model)
{
	QString name;

	if(model == CanballModel)
		name = QStringLiteral("Cannon-Ball");
	else if(model == NPlateModel)
		name = QStringLiteral("NPlate");
	else if(model == RayTraceCPUModel)
		name = QStringLiteral("RayTrace - CPU");
	else if(model == AdvancedGPUModel)
	{
		if(dynamic_cast<RayTraceGPUTextures*>(srp[model]))
			name = QStringLiteral("RayTrace - GPU");
	}

	return name;
}

void MainWindow::on_actionCompare_computed_results_triggered()
{
	ComparisonWindow* comparisonWindow = new ComparisonWindow();
	comparisonWindow->setResults(fieldResults, MAX_SRP_MODELS);

	comparisonVisualizations.push_back(comparisonWindow);
}

void MainWindow::on_pushButton_clicked()
{
	if(!ui->cb_advance->isHidden()){
		ui->cb_advance->hide();
		ui->label_InfoVis->hide();
		ui->label_InfoIcon->hide();
		ui->groupBox_4->hide();
	}
	else{
		ui->cb_advance->show();
		ui->label_InfoVis->show();
		ui->label_InfoIcon->show();
		ui->groupBox_4->show();
	}
}

void MainWindow::clearLayout(QLayout* layout, bool deleteWidgets)
{
	while (QLayoutItem* item = layout->takeAt(0))
	{
		if (deleteWidgets)
		{
			if (QWidget* widget = item->widget())
				widget->deleteLater();
		}
		if (QLayout* childLayout = item->layout())
			clearLayout(childLayout, deleteWidgets);
		delete item;
	}
}

void MainWindow::on_comboBox_highlighted(const QString &arg1)
{
}

void MainWindow::on_comboBox_currentIndexChanged(const QString &arg1)
{
	showCPUMethodInformation(arg1);
}

void MainWindow::hideAdvanceSettings(){
	ui->pushButton->hide();
	ui->cb_advance->hide();
	ui->label_InfoVis->hide();
	ui->label_InfoIcon->hide();
	ui->groupBox_4->hide();
	ui->cb_advance->setCurrentIndex(0);
}

//Load dynamic information on the main window.
void MainWindow::showCPUMethodInformation(const QString &arg1){

	QFont font1;
	font1.setBold(false);
	font1.setWeight(50);

	QFont font2;
	font2.setBold(true);
    font2.setWeight(99);

	int numFiability=0;
	int numPerformance=0;

	clearLayout(ui->gridGroupBox->layout());
	clearLayout(ui->gridWidget2->layout());

	ui->gridGroupBox->setStyleSheet("border:none;");

	QGridLayout * gridLayout =dynamic_cast<QGridLayout*>(ui->gridGroupBox->layout());
	QGridLayout * gridLayout2 =dynamic_cast<QGridLayout*>(ui->gridWidget2->layout());

	if(arg1.compare(QString("None"))==0) return;

	QString title = arg1;
	QString textInfo;

	QGridLayout *extraInfoGrid = new QGridLayout();

	QLabel *lblIcon = new QLabel();
	lblIcon->setGeometry(QRect(550, 40, 20, 20));
	QSizePolicy sizePolicy3(QSizePolicy::Ignored, QSizePolicy::Ignored);
	sizePolicy3.setHorizontalStretch(0);
	sizePolicy3.setVerticalStretch(0);
	sizePolicy3.setHeightForWidth(lblIcon->sizePolicy().hasHeightForWidth());

	if(arg1.compare(QString("Cannon-Ball (CPU)"))==0){
		hideAdvanceSettings();

		numFiability=1;
		numPerformance=6;
		textInfo = QString("This method implements a basic approximation of the computation of SRP forces. In particular, the satellite's shape is approximated by a sphere.");

		model = SRPModel::CanballModel;

		QLabel * labelCr = new QLabel;
		labelCr->setFont(font1);
		labelCr->setObjectName(QStringLiteral("labelCr"));
		labelCr->setGeometry(QRect(0, 0, 31, 17));
		labelCr->setText(QString("Cr"));
		labelCr->setContentsMargins(-20,50,00,0);
		labelCr->setMaximumWidth(40);
		extraInfoGrid->addWidget(labelCr,0,0);

		QLabel *labelIcon = new QLabel();
		labelIcon->setGeometry(QRect(0, 0, 20, 20));
		labelIcon->setSizePolicy(sizePolicy3);
		labelIcon->setFont(font1);
		labelIcon->setMaximumWidth(20);
		labelIcon->setMaximumHeight(20);
		labelIcon->setPixmap(QPixmap(QString::fromUtf8(":/resources/images/infoIcon3.png")));
		labelIcon->setScaledContents(true);
		labelIcon->setToolTip(QApplication::translate("MainWindow", "This is the reflectivity coefficient", nullptr));
		extraInfoGrid->addWidget(labelIcon,0,1);

		QSpacerItem *item = new QSpacerItem(0,30);
		extraInfoGrid->addItem(item,0,2);

		cr = new QDoubleSpinBox;
		cr->setValue(1.0f);
		cr->setMinimum(1);
		cr->setMaximum(2);
		cr->setSingleStep(0.01f);
		cr->setObjectName(QStringLiteral("dsbCr"));
		cr->setMaximumWidth(80);
		cr->setMinimumHeight(30);
		extraInfoGrid->addWidget(cr,0,3);

		QLabel * labelArea = new QLabel;
		labelArea->setFont(font1);
		labelArea->setObjectName(QStringLiteral("labelArea"));
		labelArea->setGeometry(QRect(0, 0, 31, 17));
		labelArea->setText(QString("Area"));
		labelArea->setContentsMargins(-20,30,00,0);
		labelArea->setMaximumWidth(40);
		extraInfoGrid->addWidget(labelArea,2,0);

		QLabel *labelIcon2 = new QLabel();
		labelIcon2->setGeometry(QRect(0, 0, 20, 20));
		labelIcon2->setSizePolicy(sizePolicy3);
		labelIcon2->setFont(font1);
		labelIcon2->setMaximumWidth(20);
		labelIcon2->setMaximumHeight(20);
		labelIcon2->setPixmap(QPixmap(QString::fromUtf8(":/resources/images/infoIcon3.png")));
		labelIcon2->setScaledContents(true);
		labelIcon2->setToolTip(QApplication::translate("MainWindow", "The area of the satellite", nullptr));
		extraInfoGrid->addWidget(labelIcon2,2,1);

		QSpacerItem *item2 = new QSpacerItem(0,30);
		extraInfoGrid->addItem(item2,2,2);

		area = new QDoubleSpinBox;
		area->setValue(1.0f);
		area->setMinimum(0);
		area->setMaximum(99.99f);
		area->setSingleStep(0.01f);
		area->setMaximumWidth(80);
		area->setObjectName(QStringLiteral("dsbArea"));
		area->setSuffix(" m²");
		area->setMinimumHeight(30);
		extraInfoGrid->addWidget(area,2,3);

		gridLayout2->addLayout(extraInfoGrid,4,0,6,2);

	}
	else if(arg1.compare(QString("NPlate (CPU)"))==0){
		hideAdvanceSettings();

		numFiability=2;
		numPerformance=6;
		textInfo = QString("This method implements an approximation of the computation of SRP forces. The satellite’s shape is approximated by a set of flat plates, each one with different reflectivity properties.");

		model = SRPModel::NPlateModel;

		QLabel * labelNPlate = new QLabel;
		labelNPlate->setFont(font1);
		labelNPlate->setObjectName(QStringLiteral("labelNPlate"));
		labelNPlate->setGeometry(QRect(0, 0, 31, 17));
		labelNPlate->setText(QString("Satellite Information file (*.in)"));
		labelNPlate->setMaximumWidth(210);
		labelNPlate->setMinimumWidth(210);
		//labelNPlate->setContentsMargins(-20,30,10,0);
		extraInfoGrid->addWidget(labelNPlate,0,0);

		infoSatellite = new QLineEdit();
		infoSatellite->setObjectName(QStringLiteral("text_infoSatellite"));
		infoSatellite->setGeometry(QRect(160, 0, 150, 17));
		extraInfoGrid->addWidget(infoSatellite,0,1);

		QPushButton * button = new QPushButton;
		button->setText(QStringLiteral("Select file"));
		extraInfoGrid->addWidget(button,0,2);

		connect(button,SIGNAL(clicked(bool)),this,SLOT(bttn_loadSatelliteInfo_clicked()));

		gridLayout2->addLayout(extraInfoGrid,4,0,6,3);

	}
	else if(arg1.compare(QString("RayTrace (CPU) - Multiple Scattering"))==0){

		hideAdvanceSettings();

		numFiability=6;
		numPerformance=1;

		textInfo = QString("This method cast several rays from a grid (size: Nx x Ny). Then, for each ray that intersects with a face of the satellite the SRP force is computed and (iteratively) another ray with reflected and diffuse directions are casted.");

		model = SRPModel::RayTraceCPUModel;

		QLabel * labelNx = new QLabel;
		labelNx->setFont(font1);
		labelNx->setObjectName(QStringLiteral("labelNx"));
		labelNx->setGeometry(QRect(0, 0, 31, 17));
		labelNx->setText(QString("Nx"));
		labelNx->setContentsMargins(-20,30,00,0);
		labelNx->setMaximumWidth(20);
		extraInfoGrid->addWidget(labelNx,0,0);

		QLabel *labelIcon = new QLabel();
		labelIcon->setGeometry(QRect(0, 0, 20, 20));
		labelIcon->setSizePolicy(sizePolicy3);
		labelIcon->setFont(font1);
		labelIcon->setMaximumWidth(20);
		labelIcon->setMaximumHeight(20);
		labelIcon->setPixmap(QPixmap(QString::fromUtf8(":/resources/images/infoIcon3.png")));
		labelIcon->setScaledContents(true);
		labelIcon->setToolTip(QApplication::translate("MainWindow", "Nx is the size of the grid (horizontally)", nullptr));
		extraInfoGrid->addWidget(labelIcon,0,1);

		QSpacerItem *item = new QSpacerItem(0,30);
		extraInfoGrid->addItem(item,0,2);

		nx = new QSpinBox;
		nx->setValue(50);
		nx->setMinimum(1);
		nx->setMaximum(999);
		nx->setSingleStep(1);
		nx->setObjectName(QStringLiteral("sbNx"));
		nx->setMaximumWidth(70);
		nx->setMinimumHeight(30);
		extraInfoGrid->addWidget(nx,0,3);

		QSpacerItem *item5 = new QSpacerItem(30,0, QSizePolicy::Fixed,QSizePolicy::Fixed);
		extraInfoGrid->addItem(item5,0,4);

		QLabel * labelNumRays = new QLabel;
		labelNumRays->setFont(font1);
		labelNumRays->setObjectName(QStringLiteral("labelNumRays"));
		labelNumRays->setGeometry(QRect(0, 0, 31, 17));
		labelNumRays->setText(QString("Secondary Rays"));
		labelNumRays->setContentsMargins(20,30,00,0);
		labelNumRays->setMaximumWidth(120);
		extraInfoGrid->addWidget(labelNumRays,0,5);

		QLabel *labelIcon3 = new QLabel();
		labelIcon3->setGeometry(QRect(0, 0, 20, 20));
		labelIcon3->setSizePolicy(sizePolicy3);
		labelIcon3->setFont(font1);
		labelIcon3->setMaximumWidth(20);
		labelIcon3->setMaximumHeight(20);
		labelIcon3->setPixmap(QPixmap(QString::fromUtf8(":/resources/images/infoIcon3.png")));
		labelIcon3->setScaledContents(true);
		labelIcon3->setToolTip(QApplication::translate("MainWindow", "This is the maxim number of secondary rays that will be casted (iteratively) when one of the initial rays intersect with a face of the satellite.", nullptr));
		extraInfoGrid->addWidget(labelIcon3,0,6);

		QSpacerItem *item3 = new QSpacerItem(0,30);
		extraInfoGrid->addItem(item3,0,7);

		numSecondaryRays = new QSpinBox;
		numSecondaryRays->setValue(2);
		numSecondaryRays->setMinimum(0);
		numSecondaryRays->setMaximum(20);
		numSecondaryRays->setSingleStep(1);
		numSecondaryRays->setObjectName(QStringLiteral("numSecondaryRays"));
		numSecondaryRays->setMaximumWidth(50);
		numSecondaryRays->setMinimumHeight(30);
		extraInfoGrid->addWidget(numSecondaryRays,0,8);

		QLabel * labelNy = new QLabel;
		labelNy->setFont(font1);
		labelNy->setObjectName(QStringLiteral("labelNy"));
		labelNy->setGeometry(QRect(0, 0, 31, 17));
		labelNy->setText(QString("Ny"));
		labelNy->setContentsMargins(-20,30,00,0);
		labelNy->setMaximumWidth(20);
		extraInfoGrid->addWidget(labelNy,2,0);

		QLabel *labelIcon2 = new QLabel();
		labelIcon2->setGeometry(QRect(0, 0, 20, 20));
		labelIcon2->setSizePolicy(sizePolicy3);
		labelIcon2->setFont(font1);
		labelIcon2->setMaximumWidth(20);
		labelIcon2->setMaximumHeight(20);
		labelIcon2->setPixmap(QPixmap(QString::fromUtf8(":/resources/images/infoIcon3.png")));
		labelIcon2->setScaledContents(true);
		labelIcon2->setToolTip(QApplication::translate("MainWindow", "Ny is the size of the grid (vertically)", nullptr));
		extraInfoGrid->addWidget(labelIcon2,2,1);

		QSpacerItem *item2 = new QSpacerItem(0,30);
		extraInfoGrid->addItem(item2,2,2);

		ny = new QSpinBox;
		ny->setValue(50);
		ny->setMinimum(1);
		ny->setMaximum(999);
		ny->setSingleStep(1);
		ny->setMaximumWidth(70);
		ny->setObjectName(QStringLiteral("sbNy"));
		ny->setMinimumHeight(30);
		extraInfoGrid->addWidget(ny,2,3);

		QSpacerItem *item6 = new QSpacerItem(30,0, QSizePolicy::Fixed,QSizePolicy::Fixed);
		extraInfoGrid->addItem(item6,2,4);

		QLabel * labelNumDiffuseRays = new QLabel;
		labelNumDiffuseRays->setFont(font1);
		labelNumDiffuseRays->setObjectName(QStringLiteral("labelDiffuseRays"));
		labelNumDiffuseRays->setGeometry(QRect(0, 0, 31, 17));
		labelNumDiffuseRays->setText(QString("Diffuse Rays"));
		labelNumDiffuseRays->setContentsMargins(20,30,00,0);
		labelNumDiffuseRays->setMaximumWidth(120);
		extraInfoGrid->addWidget(labelNumDiffuseRays,2,5);

		QLabel *labelIcon7 = new QLabel();
		labelIcon7->setGeometry(QRect(0, 0, 20, 20));
		labelIcon7->setSizePolicy(sizePolicy3);
		labelIcon7->setFont(font1);
		labelIcon7->setMaximumWidth(20);
		labelIcon7->setMaximumHeight(20);
		labelIcon7->setPixmap(QPixmap(QString::fromUtf8(":/resources/images/infoIcon3.png")));
		labelIcon7->setScaledContents(true);
		labelIcon7->setToolTip(QApplication::translate("MainWindow", "This is the number of diffuse rays that will be casted for each ray that has intersected with a face of the satellite.", nullptr));
		extraInfoGrid->addWidget(labelIcon7,2,6);

		QSpacerItem *item8 = new QSpacerItem(10,00);
		extraInfoGrid->addItem(item8,2,7);

		numDiffuseRays = new QSpinBox;
		numDiffuseRays->setValue(2);
		numDiffuseRays->setMinimum(0);
		numDiffuseRays->setMaximum(20);
		numDiffuseRays->setSingleStep(1);
		numDiffuseRays->setObjectName(QStringLiteral("numDiffuseRays"));
		numDiffuseRays->setMaximumWidth(50);
		numDiffuseRays->setMinimumHeight(30);
		extraInfoGrid->addWidget(numDiffuseRays,2,8);

		gridLayout2->addLayout(extraInfoGrid,4,0,6,4);
	}
	else if(arg1.compare(QString("RayTrace (GPU) - Multiple Scattering"))==0){
		numFiability=6;
		numPerformance=3;

		textInfo = QString("This method takes the satellite's faces to the GPU and then, for each pixel, it is casted a ray that if it intersects with a face of the satellite, the SRP force will be computed. Additionally, each time a ray intersects a face, other rays with reflected and diffuse directions will be casted as well.");

		ui->pushButton->show();
		model = SRPModel::AdvancedGPUModel;

		QLabel * labelNumRays = new QLabel;
		labelNumRays->setFont(font1);
		labelNumRays->setObjectName(QStringLiteral("labelNumRays"));
		labelNumRays->setGeometry(QRect(0, 0, 31, 17));
		labelNumRays->setText(QString("Secondary Rays"));
		labelNumRays->setContentsMargins(20,50,00,0);
		labelNumRays->setMaximumWidth(100);
		extraInfoGrid->addWidget(labelNumRays,0,0);

		QLabel *labelIcon3 = new QLabel();
		//label_61->setObjectName(QStringLiteral("label_61"));
		labelIcon3->setGeometry(QRect(0, 0, 20, 20));
		labelIcon3->setSizePolicy(sizePolicy3);
		labelIcon3->setFont(font1);
		labelIcon3->setMaximumWidth(20);
		labelIcon3->setMaximumHeight(20);
		labelIcon3->setPixmap(QPixmap(QString::fromUtf8(":/resources/images/infoIcon3.png")));
		labelIcon3->setScaledContents(true);
		labelIcon3->setToolTip(QApplication::translate("MainWindow", "This is the maxim number of secondary rays that will be casted (iteratively) when one of the initial rays intersect with a face of the satellite.", nullptr));
		extraInfoGrid->addWidget(labelIcon3,0,1);

		QSpacerItem *item3 = new QSpacerItem(10,0);
		extraInfoGrid->addItem(item3,0,2);

		numSecondaryRays = new QSpinBox;
		numSecondaryRays->setValue(2);
		numSecondaryRays->setMinimum(0);
		numSecondaryRays->setMaximum(20);
		numSecondaryRays->setSingleStep(1);
		numSecondaryRays->setObjectName(QStringLiteral("numSecondaryRays"));
		numSecondaryRays->setMaximumWidth(50);
		numSecondaryRays->setMinimumHeight(30);
		extraInfoGrid->addWidget(numSecondaryRays,0,3);

		QLabel * labelNumDiffuseRays = new QLabel;
		labelNumDiffuseRays->setFont(font1);
		labelNumDiffuseRays->setObjectName(QStringLiteral("labelDiffuseRays"));
		labelNumDiffuseRays->setGeometry(QRect(0, 0, 31, 17));
		labelNumDiffuseRays->setText(QString("Diffuse Rays"));
		labelNumDiffuseRays->setContentsMargins(20,30,00,0);
		labelNumDiffuseRays->setMaximumWidth(100);
		extraInfoGrid->addWidget(labelNumDiffuseRays,1,0);

		QLabel *labelIcon7 = new QLabel();
		//label_61->setObjectName(QStringLiteral("label_61"));
		labelIcon7->setGeometry(QRect(0, 0, 20, 20));
		labelIcon7->setSizePolicy(sizePolicy3);
		labelIcon7->setFont(font1);
		labelIcon7->setMaximumWidth(20);
		labelIcon7->setMaximumHeight(20);
		labelIcon7->setPixmap(QPixmap(QString::fromUtf8(":/resources/images/infoIcon3.png")));
		labelIcon7->setScaledContents(true);
		labelIcon7->setToolTip(QApplication::translate("MainWindow", "This is the number of diffuse rays that will be casted for each ray that has intersected with a face of the satellite.", nullptr));
		extraInfoGrid->addWidget(labelIcon7,1,1);

		QSpacerItem *item8 = new QSpacerItem(10,00);
		extraInfoGrid->addItem(item8,1,2);

		numDiffuseRays = new QSpinBox;
		numDiffuseRays->setValue(2);
		numDiffuseRays->setMinimum(0);
		numDiffuseRays->setMaximum(20);
		numDiffuseRays->setSingleStep(1);
		numDiffuseRays->setObjectName(QStringLiteral("numDiffuseRays"));
		numDiffuseRays->setMaximumWidth(50);
		numDiffuseRays->setMinimumHeight(30);
		extraInfoGrid->addWidget(numDiffuseRays,1,3);

		gridLayout2->addLayout(extraInfoGrid,4,0,6,2);
	}

	ui->gridGroupBox->setStyleSheet("border:1px solid grey; margin-left:3px;");

	QLabel * fiability = new QLabel;
	fiability->setFont(font1);
	fiability->setObjectName(QStringLiteral("labelFiability"));
	fiability->setGeometry(QRect(0, 0, 31, 17));
	fiability->setText(QString("Accuracy:"));
	fiability->setStyleSheet("border:none;margin-left:0px; ");
	gridLayout->addWidget(fiability,1,0,1,1);

	for(int i=0; i< numFiability; i++){
		QLabel *fiabilityIcon = new QLabel();
		QString s = QStringLiteral("fiabilityIcon");
		s+= QString::number(i);
		fiabilityIcon->setObjectName(s);
		fiabilityIcon->setGeometry(QRect(0, 0, 40, 20));
		QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
		sizePolicy3.setHorizontalStretch(0);
		sizePolicy3.setVerticalStretch(0);
		sizePolicy3.setHeightForWidth(fiabilityIcon->sizePolicy().hasHeightForWidth());
		fiabilityIcon->setSizePolicy(sizePolicy3);
		fiabilityIcon->setMaximumWidth(20);
		fiabilityIcon->setFont(font1);
		fiabilityIcon->setStyleSheet("border:1px solid black; margin-left:0px;");
		fiabilityIcon->setPixmap(QPixmap(QString::fromUtf8(":/resources/images/green.png")));
		fiabilityIcon->setScaledContents(true);
		gridLayout->addWidget(fiabilityIcon,1,i+3);
	}
	for(int i=numFiability; i<=5; i++){
		QLabel *fiabilityIcon = new QLabel();
		QString s = QStringLiteral("fiabilityIcon2");
		s+= QString::number(i);
		fiabilityIcon->setObjectName(s);
		fiabilityIcon->setGeometry(QRect(0, 0, 40, 20));
		QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
		sizePolicy3.setHorizontalStretch(0);
		sizePolicy3.setVerticalStretch(0);
		sizePolicy3.setHeightForWidth(fiabilityIcon->sizePolicy().hasHeightForWidth());
		fiabilityIcon->setSizePolicy(sizePolicy3);
		fiabilityIcon->setMaximumWidth(20);
		fiabilityIcon->setStyleSheet("border:1px solid black;margin-left:0px;");
		fiabilityIcon->setFont(font1);
		fiabilityIcon->setPixmap(QPixmap(QString::fromUtf8(":/resources/images/white.png")));
		fiabilityIcon->setScaledContents(true);
		gridLayout->addWidget(fiabilityIcon,1,i+3);
	}
	QSpacerItem * item4 = new QSpacerItem(40, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);

	gridLayout->addItem(item4,1,5+3);

	QLabel * performance = new QLabel;
	performance->setFont(font1);
	performance->setObjectName(QStringLiteral("labelPerformance"));
	performance->setGeometry(QRect(160, 0, 31, 17));
	performance->setText(QString("Performance:"));
	performance->setContentsMargins(0,30,0,0);
	performance->setStyleSheet("border:none;margin-left:0px;");
	gridLayout->addWidget(performance,2,0,2,1);

	QHBoxLayout *hLayout2 = new QHBoxLayout();

	for(int i=0; i< numPerformance; i++){
		QLabel *fiabilityIcon = new QLabel();
		QString s = QStringLiteral("performanceIcon");
		s+= QString::number(i);
		fiabilityIcon->setObjectName(s);
		fiabilityIcon->setGeometry(QRect(0, 0, 40, 20));
		QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
		sizePolicy3.setHorizontalStretch(0);
		sizePolicy3.setVerticalStretch(0);
		sizePolicy3.setHeightForWidth(fiabilityIcon->sizePolicy().hasHeightForWidth());
		fiabilityIcon->setSizePolicy(sizePolicy3);
		fiabilityIcon->setMaximumWidth(20);
		fiabilityIcon->setStyleSheet("border:1px solid black;margin-left:0px;");
		fiabilityIcon->setFont(font1);
		fiabilityIcon->setPixmap(QPixmap(QString::fromUtf8(":/resources/images/green.png")));
		fiabilityIcon->setScaledContents(true);
		gridLayout->addWidget(fiabilityIcon,2,i+3);
	}
	for(int i=numPerformance; i<=5; i++){
		QLabel *fiabilityIcon = new QLabel();
		QString s = QStringLiteral("performanceIcon2");
		s+= QString::number(i);
		fiabilityIcon->setObjectName(s);
		fiabilityIcon->setGeometry(QRect(0, 0, 40, 20));
		QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
		sizePolicy3.setHorizontalStretch(0);
		sizePolicy3.setVerticalStretch(0);
		sizePolicy3.setHeightForWidth(fiabilityIcon->sizePolicy().hasHeightForWidth());
		fiabilityIcon->setSizePolicy(sizePolicy3);
		fiabilityIcon->setMaximumWidth(20);
		fiabilityIcon->setStyleSheet("border:1px solid black;margin-left:0px;");
		fiabilityIcon->setFont(font1);
		fiabilityIcon->setPixmap(QPixmap(QString::fromUtf8(":/resources/images/white.png")));
		fiabilityIcon->setScaledContents(true);
		gridLayout->addWidget(fiabilityIcon,2,i+3);
	}
	QSpacerItem * item3 = new QSpacerItem(40, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);

	gridLayout->addItem(item3,2,5+3);

	QLabel * qTitle = new QLabel;
	qTitle->setFont(font2);
	qTitle->setObjectName(QStringLiteral("qTitle"));
	qTitle->setGeometry(QRect(160, 0, 31, 17));
	qTitle->setText(title);
	qTitle->setContentsMargins(0,30,0,0);
	gridLayout2->addWidget(qTitle,0,0);

	QSpacerItem * itemTitle = new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);

	gridLayout->addItem(itemTitle,1,0);

	QLabel * qDescription = new QLabel;
	qDescription->setFont(font1);
	qDescription->setObjectName(QStringLiteral("qDescription"));
	qDescription->setGeometry(QRect(160, 0, 51, 57));
	qDescription->setText(textInfo);
	qDescription->setContentsMargins(0,0,0,10);
	qDescription->setWordWrap(true);
	qDescription->setMinimumHeight(40);
	gridLayout2->addWidget(qDescription,2,0,3,7);

	QSpacerItem *item2 = new QSpacerItem(50,50);
	gridLayout2->addItem(item2,3,0);
}

void MainWindow::on_pushButton_2_clicked(bool checked)
{
	on_actionCompare_computed_results_triggered();
}
