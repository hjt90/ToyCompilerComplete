#pragma execution_character_set("utf-8")
#include <QString>
#include <sstream>
#include <fstream>
#include <string>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow* parent;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    parent = this;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Lex_Parser()
{
    stringstream finSyntax;
    QString input_qstr = this->ui->InputBox->toPlainText();
    stringstream input;
    input << input_qstr.toStdString();

    QFile file(":/parse/part_parse.cc");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray t = file.readAll();
    finSyntax.str(QString(t).toStdString());
    file.close();

    lex = Lexer();
    parser.clear();
    optimizer.clear();
    objectcode.clear();

    lex.analyze(input);

    parser.initSyntax(finSyntax);
    parser.analyze(lex.output());

    optimizer.divideBlocks(parser, parser.get_proc_symbolTable());
    midblock.str("");
    optimizer.outputBlocks(midblock);
    optimizer.optimizer();

    objectcode.analyseBlock(optimizer);
    objectcode.generateCode(parser.get_proc_symbolTable());

    stringstream object_code;
    objectcode.outputObjectCode(object_code);
    this->ui->outBox->clear();
    this->ui->outBox->insertPlainText(QString::fromStdString(object_code.str()));

    this->ui->json_button->setEnabled(true);
    this->ui->dot_button->setEnabled(true);
    this->ui->midcode_button->setEnabled(true);
    this->ui->IBlock_button->setEnabled(true);
    this->ui->optimizer_block_button->setEnabled(true);
    this->ui->object_Iblocks_button->setEnabled(true);
    this->ui->object_code_button->setEnabled(true);
}

void MainWindow::json_button()
{
    stringstream struction, graph;
    parser.output(struction, graph);
    this->ui->outBox->clear();
    this->ui->outBox->insertPlainText(QString::fromStdString(struction.str()));
}

void MainWindow::dot_button()
{
    stringstream struction, graph;
    parser.output(struction, graph);
    this->ui->outBox->clear();
    this->ui->outBox->insertPlainText(QString::fromStdString(graph.str()));
}

void MainWindow::midcode_button()
{
    stringstream midcode;
    parser.outputMidcode(midcode);
    this->ui->outBox->clear();
    this->ui->outBox->insertPlainText(QString::fromStdString(midcode.str()));
}

void MainWindow::IBlock_button()
{
    this->ui->outBox->clear();
    this->ui->outBox->insertPlainText(QString::fromStdString(midblock.str()));
}

void MainWindow::optimizer_block_button()
{
    stringstream optimizer_block;
    optimizer.outputBlocks(optimizer_block);
    this->ui->outBox->clear();
    this->ui->outBox->insertPlainText(QString::fromStdString(optimizer_block.str()));
}

void MainWindow::object_Iblocks_button()
{
    stringstream object_Iblocks;
    objectcode.outputIBlocks(object_Iblocks);
    this->ui->outBox->clear();
    this->ui->outBox->insertPlainText(QString::fromStdString(object_Iblocks.str()));
}

void MainWindow::object_code_button()
{
    stringstream object_code;
    objectcode.outputObjectCode(object_code);
    this->ui->outBox->clear();
    this->ui->outBox->insertPlainText(QString::fromStdString(object_code.str()));
}

