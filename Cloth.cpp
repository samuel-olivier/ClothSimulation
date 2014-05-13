#include "Cloth.h"

#include <QDebug>
#include <QSet>
#include <iostream>
#include <fstream>
#include <qmath.h>

#include "Particle.h"
#include "SpringDamper.h"
#include "Plane.h"
#include "Sphere.h"

#define checkErr(check, str) {if (check != CL_SUCCESS) {cerr << str << " : " << descriptionOfError(check) << endl; return ;}}
#define EPSILON 0.01f

string descriptionOfError(cl_int err) {
    switch (err) {
        case CL_SUCCESS:                            return "Success!";
        case CL_DEVICE_NOT_FOUND:                   return "Device not found.";
        case CL_DEVICE_NOT_AVAILABLE:               return "Device not available";
        case CL_COMPILER_NOT_AVAILABLE:             return "Compiler not available";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:      return "Memory object allocation failure";
        case CL_OUT_OF_RESOURCES:                   return "Out of resources";
        case CL_OUT_OF_HOST_MEMORY:                 return "Out of host memory";
        case CL_PROFILING_INFO_NOT_AVAILABLE:       return "Profiling information not available";
        case CL_MEM_COPY_OVERLAP:                   return "Memory copy overlap";
        case CL_IMAGE_FORMAT_MISMATCH:              return "Image format mismatch";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:         return "Image format not supported";
        case CL_BUILD_PROGRAM_FAILURE:              return "Program build failure";
        case CL_MAP_FAILURE:                        return "Map failure";
        case CL_INVALID_VALUE:                      return "Invalid value";
        case CL_INVALID_DEVICE_TYPE:                return "Invalid device type";
        case CL_INVALID_PLATFORM:                   return "Invalid platform";
        case CL_INVALID_DEVICE:                     return "Invalid device";
        case CL_INVALID_CONTEXT:                    return "Invalid context";
        case CL_INVALID_QUEUE_PROPERTIES:           return "Invalid queue properties";
        case CL_INVALID_COMMAND_QUEUE:              return "Invalid command queue";
        case CL_INVALID_HOST_PTR:                   return "Invalid host pointer";
        case CL_INVALID_MEM_OBJECT:                 return "Invalid memory object";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    return "Invalid image format descriptor";
        case CL_INVALID_IMAGE_SIZE:                 return "Invalid image size";
        case CL_INVALID_SAMPLER:                    return "Invalid sampler";
        case CL_INVALID_BINARY:                     return "Invalid binary";
        case CL_INVALID_BUILD_OPTIONS:              return "Invalid build options";
        case CL_INVALID_PROGRAM:                    return "Invalid program";
        case CL_INVALID_PROGRAM_EXECUTABLE:         return "Invalid program executable";
        case CL_INVALID_KERNEL_NAME:                return "Invalid kernel name";
        case CL_INVALID_KERNEL_DEFINITION:          return "Invalid kernel definition";
        case CL_INVALID_KERNEL:                     return "Invalid kernel";
        case CL_INVALID_ARG_INDEX:                  return "Invalid argument index";
        case CL_INVALID_ARG_VALUE:                  return "Invalid argument value";
        case CL_INVALID_ARG_SIZE:                   return "Invalid argument size";
        case CL_INVALID_KERNEL_ARGS:                return "Invalid kernel arguments";
        case CL_INVALID_WORK_DIMENSION:             return "Invalid work dimension";
        case CL_INVALID_WORK_GROUP_SIZE:            return "Invalid work group size";
        case CL_INVALID_WORK_ITEM_SIZE:             return "Invalid work item size";
        case CL_INVALID_GLOBAL_OFFSET:              return "Invalid global offset";
        case CL_INVALID_EVENT_WAIT_LIST:            return "Invalid event wait list";
        case CL_INVALID_EVENT:                      return "Invalid event";
        case CL_INVALID_OPERATION:                  return "Invalid operation";
        case CL_INVALID_GL_OBJECT:                  return "Invalid OpenGL object";
        case CL_INVALID_BUFFER_SIZE:                return "Invalid buffer size";
        case CL_INVALID_MIP_LEVEL:                  return "Invalid mip-map level";
        default: return "Unknown";
    }
}

