#include "trianglemesh.h"

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

TriangleMesh::TriangleMesh()
{
	vertices.clear();
	faces.clear();
	faceNormals.clear();
	//vertexNormals.clear();

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

bool TriangleMesh::hitTriangle(vec3 point, vec3 L, int triangleIdx, vec3& hitPoint){

	float tMin=1.e-5f;
	float tMax= 1000000.f;

	Triangle t = faces[triangleIdx];

	QVector3D N = faceNormals[t.nn];

	float d= - QVector3D::dotProduct(vertices[t.v1],N);
	vec4 vNormal = vec4(N.x(),N.y(),N.z(),d);
	vec4 vPoint = vec4(point,1.f);
	vec4 L2 = vec4(L,0.f);

	float lowerPart = dot(vNormal, L2);
	if(fabs(lowerPart)<1.e-5f)
		return false;

	float tValue = - dot(vNormal,vPoint) / lowerPart;

	if(tValue > tMin && tValue < tMax){
		QVector3D Laux(L.x,L.y,L.z);
		QVector3D intPoint = tValue*Laux + QVector3D(point.x,point.y,point.z);

		QVector3D dir1 = vertices[t.v3]-vertices[t.v1];
		QVector3D dir2 = intPoint-vertices[t.v1];
		float alpha = 0.5f * QVector3D::crossProduct(dir1,dir2).length();

		dir1 = vertices[t.v2]-vertices[t.v3];
		dir2 = intPoint-vertices[t.v3];
		float beta = 0.5f * QVector3D::crossProduct(dir1,dir2).length();

		dir1 = vertices[t.v1]-vertices[t.v2];
		dir2 = intPoint-vertices[t.v2];
		float gamma = 0.5f * QVector3D::crossProduct(dir1,dir2).length();

		dir1 = vertices[t.v3]-vertices[t.v1];
		dir2 = vertices[t.v2]-vertices[t.v1];
		float totalArea = 0.5f * QVector3D::crossProduct(dir1,dir2).length();

		if(fabs(alpha+beta+gamma-totalArea)<1.e-5f && alpha>0 && beta>0 && gamma>0){
			hitPoint = vec3(intPoint.x(),intPoint.y(),intPoint.z());
			return true;
		}

	}

	return false;

}

int TriangleMesh::hitTriangle(double pix[], double ray[], Triangle t0, double Pint[])
/*
	hit_tri: Hit Triangle function, checks if pixel-ray crosses one of
		the triangles defining the surface.

	INPUT:
		pix[3] = (px,py,pz) components of point in the pixel array
		ray[3] = (rx,ry,rz) components of RAY (rs direction w.r.t sc)
		t0		 = triangle structer we want to check intersection
	OUTPUT:
		Pint[3] = if intersection occures -> contains the intersection point,
					 else -> contains (0,0,0).
	RETURN
		0 if yes intersection
		1 if no  intersection
*/
{
	int i, que;
	double tol = 1.e-13;
	double costh, x[3], b[3], A[3][3];
	//int solve_cramer(double A[][3], double b[], double x[]);

	Pint[0] = 0.; Pint[1] = 0.; Pint[2] = 0.;

	/*	costh < 0 so that face is illuminated */

	costh = QVector3D::dotProduct(faceNormals[t0.nn],QVector3D(ray[0],ray[1],ray[2]));

	if(costh >= 0){
		return 1;

	}

	/* plane and ray are almost parallel (no possible interseciton) */
	if(fabs(costh) < tol) return 1;

	/* set linear system line with ray (t param) and
		plane with triangle (v,w param) */
	for(i = 0; i < 3; i++)
	{
		A[i][0] = -ray[i];
		A[i][1] = (vertices[t0.v2][i] - vertices[t0.v1][i]);
		A[i][2] = (vertices[t0.v3][i] - vertices[t0.v1][i]);
		b[i] = pix[i] - vertices[t0.v1][i];
	}

	/*	Solve Linear System */
	que = solveCramer(A, b, x);
	if(que == 1) return 1;

	/*	check hit conditions */
	if(x[1] < 0 || x[2] < 0 || (x[1] + x[2]) > 1) return 1; /* (v < 0 or w < 0 or v+w > 1 outside triangle) */
	if(x[0] <= 0){
		//printf("mec \n");
		return 1;
	} /* t < 0 !! (wrong side for triangle) */

	/*	Intersection Point */
	for(i = 0; i < 3; i++) Pint[i] = pix[i] + x[0]*ray[i];

	return 0;
}

int TriangleMesh::solveCramer(double A[][3], double b[], double x[])
{
	int i, j, r;
	double det, AUX[3][3], tol = 1.e-13;

	det = det3(A);
	if(det < tol) return 1;

	for(r = 0; r < 3; r++)
	{
		for(j = 0; j < 3; j++)
		{
			if(j == r)
			{ 	for(i = 0; i < 3; i++) AUX[i][j] = b[i]; }
			else
			{ 	for(i = 0; i < 3; i++) AUX[i][j] = A[i][j]; }
		}
		x[r] = det3(AUX)/det;
	}

	return 0;
}

double TriangleMesh::det3(double A[][3])
{
	double det = 0;

	det += A[0][0]*( A[1][1]*A[2][2] - A[2][1]*A[1][2]);
	det += A[1][0]*(-A[0][1]*A[2][2] + A[2][1]*A[0][2]);
	det += A[2][0]*( A[0][1]*A[1][2] - A[1][1]*A[0][2]);

	return det;
}


void TriangleMesh::prepareDataToGPU(){
	replicatedVertices.clear();
	replicatedNormals.clear();
	indexedFaces.clear();

	replicatedVertices.resize(faces.size()*3);
	replicatedNormals.resize(faces.size()*3);

	vec4 **normalsPunter=new vec4*[faces.size()*3];
	std::unordered_map<int,vec4> normalsPromig;


	uint Index=0;

	for(unsigned int i=0; i<faces.size(); i++){
		int triangleVertices[3] = {faces[i].v1,faces[i].v2,faces[i].v3};
		int face = faces[i].nn;

		for(unsigned int j=0; j<3; j++){

			if(triangleVertices[j]==3){
				int text = -1;
			}

			replicatedVertices[Index] = vec4(vertices[triangleVertices[j]].x(),vertices[triangleVertices[j]].y(),vertices[triangleVertices[j]].z(),1);

			normalsPromig[triangleVertices[j]]+=vec4(faceNormals[face].x(),faceNormals[face].y(),faceNormals[face].z(),0);
			vec4 *aux = new vec4;
			aux->x=faceNormals[face].x();
			aux->y=faceNormals[face].y();
			aux->z=faceNormals[face].z();
			aux->w=0;
			normalsPunter[Index]=aux;//&normalsPromig[triangleVertices[j]];

			indexedFaces.push_back(triangleVertices[j]);

			Index++;
		}
	}
	for( std::unordered_map<int,vec4>::iterator ii=normalsPromig.begin(); ii!=normalsPromig.end(); ++ii){
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
        min_[0] = min(min_[0], vertices[i].x());
        min_[1] = min(min_[1], vertices[i].y());
        min_[2] = min(min_[2], vertices[i].z());

        max_[0] = max(max_[0], vertices[i].x());
        max_[1] = max(max_[1], vertices[i].y());
        max_[2] = max(max_[2], vertices[i].z());
#else
        min_[0] = std::min(min_[0], vertices[i].x());
        min_[1] = std::min(min_[1], vertices[i].y());
        min_[2] = std::min(min_[2], vertices[i].z());

        max_[0] = std::max(max_[0], vertices[i].x());
        max_[1] = std::max(max_[1], vertices[i].y());
        max_[2] = std::max(max_[2], vertices[i].z());
#endif
	}
}

void TriangleMesh::sendMeshToGPU(std::unique_ptr<QGLShaderProgram> &program)
{
	GLuint idNumVertices = program->uniformLocation("numVertices");
	int numVertices = this->vertices.size();
	glUniform1i(idNumVertices,numVertices);

	GLuint idNumNormals = program->uniformLocation("numNormals");
	int numNormals = this->faceNormals.size();
	glUniform1i(idNumNormals,numNormals);

	for(uint i=0; i<numVertices;i++){
		GLuint idVertex = program->uniformLocation(QString("vertices[%1]").arg( i ));
		glUniform3f(idVertex,this->vertices[i].x(),this->vertices[i].y(),this->vertices[i].z());
	}
	for(uint i=0; i<numNormals;i++){
		GLuint idNormal = program->uniformLocation(QString("normals[%1]").arg( i ));
		glUniform3f(idNormal,this->faceNormals[i].x(),this->faceNormals[i].y(),this->faceNormals[i].z());
	}

	GLuint idNumTriangles = program->uniformLocation("numTriangles");
	int numTriangles = this->faces.size();
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
