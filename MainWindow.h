#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVariant>

namespace Ui {
class MainWindow;
}

class QTreeWidgetItem;
class Mesh;
class Cloth;
class Plane;
class Sphere;
class Node;
class Material;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void    onInitialized();
    void    onValueChanged(double value);
    void    onSearch();
    void    onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void    onCurrentMeshChanged(Mesh* mesh);
    void    onToggleWireframe();
    void    onReleaseCloth();
    void    onToggleFlag();
    void    onResetCloth();

private:
    void    _loadSceneTree();
    void    _loadNode(QTreeWidgetItem* parent, Node *current);

    Ui::MainWindow*     ui;
    Cloth*              _cloth;
    Material*           _matOriginal;
    Material*           _matFlag;
    Plane*              _plane;
    Sphere*             _sphere;
};

#endif // MAINWINDOW_H