Cloth::Cloth(Node *parent) : Mesh(parent)
{
    _gravity.setY(-9.81);
    createDOF(DOF::WindX, true, false, false, 0, 0, 0);
    createDOF(DOF::WindY, true, false, false, 0, 0, 0);
    createDOF(DOF::WindZ, true, false, false, 0, 0, 0);
    createDOF(DOF::Density, true, true, true, 1.2, 0, 3);
    createDOF(DOF::DragCoef, true, true, true, 1.2, 0, 3);
    createDOF(DOF::TranslationX, true, false, false, 0, 0, 0);
    createDOF(DOF::TranslationY, true, false, false, 0, 0, 0);
    createDOF(DOF::TranslationZ, true, false, false, 0, 0, 0);
    createDOF(DOF::Friction, true, true, true, 0.5, 0, 1);
    createDOF(DOF::Elasticity, true, true, true, 0.2, 0, 1);
    _iterationNumber = 20;
    _weight = 0.3;
    _plane = NULL;
    _sphere = NULL;
    setName("cloth");
    setType("Cloth");
    _initCL();
    _rewritePartBuffer = true;
}

Cloth::~Cloth()
{
    clear();
}

void Cloth::clear()
{
    qDeleteAll(_springDampers);
    qDeleteAll(_particles);
    _springDampers.clear();
    _particles.clear();
    _vertexFaces.clear();
}

void Cloth::setRectangularCloth(int sizeX, int sizeY, float width, float height, bool topGrab)
{
    clear();
    float dx = width / sizeX;
    float dy = height / sizeY;
    float particleWeight = weight() / (sizeX * sizeY);
    Particle* newParticle;
    QVector3D pos;
    float xRand = dx / 100;
    float yRand = dy / 100;
    float zRand = (xRand + yRand) / 2;
    int idx = -1;

    _particles.resize(sizeX * sizeY);
    _springDampers.resize(sizeX - 1 + sizeY - 1 + 4 * (sizeX - 1) * (sizeY - 1));
    _texCoords.resize(_particles.size() * 2);
    for (int y = 0; y < sizeY; ++y) {
        for (int x = 0; x < sizeX; ++x) {
            newParticle = new Particle();
            if ((topGrab && y == 0) || (!topGrab && x == 0)) {// && (x == 0 || x == sizeX - 1)) {
                newParticle->setMass(0);
            } else {
                newParticle->setMass(particleWeight);
            }
            pos.setX(x * dx + xRand - (2 * (float)rand() / RAND_MAX) * xRand);
            pos.setY(height-(y * dy + yRand - (2 * (float)rand() / RAND_MAX) * yRand));
            pos.setZ(zRand - (2 * (float)rand() / RAND_MAX) * zRand);
            newParticle->setPos(pos);
            newParticle->setIdx(y * sizeX + x);
            _particles[newParticle->idx()] = newParticle;
            if (newParticle->mass() == 0) {
                _movables << QPair<int, QVector3D>(y * sizeX + x, newParticle->pos());
            }
            if (x > 0 && y > 0) {
                _addSpringDamper(idx, _particles[(y - 1) * sizeX + (x - 1)], _particles[(y - 1) * sizeX + x], dx);
                _addSpringDamper(idx, _particles[(y - 1) * sizeX + (x - 1)], _particles[y * sizeX + (x - 1)], dy);
                _addSpringDamper(idx, _particles[(y - 1) * sizeX + x], _particles[y * sizeX + (x - 1)], sqrt(dy * dy + dx * dx));
                _addSpringDamper(idx, _particles[(y - 1) * sizeX + (x - 1)], _particles[y * sizeX + x], sqrt(dy * dy + dx * dx));

                if (y == sizeY - 1) {
                    _addSpringDamper(idx, _particles[y * sizeX + (x - 1)], _particles[y * sizeX + x], dx);
                }
                if (x == sizeX - 1) {
                    _addSpringDamper(idx, _particles[(y - 1) * sizeX + x], _particles[y * sizeX + x], dx);
                }
            }
        }
    }
    idx = -1;
    _texCoords.resize(_particles.size() * 2);
    dx = 1.0 / (sizeX - 1);
    dy = 1.0 / (sizeY - 1);
    for (int y = 0; y < sizeY; ++y) {
        for (int x = 0; x < sizeX; ++x) {
            _texCoords[(y * sizeX + x) * 2] = x * dx;
            _texCoords[(y * sizeX + x) * 2 + 1] = 1 - y * dy;
        }
    }
    setTexCoords(_texCoords);
    idx = -1;
    _faces.resize((sizeX - 1) * (sizeY - 1) * 2 * 3);
    for (int y = 0; y < sizeY - 1; ++y) {
        for (int x = 0; x < sizeX - 1; ++x) {
            _faces[++idx] = (y + 1) * sizeX + x;
            _faces[++idx] = y * sizeX + x;
            _faces[++idx] = y * sizeX + (x + 1);

            _faces[++idx] = (y + 1) * sizeX + x;
            _faces[++idx] = y * sizeX + (x + 1);
            _faces[++idx] = (y + 1) * sizeX + (x + 1);
        }
    }
    _calcBatches();
    setFaces(_faces);
    _initKernelParams();
    _updateCloth();
}

