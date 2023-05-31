#include "trianglemesh.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

TriangleMesh::TriangleMesh()
{
	vertices.clear();
	faces.clear();
	faceNormals.clear();

#ifdef _WIN32
    min_ = Eigen::Vector3f(99999,99999,99999);
#else
    min_ = Eigen::Vector3f(std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max());
#endif

	max_ = Eigen::Vector3f(std::numeric_limits<float>::lowest(),
						   std::numeric_limits<float>::lowest(),
                           std::numeric_limits<float>::lowest());
}

bool TriangleMesh::hitTriangle(const vector3& point, const vector3& L, const int triangleIdx, vector3& hitPoint){

	const precision::value_type tMin= 1.e-5L;
	const precision::value_type tMax= 1000000.L;

	const auto& t = faces[triangleIdx];

	const auto& N = faceNormals[t.nn];

	const auto d= - Common::dot(vertices[t.v1],N);
	const vector4 vNormal(N.x,N.y,N.z,d);
	const vector4 vPoint(point,1.L);
	const vector4 L2(L,0.L);

	//Avoid results when the dot product is near 0 or positive
	const auto lowerPart = Common::dot(vNormal, L2);
	const auto epsilon = std::numeric_limits<precision::value_type>::epsilon();
	if(abs(lowerPart) < epsilon)
		return false;

	const auto tValue = - dot(vNormal,vPoint) / lowerPart;

	if(tValue > tMin && tValue < tMax){
		const auto intPoint = tValue*L + point;

		auto dir1 = vertices[t.v3]-vertices[t.v1];
		auto dir2 = intPoint-vertices[t.v1];
		const auto alpha = 0.5L * Common::length(Common::cross(dir1,dir2));

		dir1 = vertices[t.v2]-vertices[t.v3];
		dir2 = intPoint-vertices[t.v3];
		const auto beta = 0.5L * Common::length(Common::cross(dir1,dir2));

		dir1 = vertices[t.v1]-vertices[t.v2];
		dir2 = intPoint-vertices[t.v2];
		const auto gamma = 0.5L * Common::length(Common::cross(dir1,dir2));

		dir1 = vertices[t.v3]-vertices[t.v1];
		dir2 = vertices[t.v2]-vertices[t.v1];
		const auto totalArea = 0.5L * Common::length(Common::cross(dir1,dir2));

		if(fabs(alpha+beta+gamma-totalArea)<1.e-5f && alpha>0.L && beta>0.L && gamma>0.L){ //1.e-5L
			hitPoint = intPoint;
			return true;
		}
	}

	return false;
}

void TriangleMesh::prepareDataToGPU(){
	replicatedVertices.clear();
	replicatedNormals.clear();
	indexedFaces.clear();

	replicatedVertices.resize(faces.size()*3);
	replicatedNormals.resize(faces.size()*3);

	vector4 **normalsPunter=new vector4*[faces.size()*3];
	std::unordered_map<int,vector4> normalsPromig;

	uint Index=0;

	for(unsigned int i=0; i<faces.size(); i++)
	{
		int triangleVertices[3] = {faces[i].v1,faces[i].v2,faces[i].v3};
		int face = faces[i].nn;

		for(unsigned int j=0; j<3; j++)
		{

			replicatedVertices[Index] = vector4(vertices[triangleVertices[j]],1);

			normalsPromig[triangleVertices[j]]+=vector4(faceNormals[face],0);
			vector4 *aux = new vector4(faceNormals[face],0);
			normalsPunter[Index]=aux;

			indexedFaces.push_back(triangleVertices[j]);

			Index++;
		}
	}
	for(std::unordered_map<int,vector4>::iterator ii=normalsPromig.begin(); ii!=normalsPromig.end(); ++ii){
	   (*ii).second=Common::normalize((*ii).second);
	}

	for(uint i=0; i< Index; i++){
		replicatedNormals[i]=*normalsPunter[i];
	}
}

