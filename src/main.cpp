#include "EmbeddedSystemX.hpp"

#include <sstream>

namespace {

void help() {
    std::cout << "Commands (case sensitive):\n"
              << "  SelftestOk | SelfTestFailed <integer> | Initialized\n"
              << "  Restart | Exit | state | help | quit\n"
              << "  Configure | ConfigX | ConfigurationEnded | Start | Stop\n"
              << "  chMode1 | chMode2 | eventX | eventY\n"
              << "Mode changes run on a worker; the console waits for completion.\n"
              << "Exit is a state-machine event, valid in Failure.\n"
              << "quit closes this console from any state.\n";
}

struct Command {
    const char* name;
    void (erts::EmbeddedSystemX::*invoke)();
};

constexpr Command commands[] = {
    {"SelftestOk", &erts::EmbeddedSystemX::SelftestOk},
    {"Initialized", &erts::EmbeddedSystemX::Initialized},
    {"Restart", &erts::EmbeddedSystemX::Restart},
    {"Exit", &erts::EmbeddedSystemX::Exit},
    {"Configure", &erts::EmbeddedSystemX::Configure},
    {"ConfigX", &erts::EmbeddedSystemX::ConfigX},
    {"ConfigurationEnded", &erts::EmbeddedSystemX::ConfigurationEnded},
    {"Start", &erts::EmbeddedSystemX::Start},
    {"Stop", &erts::EmbeddedSystemX::Stop},
    {"chMode1", &erts::EmbeddedSystemX::chMode1},
    {"chMode2", &erts::EmbeddedSystemX::chMode2},
    {"eventX", &erts::EmbeddedSystemX::eventX},
    {"eventY", &erts::EmbeddedSystemX::eventY},
};

} // namespace

int main() {
    erts::EmbeddedSystemX system;
    help();
    system.displayState();
    std::string line;
    while (!system.terminated() && std::cout << "> " && std::getline(std::cin, line)) {
        std::istringstream input(line);
        std::string command;
        if (!(input >> command)) continue;

        if (command == "SelfTestFailed") {
            int errorNo;
            if (!(input >> errorNo) || !(input >> std::ws).eof()) {
                std::cout << "Usage: SelfTestFailed <integer>\n";
                continue;
            }
            system.SelfTestFailed(errorNo);
            system.displayState();
            continue;
        }

        if (!(input >> std::ws).eof()) {
            std::cout << "This command takes no arguments.\n";
            continue;
        }
        if (command == "quit") break;
        if (command == "help") { help(); continue; }
        if (command == "state") { system.displayState(); continue; }

        bool found = false;
        for (const auto& entry : commands) {
            if (command == entry.name) {
                (system.*entry.invoke)();
                system.displayState();
                found = true;
                break;
            }
        }
        if (!found) std::cout << "Unknown command. Type help for commands.\n";
    }
}