const QVector3D &Cloth::gravity() const
{
    return _gravity;
}

void Cloth::setGravity(const QVector3D &gravity)
{
    _gravity = gravity;
}

QVector3D Cloth::wind() const
{
    return QVector3D(DOFValue(DOF::WindX), DOFValue(DOF::WindY), DOFValue(DOF::WindZ));
}

void Cloth::setWind(const QVector3D &wind)
{
    setDOFValue(DOF::WindX, wind.x());
    setDOFValue(DOF::WindY, wind.y());
    setDOFValue(DOF::WindZ, wind.z());
}

QVector3D Cloth::translation() const
{
    return QVector3D(DOFValue(DOF::TranslationX), DOFValue(DOF::TranslationY), DOFValue(DOF::TranslationZ));
}

void Cloth::setTranslation(const QVector3D &translation)
{
    setDOFValue(DOF::TranslationX, translation.x());
    setDOFValue(DOF::TranslationY, translation.y());
    setDOFValue(DOF::TranslationZ, translation.z());
}

float Cloth::density() const
{
    return DOFValue(DOF::Density);
}

void Cloth::setDensity(float density)
{
    setDOFValue(DOF::Density, density);
}

float Cloth::dragCoef() const
{
    return DOFValue(DOF::DragCoef);
}

void Cloth::setDragCoef(float dragCoef)
{
    setDOFValue(DOF::DragCoef, dragCoef);
}

float Cloth::friction() const
{
    return DOFValue(DOF::Friction);
}

void Cloth::setFriction(float friction)
{
    setDOFValue(DOF::Friction, friction);
}

float Cloth::elasticity() const
{
    return DOFValue(DOF::Elasticity);
}

void Cloth::setElasticity(float elasticity)
{
    setDOFValue(DOF::Elasticity, elasticity);
}

int Cloth::iterationNumber() const
{
    return _iterationNumber;
}

void Cloth::setIterationNumber(int iterationNumber)
{
    _iterationNumber = iterationNumber;
}

float Cloth::weight() const
{
    return _weight;
}

void Cloth::setWeight(float weight)
{
    _weight = weight;
}

void Cloth::setPlane(Plane *plane)
{
    _plane = plane;
}

void Cloth::setSphere(Sphere *sphere)
{
    _sphere = sphere;
}

#define OPEN_CL
#define PLATFORM_IDX 0

void Cloth::release()
{
#ifdef OPEN_CL
    for (QPair<int, QVector3D> current : _movables) {
        int idx = _particles[current.first]->idx() * 7;
        _particleArray[idx] = weight() / _particles.size();
    }
    _rewritePartBuffer = true;
#else
    for (QPair<int, QVector3D> current : _movables) {
        _particles[current.first]->setMass(weight() / _particles.size());
    }
#endif
    _movables.clear();
}

