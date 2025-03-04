#include "object.h"
#include <locale.h>
#include <QFile>
#include <QRegularExpression>

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/


static inline VkDeviceSize aligned(VkDeviceSize v, VkDeviceSize byteAlign)
{
    return (v + byteAlign - 1) & ~(byteAlign - 1);
}



Object::Object()
{
	mesh = NULL;
	diffuseColor = QVector3D(1,0,0);
}

void Object::initializeBuffers()
{
	uint size = mesh->replicatedVertices.size();

	glGenVertexArrays( 1, &vao );

	// vertex buffer object (VBO)
	glGenBuffers( 1, &buffer );

	glBindBuffer( GL_ARRAY_BUFFER, buffer );

	using declared_type = precision::value_type;
	glBufferData( GL_ARRAY_BUFFER, 2*sizeof(vector4)*size + 3*sizeof(declared_type)*size + sizeof(int)*size, NULL,
				  GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vector4)*size, mesh->replicatedVertices.data() );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vector4)*size, sizeof(vector4)*size, mesh->replicatedNormals.data() );
	glBufferSubData( GL_ARRAY_BUFFER, 2*sizeof(vector4)*size, sizeof(declared_type)*size, replicatedMaterialsPD.data() );
	glBufferSubData( GL_ARRAY_BUFFER, 2*sizeof(vector4)*size+sizeof(declared_type)*size,
					 sizeof(declared_type)*size, replicatedMaterialsPS.data() );
	glBufferSubData( GL_ARRAY_BUFFER, 2*sizeof(vector4)*size+2*sizeof(declared_type)*size,
					 sizeof(declared_type)*size, replicatedMaterialsRefIdx.data() );
	glBufferSubData( GL_ARRAY_BUFFER, 2*sizeof(vector4)*size+3*sizeof(declared_type)*size,
					 sizeof(int)*size, replicatedMaterialsReflectiveness.data() );

	// set up vertex arrays
	glBindVertexArray( vao );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0,  0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0,  (void*)(sizeof(vector4)*size));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0,  (void*)(2*sizeof(vector4)*size));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0,  (void*)(2*sizeof(vector4)*size+sizeof(declared_type)*size));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 0,  (void*)(2*sizeof(vector4)*size+2*sizeof(declared_type)*size));
	glEnableVertexAttribArray(4);

	glVertexAttribIPointer(5, 1, GL_INT, 0,  (void*)(2*sizeof(vector4)*size+3*sizeof(declared_type)*size));
	glEnableVertexAttribArray(5);

	glBindBuffer(GL_ARRAY_BUFFER,0);
}

void Object::initializeBuffers(VkDevice dev, QVulkanDeviceFunctions* devFuncs, uint32_t memoryType) {

    this->dev = dev;
    this->devFuncs = devFuncs;


    quint8 *vertData = reinterpret_cast<quint8*>(
        mesh->replicatedVertices.data()
    );
    const int vertexByteSize = mesh->replicatedVertices.size() * sizeof(vector4);

    quint8* normalsData = reinterpret_cast<quint8*>(
        mesh->replicatedNormals.data()
        );
    const int normalsByteSize = mesh->replicatedNormals.size() * sizeof(vector4);

    const VkDeviceSize vertexDataSize = vertexByteSize + normalsByteSize;

    // create buffer to store vertices
    VkBufferCreateInfo bufInfo;
    memset(&bufInfo, 0, sizeof(bufInfo));
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    const VkDeviceSize vertexAllocSize = vertexDataSize;
    bufInfo.size = vertexAllocSize;
    bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VkResult err = devFuncs->vkCreateBuffer(dev, &bufInfo, nullptr, &buf);
    if (err != VK_SUCCESS) {
        qFatal("Failed to create buffer: %d", err);
    }

    VkMemoryRequirements memReq;
    devFuncs->vkGetBufferMemoryRequirements(dev, buf, &memReq);

    VkMemoryAllocateInfo memAllocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        memReq.size,
        memoryType
    };

    // allocate device (GPU) memory
    err = devFuncs->vkAllocateMemory(dev, &memAllocInfo, nullptr, &bufMem);
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate memory: %d", err);

    // bind the allocated memory to the just created buffer
    err = devFuncs->vkBindBufferMemory(dev, buf, bufMem, 0);
    if (err != VK_SUCCESS)
        qFatal("Failed to bind buffer memory: %d", err);

    // map device memory into application address space to fill the buffer
    quint8 *p;
    err = devFuncs->vkMapMemory(dev, bufMem, 0, memReq.size, 0, reinterpret_cast<void **>(&p));
    if (err != VK_SUCCESS)
        qFatal("Failed to map memory: %d", err);

    memcpy(p, vertData, vertexByteSize);
    memcpy(p + vertexByteSize, normalsData, normalsByteSize);

    devFuncs->vkUnmapMemory(dev, bufMem);
}



