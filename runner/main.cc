#include "CommandRunner.h"
#include "stringprintf.h"
#include <unistd.h>

void toSafeZ(CommandRunner *r);

void wait(double time) {
    if (time <= 0) return;
    int sec = (int)time;
    long usec = (long)((time - sec) * 1000000);
    if (usec < 0)
        usec = 0;
    sleep(sec);
    usleep(usec);
}

void moveToPin(CommandRunner *r) {
    // Descend fast
    r->sendAndAck("G1 Z-2.1 F800");
    // Descend slow
    r->sendAndAck("G1 Z-0.5 F200");
    // Press on pin in X and Z direction
    r->sendAndAck("G1 X0.8 Z-0.4 F100");
    r->sendAndAck("M400");
}

// Reverse moveToPin, but in one command
void moveFromPin(CommandRunner *r) {
    r->sendAndAck("G1 X-0.8 Z3 F800");
    r->sendAndAck("M400");
}

void feedSolder1(CommandRunner *r, double solderDist) {
    std::string cmd = base::StringPrintf("G1 E%f F800", 0.5 + 5.5 + solderDist);
    r->sendAndAck(cmd);
    r->sendAndAck("M400");
    r->sendAndAck("G1 E-3.4 F800"); // 3.4
    r->sendAndAck("M400");
}

void feedSolder2(CommandRunner *r, double solderDist) {
    std::string cmd = base::StringPrintf("G1 E%f F400", 0.5 + 3.4 + solderDist);
    r->sendAndAck(cmd);
    r->sendAndAck("M400");
    r->sendAndAck("G1 E-6 F2000"); // 0.5 + 5.5
    r->sendAndAck("M400");
}

void solderPin(
    CommandRunner *r,
    double solderDist,
    double heatWait1 = 5,
    double heatWait2 = 2,
    double heatWait3 = -1) {
    moveToPin(r);
    wait(heatWait1); // Wait for pin to heat up
    feedSolder1(r, solderDist);
    wait(heatWait2); // Wait for solder to melt
    feedSolder2(r, solderDist);
    if (heatWait3 < 0)
        heatWait3 = heatWait2;
    wait(heatWait3); // Wait for solder to melt
    moveFromPin(r);
}

void solderRowV(
    CommandRunner *r,
    double startX,
    double startY,
    uint32_t numPins,
    double pinSpacing = 2.54,
    double solderDist = 1,
    double heatWait1 = 5,
    double heatWait2 = 2) {
    toSafeZ(r);
    std::string start = base::StringPrintf("G1 X%f Y%f F10000", startX, startY);
    r->sendAndAck(start.c_str());
    r->sendAndAck("M400");
    r->sendAndAck("G91");
    start = base::StringPrintf("G1 Y%f F5000", pinSpacing);
    for (int i=0; i < numPins; i++) {
        solderPin(r, solderDist, heatWait1, heatWait2);
        r->sendAndAck(start.c_str()); // //r->sendAndAck("G1 Y2.54 F5000");
        r->sendAndAck("M400");
    }
}

void solderRowH(
    CommandRunner *r,
    double startX,
    double startY,
    uint32_t numPins,
    double pinSpacing = 2.54,
    double solderDist = 1,
    double heatWait1 = 5,
    double heatWait2 = 2) {
    toSafeZ(r);
    std::string start = base::StringPrintf("G1 X%f Y%f F10000", startX + (numPins - 1) * pinSpacing, startY);
    r->sendAndAck(start.c_str());
    r->sendAndAck("M400");
    r->sendAndAck("G91");
    start = base::StringPrintf("G1 X%f F5000", -pinSpacing);
    for (int i=0; i < numPins; i++) {
        solderPin(r, solderDist, heatWait1, heatWait2);
        r->sendAndAck(start.c_str()); // //r->sendAndAck("G1 Y2.54 F5000");
        r->sendAndAck("M400");
    }
}

void toSafeZ(CommandRunner *r) {
    r->sendAndAck("G90");
    r->sendAndAck("G1 Z7.2 F800");
    r->sendAndAck("M400");
}

int main() {
    CommandRunner r("/dev/ttyUSB0");
    if (!r.isReady()) {
        return -1;
    }
    std::string rsp;
    printf("Reset line count\n");
    r.resetLineCount();
    r.resetLineCount();
    printf("Home XY\n");
    r.sendAndAck("G28 XY");
    printf("Home Z\n");
    r.sendAndAck("G28 Z");
    r.sendAndAck("M106 P0 S255");
    r.sendAndAck("M42 P27 S0");
    sleep(20);

    r.sendAndAck("G91");
    r.sendAndAck("G1 E7.5 F600");
    r.sendAndAck("M400");
    sleep(1);
    r.sendAndAck("G1 E-6 F2000");
    r.sendAndAck("M400");
    r.sendAndAck("G90");

/*
    solderRowV(&r, 44.1 + 2.54, 113.5, 20);
    solderRowV(&r, 44.1, 113.5, 20);
    solderRowV(&r, 36.2, 118.4, 4);
    solderRowH(&r, 22.5, 138.53, 5);
    solderRowH(&r, 22.5, 138.53 + 2.54, 5);
    solderRowH(&r, 33.86, 156.8, 2, 3.5, 2.5, 5, 3.5);
    solderRowH(&r, 18.1, 156.8, 2, 5.08, 3.5, 5, 4);
*/
    solderRowV(&r, 6.75 + 2.54, 96.25, 19, 2.54, 0.8, 2.5, 2);
    solderRowV(&r, 6.75       , 96.25, 19, 2.54, 0.8, 2.5, 2);
    solderRowV(&r, 67.3 + 2.54, 96.25, 19, 2.54, 0.8, 2.5, 2);
    solderRowV(&r, 67.3       , 96.25, 19, 2.54, 0.8, 2.5, 2);
    r.sendAndAck("G90");
    r.sendAndAck("G1 Z15 F800");
    r.sendAndAck("G1 X0 Y0 F10000");
    r.sendAndAck("M106 P0 S0");
    r.sendAndAck("M42 P27 S255");
    r.sendAndAck("M18");
}