#ifdef OPEN_CL
void Cloth::update(float elapsed)
{
    if (elapsed > 1.0 / 20) {
        elapsed = 1.0 / 30;
    }
    float dt = elapsed / _iterationNumber;

    cl_int err;
    for (QPair<int, QVector3D> current : _movables) {
        int idx = _particles[current.first]->idx() * 7;
        QVector3D newPos = current.second + translation();
        if (QVector3D(_particleArray[idx + 1], _particleArray[idx + 2], _particleArray[idx + 3]) != newPos) {
            _particleArray[idx + 1] = newPos.x();
            _particleArray[idx + 2] = newPos.y();
            _particleArray[idx + 3] = newPos.z();
        }
        _rewritePartBuffer = true;
    }

    err = _clGravityKernel->setArg(2, (cl_float3){gravity().x(), gravity().y(), gravity().z()});
    checkErr(err, "gravity Kernel::setArg()");


    err = _clADKernel->setArg(3, (cl_float3){wind().x(), wind().y(), wind().z()});
    checkErr(err, "wind Kernel::setArg()");

    err = _clADKernel->setArg(4, density());
    checkErr(err, "density Kernel::setArg()");

    err = _clADKernel->setArg(5, dragCoef());
    checkErr(err, "dragCoef Kernel::setArg()");


    err = _clIntegrateKernel->setArg(2, dt);
    checkErr(err, "dt Kernel::setArg()");


    err = _clIntersectKernel->setArg(1, friction());
    checkErr(err, "friction Kernel::setArg()");

    err = _clIntersectKernel->setArg(2, elasticity());
    checkErr(err, "elasticity Kernel::setArg()");

    err = _clIntersectKernel->setArg(3, _plane->start().y() + EPSILON);
    checkErr(err, "planeY Kernel::setArg()");

    err = _clIntersectKernel->setArg(4, (cl_float3){_sphere->center().x(), _sphere->center().y(), _sphere->center().z()});
    checkErr(err, "sphereCenter Kernel::setArg()");

    err = _clIntersectKernel->setArg(5, (cl_float3){_sphere->axe().x(), _sphere->axe().y(), _sphere->axe().z()});
    checkErr(err, "sphereAxe Kernel::setArg()");

    err = _clIntersectKernel->setArg(6, _sphere->radius() + EPSILON);
    checkErr(err, "sphereRadius Kernel::setArg()");

    err = _clIntersectKernel->setArg(7, _sphere->speed());
    checkErr(err, "sphereSpeed Kernel::setArg()");

    if (_rewritePartBuffer) {
        err = _clQueue->enqueueWriteBuffer(*_particleArrayBuf, true, 0, _particles.size() * sizeof(float) * 7, &_particleArray[0]);
        _rewritePartBuffer = false;
        checkErr(err, "PartBuffer Queue::enqueueWriteBuffer()");
    }

    cl::Event event;
    for (int i = 0; i < _iterationNumber; ++i) {
        err = _clQueue->enqueueNDRangeKernel(
                    *_clGravityKernel,
                    cl::NullRange,
                    cl::NDRange(_particles.size()),
                    cl::NDRange(1, 1),
                    NULL,
                    NULL);
        checkErr(err, "Gravity CommandQueue::enqueueNDRangeKernel()");
        event.wait();
        err = _clQueue->enqueueReadBuffer(
                    *_forceArrayBuf,
                    CL_TRUE,
                    0,
                    _particles.size() * 3 * sizeof(float),
                    &_forceArray[0]);
        checkErr(err, "Integrate CommandQueue::enqueueReadBuffer()");

        for (auto it = _SDBatches.begin(); it != _SDBatches.end(); ++it) {
            err = _clQueue->enqueueNDRangeKernel(
                        *_clSDKernel,
                        cl::NDRange(it->first),
                        cl::NDRange(it->second),
                        cl::NDRange(1, 1),
                        NULL,
                        NULL);
            checkErr(err, "SD CommandQueue::enqueueNDRangeKernel()");
        }

        for (auto it = _faceBatches.begin(); it != _faceBatches.end(); ++it) {
            err = _clQueue->enqueueNDRangeKernel(
                        *_clADKernel,
                        cl::NDRange(it->first),
                        cl::NDRange(it->second),
                        cl::NDRange(1, 1),
                        NULL,
                        NULL);
            checkErr(err, "AD CommandQueue::enqueueNDRangeKernel()");
        }

        err = _clQueue->enqueueNDRangeKernel(
                    *_clIntegrateKernel,
                    cl::NullRange,
                    cl::NDRange(_particles.size()),
                    cl::NDRange(1, 1),
                    NULL,
                    NULL);
        checkErr(err, "Integrate CommandQueue::enqueueNDRangeKernel()");

        err = _clQueue->enqueueNDRangeKernel(
                    *_clIntersectKernel,
                    cl::NullRange,
                    cl::NDRange(_particles.size()),
                    cl::NDRange(1, 1),
                    NULL,
                    &event);
        checkErr(err, "Intersect CommandQueue::enqueueNDRangeKernel()");
    }
    event.wait();
    err = _clQueue->enqueueReadBuffer(
                *_particleArrayBuf,
                CL_TRUE,
                0,
                _particles.size() * 7 * sizeof(float),
                &_particleArray[0]);
    checkErr(err, "Integrate CommandQueue::enqueueReadBuffer()");
    _updateCloth();
    Mesh::update(elapsed);
}
#else
void Cloth::update(float elapsed)
{
    if (elapsed > 1.0 / 20) {
        elapsed = 1.0 / 30;
    }
    float dt = elapsed / _iterationNumber;

    for (int i = 0; i < _iterationNumber; ++i) {
        for (QPair<int, QVector3D> current : _movables) {
            _particles[current.first]->setPos(current.second + translation());
        }
        for (Particle* current : _particles) {
            if (current) {
                current->applyForce(current->mass() * gravity());
            }
        }
        for (SpringDamper* current : _springDampers) {
            if (current) {
                current->computeForces();
            }
        }
        for (int i = 0; i + 2 < _faces.size(); i += 3) {
            _calcAerodynamicForce(_faces[i], _faces[i + 1], _faces[i + 2]);
        }

        for (Particle* current : _particles) {
            if (current) {
                current->update(dt);
                QVector3D p = current->pos();
                QVector3D v = current->velocity();
                if (_plane) {
                    if (p.y() < _plane->start().y() + EPSILON) {
                        p.setY(_plane->start().y() + EPSILON);
                        v.setY(-elasticity() * v.y());
                        v.setX((1-friction()) * v.x());
                        v.setZ((1-friction()) * v.z());
                        current->setPos(p);
                        current->setVelocity(v);
                    }
                }
                if (_sphere) {
                    QVector3D vec = p - _sphere->center();

                    if (vec.length() < _sphere->radius() + EPSILON) {
                        QVector3D n = vec.normalized();

                        QVector3D r = n * (_sphere->radius() + EPSILON);
                        QVector3D axe = _sphere->axe().normalized() * _sphere->speed();
                        QVector3D vs = QVector3D::crossProduct(axe, r);

//                        QVector3D r = n * (_sphere->radius() + EPSILON);
//                        QVector3D axe = _sphere->axe().normalized();
//                        QVector3D vs = QVector3D::crossProduct(n, axe).normalized();
//                        vs *= (r - axe * QVector3D::dotProduct(n, axe)).length() * _sphere->speed() ;
                        v -= vs;

                        float vc = QVector3D::dotProduct(n, v);
                        QVector3D j = -(1 + elasticity()) * vc * n;
                        QVector3D e = (n * QVector3D::dotProduct(n, v) - v).normalized();
                        QVector3D jd = friction() * j.length() * e;
                        v += j + jd + vs;
                        current->setVelocity(v);
                        current->setPos(_sphere->center() + r);
                    }
                }
            }
        }
    }
    _updateCloth();
    Mesh::update(elapsed);
}
#endif

