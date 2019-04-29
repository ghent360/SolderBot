#include "CommandRunner.h"
#include <unistd.h>

int main() {
    CommandRunner r("/dev/ttyACM0");
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
    r.sendAndAck("M106 P2 S255");
    r.sendAndAck("G90");
    r.sendAndAck("G1 Z9 F800");
    r.sendAndAck("M400");
    //r.sendAndAck("G1 X51.67 Y113.5 F10000");
    r.sendAndAck("G1 X54.14 Y113.5 F10000");
    r.sendAndAck("M400");
    r.sendAndAck("G91");
    for (int i=0; i < 20; i++) {
        r.sendAndAck("G1 Z-2.5 F800");
        r.sendAndAck("G1 Z-0.5 F200");
        r.sendAndAck("G1 X0.5 Z-0.4 F100");
        r.sendAndAck("M400");
        usleep(750000);
        r.sendAndAck("G1 E7 F800");
        r.sendAndAck("M400");
        r.sendAndAck("G1 E2 F200");
        r.sendAndAck("M400");
        sleep(3);
        r.sendAndAck("G1 E1 F200");
        r.sendAndAck("M400");
        r.sendAndAck("G1 E-7.5 F2000");
        r.sendAndAck("M400");
        usleep(500000);
        r.sendAndAck("G1 X-0.5 Z3.4 F800");
        r.sendAndAck("M400");
        r.sendAndAck("G1 Y2.54 F5000");
        r.sendAndAck("M400");
    }
    r.sendAndAck("G90");
    r.sendAndAck("G1 X0 Y0 F10000");
    //r.sendAndAck("M106 P2 S0");
    r.sendAndAck("M18");
}