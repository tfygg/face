#include <string>
#include <iostream>
#include "morphable/basel.h"
#include "../utils/landmarks.h"

int main() {
    snow::face::Landmarks::SetNumPoints(73);

    BaselModel::Initialize("../assets/basel/", "face", "0.9");

    return 0;
}
