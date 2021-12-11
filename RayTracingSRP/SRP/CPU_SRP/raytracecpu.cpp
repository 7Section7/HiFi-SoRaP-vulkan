#include "raytracecpu.h"

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

int RayTraceCPU::getNumSecondaryRays() const
{
    return numSecondaryRays;
}

void RayTraceCPU::setNumSecondaryRays(int value)
{
    numSecondaryRays = value;
}

int RayTraceCPU::getNumDiffuseRays() const
{
    return numDiffuseRays;
}

void RayTraceCPU::setNumDiffuseRays(int value)
{
    numDiffuseRays = value;
}

int RayTraceCPU::getReflectionType() const
{
    return reflectionType;
}

void RayTraceCPU::setReflectionType(int value)
{
    reflectionType = value;
}

RayTraceCPU::RayTraceCPU()
{
    reflectionType=Reflective;
}

QVector3D product(QVector3D a, QVector3D b){
    return QVector3D(a.x()*b.x(),a.y()*b.y(),a.z()*b.z());
}

QVector3D reflect(QVector3D dir,QVector3D normal){
    QVector3D r = dir-2.0f*QVector3D::dotProduct(dir,normal)*normal;
    return r.normalized();
}
QVector3D RayTraceCPU::randomInSphere()  {
    QVector3D p;
    do {
        p = 2.0f*QVector3D(drand48(),drand48(),drand48()) - QVector3D(1,1,1);
    } while (p.length() >=  1.0f);
    return p;
}

bool refract(const QVector3D& v, const QVector3D& n, float ni_over_nt, QVector3D& refracted) {
        QVector3D uv = v.normalized();
        float dt = QVector3D::dotProduct(uv, n);
        float discriminant = 1.0 - ni_over_nt*ni_over_nt*(1-dt*dt);
        if (discriminant > 0) {
            refracted = ni_over_nt*(uv - n*dt) - n*sqrt(discriminant);
            return true;
        }
        else
            return false;
}

void RayTraceCPU::computeStepSRP(double XS[], QVector3D &force, double RS[], double V1[], double V2[])
{
    double FSRP[3], costh, pixel[3], Pint[3];
    int ihit, rhit, nhit;
    double depmin;
    double xtot, ytot, xpix, ypix, d;
    double Apix;
    int NT,r,ix,iy;

    std::vector<int> auxhit;
    std::vector<double>depthit;

    double PS = ctt/msat; ///msat;

    d=safeDistance;
    nx=this->nx;
    ny=this->ny;
    cm[0]=this->cm.x();
    cm[1]=this->cm.y();
    cm[2]=this->cm.z();
    xtot=this->xtot;
    ytot=this->ytot;

    /*	Pixel Size */
    xpix = xtot/nx; ypix = ytot/ny;
    Apix = xpix*ypix;

    TriangleMesh *mesh = satellite->getMesh();
    NT = mesh->faces.size();

    Eigen::Vector3f diff = mesh->max_-mesh->min_;
    float diagonalDiff = diff.norm();
    float errorMargin = 0.1f;
    float distanceWindow = diagonalDiff+errorMargin;;

    xtot = distanceWindow;
    ytot = distanceWindow;
    xpix = xtot/nx;
    ypix = ytot/ny;
    Apix = xpix * ypix;

    d=diagonalDiff;

    auxhit.resize(NT);
    depthit.resize(NT);
    FSRP[0] = 0.; FSRP[1] = 0.; FSRP[2] = 0.;

    int aux=0;

    double v1_aux[3];
    v1_aux[0]=V1[0]; v1_aux[1]=V1[1]; v1_aux[2]=V1[2];

    double v2_aux[3];
    v2_aux[0]=V2[0]; v2_aux[1]=V2[1]; v2_aux[2]=V2[2];

    /* For each pixel in the array */
    for(ix = 1; ix <= nx; ix++)
    {
        for(iy = 1; iy <= ny; iy++)
        {
            /* pixel (ix,iy) in the grid */
            pixel[0] = ((ix-0.5)*xpix - xtot/2.)*V1[0] + ((iy-0.5)*ypix - ytot/2.)*V2[0] + d*RS[0];
            pixel[1] = ((ix-0.5)*xpix - xtot/2.)*V1[1] + ((iy-0.5)*ypix - ytot/2.)*V2[1] + d*RS[1];
            pixel[2] = ((ix-0.5)*xpix - xtot/2.)*V1[2] + ((iy-0.5)*ypix - ytot/2.)*V2[2] + d*RS[2];

            QVector3D pixelForce = computePixelForce(XS,Apix,pixel);

            FSRP[0]+= pixelForce.x();
            FSRP[1]+= pixelForce.y();
            FSRP[2]+= pixelForce.z();
            aux++;

        }
   }
    force =PS* QVector3D(FSRP[0], FSRP[1], FSRP[2]);
}

