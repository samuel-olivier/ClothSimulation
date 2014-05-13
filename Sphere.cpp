#include "Sphere.h"

#include <qmath.h>

Sphere::Sphere(Node *parent) : Mesh(parent)
{
    setName("Ball");
    setType("Sphere");
    createDOF(DOF::RotationX, true, false, false, 0, 0, 0);
    createDOF(DOF::RotationY, true, false, false, 1, 0, 0);
    createDOF(DOF::RotationZ, true, false, false, 0, 0, 0);
    createDOF(DOF::Speed, true, true, true, 0.5, -4 * M_PI, 4 * M_PI);
}

Sphere::~Sphere()
{
}

void Sphere::init()
{
    Mesh::init();
    set(QVector3D(), 1, 10);
}

void Sphere::set(const QVector3D &center, float radius, int precision)
{
    _center = center;
    _radius = radius;
    _precision = precision;
    _set();
}

const QVector3D &Sphere::center() const
{
    return _center;
}

void Sphere::setCenter(const QVector3D &center)
{
    _center = center;
    _set();
}

float Sphere::radius() const
{
    return _radius;
}

void Sphere::setRadius(float radius)
{
    _radius = radius;
    _set();
}

int Sphere::precision() const
{
    return _precision;
}

void Sphere::setPrecision(float precision)
{
    _precision = precision;
    _set();
}

QVector3D Sphere::axe() const
{
    return QVector3D(DOFValue(DOF::RotationX), DOFValue(DOF::RotationY), DOFValue(DOF::RotationZ));
}

void Sphere::setAxe(const QVector3D &axe)
{
    setDOFValue(DOF::RotationX, axe.x());
    setDOFValue(DOF::RotationY, axe.y());
    setDOFValue(DOF::RotationZ, axe.z());
}

float Sphere::speed() const
{
    return DOFValue(DOF::Speed);
}

void Sphere::setSpeed(float speed)
{
    setDOFValue(DOF::Speed, speed);
}

void Sphere::_set()
{
    _vertices.resize((_precision + 1) * (_precision + 1) * 3);
    _normals.resize(_vertices.size());
    _faces.resize(_precision * _precision * 6);
    int vIdx = 0;
    int fIdx = -1;
    for (int sl = 0; sl <= _precision; ++sl) {
        float theta = sl * M_PI / _precision;
        for (int st = 0; st <= _precision; ++st) {
            float phi = st * 2.0 * M_PI / _precision;
            float x = sin(theta) * cos(phi);
            float y = cos(theta);
            float z = sin(theta) * sin(phi);

            _vertices[vIdx] = x * _radius + _center.x();
            _vertices[vIdx + 1] = y * _radius + _center.y();
            _vertices[vIdx + 2] = z * _radius + _center.z();
            _normals[vIdx] = x;
            _normals[vIdx + 1] = y;
            _normals[vIdx + 2] = z;
            vIdx += 3;
            if (sl != _precision && st != _precision) {
                float indice1 = sl * (_precision + 1) + st;
                float indice2 = indice1 + _precision + 1;
                _faces[++fIdx] = indice1;
                _faces[++fIdx] = indice2;
                _faces[++fIdx] = indice1 + 1;
                _faces[++fIdx] = indice2;
                _faces[++fIdx] = indice2 + 1;
                _faces[++fIdx] = indice1 + 1;
            }
        }
    }
    setVertices(_vertices);
    setNormals(_normals);
    setFaces(_faces);
}





