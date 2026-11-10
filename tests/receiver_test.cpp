#include "UDP_Receiver.h"
#include "protocol.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
} // namespace

int main() {
    try {
        for (const auto text : {"0 0 0 2 0 0 0", "0 0 0 4 1 0 0", "0 0 0 0 0 0 -120",
                                "1 0 0 65 0 0 0", "1 0 0 65 1 0 0",
                                "2 -100 200 0 0 0 0", "3 0 0 0 0 1 0", "3 0 0 0 0 2 0"}) {
            const auto parsed = parse_instruction(text);
            require(format_instruction(parsed) == text, "Wire-format roundtrip failed.");
        }
        std::string terminated = "2 12 34 0 0 0 0";
        terminated.push_back('\0');
        require(parse_instruction(terminated).y == 34, "Legacy terminator rejected.");
        std::vector<std::string> invalid{
            "", "4 0 0 0 0 0 0", "1 0 0 256 0 0 0", "1 0 0 65 2 0 0", "0 0 0 32 0 0",
            "1 0 0 65 0 0 0 0", "1 0 0 65 0 0 0junk", "3 0 0 0 0 3 0", "3 0 0 0 0 0 0",
            "1 2 0 65 0 0 0", "2 2147483648 0 0 0 0 0", "0 0 0 65535 0 0 0",
            "0 0 0 2 0 0 120"
        };
        invalid.emplace_back("1 0 0\0 65 0 0 0", 15);
        invalid.emplace_back(129, '1');
        for (const auto& text : invalid) {
            bool rejected = false;
            try { (void)parse_instruction(text); } catch (const std::invalid_argument&) { rejected = true; }
            require(rejected, "Malformed packet accepted.");
        }

        UDP_Receiver receiver(0);
        const SOCKET sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        require(sender != INVALID_SOCKET, "Cannot create test sender.");
        struct Close { SOCKET socket; ~Close() { closesocket(socket); } } close{sender};
        sockaddr_in destination{};
        destination.sin_family = AF_INET;
        destination.sin_port = htons(receiver.Port());
        destination.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        for (const std::string& payload : {std::string("3 123 456"), std::string("5 0 0"), std::string(65507, 'x')}) {
            require(sendto(sender, payload.data(), static_cast<int>(payload.size()), 0,
                    reinterpret_cast<const sockaddr*>(&destination), sizeof(destination)) ==
                    static_cast<int>(payload.size()), "Send failed.");
            const auto received = receiver.Receive();
            require(received && received->payload == payload && received->sender == "127.0.0.1",
                    "Receive changed packet length/data or peer.");
        }
        require(!receiver.Receive(), "Idle receive should time out.");
        std::cout << "Protocol validation, exact UDP datagrams and timeout passed; no input applied.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
