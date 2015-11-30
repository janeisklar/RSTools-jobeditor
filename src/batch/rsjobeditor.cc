#include <QtGui>
#include <QApplication>
#include <QWidget>
#include "jobeditor/rsjobeditorapplication.h"
#include "rscommon.h"
#include "utils/rsstring.h"

class JobEditorApplication : public QApplication {
    public:
        JobEditorApplication(int &argc, char **argv) : QApplication(argc, argv)
        { }

        virtual ~JobEditorApplication() { }

        virtual bool notify(QObject * receiver, QEvent * event) {
            try {
                return QApplication::notify(receiver, event);
            } catch(std::exception& e) {
                qCritical() << "Exception thrown:" << e.what();
            }
            return false;
        }
};

int main(int argc, char *argv[])
{
    JobEditorApplication app(argc, argv);
    JobEditorWindow widget;
    if ( argc > 1 ) {
        widget.openJob(rsString(argv[1]));
    } else {
        widget.openJob(rsString(RSTOOLS_DATA_DIR"/rstools/jobs/empty.job"));
    }
    widget.show();
    return app.exec();
}
