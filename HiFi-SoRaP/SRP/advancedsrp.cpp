#include "advancedsrp.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include<QElapsedTimer>
#include<iostream>
#include<fstream>
#include <iomanip>

#define NOISE_SIZE 768 //256*3, 256 random 3d vectors

AdvancedSRP::AdvancedSRP()
{
}

void AdvancedSRP::computeSRP(Grid *results)
{
	int NEL, NAZ, AZstep, ELstep;
	precision::value_type el, az, elrad, azrad;
	vector3 RS, V1, V2, XS, cm;

	vector3 force;

	/* Right now we are not allowing the user to set the CM.
	// MOVE the stacecraft so tht CM is at the Origin
	cm = vector3{this->cm.x(),this->cm.y(),this->cm.z()};
	TriangleMesh *mesh = satellite->getMesh();
	for(r = 0; r < (int)mesh->vertices.size(); r++)
	{
		float x = mesh->vertices[r].x() - cm[0];
		float y = mesh->vertices[r].y() - cm[1];
		float z = mesh->vertices[r].z() - cm[2];
		mesh->vertices[r] = QVector3D(x,y,z);
	}*/

	AZstep = step_AZ;
	ELstep = step_EL;

	NEL = 180/ELstep+1;
	NAZ = 360/AZstep+1;

	auto start = std::chrono::steady_clock::now();

	for(int j = 0; j < NAZ; j++)     //int j = 0; j < NAZ; j++
	{
		az = -180 + j*AZstep;          //az = -180 + j*AZstep;

//az = 180;
		azrad = az*M_PI/180.;
		/* Scan EL */
		for(int i =0; i < NEL; i++)   //int i =0; i < NEL; i++
		{
			el = (-90 + i*ELstep);       //el = (-90. + i*ELstep);
//el = 90;
			elrad = el*M_PI/180.;

			/*	RS is sun position wrt CM of spacecraft and <V1,V2> generate the Pixel Array */

			RS[0] =  cos(elrad)*cos(azrad); RS[1] = cos(elrad)*sin(azrad);  RS[2] = sin(elrad);
			V1[0] = -sin(elrad)*cos(azrad); V1[1] = -sin(elrad)*sin(azrad); V1[2] = cos(elrad);
			V2[0] =  sin(azrad); 	 	 	V2[1] = -cos(azrad); 	 	 	V2[2] = 0;
			XS[0] = -RS[0]; 	 	 	 	XS[1] = -RS[1];   				XS[2] = -RS[2];

			//XS[0] = -1; XS[1] = 0;  XS[2] = 0;
			//V1[0] = 0;  V1[1] = 0;  V1[2] = -1;
			//V2[0] = 0;  V2[1] = -1; V2[2] = 0;
			computeStepSRP(XS,force,V1,V2);

			(*results)(j,i)=Output(az, el, force.x, force.y, force.z);

            if(progressBar && progressBar->value()<NEL*NAZ) progressBar->setValue(progressBar->value()+1);
			QCoreApplication::processEvents();
			if(stopExecution) return;


		}

	}

	auto end = std::chrono::steady_clock::now();
	auto diff = end - start;
	std::cout << std::chrono::duration<double, std::milli>(diff).count() << " ms" << std::endl;

	/* MOVE the stacecraft to its initial origin
	for(r = 0; r < (int)mesh->vertices.size(); r++)
	{
		float x = mesh->vertices[r].x() + cm[0];
		float y = mesh->vertices[r].y() + cm[1];
		float z = mesh->vertices[r].z() + cm[2];
		mesh->vertices[r] = QVector3D(x,y,z);
	}*/
}

vector3 AdvancedSRP::computeSRP(const vector3& XS,float angleX, float angleY, float angleZ)
{
	vector3 force;
	const Eigen::Affine3f rotationX(Eigen::AngleAxisf(-angleX,Eigen::Vector3f(1,0,0)));
	const Eigen::Affine3f rotationY(Eigen::AngleAxisf(-angleY,Eigen::Vector3f(0,1,0)));
	const Eigen::Affine3f rotationZ(Eigen::AngleAxisf(-angleZ,Eigen::Vector3f(0,0,1)));

	const auto model = rotationX.matrix()*rotationY.matrix()*rotationZ.matrix();
	const auto output = model * Eigen::Vector4f(XS.x,XS.y,XS.z,0);
	const auto outputV1 = model * Eigen::Vector4f(0,1,0,0);
	const auto outputV2 = model * Eigen::Vector4f(0,0,1,0);

	const vector3 rotatedXS{output[0],output[1],output[2]};
	const vector3 rotatedV1{outputV1[0],outputV1[1],outputV1[2]};
	const vector3 rotatedV2{outputV2[0],outputV2[1],outputV2[2]};

    computeStepSRP(rotatedXS,force,rotatedV1,rotatedV2);

    /*vector3 V1, V2;
    V1[0] = 0;  V1[1] = 0;  V1[2] = -1;
    V2[0] = 0;  V2[1] = -1; V2[2] = 0;
    computeStepSRP(XS, force, V1, V2);*/

	return force;
}

