#include <unistd.h>
#include <QDebug>
#include <QQmlContext>
#include "BackEnd.h"
#include "QVtkItem.h"

#define VERTICAL_EXAGG "verticalExagg"
#define SHOW_AXES "showAxes"

/// Initialize singleton to null
BackEnd *BackEnd::singleInstance_ = nullptr;

BackEnd::BackEnd(QQmlApplicationEngine *engine,
		 QObject *parent) : QObject(parent),
  qVtkItem_(nullptr)
{
    QObject *rootObject = engine->rootObjects().first();

    QObject::connect(rootObject, SIGNAL(qmlSignal(QString)),
                     this, SLOT(qmlSlot(QString)));

    
    qVtkItem_ = rootObject->findChild<mb_system::QVtkItem *>("qVtkItem");
    if (!qVtkItem_) {
        qCritical() << "Could not find \"qVtkItem\" in QML";
        exit(1);
    }

    selectedFileItem_ = rootObject->findChild<QObject *>("selectedFile");    
    if (!selectedFileItem_) {
        qCritical() << "Could not find \"selectedFile\" in QML";
        exit(1);      
    }
}

bool BackEnd::registerSingleton(int argc, char **argv,
                                QQmlApplicationEngine *qmlEngine) {
    if (singleInstance_) {
        qInfo() << "BackEnd::registerSingleton(): Delete existing instance";
        delete singleInstance_;
    }
    singleInstance_ = new BackEnd(qmlEngine);

    bool error = false;
    for (int i = 1; i < argc; i++) {
        if ((!strcmp(argv[i], "-I") && i < argc-1) ||
                (i == argc -1 && argv[i][0] != '-')) {
            char *filename;
            if (i == argc-1) {
                // Last argument is grid file
                filename = argv[i];
            }
            else {
                // Argument following '-I' is grid file
                filename = argv[++i];
            }

            char *fullPath = realpath(argv[i], nullptr);
            if (!fullPath) {
                fprintf(stderr, "Grid file \"%s\" not found\n", filename);
                error = true;
                break;
            }

            QString urlstring("file://" + QString(fullPath));
            QUrl qUrl(urlstring);
            qDebug() << "registerSingleton(): urlstring - " << urlstring
                     << ", qUrl - " << qUrl;

            singleInstance_->setGridFile(qUrl);
            free((void *)fullPath);
        }
        else {
            fprintf(stderr, "Unknown/incomplete option: %s\n", argv[i]);
            error = true;
        }
    }
    if (error) {
        delete singleInstance_;
        singleInstance_ = nullptr;
        fprintf(stderr, "usage: %s [-I gridfile]\n", argv[0]);
        return false;
    }
    QQmlContext *rootContext = qmlEngine->rootContext();
    rootContext->setContextProperty("BackEnd", singleInstance_);
    return true;
}


bool BackEnd::setGridFile(QUrl fileURL) {

    qDebug() << "*** setGridFile() - " << fileURL;

    char *gridFilename = strdup(fileURL.toLocalFile().toLatin1().data());
    qVtkItem_->setGridFilename(gridFilename);
    qVtkItem_->update();

    selectedFileItem_->setProperty("text", fileURL.toLocalFile());

    return true;
}


void BackEnd::showAxes(bool show) {
  qDebug() << "BackEnd::showAxes(" << show << ")";
  qVtkItem_->showAxes(show);
  qVtkItem_->update();
}

void BackEnd::qmlSlot(const QString &qmsg) {
    qDebug() << "qmlSlot() qmsg: " << qmsg;

    QByteArray a;
    a.append(qmsg);
    char *msg = a.data();
    std::cout << "msg: " << msg << std::endl;

    if (!strncmp(msg, VERTICAL_EXAGG, strlen(VERTICAL_EXAGG))) {
      float verticalExagg = atof(msg + strlen(VERTICAL_EXAGG));
      std::cout << "vertical exagg: " << verticalExagg << std::endl;
      qVtkItem_->setVerticalExagg(verticalExagg);
      qVtkItem_->update();
    }
    else if (!strncmp(msg, SHOW_AXES, strlen(SHOW_AXES))) {
      if (strstr(msg, "true")) {
        showAxes(true);
      }
      else {
        showAxes(false);
      }
    }
  }
