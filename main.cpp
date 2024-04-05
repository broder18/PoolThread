#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include "thread_pool.h"

void test_void(std::shared_ptr<int> res)
{
    qDebug() << "running";
    //res = std::make_shared<int>(10);

    for(auto ind = 0; ind < 10; ind++)
        *res+=ind;

}

void test_void2(std::shared_ptr<float> res)
{
    qDebug() << "running 2";
    //res = std::make_shared<float>(6.0f);
    for(auto ind = 1; ind < 10; ind++)
        *res += *res / ind;
}

int main(int argc, char *argv[])
{
    //QApplication a(argc, argv);
    //MainWindow w;
    //w.show();
    //return a.exec();
    ThreadPool pool;
    std::shared_ptr<int> res = std::make_shared<int>(10);
    std::shared_ptr<float> res2 = std::make_shared<float>(6.0f);
    pool.addTask(test_void, res);
    pool.addTask(test_void2, res2);
    pool.start();
    pool.wait_all();
    qDebug() << *res;
    qDebug() << *res2;
    return 0;
}
