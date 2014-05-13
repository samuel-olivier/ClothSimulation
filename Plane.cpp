#include "Plane.h"

#include <QVector>

Plane::Plane(Node *parent) : Mesh(parent)
{
    setName("Floor");
    setType("Plane");
}

Plane::~Plane()
{
}

void    Plane::init()
{
    Mesh::init();
    set(1, 1, 2, 2, QVector3D());
}

void Plane::set(float width, float height, int intervalX, int intervalY, QVector3D const& start)
{
    _width = width;
    _height = height;
    _numberX = intervalX;
    _numberY = intervalY;
    _start = start;
    _set();
}

float Plane::width() const
{
    return _width;
}

void Plane::setWidth(float width)
{
    _width = width;
    _set();
}

float Plane::height() const
{
    return _height;
}

void Plane::setHeight(float height)
{
    _height = height;
    _set();
}

int Plane::numberX() const
{
    return _numberX;
}

void Plane::setNumberX(int intervalX)
{
    _numberX = intervalX;
    _set();
}

int Plane::numberY() const
{
    return _numberY;
}

void Plane::setNumberY(int intervalY)
{
    _numberY = intervalY;
    _set();
}

QVector3D const&    Plane::start() const {
    return _start;
}

void    Plane::setStart(QVector3D const& start) {
    _start = start;
    _set();
}

void Plane::_set()
{
    float dx = _width / (_numberX - 1);
    float dy = _height / (_numberY - 1);
    int idx = -1;

    _vertices.resize(3 * _numberX * _numberY);
    _normals.resize(_vertices.size());
    _faces.resize(6 * (_numberX - 1) * (_numberY - 1));

    for (int y = 0; y < _numberY; ++y) {
        for (int x = 0; x < _numberX; ++x) {
            int start = (y * _numberX + x) * 3;
            _vertices[start + 0] = _start.x() + x * dx;
            _vertices[start + 1] = _start.y();
            _vertices[start + 2] = _start.z() + y * dy;
            _normals[start + 0] = 0;
            _normals[start + 1] = 1;
            _normals[start + 2] = 0;
            if (x < _numberX - 1 && y < _numberY - 1) {
                _faces[++idx] = (y + 1) * _numberX + x;
                _faces[++idx] = y * _numberX + x;
                _faces[++idx] = y * _numberX + (x + 1);

                _faces[++idx] = (y + 1) * _numberX + x;
                _faces[++idx] = y * _numberX + (x + 1);
                _faces[++idx] = (y + 1) * _numberX + (x + 1);
            }
        }
    }
    setVertices(_vertices);
    setNormals(_normals);
    setFaces(_faces);
}