QVector3D RayTraceCPU::computePixelForce(double XS[], double Apix, double pixel[])
{
    this->pixel = QVector3D(pixel[0],pixel[1],pixel[2]);

    double dir[3];
    dir[0] = XS[0];         dir[1] = XS[1];         dir[2] = XS[2];

    double point[3];
    point[0] = pixel[0];    point[1] = pixel[1];    point[2] = pixel[2];

    QVector3D totalForce;
    double importance[3];
    importance[0]=1;        importance[1]=1;        importance[2]=1;

    totalForce = rayTrace(Apix,point,dir,importance,numSecondaryRays);
    //totalForce = computeFinalForce(Apix,point,dir,importance,numSecondaryRays+1);

    return totalForce;
}

QVector3D RayTraceCPU::rayTrace(double Apix,double point[], double dir[], double importance[],int numSecondaryRays){

    QVector3D null(0,0,0);

    if(numSecondaryRays < 0){
        return null;
    }
    QVector3D imp(importance[0],importance[1],importance[2]);
    if(imp.length()<1.e-5){
        return null;
    }

    double pointInt[3];
    int rhit = hit(point,dir,pointInt);

    if(rhit != -1){
        QVector3D totalForce;

        QVector3D localForce = computeForce(rhit,dir,Apix);
        totalForce = product(localForce,imp);

        if(numSecondaryRays == 0)
            return totalForce;

        //PS contribution
        QVector3D forcePS = QVector3D(0,0,0);
        if(reflectionType!=Lambertian){
            double psImportance[3],psDir[3], psPointInt[3];
            psDir[0]=dir[0];    psDir[1]=dir[1];    psDir[2]=dir[2];
            psPointInt[0]=pointInt[0];    psPointInt[1]=pointInt[1];    psPointInt[2]=pointInt[2];

            scatter(psPointInt,rhit,psDir,psImportance, Reflective);

            psImportance[0]*=importance[0]; psImportance[1]*=importance[1]; psImportance[2]*=importance[2];

            forcePS = rayTrace(Apix,psPointInt,psDir,psImportance,numSecondaryRays-1);

        }

        //PD contribution
        QVector3D forcePD = QVector3D(0,0,0);

        int kernelSize = numDiffuseRays;
        for(int i=0; i< kernelSize; i++){
            double pdImportance[3],pdDir[3], pdPointInt[3];
            pdDir[0]=dir[0];    pdDir[1]=dir[1];    pdDir[2]=dir[2];
            pdPointInt[0]=pointInt[0];    pdPointInt[1]=pointInt[1];    pdPointInt[2]=pointInt[2];

            scatter(pdPointInt,rhit,pdDir,pdImportance, Lambertian);

            pdImportance[0]*=importance[0]; pdImportance[1]*=importance[1]; pdImportance[2]*=importance[2];
            forcePD += rayTrace(Apix,pdPointInt,pdDir,pdImportance,numSecondaryRays-1);
        }
        if(kernelSize>0)
            forcePD = 1.0f/kernelSize *forcePD;
        else
            forcePD = QVector3D(0,0,0);

        totalForce += forcePS + forcePD;
        return totalForce;
    }
    else{
        return null;
    }
}

float PHI = 1.61803398874989484820459 * 0.1; // Golden Ratio
float PI  = 3.14159265358979323846264 * 0.1; // PI
float SQ2 = 1.41421356237309504880169 * 10000.0; // Square Root of Two

float goldNoise( vec2 coordinate,float seed){

    float value=tan(distance(vec3(coordinate*(seed+PHI),0), vec3(PHI, PI,0)))*SQ2;
    return value - floor(value);
}

float rand(vec2 n) {
        float value=sin(dot(n, vec2(12.9898f, 4.1414f))) * 43758.5453f;
        return value - floor(value);
}

QVector3D RayTraceCPU::randomInSphere(QVector3D hitPoint){

    QVector3D p;
    do{
        QVector3D aux= QVector3D(goldNoise(vec2(aux.x(),aux.y()),seed), goldNoise(vec2(aux.y(),aux.z()),seed), goldNoise(vec2(aux.x(),aux.z()),seed));
        p= 2.0f*aux - QVector3D(1.0f,1.0f,1.0f);
        seed =  seed*seed - floor(seed*seed);
    }while(p.length()>=1.0f);
    return p;
}

