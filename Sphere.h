#ifndef SPHERE_H
#define SPHERE_H

#include "Mesh.h"

class Sphere : public Mesh
{
public:
    Sphere(Node* parent);
    virtual ~Sphere();

    virtual void    init();
    void    set(QVector3D const& center, float radius, int precision);

    QVector3D const&    center() const;
    void                setCenter(QVector3D const& center);

    float   radius() const;
    void    setRadius(float radius);

    int     precision() const;
    void    setPrecision(float precision);

    QVector3D   axe() const;
    void        setAxe(QVector3D const& axe);

    float       speed() const;
    void        setSpeed(float speed);

private:
    void    _set();

    QVector3D   _center;
    float       _radius;
    int         _precision;
};

#endif // SPHERE_H
