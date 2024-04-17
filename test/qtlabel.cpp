#include <QApplication>
#include <QLabel>
#include <QImage>
#include <QPixmap>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QLabel label;
    QPixmap pixmap("frame0.jpg");
    label.setPixmap(pixmap);
    label.show();
    return a.exec();
}
