#include <stdio.h>
#include "vehicle_simulation.h"

int main(void) {
    if (RunDrivingSimulation() != 0) {
        fprintf(stderr, "Driving simulation failed\n");
        return 1;
    }

    if (RunControlSimulation() != 0) {
        fprintf(stderr, "Control simulation failed\n");
        return 1;
    }

    return 0;
}