void Cloth::_addSpringDamper(int &idx, Particle *p1, Particle *p2, float length, float springConstant, float dampingFactor)
{
    SpringDamper* current = new SpringDamper();
    current->setP1(p1);
    current->setP2(p2);
    current->setRestLength(length);
    current->setSpringConstant(springConstant);
    current->setDampingFactor(dampingFactor);
    current->setIdx(++idx);
    p1->addSpringDamper(current);
    p2->addSpringDamper(current);
    _springDampers[current->idx()] = current;
}

void Cloth::_updateCloth()
{
    _vertices.resize(_particles.size() * 3);
#ifdef OPEN_CL
    for (int i = 0; i < _particles.size(); ++i) {
        _vertices[3 * i] = _particleArray[7 * i + 1];
        _vertices[3 * i + 1] = _particleArray[7 * i + 2];
        _vertices[3 * i + 2] = _particleArray[7 * i + 3];
    }
#else
    int idx = -1;
    for (Particle* particle : _particles) {
        if (particle) {
            _vertices[++idx] = particle->pos().x();
            _vertices[++idx] = particle->pos().y();
            _vertices[++idx] = particle->pos().z();
        }
    }
#endif
    setVertices(_vertices);
    _updateNormals();
}

void Cloth::_updateNormals()
{
    QVector<QVector3D> vertexNormals(_vertices.size() / 3);
    QVector<float> newNormals(_vertices.size(), 0);
    bool first = _vertexFaces.isEmpty();
    if (first) {
        _vertexFaces.resize(_vertices.size() / 3);
        _vertexFaces.fill(0);
    }
    for (int i = 0; i < _faces.size(); i += 3) {
        QVector3D a(_vertices[_faces[i] * 3 + 0],
                _vertices[_faces[i] * 3 + 1],
                _vertices[_faces[i] * 3 + 2]);
        QVector3D b(_vertices[_faces[i + 1] * 3 + 0],
                _vertices[_faces[i + 1] * 3 + 1],
                _vertices[_faces[i + 1] * 3 + 2]);
        QVector3D c(_vertices[_faces[i + 2] * 3 + 0],
                _vertices[_faces[i + 2] * 3 + 1],
                _vertices[_faces[i + 2] * 3 + 2]);
        QVector3D n = QVector3D::crossProduct(b - a, c - a).normalized();
        vertexNormals[_faces[i]] += n;
        vertexNormals[_faces[i + 1]] += n;
        vertexNormals[_faces[i + 2]] += n;
        if (first) {
            ++_vertexFaces[_faces[i + 0]];
            ++_vertexFaces[_faces[i + 1]];
            ++_vertexFaces[_faces[i + 2]];
        }
    }
    for (int i = 0; i < vertexNormals.size(); ++i) {
        QVector3D n = (vertexNormals[i] / _vertexFaces[i]).normalized();
        int start = 3 * i;
        newNormals[start + 0] = n.x();
        newNormals[start + 1] = n.y();
        newNormals[start + 2] = n.z();
    }
    setNormals(newNormals);
}

