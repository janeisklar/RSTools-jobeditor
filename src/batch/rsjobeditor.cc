#include <QApplication>
#include <QWidget>
#include "jobeditor/rsjobeditorapplication.h"
#include "rscommon.h"
#include "utils/rsstring.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    JobEditorWindow widget;
    if ( argc > 1 ) {
        widget.openJob(argv[1]);
    } else {
        widget.openJob(rsString(RSTOOLS_DATA_DIR"/rstools/jobs/empty.job"));
    }
    widget.show();
    return app.exec();
}
