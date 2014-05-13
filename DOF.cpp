#include "DOF.h"

QHash<DOF::Type, QString>  DOF::_names;


DOF::DOF(Type type, bool enabled)
{
    if (_names.isEmpty()) {
        _names[DOF::RotationX] = "Rotation X";
        _names[DOF::RotationY] = "Rotation Y";
        _names[DOF::RotationZ] = "Rotation Z";
        _names[DOF::TranslationX] = "Translation X";
        _names[DOF::TranslationY] = "Translation Y";
        _names[DOF::TranslationZ] = "Translation Z";
        _names[DOF::ColorR] = "Color R";
        _names[DOF::ColorG] = "Color G";
        _names[DOF::ColorB] = "Color B";
        _names[DOF::ColorA] = "Color A";
        _names[DOF::Diffuse] = "Diffuse";
        _names[DOF::Specular] = "Specular";
        _names[DOF::MorphTarget] = "Level";
        _names[DOF::Speed] = "Speed";
        _names[DOF::WindX] = "Wind X";
        _names[DOF::WindY] = "Wind Y";
        _names[DOF::WindZ] = "Wind Z";
        _names[DOF::Density] = "Density";
        _names[DOF::DragCoef] = "Drag Coef";
        _names[DOF::Friction] = "Friction";
        _names[DOF::Elasticity] = "Elasticity";

    }
    _hasMin = false;
    _min = 0;
    _hasMax = false;
    _max = 0;
    _value = 0;
    _enabled = enabled;
    _type = type;
    _name = _names[type];
}

DOF::~DOF()
{
}

float DOF::value() const
{
    return _value;
}

void DOF::setValue(float val)
{
    if (_hasMin && val < _min)
        val = _min;
    else if (_hasMax && val > _max)
        val = _max;
    _value = val;
}

bool DOF::hasMin() const
{
    return _hasMin;
}

void DOF::setHasMin(bool hasMin)
{
    _hasMin = hasMin;
}

float DOF::min() const
{
    return _min;
}

void DOF::setMin(float val)
{
    _min = val;
    if (_value < _min)
        _value = _min;
    _hasMin = true;
}

bool DOF::hasMax() const
{
    return _hasMax;
}

void DOF::setHasMax(bool hasMax)
{
    _hasMax = hasMax;
}

float DOF::max() const
{
    return _max;
}

void DOF::setMax(float val)
{
    _max = val;
    if (_value > _max)
        _value = _max;
    _hasMax = true;
}

bool DOF::isEnabled() const
{
    return _enabled;
}

void DOF::setEnabled(bool enabled)
{
    _enabled = enabled;
}

const QString &DOF::name() const
{
    return _name;
}

void DOF::setName(const QString &name)
{
    _name = name;
}

DOF::Type DOF::type() const
{
    return _type;
}

void DOF::setType(DOF::Type type)
{
    _type = type;
}