vector3 AdvancedSRP::computeSRP(const vector3& XS, Eigen::Matrix4f &satelliteRotation)
{
	vector3 force;

	// Do not use 'auto' in the next variables, it is important to specify their types explicitly.
	const Eigen::Matrix4f model = satelliteRotation.inverse();
	const Eigen::Vector4f output = model * Eigen::Vector4f(XS.x,XS.y,XS.z,0);
	const Eigen::Vector4f outputV1 = model * Eigen::Vector4f(0,1,0,0);
	const Eigen::Vector4f outputV2 = model * Eigen::Vector4f(0,0,1,0);

	const vector3 rotatedXS{output[0],output[1],output[2]};
	const vector3 rotatedV1{outputV1[0],outputV1[1],outputV1[2]};
	const vector3 rotatedV2{outputV2[0],outputV2[1],outputV2[2]};

	computeStepSRP(rotatedXS,force,rotatedV1,rotatedV2);

	return force;
}

void AdvancedSRP::saveResults(Grid *results)
{
	saveResultsToFile(nx,cm,results);
}

const float* AdvancedSRP::getUniformNoiseTexture()
{
	const static auto noiseTexture = new float[NOISE_SIZE]{
		0.594403, 0.922066, 0.550353, 0.988954, 0.769224, 0.0891066, 0.538367, 0.521947, 0.188884, 0.971494, 0.32517,
		0.348567, 0.182557, 0.943941, 0.620883, 0.425365, 0.420159, 0.0434429, 0.679022, 0.245667, 0.195742, 0.879963,
		0.800856, 0.842113, 0.241609, 0.769301, 0.806611, 0.816287, 0.86006, 0.341953, 0.745569, 0.47212, 0.696836,
		0.301674, 0.460147, 0.791209, 0.0776332, 0.569335, 0.615787, 0.076318, 0.34677, 0.481201, 0.452154, 0.776564,
		0.578637, 0.118577, 0.617211, 0.965098, 0.719562, 0.154592, 0.0607718, 0.0243967, 0.203775, 0.0560992, 0.845485,
		0.233007, 0.368658, 0.984332, 0.936538, 0.875328, 0.813753, 0.985692, 0.201679, 0.558833, 0.63012, 0.733526,
		0.630741, 0.448916, 0.221356, 0.577214, 0.532918, 0.0284153, 0.21935, 0.641381, 0.173799, 0.500292, 0.396309,
		0.187817, 0.747202, 0.765194, 0.513588, 0.0850314, 0.984094, 0.59481, 0.670498, 0.482356, 0.850379, 0.21282,
		0.737594, 0.753374, 0.196986, 0.593339, 0.00584518, 0.376476, 0.949207, 0.557176, 0.500733, 0.202514, 0.177091,
		0.836509, 0.648723, 0.307283, 0.172336, 0.961367, 0.969545, 0.342882, 0.73365, 0.843621, 0.6091, 0.345591,
		0.882057, 0.248299, 0.843557, 0.815958, 0.300777, 0.520202, 0.0496209, 0.584921, 0.990734, 0.710892, 0.204262,
		0.106666, 0.642544, 0.782821, 0.40724, 0.572469, 0.470587, 0.0570822, 0.76976, 0.444001, 0.142481, 0.279171,
		0.606524, 0.186547, 0.109833, 0.00491729, 0.346769, 0.352266, 0.850111, 0.575647, 0.525143, 0.78077, 0.15329,
		0.0419025, 0.392431, 0.551238, 0.613375, 0.799487, 0.933483, 0.0168599, 0.277249, 0.232872, 0.00920358,
		0.0635821, 0.817647, 0.931527, 0.872657, 0.857219, 0.886493, 0.698647, 0.694019, 0.144104, 0.924012, 0.160054,
		0.0078558, 0.985171, 0.594151, 0.464537, 0.337264, 0.441741, 0.274764, 0.954457, 0.480246, 0.223072, 0.325877,
		0.758055, 0.059464, 0.852184, 0.776577, 0.64253, 0.647027, 0.175858, 0.427403, 0.991142, 0.76985, 0.644459,
		0.257952, 0.910303, 0.113395, 0.707576, 0.209533, 0.70405, 0.177173, 0.927703, 0.0602924, 0.176536, 0.278672,
		0.833327, 0.510331, 0.549453, 0.972588, 0.589761, 0.765851, 0.374275, 0.723293, 0.377973, 0.0494655, 0.57775,
		0.814772, 0.423605, 0.0808521, 0.782026, 0.25693, 0.326586, 0.565034, 0.227696, 0.209997, 0.413172, 0.73274,
		0.366134, 0.187217, 0.627276, 0.748106, 0.351677, 0.622821, 0.785401, 0.783284, 0.187168, 0.388953, 0.726471,
		0.036004, 0.622482, 0.928275, 0.634682, 0.613486, 0.0579181, 0.549891, 0.293147, 0.818578, 0.351232, 0.754839,
		0.238438, 0.155316, 0.962656, 0.908522, 0.65346, 0.778007, 0.587503, 0.983527, 0.0704616, 0.816275, 0.571808,
		0.779092, 0.717771, 0.882681, 0.800832, 0.549081, 0.415446, 0.311828, 0.603694, 0.28678, 0.559986, 0.811492,
		0.848558, 0.284527, 0.0987714, 0.637656, 0.906153, 0.441658, 0.855466, 0.112327, 0.97369, 0.377062, 0.596995,
		0.299941, 0.71065, 0.212731, 0.457611, 0.461266, 0.660583, 0.145195, 0.901739, 0.607074, 0.865998, 0.761912,
		0.667528, 0.446859, 0.220435, 0.0276631, 0.354873, 0.183628, 0.341769, 0.251628, 0.0642717, 0.435158, 0.268701,
		0.54179, 0.139236, 0.318905, 0.618854, 0.596015, 0.81374, 0.413578, 0.275702, 0.708753, 0.692529, 0.492098,
		0.0980101, 0.0553676, 0.612088, 0.958682, 0.776261, 0.51832, 0.209277, 0.54183, 0.55483, 0.663303, 0.593174,
		0.917313, 0.912848, 0.276868, 0.170722, 0.925805, 0.591188, 0.110693, 0.794042, 0.614968, 0.832181, 0.307402,
		0.245653, 0.280151, 0.908691, 0.140378, 0.315581, 0.0549465, 0.0319878, 0.315482, 0.968026, 0.979307, 0.673207,
		0.815974, 0.834148, 0.889014, 0.319296, 0.384641, 0.814238, 0.540211, 0.971711, 0.884017, 0.441911, 0.389136,
		0.678297, 0.470639, 0.791185, 0.84251, 0.902909, 0.0110184, 0.586804, 0.720842, 0.206557, 0.761061, 0.0700813,
		0.807483, 0.862427, 0.528313, 0.654776, 0.98511, 0.0278816, 0.93688, 0.834169, 0.68525, 0.930164, 0.883541,
		0.455765, 0.922807, 0.619755, 0.532624, 0.0584386, 0.735748, 0.0221372, 0.506559, 0.244778, 0.844232, 0.169338,
		0.592858, 0.883768, 0.865335, 0.039692, 0.178127, 0.464105, 0.650268, 0.269529, 0.0122605, 0.962135, 0.907188,
		0.945458, 0.987795, 0.0718769, 0.131918, 0.794555, 0.967612, 0.772979, 0.524778, 0.74787, 0.409995, 0.141945,
		0.0171446, 0.572761, 0.703706, 0.880632, 0.453436, 0.260553, 0.575489, 0.336411, 0.411523, 0.92831, 0.896612,
		0.128299, 0.0815215, 0.864071, 0.443578, 0.561167, 0.743079, 0.223654, 0.21338, 0.696472, 0.98903, 0.565626,
		0.26632, 0.357942, 0.925242, 0.382033, 0.950417, 0.849132, 0.71794, 0.918649, 0.577374, 0.586954, 0.878475,
		0.629557, 0.0352797, 0.594189, 0.210772, 0.61154, 0.759776, 0.605896, 0.9413, 0.0116616, 0.28585, 0.217119,
		0.558942, 0.489796, 0.48301, 0.0663952, 0.088022, 0.297706, 0.00784055, 0.742167, 0.963925, 0.811738, 0.63032,
		0.738044, 0.437826, 0.189358, 0.624003, 0.835201, 0.352487, 0.302987, 0.0698635, 0.673726, 0.276073, 0.784158,
		0.834622, 0.513292, 0.350623, 0.935335, 0.38578, 0.30187, 0.874631, 0.622177, 0.148139, 0.764749, 0.0970329,
		0.496295, 0.536546, 0.641232, 0.1225, 0.348439, 0.912715, 0.354931, 0.56403, 0.368726, 0.303395, 0.118173,
		0.161499, 0.133827, 0.807487, 0.878809, 0.975259, 0.748727, 0.100423, 0.21703, 0.904836, 0.00151255, 0.211782,
		0.971183, 0.934073, 0.507816, 0.551219, 0.891012, 0.854742, 0.378836, 0.50216, 0.680182, 0.878899, 0.703673,
		0.871159, 0.589622, 0.767227, 0.372228, 0.845271, 0.120632, 0.829506, 0.856551, 0.0952556, 0.130158, 0.797717,
		0.223995, 0.660533, 0.680479, 0.982999, 0.672239, 0.366006, 0.632054, 0.25824, 0.150065, 0.279203, 0.347392,
		0.0171847, 0.894525, 0.847611, 0.760339, 0.0175633, 0.83062, 0.160112, 0.381854, 0.147895, 0.987843, 0.206857,
		0.634541, 0.243701, 0.908998, 0.608337, 0.473303, 0.705666, 0.646885, 0.947763, 0.50896, 0.731099, 0.515497,
		0.0581689, 0.669188, 0.701873, 0.946199, 0.755986, 0.408749, 0.444479, 0.210124, 0.14041, 0.388836, 0.867917,
		0.550244, 0.737333, 0.602369, 0.4305, 0.345818, 0.498776, 0.0514195, 0.210451, 0.336349, 0.622204, 0.167799,
		0.303925, 0.107617, 0.139354, 0.869936, 0.273143, 0.643914, 0.50539, 0.524598, 0.337241, 0.524619, 0.486324,
		0.485483, 0.670359, 0.548491, 0.502975, 0.37528, 0.523155, 0.516415, 0.737741, 0.407631, 0.948036, 0.724982,
		0.017738, 0.999668, 0.58399, 0.403922, 0.301188, 0.742522, 0.497729, 0.0738198, 0.128964, 0.107472, 0.657104,
		0.102533, 0.10373, 0.112268, 0.281282, 0.901113, 0.473179, 0.187229, 0.298094, 0.632231, 0.889898, 0.28542,
		0.70367, 0.704438, 0.717022, 0.473164, 0.579723, 0.303285, 0.243491, 0.459187, 0.449385, 0.854164, 0.458037,
		0.997745, 0.436302, 0.532002, 0.766962, 0.760043, 0.191019, 0.694333, 0.789944, 0.227572, 0.843337, 0.956473,
		0.21331, 0.266153, 0.753873, 0.151568, 0.475843, 0.207979, 0.668593, 0.310749, 0.857087, 0.0633476, 0.22699,
		0.0414376, 0.747284, 0.779834, 0.3667, 0.70269, 0.0145852, 0.841597, 0.695836, 0.0145556, 0.969048, 0.87681,
		0.671293, 0.72138, 0.576002, 0.984955, 0.38367, 0.218446, 0.748154, 0.974379, 0.040429, 0.160214, 0.811969,
		0.829075, 0.310615, 0.895152, 0.265081, 0.652226, 0.74959, 0.534531, 0.402185, 0.527962, 0.931415, 0.27,
		0.413335, 0.450803, 0.811309, 0.78821, 0.605029, 0.970227, 0.494011, 0.693733, 0.943725, 0.116083, 0.748385,
		0.615554, 0.393995, 0.604465, 0.970706, 0.701529, 0.491407, 0.0221013, 0.00147822, 0.700077, 0.887741, 0.38215,
		0.218487, 0.51757, 0.809615, 0.527273, 0.461816, 0.291581, 0.0971587, 0.297097, 0.717897, 0.467769, 0.397307,
		0.41992, 0.938676, 0.40516, 0.222452, 0.212878, 0.821914, 0.819227, 0.0191146, 0.201548, 0.659342, 0.00695886,
		0.532255, 0.649074, 0.912694, 0.640227, 0.26994, 0.0768398, 0.960863, 0.557951, 0.31923, 0.228176, 0.661366,
		0.985193, 0.470293, 0.795236, 0.864384, 0.559677, 0.640857, 0.311609, 0.572461, 0.620987, 0.940839, 0.78144,
		0.767642, 0.0385539, 0.509916, 0.753294, 0.356415, 0.47034, 0.139481, 0.113323, 0.626934, 0.0559004
	};
	return noiseTexture;
}

