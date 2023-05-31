#include "comparisonwindow.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include<iostream>
#include<fstream>
#include <sstream>
#include<iomanip>

void ComparisonWindow::setResults(CategoryResult *value, int maxSRPModels)
{
	results = value;

	QStringList list;
	int index=0;

	for(int i=0; i<maxSRPModels; i++){
		std::vector<Result> particularResults = results[i].getResults();
		std::unordered_map<size_t,int> repetitions;
		std::hash<std::string> hash_fn;
		for(uint j=0; j< particularResults.size();j++){

			Result r = particularResults[j];
			QString num;
			size_t hashValue =hash_fn(r.getName().toStdString());
			int numAux;
			if(repetitions.find(hashValue) != repetitions.end()){
				numAux = repetitions[hashValue]+1;
				num = QString::number(numAux);
				repetitions[hashValue]+=1;
			}else{
				numAux=1;
				num = QString("");
				repetitions[hashValue]=1;
			}
			QString name = r.getName() + QString(" ") + num;
			list.append(name);

			map[index]=QVector3D(i,j,numAux);
			index++;
		}
	}

	leftBox->addItems(list);
	rightBox->addItems(list);

	if(list.size()==0)
		compareButton->setEnabled(false);

	this->show();
}

void ComparisonWindow::computeDifference()
{
	int idxModel1 = leftBox->currentIndex();
	int idxModel2 = rightBox->currentIndex();

	QVector3D idxAuxModel1 = map[idxModel1];
	QVector3D idxAuxModel2 = map[idxModel2];

	Result r= results[(int)idxAuxModel1.x()].getResults()[(int)idxAuxModel1.y()];
	Result r2= results[(int)idxAuxModel2.x()].getResults()[(int)idxAuxModel2.y()];

	Grid *normGrid1 = r.getOutput();
	Grid *normGrid2 = r2.getOutput();

	QVector2D size1 = normGrid1->getSizes();
	QVector2D size2 = normGrid2->getSizes();

	errorComparison->setStyleSheet("");

	if(fabs(size1.x()-size2.x())>0.1 && fabs(size1.y()-size2.y())>0.1){
		errorComparison->setStyleSheet("color: red;");
		errorComparison->setText("Comparison Error: Azimuth steps AND Elevation steps are different between the two Outputs.");
		return;
	}
	else if(fabs(size1.x()-size2.x())>0.1){
		errorComparison->setStyleSheet("color: red;");
		errorComparison->setText("Comparison Error: Azimuth steps are different between the two Outputs.");
		return;
	}
	else if(fabs(size1.y()-size2.y())>0.1){
		errorComparison->setStyleSheet("color: red;");
		errorComparison->setText("Comparison Error: Elevation steps are different between the two Outputs.");
		return;
	}
	Grid *diffGrid = new Grid((int)size1.x(),(int)size2.y());
	long double MSE =0;
	double maxError = -1;
	for (int i=0; i< (int)size1.x(); i++){
		for(int j=0; j <(int) size1.y(); j++){
			Output o1 = (*normGrid1)(i,j);
			Output o2 = (*normGrid2)(i,j);
			QVector3D forces1 = o1.getForces();
			QVector3D forces2 = o2.getForces();
			QVector3D f = forces1-forces2;
			MSE+= f.length();
			if(f.length()>maxError){
				maxError=f.length();
			}

			Output o(o1.getAzimuth(),o1.getElevation(),f.x(),f.y(),f.z());
			(*diffGrid)(i,j) = o;
		}
	}

	std::stringstream stream;
	stream << std::scientific << std::setprecision(4) << maxError;
	std::string maxErrorText = "The largest difference between both selected models is: "+stream.str();
	errorComparison->setText(QString(maxErrorText.data()));

	std::stringstream stream2;
	stream2 << std::scientific << std::setprecision(4) << MSE / (size1.x() * size1.y());
	std::string MSEText = "The MSE is: "+stream2.str();
	errorComparisonExtraInfo->setText(MSEText.data());

	QString nameModel1 = r.getName();
	nameModel1+= idxAuxModel1.z()>1?QString(" ")+QString::number((int)idxAuxModel1.z()):QString("");
	QString nameModel2 = r2.getName();
	nameModel2+= idxAuxModel2.z()>1?QString(" ")+QString::number((int)idxAuxModel2.z()):QString("");

	QString name = QString("Difference between ")+ nameModel1 + QString(" and ") +nameModel2;
	VisualizationWindow *v = new VisualizationWindow();
	v->plotResults(diffGrid,name);

	visualizationWindows.push_back(v);
}

ComparisonWindow::ComparisonWindow(QWidget *parent):QWidget(parent)
{
	this->setWindowTitle(QString("Comparison Window"));
	this->setGeometry(0,0,64,64);

	QVBoxLayout *vLayout = new QVBoxLayout(this);

	QFont font1;
	font1.setBold(false);
	font1.setWeight(50);

	QLabel * initText = new QLabel;
	initText->setFont(font1);
	initText->setObjectName(QStringLiteral("labelInitTextComparison"));
	initText->setGeometry(QRect(160, 0, 31, 17));
	initText->setText(QString("Select the two output to compare:"));
	initText->setContentsMargins(0,30,0,0);
	vLayout->addWidget(initText);

	QHBoxLayout *gLayout = new QHBoxLayout();
	vLayout->addLayout(gLayout);

	leftBox = new QComboBox;
	gLayout->addWidget(leftBox);

	rightBox = new QComboBox;
	gLayout->addWidget(rightBox);

	QSpacerItem *item2 = new QSpacerItem(30,30,QSizePolicy::Fixed,QSizePolicy::Fixed);
	vLayout->addItem(item2);

	compareButton = new QToolButton;
	compareButton->setIcon(QIcon("://resources/images/balanceIcon.png"));
	compareButton->setIconSize(QSize(146,105));
	compareButton->setMaximumWidth(105);
	QHBoxLayout * hLayout = new QHBoxLayout;
	hLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop );
	hLayout->addWidget(compareButton);
	vLayout->addLayout(hLayout);

	QSpacerItem *item = new QSpacerItem(30,30,QSizePolicy::Fixed,QSizePolicy::Fixed);
	vLayout->addItem(item);

	errorComparison = new QLabel;
	errorComparison->setFont(font1);
	errorComparison->setObjectName(QStringLiteral("labelInfoErrorTextComparison"));
	errorComparison->setGeometry(QRect(160, 0, 31, 17));
	errorComparison->setText(QString(""));
	errorComparison->setContentsMargins(0,30,0,0);
	vLayout->addWidget(errorComparison);

	errorComparisonExtraInfo = new QLabel;
	errorComparisonExtraInfo->setFont(font1);
	errorComparisonExtraInfo->setObjectName(QStringLiteral("labelInfoErrorTextComparison"));
	errorComparisonExtraInfo->setGeometry(QRect(160, 0, 31, 17));
	errorComparisonExtraInfo->setText(QString(""));
	errorComparisonExtraInfo->setContentsMargins(0,30,0,0);
	vLayout->addWidget(errorComparisonExtraInfo);

	QObject::connect(compareButton, &QToolButton::pressed, [this]() {
				this->computeDifference();
			});
}

ComparisonWindow::~ComparisonWindow()
{
	delete leftBox;
	delete rightBox;
	delete errorComparison;
	delete errorComparisonExtraInfo;
	delete compareButton;

	for(uint i=0; i<visualizationWindows.size();i++){
		if(visualizationWindows[i])
			delete visualizationWindows[i];
	}
}
