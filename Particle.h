#ifndef PARTICLE_H
#define PARTICLE_H

#include <QVector3D>
#include <QList>

class SpringDamper;

class Particle
{
public:
    Particle();
    virtual ~Particle();

    float   mass() const;
    void    setMass(float mass);

    QVector3D const&    pos() const;
    void                setPos(QVector3D const& pos);

    QVector3D const&    velocity() const;
    void                setVelocity(QVector3D const& velocity);

    QVector3D const&    force() const;
    void                applyForce(QVector3D const& force);
    void                resetForce();

    void    update(float elapsed);

    int     idx() const;
    void    setIdx(int idx);

    QList<SpringDamper*> const& springDampers() const;
    void                        addSpringDamper(SpringDamper* damper);

private:
    float       _mass;
    QVector3D   _pos;
    QVector3D   _velocity;
    QVector3D   _force;
    int         _idx;
    QList<SpringDamper*> _springDampers;
};

#endif // PARTICLE_H
