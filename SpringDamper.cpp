#include "SpringDamper.h"

#include "Particle.h"

SpringDamper::SpringDamper()
{
    _springConstant = 0.5;
    _dampingFactor = 0.2;
    _restLength = 1;
    _p1 = NULL;
    _p2 = NULL;
    _idx = 0;
}

SpringDamper::~SpringDamper()
{

}

float SpringDamper::springConstant() const
{
    return _springConstant;
}

void SpringDamper::setSpringConstant(float springConstant)
{
    _springConstant = springConstant;
}

float SpringDamper::dampingFactor() const
{
    return _dampingFactor;
}

void SpringDamper::setDampingFactor(float dampingFactor)
{
    _dampingFactor = dampingFactor;
}

float SpringDamper::restLength() const
{
    return _restLength;
}

void SpringDamper::setRestLength(float restLength)
{
    _restLength = restLength;
}

Particle *SpringDamper::p1() const
{
    return _p1;
}

void SpringDamper::setP1(Particle *p1)
{
    _p1 = p1;
}

Particle *SpringDamper::p2() const
{
    return _p2;
}

void SpringDamper::setP2(Particle *p2)
{
    _p2 = p2;
}

void SpringDamper::computeForces()
{
    if (!_p1 || !_p2) {
        return ;
    }
    QVector3D e = _p2->pos() - _p1->pos();
    float l = e.length();
    e.normalize();
    float v1 = QVector3D::dotProduct(_p1->velocity(), e);
    float v2 = QVector3D::dotProduct(_p2->velocity(), e);
    QVector3D f = (-_springConstant * (_restLength - l) - _dampingFactor * (v1 - v2)) * e;
    _p1->applyForce(f);
    _p2->applyForce(-f);
}

int SpringDamper::idx() const
{
    return _idx;
}

void SpringDamper::setIdx(int idx)
{
    _idx = idx;
}
