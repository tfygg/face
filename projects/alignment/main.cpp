#include "label/window.h"
using namespace snow::face;

int main() {
    snow::App app;
    LabelWindow *win = new LabelWindow();
    win->openImage("../assets/fate.jpg");
    app.addWindow(win);
    app.run();

    return 0;
}