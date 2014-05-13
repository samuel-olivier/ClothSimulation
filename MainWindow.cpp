#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>

#include "Scene.h"
#include "TreeItem.h"
#include "Joint.h"
#include "BallJoint.h"
#include "TranslationalJoint.h"
#include "DoubleSpinBox.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Cloth.h"
#include "Plane.h"
#include "Sphere.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    _cloth = NULL;
    _plane = NULL;
    _sphere = NULL;
    _matOriginal = NULL;
    _matFlag = NULL;
    ui->setupUi(this);
    connect(ui->glView, SIGNAL(initialized()), SLOT(onInitialized()));
    connect(ui->searchEdit, SIGNAL(returnPressed()), SLOT(onSearch()));
    connect(ui->skeletonTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(onCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(ui->glView, SIGNAL(meshSelected(Mesh*)), SLOT(onCurrentMeshChanged(Mesh*)));
    connect(ui->action_Toggle_Wireframe, SIGNAL(triggered()), SLOT(onToggleWireframe()));
    connect(ui->action_Release_Cloth, SIGNAL(triggered()), SLOT(onReleaseCloth()));
    connect(ui->action_Toggle_Flag, SIGNAL(triggered()), SLOT(onToggleFlag()));
    connect(ui->action_Reset_Cloth, SIGNAL(triggered()), SLOT(onResetCloth()));
    ui->skeletonTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onInitialized()
{
    _plane = new Plane(NULL);
    _plane->init();
    _plane->set(20, 20, 30, 30, QVector3D(-10, -5, -10));

    _sphere = new Sphere(NULL);
    _sphere->init();
    _sphere->set(QVector3D(1.5, 0, 3), 0.8, 15);

    ui->glView->scene()->addNode(_plane);
    ui->glView->scene()->addNode(_sphere);

    onResetCloth();

}

void MainWindow::onValueChanged(double value)
{
    DoubleSpinBox<DOF*>*   current = dynamic_cast<DoubleSpinBox<DOF*>*>(sender());

    if (!current)
        return;
    DOF*   DOF = current->userData();
    if (!DOF)
        return ;
    DOF->setValue(value);
}

void MainWindow::onSearch()
{
    QString search = ui->searchEdit->text();
    QTreeWidgetItemIterator it(ui->skeletonTree);
    while (*it) {
        if ((*it)->text(0).contains(search)) {
            ui->skeletonTree->setCurrentItem(*it);
            return ;
        }
        ++it;
    }
}

void MainWindow::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    TreeItem<Joint*>* itemJoint = dynamic_cast<TreeItem<Joint*>*>(current);
    Joint* joint = NULL;
    if (itemJoint) {
        joint = itemJoint->userData();
        if (!joint)
            return ;
        Node* box = joint->children().size() == 0 ? NULL : *(joint->children().begin());
        renderer->setCurrentMesh(dynamic_cast<Mesh*>(box));
    }
}

void MainWindow::onCurrentMeshChanged(Mesh *mesh)
{
    if (!mesh)
        return ;
    Node* toSearch = mesh;
    Joint* joint = dynamic_cast<Joint*>(mesh->parent());
    if (joint && mesh->name() == "Box")
        toSearch = joint;
    QTreeWidgetItemIterator it(ui->skeletonTree);
    while (*it) {
        TreeItem<Node*>* current = dynamic_cast<TreeItem<Node*>*>(*it);
        if (current && current->userData() == toSearch) {
            QTreeWidgetItem* toSelect = *it;
            ui->skeletonTree->blockSignals(true);
            ui->skeletonTree->setCurrentItem(toSelect);
            toSelect->setExpanded(true);
            if (toSelect->childCount() > 0) {
                toSelect->child(0)->setExpanded(true);
            }
            ui->skeletonTree->blockSignals(false);
            renderer->setCurrentMesh(mesh);
            return ;
        }
        ++it;
    }
}

void MainWindow::onToggleWireframe()
{
    renderer->setWireframe(!renderer->isWireframe());
}

void MainWindow::onReleaseCloth()
{
    if (_cloth) {
        _cloth->release();
    }
}

void MainWindow::onToggleFlag()
{
    if (_cloth) {
        Material* current = _cloth->material();
        Material* newMat = current == _matOriginal ? _matFlag : _matOriginal;
        _cloth->setMaterial(newMat);
        _cloth->removeChild(current);
        _cloth->addChild(newMat);
        _loadSceneTree();
    }
}

void MainWindow::onResetCloth()
{
    if (_cloth) {
        delete _cloth;
    }
    _cloth = new Cloth(NULL);
    _cloth->init();
    _cloth->setRectangularCloth(40, 40, 3, 3, true);
//    _cloth->setRectangularCloth(30, 30, 1, 3, false);

    _matOriginal = _cloth->material();
    _matFlag = new Material(NULL);
    _matFlag->loadTexture(ROOT_PATH "/flag.png");

    _cloth->setPlane(_plane);
    _cloth->setSphere(_sphere);

    ui->glView->scene()->addNode(_cloth);

    _loadSceneTree();
}

void MainWindow::_loadSceneTree()
{
    ui->skeletonTree->clear();
    Scene* scene = ui->glView->scene();
    _loadNode(NULL, scene->root());
}

void MainWindow::_loadNode(QTreeWidgetItem *parent, Node *current)
{
    TreeItem<Node*>* currentItem;
    if (parent == NULL) {
        currentItem = new TreeItem<Node*>(ui->skeletonTree);
        currentItem->setExpanded(true);
    } else {
        currentItem = new TreeItem<Node*>(parent);
    }
    currentItem->setUserData(current);
    currentItem->setText(0, current->name());
    currentItem->setText(1, current->type());
    QList<DOF*> const& DOFs = current->DOFs();
    if (DOFs.size() > 0) {
        TreeItem<Node*>* DOFsItem = new TreeItem<Node*>(currentItem);
        DOFsItem->setUserData(current);
        DOFsItem->setText(0, "DOFs");
        foreach (DOF* currentDOF, DOFs) {
            TreeItem<DOF*>* DOFItem = new TreeItem<DOF*>(DOFsItem);
            DOFItem->setUserData(currentDOF);
            DOFItem->setText(0, currentDOF->name());

            TreeItem<DOF*>* DOFValueItem = new TreeItem<DOF*>(DOFItem);
            DOFValueItem->setUserData(currentDOF);
            DOFValueItem->setText(0, "Value");
            if (currentDOF->isEnabled()) {
                DoubleSpinBox<DOF*>*    spinValue = new DoubleSpinBox<DOF*>(ui->skeletonTree);
                spinValue->setUserData(currentDOF);
                spinValue->setSingleStep(0.01);
                ui->skeletonTree->setItemWidget(DOFValueItem, 1, spinValue);
                if (currentDOF->hasMin())
                    spinValue->setMinimum(currentDOF->min());
                else
                    spinValue->setMinimum(-1000000);
                if (currentDOF->hasMax())
                    spinValue->setMaximum(currentDOF->max());
                else
                    spinValue->setMaximum(1000000);
                spinValue->setValue(currentDOF->value());
                connect(spinValue, SIGNAL(valueChanged(double)), SLOT(onValueChanged(double)));
            } else {
                DOFValueItem->setText(1, QString::number(currentDOF->value()));
            }

            if (currentDOF->hasMin()) {
                TreeItem<DOF*>* DOFMinItem = new TreeItem<DOF*>(DOFItem);
                DOFMinItem->setUserData(currentDOF);
                DOFMinItem->setText(0, "Min");
                DOFMinItem->setText(1, QString::number(currentDOF->min()));
            }

            if (currentDOF->hasMax()) {
                TreeItem<DOF*>* DOFMaxItem = new TreeItem<DOF*>(DOFItem);
                DOFMaxItem->setUserData(currentDOF);
                DOFMaxItem->setText(0, "Max");
                DOFMaxItem->setText(1, QString::number(currentDOF->max()));
            }
        }
    }
    foreach (Node* currentChild, current->children()) {
        if (currentChild->visible())
            _loadNode(currentItem, currentChild);
    }
}