void Object::vkdraw() {

}


void Object::draw(std::unique_ptr<QGLShaderProgram> &program_)
{
	if(mesh)
	{
		GLuint diffuseLocation=program_->uniformLocation("diffuse");

		glUniform3f(diffuseLocation,diffuseColor.x(),diffuseColor.y(),diffuseColor.z());

		glBindVertexArray( vao );

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays( GL_TRIANGLES, 0, mesh->replicatedVertices.size());

		glBindVertexArray(0);
	}
}

void Object::setMesh(TriangleMesh *mesh)
{
	this->mesh = mesh;
}

Box3D Object::computeBoundingBox()
{
	vector3    pmin, pmax;
	Box3D capsa;

	pmin.x = mesh->vertices[0].x;
	pmin.y = mesh->vertices[0].y;
	pmin.z = mesh->vertices[0].z;
	pmax = pmin;
	for(uint i=1; i<mesh->vertices.size(); i++) {
		if(mesh->vertices[i].x <pmin[0])
			pmin[0] = mesh->vertices[i].x;
		if(mesh->vertices[i].y <pmin[1])
			pmin[1] = mesh->vertices[i].y;
		if(mesh->vertices[i].z <pmin[2])
			pmin[2] = mesh->vertices[i].z;

		if(mesh->vertices[i].x >pmax[0])
			pmax[0] = mesh->vertices[i].x;
		if(mesh->vertices[i].y >pmax[1])
			pmax[1] = mesh->vertices[i].y;
		if(mesh->vertices[i].z >pmax[2])
			pmax[2] = mesh->vertices[i].z;
	}
	capsa.pmin = pmin;
	capsa.a = pmax[0]-pmin[0];
	capsa.h = pmax[1]-pmin[1];
	capsa.p = pmax[2]-pmin[2];
	return capsa;
}

bool Object::isLoaded()
{
	return mesh;
}

