#include "CommandRunner.h"

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
    r.sendAndAck("G1 Z3 F200");
    r.sendAndAck("M400");
    r.sendAndAck("G1 X60 Y50 F2000");
    r.sendAndAck("M400");
    r.sendAndAck("G1 X0 Y100 F700");
    r.sendAndAck("M400");
    r.sendAndAck("M18");
}