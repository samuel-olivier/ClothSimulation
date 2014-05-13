#ifndef SPRINGDAMPER_H
#define SPRINGDAMPER_H

class Particle;

class SpringDamper
{
public:
    SpringDamper();
    virtual ~SpringDamper();

    float   springConstant() const;
    void    setSpringConstant(float springConstant);

    float   dampingFactor() const;
    void    setDampingFactor(float dampingFactor);

    float   restLength() const;
    void    setRestLength(float restLength);

    Particle*   p1() const;
    void        setP1(Particle* p1);

    Particle*   p2() const;
    void        setP2(Particle* p2);

    void    computeForces();

    int     idx() const;
    void    setIdx(int idx);

private:
    float       _springConstant;
    float       _dampingFactor;
    float       _restLength;
    Particle*   _p1;
    Particle*   _p2;
    int         _idx;
};

#endif // SPRINGDAMPER_H
