#version 330

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#define Reflective 0
#define Transparent 1
#define Lambertian 2

layout (location = 0)out vec4 gAlbedo;
layout (location = 1) out vec3 gNormal;

smooth in vec4 worldNormal;
smooth in vec4 worldVertex;

in float fragmentPD;
in float fragmentPS;
in float fragmentRefIdx;
flat in int fragmentReflectiveness;

uniform vec3 lightDirection;

uniform vec3 worldCamPos;
uniform mat4 model;

uniform int debugMode;

uniform vec3 diffuse;

uniform int numMaterials;
uniform int numVertices;
uniform int numTriangles;

uniform int numSecondaryRays;
uniform int numDiffuseRays;

uniform sampler1D texTriangles;
uniform sampler1D texVertices;
uniform sampler1D texMaterials1;
uniform sampler1D texMaterials2;

uniform int facesTextureSize;


uniform int numNoiseValues;
uniform sampler1D texNoise;

struct Triangle{
    float v1;
    float v2;
    float v3;
    float mat;
};

struct Material{
    float ps;
    float pd;
    float refIdx;
    int reflectiveness;
};

struct HitInfo{
    vec3 currentPoint;
    vec3 currentL;
    vec3 currentN;
    vec3 currentImportance;
    float currentPS;
    float currentPD;
    float currentRefIdx;
    int currentReflectiveness;
};

struct RayTraceStep{
  HitInfo hitInfo;

  vec3 force;
  int currentStep;
  int currentComputation;
};

vec3 product(vec3 a, vec3 b){
    return vec3(a.x*b.x,a.y*b.y,a.z*b.z);
}
Triangle getTriangle(int triangleIdx){

    float texCoord = triangleIdx;
    texCoord/=facesTextureSize;

    vec4 triangleInfo = texture(texTriangles,texCoord+1.e-6f);

    Triangle t;
    t.v1 = triangleInfo.r;
    t.v2 = triangleInfo.g;
    t.v3 = triangleInfo.b;
    t.mat =triangleInfo.a;

    return t;
}
//matIdx is supposed to be already between 0 and 1.
Material getMaterial(float matIdx){
    Material m;

    vec3 materialInfo1 = texture(texMaterials1, matIdx+1.e-4f).xyz;

    m.ps = materialInfo1.r;
    m.pd = materialInfo1.g;
    m.refIdx = 1.0f / materialInfo1.b;

    float reflectiveness = texture(texMaterials2,matIdx+1.e-4f).x;

    int ref;
    if(reflectiveness<0.25f) ref = 0;
    else if(reflectiveness>0.75f) ref = 2;
    else ref = 1;

    m.reflectiveness = ref;

    return m;
}

vec3 getVertex(float vId){
    vec3 v;

    vec3 vertexInfo = texture(texVertices,vId+1.e-4f).xyz;
    v.x = vertexInfo.x * 2 - 1;
    v.y = vertexInfo.y * 2 - 1;
    v.z = vertexInfo.z * 2 - 1;

    return v;
}

vec3 computeNormal(int triangleIdx){

    Triangle t = getTriangle(triangleIdx);

    vec3 v1 = getVertex(t.v1);
    vec3 v2 = getVertex(t.v2);
    vec3 v3 = getVertex(t.v3);

    vec3 dir1 = v2 - v1;
    vec3 dir2 = v3 - v1;
    vec3 N = normalize(cross(dir1,dir2));

    return N;
}

vec3 computeSRP(vec3 L, vec3 N, float ps, float pd, float Apix){
    float costh = dot(N, L);

    float FSRP0 = -Apix*( (1. - ps)*L.x + 2.*( ps*costh + pd/3. )*N.x );
    float FSRP1 = -Apix*( (1. - ps)*L.y + 2.*( ps*costh + pd/3. )*N.y );
    float FSRP2 = -Apix*( (1. - ps)*L.z + 2.*( ps*costh + pd/3. )*N.z );

    vec3 force = vec3(FSRP0,FSRP1, FSRP2);
    return force;
}

