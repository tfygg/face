#include "label/window.h"
using namespace snow::face;

int main() {
    // set landmarks
    Landmarks::SetNumPoints(73);
    Landmarks::SetGenreList({"auto", "manual"});
    
    snow::App app;
    LabelWindow *win = new LabelWindow({
        "../assets/fate.jpg",
        "../assets/0-0-1.mkv",
        "../assets/hello.flv"
    });
    app.addWindow(win);
    app.run();

    return 0;
}