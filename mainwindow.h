#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma execution_character_set("utf-8")
#include <QMainWindow>
#include <sstream>
#include "lexer.h"
#include "Parsing.h"
#include "Optimizer.h"
#include "objectCode.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Lexer lex;
    parsing parser;
    Optimizer optimizer;
    ObjectCode objectcode;
    stringstream midblock;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void Lex_Parser();
    void json_button();
    void dot_button();
    void midcode_button();
    void IBlock_button();
    void optimizer_block_button();
    void object_Iblocks_button();
    void object_code_button();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
