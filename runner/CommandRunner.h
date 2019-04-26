#pragma once

#include <string>
#include <vector>

class CommandRunner {
public:
    CommandRunner(int fileNo) : lineNo(1), file(fileNo) {}
    CommandRunner(const char* fileName);

    void sendCommand(const std::string& cmd, std::string* response = nullptr);
    bool sendAndAck(const std::string& cmd);
    void resetLineCount();
    std::string readResponse();
    std::string readData();
    bool isReady() const { return file >= 0; }

private:
    uint8_t crc(const std::string&) const;
    void send(const std::string& cmd) const;
    void formatLine(const std::string& cmd, std::string* line) const;
    static bool isRetriable(const std::string& response);
    static bool isResend(const std::string& response);
    static bool isError(const std::string& response);
    static bool isok(const std::string& response);
    static void splitResponse(
        const std::string& response,
        std::vector<std::string>* split);
    static bool responseContains(
        const std::vector<std::string> responses,
        bool (*cb)(const std::string&));
    void flush();
    

    int lineNo;
    int file;
};