void Cloth::_calcAerodynamicForce(int id1, int id2, int id3)
{
    Particle *p1 = _particles[id1];
    Particle *p2 = _particles[id2];
    Particle *p3 = _particles[id3];
    if (!p1 || !p2 || !p3)
        return ;
    QVector3D ns = QVector3D::crossProduct(p2->pos() - p1->pos(), p3->pos() - p1->pos());
    QVector3D v = (p1->velocity() + p2->velocity() + p3->velocity()) / 3;
    v -= wind();
    QVector3D f = -density() * dragCoef() * (v.length() * QVector3D::dotProduct(v, ns) / (12 * ns.length())) * ns;
    p1->applyForce(f);
    p2->applyForce(f);
    p3->applyForce(f);
}

void Cloth::_calcBatches()
{
    {
        _SDBatches.clear();
        QVector<bool>   used(_particles.size(), false);
        QList<SpringDamper*> remaining = _springDampers.toList();
        int currentSize = 0;
        int start = 0;
        while (!remaining.isEmpty()) {
            for (auto it = remaining.begin(); it != remaining.end();) {
                SpringDamper* current = *it;
                if (used[current->p1()->idx()] == false && used[current->p2()->idx()] == false) {
                    it = remaining.erase(it);
                    used[current->p1()->idx()] = true;
                    used[current->p2()->idx()] = true;
                    _springDampers[start + currentSize] = current;
                    ++currentSize;
                } else {
                    ++it;
                }
            }
            used.fill(false);
            _SDBatches.append(QPair<int, int>(start, currentSize));
            start += currentSize;
            currentSize = 0;
        }
    }

    {
        _faceBatches.clear();
        QVector<bool>   used(_particles.size(), false);
        QList<int> remaining = _faces.toList();
        int currentSize = 0;
        int start = 0;
        while (!remaining.isEmpty()) {
            for (auto it = remaining.begin(); it != remaining.end();) {
                int id1 = *it;
                int id2 = *(it + 1);
                int id3 = *(it + 2);
                if (used[id1] == false && used[id2] == false && used[id3] == false) {
                    it = remaining.erase(it);
                    it = remaining.erase(it);
                    it = remaining.erase(it);
                    used[id1] = true;
                    used[id2] = true;
                    used[id3] = true;
                    _faces[start * 3 + currentSize * 3] = id1;
                    _faces[start * 3 + currentSize * 3 + 1] = id2;
                    _faces[start * 3 + currentSize * 3 + 2] = id3;
                    ++currentSize;
                } else {
                    ++it;
                    ++it;
                    ++it;
                }
            }
            used.fill(false);
            _faceBatches.append(QPair<int, int>(start, currentSize));
            start += currentSize;
            currentSize = 0;
        }
        qDebug() << _faceBatches << _faces.size();
    }
}

void Cloth::_initCL()
{
    cl_int err;
    vector< cl::Platform > platformList;
    cl::Platform::get(&platformList);
    checkErr(platformList.size()!=0 ? CL_SUCCESS : -1, "cl::Platform::get");
    cerr << "Platform number is: " << platformList.size() << endl;
    string platformVendor;
    platformList[PLATFORM_IDX].getInfo((cl_platform_info)CL_PLATFORM_VENDOR, &platformVendor);
    cerr << "Platform is by: " << platformVendor << "\n";

    cl_context_properties cprops[3] =
    {CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[PLATFORM_IDX])(), 0};
    _clContext = new cl::Context(
                CL_DEVICE_TYPE_GPU,
                cprops,
                NULL,
                NULL,
                &err);
    checkErr(err, "Context::Context()");

    _clDevices = _clContext->getInfo<CL_CONTEXT_DEVICES>();
    checkErr(_clDevices.size() > 0 ? CL_SUCCESS : -1, "devices.size() > 0");
    ifstream file(ROOT_PATH "/particle.cl", ifstream::in);
    checkErr(file.is_open() ? CL_SUCCESS:-1, "particle.cl");
    string prog(istreambuf_iterator<char>(file),
                (istreambuf_iterator<char>()));

    cl::Program::Sources source(1, std::make_pair(prog.c_str(), prog.length()+1));
    _clProgram = new cl::Program(*_clContext, source);
    err = _clProgram->build(_clDevices,"");
    checkErr(err, "Program::build()");

    _clGravityKernel = new cl::Kernel(*_clProgram, "applyGravity", &err);
    checkErr(err, "gravity Kernel::Kernel()");

    _clSDKernel = new cl::Kernel(*_clProgram, "computeSpringDamper", &err);
    checkErr(err, "SpringDamper Kernel::Kernel()");

    _clADKernel = new cl::Kernel(*_clProgram, "computeAerodynamicForces", &err);
    checkErr(err, "AeroDynamic Kernel::Kernel()");

    _clIntegrateKernel = new cl::Kernel(*_clProgram, "integrate", &err);
    checkErr(err, "integrate Kernel::Kernel()");

    _clIntersectKernel = new cl::Kernel(*_clProgram, "intersect", &err);
    checkErr(err, "intersect Kernel::Kernel()");

    _clQueue = new cl::CommandQueue(*_clContext, _clDevices[0], 0, &err);
    checkErr(err, "CommandQueue::CommandQueue()");
}

