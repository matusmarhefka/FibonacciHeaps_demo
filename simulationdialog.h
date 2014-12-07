#ifndef SIMULATIONDIALOG_H
#define SIMULATIONDIALOG_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QPointF>
#include <QMessageBox>
#include <cmath>
#include "interface.h"
#include "Prim.h"
#include "primsignal.h"
#include "gplace.h"
#include "gedge.h"


int weight(unsigned u, unsigned v);
void sigEvent(unsigned event);
void simulation(unsigned root);


namespace Ui {
class SimulationDialog;
}

class SimulationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SimulationDialog(unsigned m_root_id, QWidget *parent = 0);
    ~SimulationDialog();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void sig_backend(unsigned event);
    void on_verticalSlider_valueChanged(int value);

private:
    void initSimulation();
    void exitError(QString msg);
    void initPrimCode();
    void actualizeGraph();
    void printGraph();

    std::thread Simulation;
    bool running;
    bool step_in_progress;
    unsigned root_id;

    void drawGraph();

    QStringList prim_code;
    int prim_pos;

    Ui::SimulationDialog *ui;
    QGraphicsScene *scene;
    QGraphicsScene *scene2;
    QPen blackpen;
    QPen redpen;

    QGraphicsLineItem *line;
    QLineF *new_line;
    const qreal diameter = 30;
    const qreal radius = 15;
};

#endif // SIMULATIONDIALOG_H
