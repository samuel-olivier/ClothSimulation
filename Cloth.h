#ifndef CLOTH_H
#define CLOTH_H

#include "Mesh.h"

#include <mutex>
#include <CL/cl.hpp>
#include <vector>

class Particle;
class SpringDamper;
class Plane;
class Sphere;

#define ROOT_PATH "C:/Users/samuel/Documents/Dev/ClothSimulation/Files"

using namespace std;

class Cloth : public Mesh
{
public:
    Cloth(Node* parent);
    virtual ~Cloth();

    void    clear();
    void    setRectangularCloth(int sizeX, int sizeY, float width, float height, bool topGrab);

    QVector3D const&    gravity() const;
    void                setGravity(QVector3D const& gravity);

    QVector3D   wind() const;
    void        setWind(QVector3D const& wind);

    QVector3D   translation() const;
    void        setTranslation(QVector3D const& translation);

    float   density() const;
    void    setDensity(float density);

    float   dragCoef() const;
    void    setDragCoef(float dragCoef);

    float   friction() const;
    void    setFriction(float friction);

    float   elasticity() const;
    void    setElasticity(float elasticity);

    int     iterationNumber() const;
    void    setIterationNumber(int iterationNumber);

    float   weight() const;
    void    setWeight(float weight);

    void    setPlane(Plane* plane);
    void    setSphere(Sphere* sphere);

    void    release();

    void    update(float elapsed);

private:
    void    _addSpringDamper(int& idx, Particle* p1, Particle* p2, float length, float springConstant = 0.8, float dampingFactor = 0.003);
    void    _updateCloth();
    void    _updateNormals();
    void    _calcAerodynamicForce(int id1, int id2, int id3);
    void    _calcBatches();

    void    _initCL();
    void    _initKernelParams();

    QVector<int>            _vertexFaces;
    QVector3D               _gravity;
    int                     _iterationNumber;
    QVector<Particle*>      _particles;
    QVector<SpringDamper*>  _springDampers;
    float                   _weight;
    QList<QPair<int, QVector3D>>    _movables;
    Plane*                  _plane;
    Sphere*                 _sphere;
    QList<QPair<int, int>>  _SDBatches;
    QList<QPair<int, int>>  _faceBatches;

    cl::Context*            _clContext;
    vector<cl::Device>      _clDevices;
    cl::Program*            _clProgram;
    cl::Kernel*             _clGravityKernel;
    cl::Kernel*             _clSDKernel;
    cl::Kernel*             _clADKernel;
    cl::Kernel*             _clIntegrateKernel;
    cl::Kernel*             _clIntersectKernel;
    cl::CommandQueue*       _clQueue;

    float*                  _particleArray;
//    int*                    _partSPIndexes;
    int*                    _SPIndexes;
    float*                  _SPArray;
    float*                  _forceArray;
    int*                    _faceArray;

    cl::Buffer*             _particleArrayBuf;
//    cl::Buffer*             _partSPIndexesBuf;
    cl::Buffer*             _SPIndexesBuf;
    cl::Buffer*             _SPArrayBuf;
    cl::Buffer*             _forceArrayBuf;
    cl::Buffer*             _faceArrayBuf;
    bool                    _rewritePartBuffer;

//    cl::Buffer*             _particleArrayBufIntegrate;
//    cl::Buffer*             _forceArrayBufIntegrate;
};

#endif // CLOTH_H