vec3 computeForce(int triangleIdx,vec3 L, float Apix){

    Triangle t = getTriangle(triangleIdx);
    Material m = getMaterial(t.mat);

    vec3 N = computeNormal(triangleIdx);
    float ps = m.ps;
    float pd = m.pd;

    vec3 force=computeSRP(L,N,ps,pd,Apix);
    return force;
}

bool hitTriangle(vec3 point, vec3 L, int triangleIdx, out vec3 hitPoint){

    float tMin=1.e-5f;
    float tMax= 10000000.f;

    Triangle t = getTriangle(triangleIdx);

    vec3 v1 = getVertex(t.v1);
    vec3 N = computeNormal(triangleIdx);

    float d= - dot(v1,N);
    vec4 vNormal = vec4(N,d);
    vec4 vPoint = vec4(point,1.f);
    vec4 L2 = vec4(L,0.f);

    float lowerPart = dot(vNormal, L2);
    if(abs(lowerPart)<1.e-5f)
        return false;

    float tValue = - dot(vNormal,vPoint) / lowerPart;

    if(tValue > tMin && tValue < tMax){

        vec3 v2 = getVertex(t.v2);
        vec3 v3 = getVertex(t.v3);

        vec3 intPoint = tValue*L + point;

        vec3 dir1 = v3-v1;
        vec3 dir2 = intPoint-v1;
        float alpha = 0.5f * length(cross(dir1,dir2));

        dir1 = v2-v3;
        dir2 = intPoint-v3;
        float beta = 0.5f * length(cross(dir1,dir2));

        dir1 = v1-v2;
        dir2 = intPoint-v2;
        float gamma = 0.5f * length(cross(dir1,dir2));

        dir1 = v3-v1;
        dir2 = v2-v1;
        float totalArea = 0.5f * length(cross(dir1,dir2));

        if(abs(alpha+beta+gamma-totalArea)<1.e-5f && alpha>0 && beta>0 && gamma>0){
            hitPoint = intPoint;
            return true;
        }

    }

    return false;

}

//Give the triangleId of the triangle that has the closest intersection to the ray.
int hit(vec3 point, vec3 L, out vec3 hitPoint){

    vec3 intPoint;
    float depMin = 10000000.f;
    int triangleIdx;
    int hitTriangleIdx = -1;

    for(triangleIdx=0; triangleIdx<numTriangles; triangleIdx++){

        if(hitTriangle(point,L,triangleIdx,intPoint)){

            vec3 diff = point-intPoint;
            float depHit = length(diff);

            if(depHit<depMin){
                depMin=depHit;
                hitTriangleIdx=triangleIdx;
                hitPoint=intPoint;
            }
        }
    }
    return hitTriangleIdx;

}

float rand(vec2 n) {
        return fract(sin(dot(n, vec2(12.9898f, 4.1414f))) * 43758.5453f);
}

float seed =0;

vec3 getRandomVector(){
    seed = mod(seed,numNoiseValues);
    float coord = seed/numNoiseValues;

    vec3 rand = texture(texNoise,coord).xyz;
    seed++;

    return rand;
}

vec3 randomInSphere(){

    vec3 p;
    do{
        p= 2.0f* getRandomVector() - vec3(1.0f,1.0f,1.0f);
    }while(length(p)>=1.0f);
    return p;
}


void scatter(inout HitInfo hitInfo){


    vec3 point = hitInfo.currentPoint;
    vec3 dir = hitInfo.currentL;
    float ps=hitInfo.currentPS;
    float pd = hitInfo.currentPD;
    float refIdx = hitInfo.currentRefIdx;
    int reflectiveness = hitInfo.currentReflectiveness;
    vec3 N = hitInfo.currentN;

    if(reflectiveness == 0){ //Reflective

        vec3 reflected = reflect(dir,N);
        normalize(reflected);
        point = point +1.e-2f*reflected;

        hitInfo.currentPoint = point;
        hitInfo.currentL=reflected;
        hitInfo.currentImportance = vec3(ps,ps,ps);
    }
    else{// if(reflectiveness == 2){ //Lambertian

        vec3 target = point + N + randomInSphere();
        vec3 reflected = target - point;
        normalize(reflected);
        point = point +1.e-2f*reflected;

        hitInfo.currentPoint = point;
        hitInfo.currentL=reflected;
        hitInfo.currentImportance = vec3(pd,pd,pd);

    }

}