void copyHitInfo(HitInfo &hitInfo, HitInfo &outHitInfo){
    outHitInfo.currentImportance=hitInfo.currentImportance;
    outHitInfo.currentL = hitInfo.currentL;
    outHitInfo.currentN = hitInfo.currentN;
    outHitInfo.currentPS = hitInfo.currentPS;
    outHitInfo.currentPD = hitInfo.currentPD;
    outHitInfo.currentRefIdx = hitInfo.currentRefIdx;
    outHitInfo.currentReflectiveness= hitInfo.currentReflectiveness;
    outHitInfo.currentPoint= hitInfo.currentPoint;
}

void RayTraceCPU::scatter( HitInfo &hitInfo){

    QVector3D point = hitInfo.currentPoint;
    QVector3D dir = hitInfo.currentL;
    float ps=hitInfo.currentPS;
    float pd = hitInfo.currentPD;
    float refIdx = hitInfo.currentRefIdx;
    int reflectiveness = hitInfo.currentReflectiveness;
    QVector3D N = hitInfo.currentN;

    if(reflectiveness == 0){ //Reflective

        QVector3D reflected = reflect(dir,N);
        reflected.normalize();
        point = point +0.1f*reflected;

        hitInfo.currentPoint = point;
        hitInfo.currentL=reflected;
        hitInfo.currentImportance = QVector3D(ps,ps,ps);
    }
    else { //Lambertian

        QVector3D target = point + N + randomInSphere();//point);
        QVector3D reflected = target - point;
        reflected.normalize();
        point = point +0.1f*reflected;

        hitInfo.currentPoint = point;
        hitInfo.currentL=reflected;
        hitInfo.currentImportance = QVector3D(pd,pd,pd);
    }
}

QVector3D RayTraceCPU::computeScatteredForce(HitInfo &hitInfo, float Apix){
    QVector3D totalForce = QVector3D(0,0,0);
    QVector3D importance = hitInfo.currentImportance;

    scatter(hitInfo);
    QVector3D hitPoint;

    double pointInt[3];
    pointInt[0]=hitInfo.currentPoint.x(); pointInt[1]=hitInfo.currentPoint.y(); pointInt[2]=hitInfo.currentPoint.z();
    double xs[3];
    xs[0]=hitInfo.currentL.x(); xs[1]=hitInfo.currentL.y(); xs[2]=hitInfo.currentL.z();
    double pointAux[3];

    int triangleIdx = hit(pointInt,xs,pointAux);
    if(triangleIdx!=-1){
        hitInfo.currentImportance = product(hitInfo.currentImportance,importance);
        QVector3D localForce = computeForce(triangleIdx,xs,Apix);
        totalForce+= product(localForce,hitInfo.currentImportance);

        TriangleMesh *mesh= this->satellite->getMesh();
        hitInfo.currentN = mesh->faceNormals[mesh->faces[triangleIdx].nn];


        Material m = satellite->getMaterial(mesh->faces[triangleIdx].rf);
        hitInfo.currentPS = m.ps;
        hitInfo.currentPD = m.pd;
        hitInfo.currentRefIdx = m.refIdx;

        hitInfo.currentPoint = QVector3D(pointAux[0],pointAux[1],pointAux[2]);

    }
    else{
        hitInfo.currentImportance=importance;
    }

    return totalForce;
}
QVector3D RayTraceCPU::computeFinalForce(double Apix,double point[], double dir[], double import[],int numSecondaryRays){

    QVector3D totalForce;
    bool foundNextHit = true;

    seed=5;

    QVector3D importance = QVector3D(1.f,1.f,1.f);

    HitInfo hitInfo;
    hitInfo.currentImportance =importance;
    hitInfo.currentPoint = QVector3D(point[0],point[1],point[2]);
    hitInfo.currentL = QVector3D(dir[0],dir[1],dir[2]);

    QVector3D hitPoint;
    double pointAux[3];

    int triangleIdx = hit(point,dir,pointAux);
    if(triangleIdx!=-1){
        hitPoint = QVector3D(pointAux[0],pointAux[1],pointAux[2]);
        pixel = hitPoint;

        double xs[3];
        xs[0]=hitInfo.currentL.x(); xs[1]=hitInfo.currentL.y(); xs[2]=hitInfo.currentL.z();
        QVector3D force = computeForce(triangleIdx,xs,Apix);
        totalForce = product(force,importance);

        if(numSecondaryRays == 0)
            return totalForce;

        RayTraceStep steps[numSecondaryRays];

        for(int i=0; i< numSecondaryRays; i++){
            steps[i].currentStep=i;
            if(reflectionType==Lambertian)
                steps[i].currentComputation=1;
            else
                steps[i].currentComputation=0;
            steps[i].force =QVector3D(0,0,0);
            steps[i].hitInfo.currentReflectiveness=0;
        }
        TriangleMesh *mesh = satellite->getMesh();
        Material m = this->satellite->getMaterial(mesh->faces[triangleIdx].rf);
        steps[0].hitInfo.currentImportance=importance;
        steps[0].hitInfo.currentL=hitInfo.currentL;
        steps[0].hitInfo.currentPoint=hitPoint;
        steps[0].hitInfo.currentPS=m.ps;
        steps[0].hitInfo.currentPD=m.pd;
        steps[0].hitInfo.currentN =mesh->faceNormals[mesh->faces[triangleIdx].nn];

        int diffuseRays = numDiffuseRays;

        bool stop = false;
        int i=0;
        int currentStep=0;
        while(!stop && i<1000){

            if(steps[currentStep].currentComputation<1+diffuseRays){
                HitInfo hitInfoAux;
                copyHitInfo(steps[currentStep].hitInfo,hitInfoAux);

                int r; float aux;
                if(steps[currentStep].currentComputation<1)  {r=0; aux=1;}
                else    {r=2; aux=1.0f/diffuseRays;}

                hitInfoAux.currentReflectiveness=r;
                QVector3D f = computeScatteredForce(hitInfoAux,Apix);
                steps[currentStep].force += aux*f;

                if(f.length()<1.e-5 || hitInfoAux.currentImportance.length()<1.e-5 || currentStep == numSecondaryRays-1){
                    steps[currentStep].currentComputation++;
                }
                else{

                    copyHitInfo(hitInfoAux,steps[currentStep+1].hitInfo);
                    if(reflectionType==Lambertian)
                        steps[currentStep+1].currentComputation=1;
                    else
                        steps[currentStep+1].currentComputation=0;
                    steps[currentStep+1].force=QVector3D(0,0,0);

                    currentStep++;
                }
            }
            else{ //Move backward.

                if(currentStep==0)  stop=true;
                else{
                    float aux2=1;
                    if(steps[currentStep].hitInfo.currentReflectiveness==2)
                        aux2=1.0f/diffuseRays;
                    steps[currentStep-1].force += aux2*steps[currentStep].force;

                    currentStep--;
                    steps[currentStep].currentComputation++;
                }
            }

            i++;
        }
        if(steps[0].force.length()>1.e-9){
            int i=1;
        }
    }
    return totalForce;
}