void Cloth::_initKernelParams()
{
    _particleArray = new float[_particles.size() * 7];
    _SPIndexes = new int[_springDampers.size() * 2];
    _SPArray = new float[_springDampers.size() * 3];
    _forceArray = new float[_particles.size() * 3];
    _faceArray = new int[_faces.size()];

    int pId = -1;
    for (Particle* part : _particles) {
        _particleArray[++pId] = part->mass();
        _particleArray[++pId] = part->pos().x();
        _particleArray[++pId] = part->pos().y();
        _particleArray[++pId] = part->pos().z();
        _particleArray[++pId] = part->velocity().x();
        _particleArray[++pId] = part->velocity().y();
        _particleArray[++pId] = part->velocity().z();
    }
    int SPId1 = -1;
    int SPId2 = -1;
    for (SpringDamper* sp : _springDampers) {
        _SPIndexes[++SPId1] = sp->p1()->idx();
        _SPIndexes[++SPId1] = sp->p2()->idx();
        _SPArray[++SPId2] = sp->restLength();
        _SPArray[++SPId2] = sp->dampingFactor();
        _SPArray[++SPId2] = sp->springConstant();
    }
    for (int i = 0; i < _faces.size(); ++i) {
        _faceArray[i] = _faces[i];
    }

    cl_int err;
    _particleArrayBuf = new cl::Buffer(
                *_clContext,
                CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                _particles.size() * 7 * sizeof(float),
                &_particleArray[0],
                &err);
    checkErr(err, "particleArray Buffer::Buffer()");

    _SPIndexesBuf = new cl::Buffer(
                *_clContext,
                CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                _springDampers.size() * 2 * sizeof(int),
                &_SPIndexes[0],
                &err);
    checkErr(err, "SPIndexes Buffer::Buffer()");

    _SPArrayBuf = new cl::Buffer(
                *_clContext,
                CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                _springDampers.size() * 3 * sizeof(float),
                &_SPArray[0],
                &err);
    checkErr(err, "SPArray Buffer::Buffer()");

    _forceArrayBuf = new cl::Buffer(
                *_clContext,
                CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                _particles.size() * 3 * sizeof(float),
                &_forceArray[0],
                &err);
    checkErr(err, "forceArray Buffer::Buffer()");

    _faceArrayBuf = new cl::Buffer(
                *_clContext,
                CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                _faces.size() * sizeof(int),
                &_faceArray[0],
                &err);
    checkErr(err, "faceArray Buffer::Buffer()");

    err = _clGravityKernel->setArg(0, *_particleArrayBuf);
    checkErr(err, "particleArrayGravity Kernel::setArg()");

    err = _clGravityKernel->setArg(1, *_forceArrayBuf);
    checkErr(err, "forceArrayGravity Kernel::setArg()");


    err = _clSDKernel->setArg(0, *_particleArrayBuf);
    checkErr(err, "particleArraySD Kernel::setArg()");

    err = _clSDKernel->setArg(1, *_SPIndexesBuf);
    checkErr(err, "SPIndexesSD Kernel::setArg()");

    err = _clSDKernel->setArg(2, *_SPArrayBuf);
    checkErr(err, "SPArraySD Kernel::setArg()");

    err = _clSDKernel->setArg(3, *_forceArrayBuf);
    checkErr(err, "forceArraySD Kernel::setArg()");


    err = _clADKernel->setArg(0, *_faceArrayBuf);
    checkErr(err, "faceArrayAD Kernel::setArg()");

    err = _clADKernel->setArg(1, *_particleArrayBuf);
    checkErr(err, "particleArrayAD Kernel::setArg()");

    err = _clADKernel->setArg(2, *_forceArrayBuf);
    checkErr(err, "forceArrayAD Kernel::setArg()");


    err = _clIntegrateKernel->setArg(0, *_particleArrayBuf);
    checkErr(err, "particleArrayIntegrate Kernel::setArg()");

    err = _clIntegrateKernel->setArg(1, *_forceArrayBuf);
    checkErr(err, "forceArrayIntegrate Kernel::setArg()");

    err = _clIntegrateKernel->setArg(0, *_particleArrayBuf);
    checkErr(err, "particleArrayIntegrate Kernel::setArg()");


    err = _clIntersectKernel->setArg(0, *_particleArrayBuf);
    checkErr(err, "particleArrayIntersect Kernel::setArg()");
}