void copyHitInfo(in HitInfo hitInfo,out HitInfo outHitInfo){
    outHitInfo.currentImportance=hitInfo.currentImportance;
    outHitInfo.currentL = hitInfo.currentL;
    outHitInfo.currentN = hitInfo.currentN;
    outHitInfo.currentPS = hitInfo.currentPS;
    outHitInfo.currentPD = hitInfo.currentPD;
    outHitInfo.currentRefIdx = hitInfo.currentRefIdx;
    outHitInfo.currentReflectiveness= hitInfo.currentReflectiveness;
    outHitInfo.currentPoint= hitInfo.currentPoint;
}


//Check if the ray intersects any triangle and get the closest one in order
//to compute the force on the intersection.
vec3 computeScatteredForce(inout HitInfo hitInfo, float Apix){
    vec3 totalForce = vec3(0,0,0);
    vec3 importance = hitInfo.currentImportance;
    scatter(hitInfo);
    vec3 hitPoint;

    int triangleIdx = hit(hitInfo.currentPoint,hitInfo.currentL,hitPoint);
    if(triangleIdx!=-1){
        hitInfo.currentImportance = product(hitInfo.currentImportance,importance);
        vec3 localForce = computeForce(triangleIdx,hitInfo.currentL,Apix);
        totalForce+= product(localForce,hitInfo.currentImportance);

        hitInfo.currentN = computeNormal(triangleIdx);

        Triangle t = getTriangle(triangleIdx);
        Material m = getMaterial(t.mat);
        hitInfo.currentPS = m.ps;
        hitInfo.currentPD = m.pd;
        hitInfo.currentRefIdx = m.refIdx;

        hitInfo.currentPoint = hitPoint;

    }
    else{
        hitInfo.currentImportance=importance;
    }


    return totalForce;
}

