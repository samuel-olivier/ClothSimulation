#ifndef PLANE_H
#define PLANE_H

#include "Mesh.h"

class Plane : public Mesh
{
public:
    Plane(Node* parent);
    virtual ~Plane();

    virtual void    init();
    void    set(float width, float height, int numberX, int numberY, QVector3D const& start);

    float   width() const;
    void    setWidth(float width);

    float   height() const;
    void    setHeight(float height);

    int     numberX() const;
    void    setNumberX(int numberX);

    int     numberY() const;
    void    setNumberY(int numberY);

    QVector3D const&    start() const;
    void    setStart(QVector3D const& start);

private:
    void    _set();

    float   _width;
    float   _height;
    int     _numberX;
    int     _numberY;
    QVector3D   _start;
};

#endif // BOX_H