const float* AdvancedSRP::getFixedUniformNoiseTexture()
{
	const static auto noiseTexture = new float[NOISE_SIZE]{
		0.749589, 0.849583, 0.442557, 0.27568, 0.333301, 0.453156, 0.211758, 0.255255, 0.784295, 0.874185, 0.772036,
		0.523167, 0.138408, 0.411064, 0.315422, 0.525589, 0.278148, 0.535166, 0.37625, 0.388909, 0.240991, 0.775601,
		0.443929, 0.844431, 0.318546, 0.353829, 0.283561, 0.208308, 0.5569, 0.574755, 0.503877, 0.727686, 0.69751,
		0.808144, 0.412374, 0.288402, 0.372353, 0.291805, 0.698885, 0.637191, 0.84331, 0.828702, 0.861944, 0.26896,
		0.590965, 0.45016, 0.358695, 0.20001, 0.564821, 0.107135, 0.275373, 0.476137, 0.812817, 0.598131, 0.480286,
		0.124351, 0.356237, 0.395495, 0.121021, 0.537401, 0.365075, 0.575553, 0.276733, 0.484369, 0.339461, 0.294642,
		0.326154, 0.351582, 0.304521, 0.561717, 0.890625, 0.279801, 0.573709, 0.535305, 0.880612, 0.788649, 0.746517,
		0.253145, 0.609727, 0.178534, 0.681832, 0.301776, 0.277368, 0.498199, 0.149212, 0.731027, 0.400551, 0.782148,
		0.65371, 0.329596, 0.786124, 0.103173, 0.564611, 0.165318, 0.413769, 0.783204, 0.3121, 0.606754, 0.196893,
		0.56395, 0.45225, 0.839723, 0.573841, 0.951652, 0.319799, 0.483202, 0.695649, 0.521666, 0.535786, 0.171942,
		0.838155, 0.845091, 0.781719, 0.647849, 0.493908, 0.601048, 0.255201, 0.612073, 0.64384, 0.950396, 0.287864,
		0.2619, 0.461121, 0.641039, 0.785668, 0.530724, 0.41072, 0.372512, 0.417451, 0.428243, 0.89166, 0.586093,
		0.268256, 0.810721, 0.701134, 0.610036, 0.493915, 0.862155, 0.18217, 0.616815, 0.7156, 0.304931, 0.539675,
		0.358066, 0.681669, 0.600517, 0.281068, 0.38569, 0.774996, 0.669453, 0.444124, 0.113611, 0.665634, 0.532923,
		0.0801517, 0.606421, 0.0935839, 0.715595, 0.512996, 0.260387, 0.494636, 0.926594, 0.832395, 0.189149, 0.70149,
		0.36725, 0.844186, 0.797159, 0.19732, 0.291397, 0.761942, 0.28227, 0.500304, 0.908666, 0.347597, 0.728498,
		0.73947, 0.317463, 0.460177, 0.155251, 0.446835, 0.575726, 0.68919, 0.753257, 0.653699, 0.116839, 0.588644,
		0.190566, 0.729236, 0.599712, 0.540536, 0.318617, 0.79282, 0.585182, 0.317107, 0.366106, 0.64497, 0.846531,
		0.614626, 0.736953, 0.138118, 0.454728, 0.25744, 0.360346, 0.294688, 0.383393, 0.26889, 0.428775, 0.909263,
		0.498537, 0.057379, 0.61439, 0.40697, 0.780475, 0.376478, 0.660123, 0.839365, 0.504028, 0.523282, 0.361713,
		0.386498, 0.193844, 0.458097, 0.26265, 0.542681, 0.135489, 0.381245, 0.707441, 0.879707, 0.628633, 0.383487,
		0.331848, 0.507752, 0.110559, 0.622706, 0.627278, 0.410805, 0.799666, 0.256261, 0.506217, 0.383171, 0.54956,
		0.862257, 0.219922, 0.54115, 0.55093, 0.359662, 0.468732, 0.825125, 0.377769, 0.37797, 0.409843, 0.296253,
		0.291504, 0.876371, 0.541112, 0.476859, 0.790268, 0.656184, 0.573016, 0.809512, 0.498296, 0.674491, 0.947746,
		0.313813, 0.477837, 0.909509, 0.425414, 0.359035, 0.892268, 0.728959, 0.680926, 0.721099, 0.162644, 0.461501,
		0.490867, 0.568573, 0.922374, 0.46117, 0.691117, 0.195924, 0.38928, 0.712836, 0.175539, 0.721111, 0.533453,
		0.209399, 0.558548, 0.534156, 0.480704, 0.805692, 0.221632, 0.105073, 0.471577, 0.560153, 0.658167, 0.7001,
		0.564646, 0.671634, 0.14942, 0.824417, 0.688575, 0.700127, 0.737108, 0.803521, 0.686085, 0.72729, 0.714544,
		0.883723, 0.206244, 0.690821, 0.469583, 0.635211, 0.201453, 0.607631, 0.705023, 0.154837, 0.704944, 0.40809,
		0.886904, 0.490931, 0.43563, 0.878604, 0.797724, 0.101801, 0.412408, 0.644656, 0.642267, 0.809052, 0.741783,
		0.212761, 0.56269, 0.217547, 0.542925, 0.290849, 0.710761, 0.521054, 0.627602, 0.835216, 0.752172, 0.165112,
		0.471491, 0.881575, 0.726752, 0.467133, 0.763021, 0.233942, 0.307694, 0.376428, 0.482995, 0.668424, 0.743483,
		0.467752, 0.335633, 0.622585, 0.463143, 0.586575, 0.79853, 0.708935, 0.808265, 0.491788, 0.617538, 0.0613627,
		0.401118, 0.443701, 0.860689, 0.599852, 0.816166, 0.448949, 0.614102, 0.331606, 0.881878, 0.573577, 0.393603,
		0.13692, 0.345843, 0.428951, 0.219881, 0.195531, 0.570288, 0.3368, 0.465531, 0.670684, 0.297488, 0.601232,
		0.773881, 0.198561, 0.708531, 0.633191, 0.19003, 0.612862, 0.700083, 0.703975, 0.430755, 0.305853, 0.572526,
		0.242363, 0.325105, 0.832515, 0.239739, 0.277793, 0.469303, 0.363627, 0.333939, 0.904908, 0.538416, 0.529972,
		0.508581, 0.423991, 0.922179, 0.486282, 0.522099, 0.824987, 0.364953, 0.393357, 0.0289477, 0.496328, 0.184556,
		0.399975, 0.502754, 0.687381, 0.738039, 0.149402, 0.401079, 0.235166, 0.147314, 0.292712, 0.286537, 0.662286,
		0.852642, 0.744078, 0.719546, 0.270657, 0.342183, 0.0860088, 0.793054, 0.0984327, 0.47344, 0.1428, 0.412879,
		0.166582, 0.94096, 0.704101, 0.417086, 0.33189, 0.829351, 0.649978, 0.591363, 0.510516, 0.946073, 0.856433,
		0.76804, 0.593724, 0.0945876, 0.457723, 0.294149, 0.85467, 0.313397, 0.323973, 0.581198, 0.500101, 0.694367,
		0.710934, 0.659017, 0.778264, 0.27202, 0.423216, 0.918664, 0.292662, 0.293377, 0.569608, 0.804901, 0.630838,
		0.205595, 0.712165, 0.451342, 0.725071, 0.391157, 0.379162, 0.904345, 0.053976, 0.645421, 0.547728, 0.408039,
		0.556703, 0.126251, 0.0363128, 0.5492, 0.641756, 0.371067, 0.148122, 0.65871, 0.798432, 0.551796, 0.677579,
		0.426881, 0.641932, 0.338756, 0.453327, 0.586859, 0.232559, 0.569749, 0.225442, 0.715311, 0.338143, 0.71646,
		0.390511, 0.388939, 0.607643, 0.431654, 0.223053, 0.774419, 0.192488, 0.491012, 0.378965, 0.27453, 0.521349,
		0.296541, 0.467053, 0.354279, 0.282901, 0.77741, 0.660701, 0.105318, 0.676451, 0.39074, 0.251128, 0.102953,
		0.316754, 0.28022, 0.374638, 0.495345, 0.555474, 0.379438, 0.514627, 0.881317, 0.76834, 0.304541, 0.149336,
		0.505856, 0.690065, 0.729916, 0.704792, 0.401385, 0.459374, 0.149686, 0.731127, 0.369658, 0.894385, 0.31002,
		0.14659, 0.517261, 0.777553, 0.232858, 0.187961, 0.897373, 0.332011, 0.62412, 0.631923, 0.224555, 0.893715,
		0.730914, 0.486781, 0.175951, 0.773554, 0.684791, 0.459535, 0.499369, 0.430071, 0.331529, 0.40376, 0.214593,
		0.538036, 0.131613, 0.592914, 0.349425, 0.551603, 0.408481, 0.0877759, 0.478319, 0.662675, 0.577895, 0.30561,
		0.511238, 0.0842985, 0.259371, 0.623581, 0.273035, 0.633961, 0.848535, 0.436356, 0.23416, 0.481432, 0.235707,
		0.246734, 0.606348, 0.570808, 0.385302, 0.11747, 0.356653, 0.613372, 0.568991, 0.187686, 0.283552, 0.533205,
		0.211888, 0.854629, 0.542199, 0.475671, 0.658549, 0.780069, 0.514135, 0.700528, 0.756892, 0.578886, 0.173545,
		0.870906, 0.431812, 0.721448, 0.572948, 0.180811, 0.255336, 0.566815, 0.162261, 0.30057, 0.294288, 0.583072,
		0.561033, 0.516628, 0.426625, 0.553387, 0.25537, 0.854299, 0.433813, 0.775681, 0.732387, 0.665763, 0.224589,
		0.784086, 0.647483, 0.217528, 0.875963, 0.372731, 0.861214, 0.420296, 0.818568, 0.730963, 0.705621, 0.657642,
		0.313801, 0.0806737, 0.462635, 0.45995, 0.724016, 0.637398, 0.74766, 0.442588, 0.144474, 0.332403, 0.532696,
		0.509685, 0.623036, 0.547089, 0.624983, 0.38469, 0.701624, 0.462678, 0.287694, 0.151034, 0.513581, 0.302342,
		0.618027, 0.451, 0.315745, 0.222959, 0.702909, 0.19159, 0.387242, 0.426949, 0.795809, 0.389354, 0.955927,
		0.504827, 0.433798, 0.839653, 0.63011, 0.774234, 0.481876, 0.656243, 0.129318, 0.681759, 0.114312, 0.570525,
		0.174584, 0.267384, 0.316554, 0.875733, 0.815058, 0.56034, 0.390387, 0.883213, 0.207003, 0.513808, 0.738994,
		0.838785, 0.350265, 0.289163, 0.523661, 0.586642, 0.617091, 0.598781, 0.650593, 0.558606, 0.845929, 0.631987,
		0.114503, 0.44229, 0.267863, 0.729466, 0.348197, 0.71106, 0.744567, 0.384404, 0.189355, 0.366826, 0.540146,
		0.35825, 0.108521, 0.319115, 0.892264, 0.685064, 0.444551, 0.629054, 0.743584, 0.855567, 0.676917, 0.769378,
		0.666269, 0.785208, 0.657126, 0.416079, 0.872795, 0.444801, 0.234278, 0.732802, 0.755639, 0.239753, 0.318295,
		0.446958, 0.423825, 0.368991, 0.634179, 0.380362, 0.268365, 0.709377, 0.206286, 0.523434, 0.16993, 0.446627,
		0.611551, 0.555306, 0.360782, 0.714728, 0.52545, 0.469648, 0.537614, 0.368746, 0.49229
	};
	return noiseTexture;
}


