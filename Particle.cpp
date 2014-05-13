#include "Particle.h"

Particle::Particle()
{
    _mass = 0;
    _idx = 0;
}

Particle::~Particle()
{
}

float Particle::mass() const
{
    return _mass;
}

void Particle::setMass(float mass)
{
    _mass = mass;
}

const QVector3D &Particle::pos() const
{
    return _pos;
}

void Particle::setPos(const QVector3D &pos)
{
    _pos = pos;
}

const QVector3D &Particle::velocity() const
{
    return _velocity;
}

void Particle::setVelocity(const QVector3D &velocity)
{
    _velocity = velocity;
}

const QVector3D &Particle::force() const
{
    return _force;
}

void Particle::applyForce(const QVector3D &force)
{
    _force += force;
}

void Particle::resetForce()
{
    _force = QVector3D();
}

void Particle::update(float elapsed)
{
    if (mass() > 0) {
        QVector3D a = force() / mass();
        setVelocity(velocity() + a * elapsed);
        setPos(pos() + velocity() * elapsed);
    }
    resetForce();
}

int Particle::idx() const
{
    return _idx;
}

void Particle::setIdx(int idx)
{
    _idx = idx;
}

const QList<SpringDamper *> &Particle::springDampers() const
{
    return _springDampers;
}

void Particle::addSpringDamper(SpringDamper *damper)
{
    _springDampers.append(damper);
}
