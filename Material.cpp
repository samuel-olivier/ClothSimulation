#include "Material.h"

#include <QDebug>

Material::Material(Node *parent) : Node(parent)
{
    setName("material");
    setType("Material");
    _texId = 0;
    _hasTexture = false;

    createDOF(DOF::ColorR, true, true, true, 200, 0, 255);
    createDOF(DOF::ColorG, true, true, true, 200, 0, 255);
    createDOF(DOF::ColorB, true, true, true, 200, 0, 255);
    createDOF(DOF::ColorA, true, true, true, 255, 0, 255);
    createDOF(DOF::Diffuse, true, true, true, 0, 1, 1);
    createDOF(DOF::Specular, true, true, true, 0, 1, 1);
}

Material::~Material()
{
}

void Material::setColor(const QColor &color)
{
    setDOFValue(DOF::ColorR, color.red());
    setDOFValue(DOF::ColorG, color.green());
    setDOFValue(DOF::ColorB, color.blue());
    setDOFValue(DOF::ColorA, color.alpha());
}

QColor Material::color() const
{
    return QColor(DOFValue(DOF::ColorR),
                  DOFValue(DOF::ColorG),
                  DOFValue(DOF::ColorB),
                  DOFValue(DOF::ColorA));
}

bool Material::hasTexture() const
{
    return _hasTexture;
}

void Material::setHasTexture(bool hasTexture)
{
    if (_texId > 0)
        _hasTexture = hasTexture;
    else
        _hasTexture = false;
}

GLuint Material::texture() const
{
    return _texId;
}

bool Material::loadTexture(const QString &filename)
{
    QImage img(filename);
    if (img.isNull())
        return false;
    img = QGLWidget::convertToGLFormat(img);
    glGenTextures(1, &_texId);
    glBindTexture(GL_TEXTURE_2D, _texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    setHasTexture(true);
    return hasTexture();
}

float Material::diffuseCoef() const
{
    return DOFValue(DOF::Diffuse);
}

void Material::setDiffuseCoef(float diffuseCoef)
{
    setDOFValue(DOF::Diffuse, diffuseCoef);
}

float Material::specularCoef() const
{
    return DOFValue(DOF::Specular);
}

void Material::setSpecularCoef(float specularCoef)
{
    setDOFValue(DOF::Specular, specularCoef);
}