const float* AdvancedSRP::getNormalNoiseTexture()
{
	const static auto noiseTexture = new float[NOISE_SIZE]{
		0.517647, 0.52549, 0.658824,0.603922, 0.490196, 0.364706,0.407843, 0.552941, 0.721569,0.701961, 0.568627, 0.278431,0.384314, 0.501961, 0.631373,
		0.435294, 0.478431, 0.560784,0.698039, 0.396078, 0.505882,0.266667, 0.584314, 0.529412,0.584314, 0.65098, 0.439216,0.4, 0.482353, 0.403922,0.74902, 0.458824, 0.47451,
		0.384314, 0.337255, 0.45098,0.603922, 0.486275, 0.729412,0.341176, 0.521569, 0.25098,0.6, 0.721569, 0.603922,0.509804, 0.388235, 0.478431,0.45098, 0.384314, 0.435294,
		0.364706, 0.541176, 0.690196,0.623529, 0.47451, 0.545098,0.615686, 0.462745, 0.498039,0.262745, 0.745098, 0.298039,0.501961, 0.462745, 0.639216,0.552941, 0.396078, 0.537255,
		0.439216, 0.478431, 0.498039,0.627451, 0.639216, 0.529412,0.615686, 0.364706, 0.258824,0.505882, 0.572549, 0.494118,0.482353, 0.592157, 0.666667,0.435294, 0.258824, 0.443137,
		0.427451, 0.509804, 0.584314,0.419608, 0.490196, 0.337255,0.733333, 0.596078, 0.501961,0.388235, 0.639216, 0.482353,0.560784, 0.501961, 0.560784,0.490196, 0.454902, 0.419608,
		0.462745, 0.533333, 0.74902,0.376471, 0.298039, 0.278431,0.486275, 0.670588, 0.603922,0.6, 0.313726, 0.54902,0.521569, 0.741176, 0.47451,0.482353, 0.494118, 0.521569,
		0.639216, 0.501961, 0.556863,0.411765, 0.25098, 0.384314,0.360784, 0.584314, 0.545098,0.592157, 0.470588, 0.345098,0.415686, 0.678431, 0.623529,0.501961, 0.337255, 0.607843,
		0.721569, 0.564706, 0.254902,0.486275, 0.54902, 0.517647,0.509804, 0.564706, 0.72549,0.360784, 0.447059, 0.501961,0.427451, 0.54902, 0.258824,0.517647, 0.254902, 0.596078,
		0.501961, 0.505882, 0.396078,0.631373, 0.619608, 0.576471,0.490196, 0.423529, 0.635294,0.482353, 0.517647, 0.482353,0.462745, 0.435294, 0.333333,0.458824, 0.717647, 0.686275,
		0.454902, 0.486275, 0.32549,0.631373, 0.498039, 0.462745,0.356863, 0.517647, 0.517647,0.717647, 0.384314, 0.501961,0.290196, 0.560784, 0.721569,0.537255, 0.501961, 0.337255,
		0.509804, 0.411765, 0.647059,0.635294, 0.576471, 0.270588,0.490196, 0.368627, 0.729412,0.321569, 0.690196, 0.427451,0.521569, 0.407843, 0.541176,0.596078, 0.447059, 0.329412,
		0.47451, 0.615686, 0.482353,0.603922, 0.333333, 0.537255,0.301961, 0.568627, 0.698039,0.678431, 0.54902, 0.345098,0.407843, 0.556863, 0.415686,0.6, 0.529412, 0.556863,
		0.486275, 0.278431, 0.517647,0.443137, 0.705882, 0.662745,0.537255, 0.4, 0.313726,0.486275, 0.615686, 0.678431,0.552941, 0.294118, 0.313726,0.466667, 0.717647, 0.678431,
		0.392157, 0.478431, 0.486275,0.662745, 0.333333, 0.431373,0.513726, 0.447059, 0.592157,0.258824, 0.721569, 0.34902,0.6, 0.47451, 0.65098,0.588235, 0.309804, 0.478431,
		0.309804, 0.721569, 0.368627,0.505882, 0.392157, 0.654902,0.509804, 0.635294, 0.278431,0.541176, 0.25098, 0.568627,0.615686, 0.505882, 0.643137,0.337255, 0.67451, 0.415686,
		0.541176, 0.360784, 0.584314,0.67451, 0.662745, 0.298039,0.521569, 0.533333, 0.678431,0.333333, 0.439216, 0.529412,0.592157, 0.380392, 0.47451,0.454902, 0.682353, 0.305882,
		0.396078, 0.513726, 0.494118,0.52549, 0.368627, 0.698039,0.45098, 0.521569, 0.52549,0.729412, 0.411765, 0.290196,0.498039, 0.690196, 0.721569,0.341176, 0.415686, 0.478431,
		0.470588, 0.458824, 0.470588,0.658824, 0.486275, 0.396078,0.529412, 0.505882, 0.415686,0.47451, 0.607843, 0.721569,0.537255, 0.52549, 0.427451,0.345098, 0.341176, 0.576471,
		0.533333, 0.427451, 0.298039,0.411765, 0.517647, 0.494118,0.717647, 0.658824, 0.486275,0.427451, 0.52549, 0.486275,0.580392, 0.396078, 0.498039,0.262745, 0.576471, 0.647059,
		0.529412, 0.435294, 0.552941,0.462745, 0.403922, 0.380392,0.556863, 0.717647, 0.415686,0.694118, 0.337255, 0.560784,0.301961, 0.635294, 0.67451,0.513726, 0.372549, 0.356863,
		0.443137, 0.564706, 0.658824,0.498039, 0.415686, 0.384314,0.603922, 0.447059, 0.545098,0.639216, 0.556863, 0.396078,0.498039, 0.513726, 0.486275,0.470588, 0.662745, 0.686275,
		0.4, 0.32549, 0.27451,0.415686, 0.580392, 0.67451,0.552941, 0.505882, 0.423529,0.517647, 0.478431, 0.521569,0.635294, 0.447059, 0.372549,0.509804, 0.560784, 0.486275,
		0.407843, 0.364706, 0.623529,0.564706, 0.509804, 0.623529,0.321569, 0.733333, 0.254902,0.705882, 0.266667, 0.709804,0.333333, 0.662745, 0.521569,0.556863, 0.552941, 0.27451,
		0.431373, 0.498039, 0.705882,0.607843, 0.396078, 0.356863,0.486275, 0.619608, 0.662745,0.517647, 0.376471, 0.345098,0.529412, 0.603922, 0.462745,0.482353, 0.313726, 0.686275,
		0.545098, 0.690196, 0.458824,0.270588, 0.52549, 0.333333,0.592157, 0.494118, 0.509804,0.529412, 0.486275, 0.478431,0.592157, 0.466667, 0.509804,0.509804, 0.396078, 0.52549,
		0.27451, 0.498039, 0.701961,0.72549, 0.415686, 0.494118,0.45098, 0.501961, 0.47451,0.305882, 0.596078, 0.513726,0.576471, 0.627451, 0.270588,0.623529, 0.305882, 0.737255,
		0.54902, 0.658824, 0.333333,0.447059, 0.560784, 0.647059,0.529412, 0.388235, 0.294118,0.427451, 0.360784, 0.619608,0.376471, 0.678431, 0.615686,0.482353, 0.333333, 0.266667,
		0.741176, 0.733333, 0.717647,0.278431, 0.290196, 0.313726,0.717647, 0.643137, 0.521569,0.282353, 0.388235, 0.682353,0.709804, 0.501961, 0.266667,0.392157, 0.501961, 0.513726,
		0.435294, 0.654902, 0.470588,0.545098, 0.513726, 0.682353,0.411765, 0.505882, 0.337255,0.478431, 0.337255, 0.709804,0.682353, 0.529412, 0.352941,0.384314, 0.384314, 0.501961,
		0.45098, 0.517647, 0.439216,0.572549, 0.552941, 0.721569,0.411765, 0.576471, 0.494118,0.529412, 0.596078, 0.396078,0.611765, 0.490196, 0.615686,0.376471, 0.360784, 0.427451,
		0.560784, 0.505882, 0.435294,0.631373, 0.654902, 0.588235,0.537255, 0.301961, 0.388235,0.313726, 0.52549, 0.498039,0.521569, 0.458824, 0.415686,0.509804, 0.576471, 0.623529,
		0.619608, 0.607843, 0.423529,0.286275, 0.388235, 0.686275,0.596078, 0.392157, 0.309804,0.423529, 0.721569, 0.611765,0.564706, 0.27451, 0.403922,0.611765, 0.533333, 0.67451,
		0.403922, 0.658824, 0.513726,0.521569, 0.490196, 0.27451,0.392157, 0.392157, 0.635294,0.580392, 0.662745, 0.533333,0.403922, 0.431373, 0.52549,0.529412, 0.329412, 0.290196,
		0.717647, 0.498039, 0.733333,0.505882, 0.54902, 0.345098,0.341176, 0.662745, 0.415686,0.490196, 0.458824, 0.733333,0.490196, 0.411765, 0.360784,0.431373, 0.466667, 0.596078,
		0.690196, 0.682353, 0.376471,0.552941, 0.47451, 0.521569,0.494118, 0.376471, 0.411765,0.368627, 0.658824, 0.52549,0.490196, 0.498039, 0.654902,0.470588, 0.482353, 0.529412,
		0.580392, 0.356863, 0.470588,0.505882, 0.670588, 0.435294,0.435294, 0.392157, 0.498039,0.454902, 0.490196, 0.588235,0.466667, 0.541176, 0.447059,0.733333, 0.427451, 0.345098,
		0.4, 0.627451, 0.745098,0.435294, 0.423529, 0.364706,0.588235, 0.392157, 0.392157,0.52549, 0.670588, 0.729412,0.439216, 0.517647, 0.290196,0.619608, 0.392157, 0.482353,
		0.282353, 0.623529, 0.509804,0.717647, 0.501961, 0.729412,0.494118, 0.407843, 0.301961,0.403922, 0.513726, 0.647059,0.596078, 0.34902, 0.447059,0.490196, 0.615686, 0.388235,
		0.266667, 0.596078, 0.556863,0.705882, 0.435294, 0.423529,0.45098, 0.513726, 0.694118,0.560784, 0.533333, 0.345098,0.337255, 0.352941, 0.537255,0.690196, 0.52549, 0.423529,
		0.27451, 0.654902, 0.513726,0.482353, 0.258824, 0.533333,0.564706, 0.662745, 0.541176,0.54902, 0.537255, 0.654902,0.615686, 0.537255, 0.415686,0.509804, 0.278431, 0.470588,
		0.505882, 0.662745, 0.431373,0.27451, 0.47451, 0.607843,0.611765, 0.435294, 0.498039,0.423529, 0.407843, 0.415686,0.490196, 0.745098, 0.513726,0.501961, 0.262745, 0.396078,
		0.517647, 0.662745, 0.592157,0.588235, 0.580392, 0.537255,0.560784, 0.34902, 0.376471,0.435294, 0.65098, 0.623529,0.34902, 0.25098, 0.368627
	};
	return noiseTexture;
}