//This computes the SRP force for the fragment.
//In particular, it casts several additional rays when one of the primary
//rays intersects with the mesh.
vec3 computeFinalForce(vec3 point,vec3 L, vec3 N, float ps, float pd, float refIdx,int reflectiveness, float Apix){

    vec3 totalForce;
    bool foundNextHit = true;

    vec3 currentN;
    vec3 currentL = normalize(L);
    vec3 currentPoint = point;
    float currentPs,currentPd,currentRefIdx;
    int currentReflectiveness;
    vec3 hitPoint;

    vec3 importance = vec3(1.f,1.f,1.f);

    vec3 pointAux = point -1.e-2f*currentL;
    int triangleIdx = hit(pointAux,currentL,currentPoint);
    if(triangleIdx==-1) return vec3(0,0,0);

    Triangle t = getTriangle(triangleIdx);
    Material m = getMaterial(t.mat);

    currentN = computeNormal(triangleIdx);

    currentPs = m.ps;
    currentPd = m.pd;
    currentRefIdx = m.refIdx;
    currentReflectiveness = m.reflectiveness;

    vec3 force = computeForce(triangleIdx,currentL,Apix);
    totalForce = product(force,importance);

    if(numSecondaryRays == 0)
        return totalForce;

    //For optimization, we decided to create a stack 'steps'
    //in order to save the accumulated forces of each iteration.
    RayTraceStep steps[20];

    for(int i=0; i< numSecondaryRays; i++){
        steps[i].currentStep=i;
        steps[i].currentComputation=0;
        steps[i].force =vec3(0,0,0);
        steps[i].hitInfo.currentReflectiveness=0;
    }
    steps[0].hitInfo.currentImportance=importance;
    steps[0].hitInfo.currentL=L;
    steps[0].hitInfo.currentPoint=currentPoint;
    steps[0].hitInfo.currentPS=currentPs;
    steps[0].hitInfo.currentPD=currentPd;
    steps[0].hitInfo.currentN =currentN;

    int diffuseRays = numDiffuseRays;

    bool stop = false;
    int i=0;
    int currentStep=0;
    while(!stop && i<1000){ //i is a timeout counter.

        if(steps[currentStep].currentComputation<1+diffuseRays){

            //Get the information from the previous iteration.
            HitInfo hitInfoAux;
            copyHitInfo(steps[currentStep].hitInfo,hitInfoAux);

            //Depending on the current step of computation we are in,
            //we compute a secondary ray based on specular or diffuse reflection.
            int r; float aux;
            if(steps[currentStep].currentComputation<1)  {r=0; aux=1;}
            else    {r=2; aux=1.0f/diffuseRays;}

            //Compute force on the intersection of the ray and add it to the accumulated force.
            hitInfoAux.currentReflectiveness=r;
            vec3 f = computeScatteredForce(hitInfoAux,Apix);
            steps[currentStep].force += aux*f;

            if(length(f)<1.e-5 || length(hitInfoAux.currentImportance)<1.e-5 || currentStep == numSecondaryRays-1){
                //This secondary ray didn't apport anything, so we move forward on the next secondary ray computation.
                //(next iteration).
                steps[currentStep].currentComputation++;
            }
            else{
                //Or, the secondary ray hit a triangle, then we need to compute new secondary rays from this intersected point.
                copyHitInfo(hitInfoAux,steps[currentStep+1].hitInfo);
                steps[currentStep+1].currentComputation=0;
                steps[currentStep+1].force=vec3(0,0,0);

                currentStep++;
            }
        }

        else{//Move backward.

            //Once all the secondary rays were computed, we move backward in the stack.
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

    totalForce+= steps[0].force;

    return totalForce;

}

void main (void) {

    float ps=fragmentPS;
    float pd=fragmentPD;
    vec3 N = normalize(worldNormal.xyz);
    vec3 L = normalize(lightDirection.xyz);
    float Apix = 1.0f;//(4/50f) * (4/50f);
    float Apix2 = 1.0f;

    vec3 force = computeFinalForce(worldVertex.xyz,L,N,ps,pd,fragmentRefIdx,fragmentReflectiveness,Apix);

    vec3 hitPoint;
    vec3 currentPoint = worldVertex.xyz - 0.5f*L;
    int triangleIdxAux = hit(currentPoint,L,hitPoint);
    if(triangleIdxAux!=-1){
        gNormal = 0.5f*(N+vec3(1.0f,1.0f,1.0f));

    }

    if(length(force)<0.0001) {
        gAlbedo = vec4(0,0,0,1);
        //discard;
        return;
    }

    vec3 normalizedForce = 0.125f*vec3(force.x+4,force.y+4,force.z+4);
    gAlbedo = vec4(Apix2*normalizedForce,1.0f);


    if(debugMode == 2){ //Show face normals
        if(N.x <0)
            gAlbedo = vec4(1,0,0,1);
        else if(N.x >0)
            gAlbedo = vec4(0.5,0,0,1);
        else if(N.y <0)
            gAlbedo = vec4(0,1,0,1);
        else if(N.y >0)
            gAlbedo = vec4(0,0.5,0,1);
        else if(N.z <0)
            gAlbedo = vec4(0,0,0.5,1);
        else if(N.z >0)
            gAlbedo = vec4(0,1,1,1);

    }
    else if(debugMode == 3){ //Debug light direction
        gAlbedo = vec4(L,1);
    }

    //Drawing the light axes as yellow.
    if(length(worldNormal.xyz)<0.1f)
        gAlbedo = vec4(1.0f,1.0f,0.0f,1.0f);

}