//void Cloth::_initKernelParams()
//{
//    _particleArray = new float[_particles.size() * 7];
//    _partSPIndexes = new int[_particles.size()];
//    _SPIndexes = new int[_springDampers.size() * 2];
//    _SPArray = new float[_springDampers.size() * 5];
//    _forceArray = new float[_particles.size() * 3];

//    int pId = -1;
//    int pSPId = -1;
//    int SPId = -1;
//    for (Particle* part : _particles) {
//        _particleArray[++pId] = part->mass();
//        _particleArray[++pId] = part->pos().x();
//        _particleArray[++pId] = part->pos().y();
//        _particleArray[++pId] = part->pos().z();
//        _particleArray[++pId] = part->velocity().x();
//        _particleArray[++pId] = part->velocity().y();
//        _particleArray[++pId] = part->velocity().z();

//        _partSPIndexes[++pSPId] = SPId + 1;

//        for (SpringDamper* sp : part->springDampers()) {
//            _SPIndexes[++SPId] = sp->idx();
//        }
//    }
//    SPId = -1;
//    for (SpringDamper* sp : _springDampers) {
//        _SPArray[++SPId] = sp->p1()->idx();
//        _SPArray[++SPId] = sp->p2()->idx();
//        _SPArray[++SPId] = sp->restLength();
//        _SPArray[++SPId] = sp->dampingFactor();
//        _SPArray[++SPId] = sp->springConstant();
//    }

//    cl_int err;
//    _particleArrayBuf = new cl::Buffer(
//                *_clContext,
//                CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
//                _particles.size() * 7 * sizeof(float),
//                &_particleArray[0],
//                &err);
//    checkErr(err, "particleArrayCompute Buffer::Buffer()");

//    _partSPIndexesBuf = new cl::Buffer(
//                *_clContext,
//                CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
//                _particles.size() * sizeof(int),
//                &_partSPIndexes[0],
//                &err);
//    checkErr(err, "partSPIndexesCompute Buffer::Buffer()");

//    _SPIndexesBuf = new cl::Buffer(
//                *_clContext,
//                CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
//                _springDampers.size() * sizeof(int) * 2,
//                &_SPIndexes[0],
//                &err);
//    checkErr(err, "SPIndexesCompute Buffer::Buffer()");

//    _SPArrayBuf = new cl::Buffer(
//                *_clContext,
//                CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
//                _springDampers.size() * sizeof(float) * 5,
//                &_SPArray[0],
//                &err);
//    checkErr(err, "SPArrayCompute Buffer::Buffer()");

//    _forceArrayBuf = new cl::Buffer(
//                *_clContext,
//                CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
//                _particles.size() * 3 * sizeof(float),
//                &_forceArray[0],
//                &err);
//    checkErr(err, "forceArrayCompute Buffer::Buffer()");

////    _particleArrayBufIntegrate = new cl::Buffer(
////                *_clContext,
////                CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
////                _particles.size() * 7 * sizeof(float),
////                &_particleArray[0],
////                &err);
////    checkErr(err, "particleArrayIntegrate Buffer::Buffer()");

////    _forceArrayBufIntegrate = new cl::Buffer(
////                *_clContext,
////                CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
////                _particles.size() * 3 * sizeof(float),
////                &_forceArray[0],
////                &err);
////    checkErr(err, "forceArrayIntegrate Buffer::Buffer()");

//    err = _clGravityKernel->setArg(0, *_particleArrayBuf);
//    checkErr(err, "particleArrayCompute Kernel::setArg()");

//    err = _clGravityKernel->setArg(1, *_partSPIndexesBuf);
//    checkErr(err, "partSPIndexesCompute Kernel::setArg()");

//    err = _clGravityKernel->setArg(2, *_SPIndexesBuf);
//    checkErr(err, "SPIndexesBufCompute Kernel::setArg()");

//    err = _clGravityKernel->setArg(3, *_SPArrayBuf);
//    checkErr(err, "SPArrayCompute Kernel::setArg()");

//    err = _clGravityKernel->setArg(4, *_forceArrayBuf);
//    checkErr(err, "forceArrayCompute Kernel::setArg()");

//    err = _clGravityKernel->setArg(5, _springDampers.size() * 2);
//    checkErr(err, "SPSize Kernel::setArg()");


//    err = _clIntegrateKernel->setArg(0, *_particleArrayBuf);
//    checkErr(err, "particleArrayCompute Kernel::setArg()");

//    err = _clIntegrateKernel->setArg(1, *_forceArrayBuf);
//    checkErr(err, "forceArrayCompute Kernel::setArg()");
//}