int Object::loadOBJ(char *nameobj, char *namemtl)
{
	mesh = new TriangleMesh();
	QString fileName( nameobj );
	QFile file(fileName);
	if(file.exists() && file.open(QFile::ReadOnly | QFile::Text))
	{
		QVector<vector3> v, vn;
		QVector<vector2> vt;
		uint numVertices = 0, numFaces = 0, numNormals = 0, numMaterials = 0;
		uint currentMaterialIndex = 0;
		std::map<uint,vector3> faceNormals;

		while(!file.atEnd())
		{
			/*	Check to avoid memory failure or to complex structures */
			if(numVertices > VMAX || numNormals > VMAX || numFaces > FMAX || numMaterials > TMAX)
			{
				printf("The file object you are trying to load is too big\n");
				printf(" Ruttine EXIT \n");
				return 1;
			}

			QString line = file.readLine().trimmed();
			QStringList lineParts = line.split(QRegularExpression("\\s+"));
			if(lineParts.count() > 0)
			{
				// if it’s a comment
				if(lineParts.at(0).compare("#", Qt::CaseInsensitive) == 0)
				{
					qDebug() << line.remove(0, 1).trimmed();
				}
				// if it’s a material
				else if(lineParts.at(0).compare("usemtl", Qt::CaseInsensitive) == 0)
				{
					// We search the material in the vector of materials.
					// If it's not in the vector, we added it to the back of the vector
					auto materialName = lineParts.at(1);
					auto materialFound = std::find_if( materials.begin(), materials.end(),
							[&](Material mat){ return mat.namemat == materialName; } );
					// Index of the material in the vector or the new one that will be added to the back
					currentMaterialIndex = std::distance(materials.begin(), materialFound);

					if( materialFound == materials.end() )
					{
						materials.push_back( Material( materialName ) );
					}
				}
				// if it’s a vertex position (v)
				else if(lineParts.at(0).compare("v", Qt::CaseInsensitive) == 0)
				{
					mesh->vertices.push_back(vector3(lineParts.at(1).toDouble(),lineParts.at(2).toDouble(),
							lineParts.at(3).toDouble()));
				}
				// if it’s a normal (vn)
				else if(lineParts.at(0).compare("vn", Qt::CaseInsensitive) == 0)
				{
					mesh->faceNormals.push_back(vector3(lineParts.at(1).toDouble(),lineParts.at(2).toDouble(),
							lineParts.at(3).toDouble()));
				}
				// if it’s face data (f)
				// there’s an assumption here that faces are all triangles
				else if(lineParts.at(0).compare("f", Qt::CaseInsensitive) == 0)
				{
					/*	STORE on variable faces the vertex/normal (v1,v2,v3)/(n1) */
					Triangle t;

					/*
					 * We accept the faces with the next formats:
					 *   "f vtx1/txt1/nn1 vtx2/txt2/nn2 vtx3/txt3/nn3\n"
					 *   "f vtx1//nn1 vtx2//nn2 vtx3//nn3\n"
					 */

					t.v1 = lineParts.at(1).split("/").at(0).toInt() - 1;
					t.v2 = lineParts.at(2).split("/").at(0).toInt() - 1;
					t.v3 = lineParts.at(3).split("/").at(0).toInt() - 1;
					uint nn1 = lineParts.at(1).split("/").at(2).toInt() - 1;
					uint nn2 = lineParts.at(2).split("/").at(2).toInt() - 1;
					uint nn3 = lineParts.at(3).split("/").at(2).toInt() - 1;

					/*	CHECK that all normal-vectors in face are the same */
					if(nn1 != nn2 || nn1 != nn3 || nn2 != nn3)
					{
						printf("There is a problem with the nn vectors defining the triangle (%d %d %d)!!\n",
								nn1, nn2, nn3);
						return 1;
					}

					t.nn = nn1;

					/* Compute and STORE the face Area */
					vector3 v1, v2;
					for(uint i = 0; i < 3; i++){
						v1[i] = mesh->vertices[t.v2][i] - mesh->vertices[t.v1][i];
						v2[i] = mesh->vertices[t.v3][i] - mesh->vertices[t.v1][i];
					}
					vector3 n;
					n[0] =  v1[1]*v2[2] - v2[1]*v1[2];
					n[1] = -v1[0]*v2[2] + v2[0]*v1[2];
					n[2] =  v1[0]*v2[1] - v2[0]*v1[1];
					t.A = sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2])/2.;

					faceNormals[nn1] = Common::normalize(n);

					/*	STORE the material index */
					t.rf = currentMaterialIndex;
					mesh->faces.push_back(t);
				}
			}
		}
		file.close();

		// In case normals are not loaded or have not the same size as 'faceNormals', we use 'faceNormals'
		if( mesh->faceNormals.empty() || mesh->faceNormals.size() < faceNormals.size() )
		{
			mesh->faceNormals.clear();
			for(uint i = 0; i < faceNormals.size(); ++i)
			{
				mesh->faceNormals.emplace_back(faceNormals[i]);
			}
		}
	}
	else
	{
		printf("There is a problem loading the OBJ file!!\n");
		return 1;
	}

	std::vector<bool> foundMaterials( materials.size(), false );

	QString materialFileName( namemtl );
	qDebug() << "Mat file name:" << materialFileName;
	QFile materialFile(materialFileName);
	if(materialFile.exists() && materialFile.open(QFile::ReadOnly | QFile::Text))
	{
		while(!materialFile.atEnd())
		{
			QString line = materialFile.readLine().trimmed();
			QStringList lineParts = line.split(QRegularExpression("\\s+"));
			if(lineParts.count() > 0 && lineParts.at(0).compare("newmtl", Qt::CaseInsensitive) == 0)
			{
				auto materialName = lineParts.at(1);
				auto materialFound = std::find_if( materials.begin(), materials.end(),
						[&](Material mat){ return mat.namemat == materialName; } );
				// Index of the material in the vector or the new one that will be added to the back
				uint currentMaterialIndex = std::distance(materials.begin(), materialFound);

				if( materialFound == materials.end() )
				{
					printf(" material on .mtl file that is not on .obj (%s)!!\n", materialName.toStdString().data() );
					return 1;
				}

				foundMaterials[ currentMaterialIndex ] = true;

				// The next lines must specify the Kd and Ks of the material
				if( materialFile.atEnd() )
				{
					printf(" material %s on .mtl file must define Kd value in the next line of 'newmtl'!!\n",
						   materialName.toStdString().data());
					return 1;
				}
				const QString lineKd = materialFile.readLine().trimmed();
				const auto linePartsKd = lineKd.split(QRegularExpression("\\s+"));
				if(linePartsKd.count() > 0 && linePartsKd.at(0).compare("Kd", Qt::CaseInsensitive) == 0)
				{
					materials[currentMaterialIndex].pd = linePartsKd.at(1).toDouble();
				}
				else
				{
					printf(" material %s on .mtl file must define Kd value in the next line of 'newmtl'!!\n",
						   materialName.toStdString().data());
					return 1;
				}

				if( materialFile.atEnd() )
				{
					printf(" material %s on .mtl file must define Kd value in the next line of 'newmtl'!!\n",
						   materialName.toStdString().data());
					return 1;
				}
				const QString lineKs = materialFile.readLine().trimmed();
				const auto linePartsKs = lineKs.split(QRegularExpression("\\s+"));
				if(linePartsKs.count() > 0 && linePartsKs.at(0).compare("Ks", Qt::CaseInsensitive) == 0)
				{
					materials[currentMaterialIndex].ps = linePartsKs.at(1).toDouble();
				}
				else
				{
					printf(" material %s on .mtl file must define Ks value in the next line of 'Kd'!!\n",
						   materialName.toStdString().data());
					return 1;
				}
			}
		}
		materialFile.close();
	}

	/*Final Check, all the materials defined in .obj have to be in .mtl */
	for( uint i = 0; i<materials.size(); ++i)
	{
		if(!foundMaterials[i])
		{
			printf("Material %d (%s) is not filled in!!\n", i, materials[i].namemat.toStdString().data());
			return -1;
		}
	}

	/* Write Materials and Properties on screen. */
	for(uint i = 0; i < materials.size(); ++i){
		printf("%d  %s  ps = %Lf pd = %Lf \n", i, materials[i].namemat.toStdString().data(),
			   (long double) materials[i].ps, (long double) materials[i].pd);
	}

	mesh->computeBoundingBox();

	Eigen::Vector3f center = (mesh->max_+mesh->min_)/2.0f;
	vector3 qCenter = vector3(center[0],center[1],center[2]);
	for(uint i=0; i<mesh->vertices.size(); i++) mesh->vertices[i]-=qCenter;

	mesh->computeBoundingBox();

	mesh->prepareDataToGPU();
	prepareMaterialsToGPU();

	return 0;
}

