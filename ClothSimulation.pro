#-------------------------------------------------
#
# Project created by QtCreator 2014-01-17T17:22:14
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ClothSimulation
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    Mesh.cpp \
    Scene.cpp \
    Node.cpp \
    Renderer.cpp \
    BasicFragmentShader.cpp \
    BasicShaderProgram.cpp \
    BasicGLWidget.cpp \
    BasicVertexShader.cpp \
    Box.cpp \
    Joint.cpp \
    DOF.cpp \
    BallJoint.cpp \
    Skeleton.cpp \
    BasicTokenizer.cpp \
    Camera.cpp \
    TrackBallCamera.cpp \
    TranslationalJoint.cpp \
    FixedJoint.cpp \
    PrismaticJoint.cpp \
    HingeJoint.cpp \
    Skin.cpp \
    Material.cpp \
    MorphTarget.cpp \
    AnimationClip.cpp \
    Channel.cpp \
    Keyframe.cpp \
    Pose.cpp \
    Rig.cpp \
    SkeletonRig.cpp \
    AnimationPlayer.cpp \
    FreeJoint.cpp \
    Particle.cpp \
    SpringDamper.cpp \
    Cloth.cpp \
    Plane.cpp \
    Sphere.cpp

HEADERS  += MainWindow.h \
    Mesh.h \
    Scene.h \
    Node.h \
    Renderer.h \
    BasicFragmentShader.h \
    BasicShaderProgram.h \
    BasicGLWidget.h \
    BasicVertexShader.h \
    Box.h \
    Joint.h \
    DOF.h \
    BallJoint.h \
    Skeleton.h \
    BasicTokenizer.h \
    TreeItem.h \
    DoubleSpinBox.h \
    Camera.h \
    TrackBallCamera.h \
    TranslationalJoint.h \
    FixedJoint.h \
    PrismaticJoint.h \
    HingeJoint.h \
    Skin.h \
    Material.h \
    MorphTarget.h \
    AnimationClip.h \
    Channel.h \
    Keyframe.h \
    Pose.h \
    Rig.h \
    SkeletonRig.h \
    AnimationPlayer.h \
    FreeJoint.h \
    Particle.h \
    SpringDamper.h \
    Cloth.h \
    Plane.h \
    Sphere.h

FORMS    += MainWindow.ui

#QMAKE_CXXFLAGS += -std=c++0x

#LIBS += -L$$PWD/../../../../../Program Files (x86)/AMD APP SDK/2.9/lib/x86/ -lOpenCL.lib -llibOpenCL.dll

#INCLUDEPATH += c:/Program Files (x86)/AMD APP SDK/2.9/include
#DEPENDPATH += c:/Program Files (x86)/AMD APP SDK/2.9/include
