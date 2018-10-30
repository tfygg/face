#include "label/window.h"
using namespace snow::face;

int main() {
    snow::App app;
    LabelWindow *win = new LabelWindow({
        "../assets/fate.jpg",
        "../assets/0-0-1.mkv",
    });
    app.addWindow(win);
    app.run();

    return 0;
}