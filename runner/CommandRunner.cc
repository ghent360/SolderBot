#include "CommandRunner.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <memory.h>
#include "stringprintf.h"

CommandRunner::CommandRunner(const char* fileName) : lineNo(0), file(-1) {
    int f = open(
        fileName, 
        O_RDWR | O_CLOEXEC | O_NOCTTY | O_NDELAY | O_NONBLOCK);
    if (f >= 0) {
        struct termios tty;

        memset(&tty, 0, sizeof tty);
        if ( tcgetattr(f, &tty) != 0 ) {
            std::string err = base::StringPrintf("Error getting TTY attributes of %s", fileName);
            perror(err.c_str());
            close(f);
        } else {
            cfsetospeed (&tty, (speed_t)B115200);
            cfsetispeed (&tty, (speed_t)B115200);

            tty.c_cflag     &=  ~PARENB;            // Make 8n1
            tty.c_cflag     &=  ~CSTOPB;
            tty.c_cflag     &=  ~CSIZE;
            tty.c_cflag     |=  CS8;

            tty.c_cflag     &=  ~CRTSCTS;           // no flow control
            tty.c_cc[VMIN]   =  1;                  // read doesn't block
            tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
            tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines
            cfmakeraw(&tty);
            /* Flush Port, then applies attributes */
            tcflush(f, TCIFLUSH);
            if ( tcsetattr(f, TCSANOW, &tty) != 0) {
                std::string err = base::StringPrintf("Error setting TTY attributes of %s", fileName);
                perror(err.c_str());
                close(f);
            } else {
                file = f;
                lineNo = 1;
                usleep(10000);
                flush();
            }
        }
    } else {
        std::string err = base::StringPrintf("Error openinf terminal %s", fileName);
        perror(err.c_str());
    }
}

void CommandRunner::flush() {
    std::string data;
    do {
        data = readData();
    } while (!data.empty());
}

void CommandRunner::resetLineCount() {
    lineNo = 0;
    sendAndAck("M110");
}

uint8_t CommandRunner::crc(const std::string& cmd) const {
    uint8_t crc = 0;
    for(char c : cmd) {
        crc ^= c;
    }
    return crc;
}

void CommandRunner::formatLine(const std::string& cmd, std::string* line) const {
    base::SStringPrintf(line, "N%d %s", lineNo, cmd.c_str());
    uint8_t crcVal = crc(*line);
    base::StringAppendF(line, "*%d\n", crcVal);
}

void CommandRunner::sendCommand(const std::string& cmd, std::string* response) {
    std::string line;
    formatLine(cmd, &line);
    do {
        send(line);
        if (response) {
            do {
                *response = readResponse();
            } while (isRetriable(*response));
        }
    } while (response && isResend(*response));
    lineNo++;
}

void CommandRunner::splitResponse(
    const std::string& response,
    std::vector<std::string>* split) {
    const static std::string delimiter("\n");

    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;

    while ((pos_end = response.find(delimiter, pos_start)) != std::string::npos) {
        token = response.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        if (!token.empty()) {
            split->push_back(token);
        }
    }

    if (pos_start < response.length()) {
        split->push_back(response.substr(pos_start));
    }
}

bool CommandRunner::isRetriable(const std::string& response) {
    static const std::string busy("busy:");
    static const std::string echobusy("echo:busy:");

    return response.compare(0, busy.length(), busy) == 0 ||
        response.compare(0, echobusy.length(), echobusy) == 0;
}

bool CommandRunner::isResend(const std::string& response) {
    static const std::string rs("rs");
    static const std::string resend("Resend");

    return response.compare(0, rs.length(), rs) == 0 ||
        response.compare(0, resend.length(), resend) == 0;
}

bool CommandRunner::isError(const std::string& response) {
    static const std::string er("!!");
    static const std::string error("Error:");

    return response.compare(0, er.length(), er) == 0 ||
        response.compare(0, error.length(), error) == 0;
}

bool CommandRunner::isok(const std::string& response) {
    static const std::string ok("ok");

    return response.compare(0, ok.length(), ok) == 0;
}

bool CommandRunner::responseContains(
        const std::vector<std::string> responses,
        bool (*cb)(const std::string&)) {
    for(const auto& r : responses) {
        if (cb(r)) {
            return true;
        }
    }
    return false;
}

bool CommandRunner::sendAndAck(const std::string& cmd) {
    std::string resp;
    sendCommand(cmd, &resp);
    std::vector<std::string> responses;
    splitResponse(resp, &responses);
    return responseContains(responses, isok) && !responseContains(responses, isError);
}

void CommandRunner::send(const std::string& line) const {
    ssize_t result;
    const char* bfr = line.c_str();
    size_t size = line.size();
    printf("! %s\n", bfr);
    do {
        result = ::write(file, bfr, size);
        bfr += result;
        size -= result;
        if (size > 0 && result <= 0) {
            usleep(250000);
        }
    } while (size > 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
}

std::string CommandRunner::readResponse() {
    std::string response;
    std::string rsp;
    do {
        rsp = readData();
        response.append(rsp);
        if (rsp.empty()) {
            usleep(10000);
        }
    } while (response.empty() || response.find('\n') == response.npos);
    return response;
}

std::string CommandRunner::readData() {
    std::string result;
    char buffer[513];
    ssize_t res;

    do {
        res = ::read(file, buffer, sizeof(buffer) - 1);
        if (res > 0) {
            buffer[res] = 0;
            result.append(buffer);
            printf("> %s\n", buffer);
        }
    } while (res > 0);
    return result;
}