void TriangleMesh::computeBoundingBox()
{

#ifdef _WIN32
    min_ = Eigen::Vector3f(99999,99999,99999);
#else
	min_ = Eigen::Vector3f(std::numeric_limits<float>::max(),
						   std::numeric_limits<float>::max(),
						   std::numeric_limits<float>::max());
#endif

	max_ = Eigen::Vector3f(std::numeric_limits<float>::lowest(),
						   std::numeric_limits<float>::lowest(),
						   std::numeric_limits<float>::lowest());

	const int kVertices = vertices.size();
    for (int i = 0; i < kVertices; ++i)
    {
#ifdef _WIN32
		min_[0] = min(min_[0], (float)vertices[i].x);
		min_[1] = min(min_[1], (float)vertices[i].y);
		min_[2] = min(min_[2], (float)vertices[i].z);

		max_[0] = max(max_[0], (float)vertices[i].x);
		max_[1] = max(max_[1], (float)vertices[i].y);
		max_[2] = max(max_[2], (float)vertices[i].z);
#else
		min_[0] = std::min(min_[0], (float)vertices[i].x);
		min_[1] = std::min(min_[1], (float)vertices[i].y);
		min_[2] = std::min(min_[2], (float)vertices[i].z);

		max_[0] = std::max(max_[0], (float)vertices[i].x);
		max_[1] = std::max(max_[1], (float)vertices[i].y);
		max_[2] = std::max(max_[2], (float)vertices[i].z);
#endif
	}
	// In case the object is a plane or a 2 dimensional object, we add a little offset/desviation
	// to obtain a 3D box.
	const auto epsilon = std::numeric_limits<float>::epsilon();
	for(uint i = 0; i<3; i++)
	{
		if(fabs(max_[i]-min_[i]) < epsilon)
		{
			max_[i] += 0.1f;
			min_[i] += -0.1f;
		}
	}
}

void TriangleMesh::sendMeshToGPU(std::unique_ptr<QGLShaderProgram> &program)
{
	GLuint idNumVertices = program->uniformLocation("numVertices");
	const auto numVertices = this->vertices.size();
	glUniform1i(idNumVertices,numVertices);

	GLuint idNumNormals = program->uniformLocation("numNormals");
	const auto numNormals = this->faceNormals.size();
	glUniform1i(idNumNormals,numNormals);

	for(uint i=0; i<numVertices;i++){
		GLuint idVertex = program->uniformLocation(QString("vertices[%1]").arg( i ));
		glUniform3f(idVertex,this->vertices[i].x,this->vertices[i].y,this->vertices[i].z);
	}
	for(uint i=0; i<numNormals;i++){
		GLuint idNormal = program->uniformLocation(QString("normals[%1]").arg( i ));
		glUniform3f(idNormal,this->faceNormals[i].x,this->faceNormals[i].y,this->faceNormals[i].z);
	}

	GLuint idNumTriangles = program->uniformLocation("numTriangles");
	const auto numTriangles = this->faces.size();
	glUniform1i(idNumTriangles,numTriangles);

	for(uint i=0; i< numTriangles;i++){
		GLuint idVertex1 = program->uniformLocation(QString("triangles[%1].v1").arg( i ));
		GLuint idVertex2 = program->uniformLocation(QString("triangles[%1].v2").arg( i ));
		GLuint idVertex3 = program->uniformLocation(QString("triangles[%1].v3").arg( i ));
		GLuint idMat = program->uniformLocation(QString("triangles[%1].material").arg( i ));
		GLuint idNormal = program->uniformLocation(QString("triangles[%1].normal").arg( i ));

		glUniform1i(idVertex1,this->faces[i].v1);
		glUniform1i(idVertex2,this->faces[i].v2);
		glUniform1i(idVertex3,this->faces[i].v3);
		glUniform1f(idMat,this->faces[i].rf);
		glUniform1f(idNormal,this->faces[i].nn);
	}
}