TriangleMesh *Object::getMesh()
{
	return mesh;
}

Material &Object::getMaterial(int i)
{
	return materials[i];
}

void Object::setReflectivenessInMaterials(Reflectiveness r)
{
	for(uint i=0; i<materials.size();i++){
		materials[i].r=r;
	}
}

void Object::setRefractiveIndexInMaterials(const precision::value_type& refIdx)
{
	for(uint i=0; i<materials.size();i++){
		materials[i].refIdx=refIdx;
	}
}

void Object::sendObjectToGPU(std::unique_ptr<QGLShaderProgram> &program)
{
	this->sendMaterialsToGPU(program);
	mesh->sendMeshToGPU(program);
}

void Object::prepareMaterialsToGPU(){

	replicatedMaterialsPD.clear();
	replicatedMaterialsPS.clear();
	replicatedMaterialsReflectiveness.clear();

	for(uint i=0;i<mesh->faces.size();i++){
		for(uint j=0; j<3; j++){
			replicatedMaterialsPD.push_back(materials[mesh->faces[i].rf].pd);
			replicatedMaterialsPS.push_back(materials[mesh->faces[i].rf].ps);
			replicatedMaterialsReflectiveness.push_back((materials[mesh->faces[i].rf].r));
			replicatedMaterialsRefIdx.push_back(materials[mesh->faces[i].rf].refIdx);
		}
	}
}

QVector3D Object::getDiffuseColor() const
{
	return diffuseColor;
}

void Object::setDiffuseColor(const QVector3D &value)
{
	diffuseColor = value;
}

int Object::getNumMaterials()
{
	return materials.size();
}

void Object::sendMaterialsToGPU(std::unique_ptr<QGLShaderProgram> &program)
{
	GLuint idNumMaterials = program->uniformLocation("numMaterials");
	int numMaterials = this->materials.size();
	glUniform1i(idNumMaterials,numMaterials);

	for(int i=0; i< numMaterials;i++){
		GLuint idPs = program->uniformLocation(QString("materials[%1].ps").arg( i ));
		GLuint idPd = program->uniformLocation(QString("materials[%1].pd").arg( i ));
		GLuint idReflectiveness = program->uniformLocation(QString("materials[%1].reflectiveness").arg( i ));
		GLuint idRefIdx = program->uniformLocation(QString("materials[%1].refIdx").arg( i ));

		glUniform1f(idPs,(float)this->materials[i].ps);
		glUniform1f(idPd,(float)this->materials[i].pd);
		glUniform1f(idRefIdx,this->materials[i].refIdx);
		glUniform1i(idReflectiveness,this->materials[i].r);
	}
}

void Object::destroyBuffers() {
    if (buf) {
        devFuncs->vkDestroyBuffer(dev, buf, nullptr);
        buf = VK_NULL_HANDLE;
    }

    if (bufMem) {
        devFuncs->vkFreeMemory(dev, bufMem, nullptr);
        bufMem = VK_NULL_HANDLE;
    }
}