QVector3D RayTraceCPU::computeForce(int rhit, double XS[], double Apix)
{
    TriangleMesh *mesh = satellite->getMesh();
    double ps, pd;
    double costh = XS[0]*mesh->faceNormals[mesh->faces[rhit].nn].x() + XS[1]*mesh->faceNormals[mesh->faces[rhit].nn].y() + XS[2]*mesh->faceNormals[mesh->faces[rhit].nn].z();
    ps = satellite->getMaterial(mesh->faces[rhit].rf).ps; 	pd = satellite->getMaterial(mesh->faces[rhit].rf).pd;
    double forceX= -Apix*( (1. - ps)*XS[0] + 2.*( ps*costh + pd/3. )*mesh->faceNormals[mesh->faces[rhit].nn].x() );
    double forceY= -Apix*( (1. - ps)*XS[1] + 2.*( ps*costh + pd/3. )*mesh->faceNormals[mesh->faces[rhit].nn].y() );
    double forceZ= -Apix*( (1. - ps)*XS[2] + 2.*( ps*costh + pd/3. )*mesh->faceNormals[mesh->faces[rhit].nn].z() );

    return QVector3D(forceX,forceY,forceZ);
}

void RayTraceCPU::scatter(double pointInt[],int rhit, double xs[], double importance[], Reflectiveness r)
{
    TriangleMesh *mesh = satellite->getMesh();
    Material m = satellite->getMaterial(mesh->faces[rhit].rf);

    QVector3D dir(xs[0],xs[1],xs[2]);
    QVector3D point(pointInt[0],pointInt[1],pointInt[2]);
    QVector3D normal = mesh->faceNormals[mesh->faces[rhit].nn];

    if(r == Reflective){
        QVector3D reflected = reflect(dir, normal);
        reflected.normalize();

        point = point + 1.e-4f*reflected;

        pointInt[0] = point.x(); pointInt[1] = point.y(); pointInt[2] = point.z();
        xs[0] = reflected.x(); xs[1] = reflected.y(); xs[2] = reflected.z();
        importance[0] = m.ps; importance[1] = m.ps; importance[2] = m.ps;
    }
    else if( r == Lambertian){
        QVector3D target = point+normal+randomInSphere();
        QVector3D reflected = target-point;
        reflected.normalize();

        point = point + 1.e-4f*reflected;

        pointInt[0] = point.x(); pointInt[1] = point.y(); pointInt[2] = point.z();
        xs[0] = reflected.x(); xs[1] = reflected.y(); xs[2] = reflected.z();
        importance[0] = m.pd; importance[1] = m.pd; importance[2] = m.pd;

    }
}
