void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
#include <iostream>
#include <string>
#include <vector>

// A list of known OUI values assigned by the IEEE
std::vector<int> knownOUIs = {0x0000C0, 0x08002B, 0x080030};

bool isRandomizedMAC(std::string macAddress) {
    // Extract the first 24 bits from the MAC address
    std::string ouiStr = macAddress.substr(0, 8);

    // Convert the 24-bit value to an integer
    int oui = std::stoi(ouiStr, nullptr, 16);

    // Check if the OUI matches any known OUIs
    for (int i = 0; i < knownOUIs.size(); i++) {
        if (oui == knownOUIs[i]) {
            return false;
        }
    }

    return true;
}

int main() {
    std::string mac1 = "02:00:00:00:00:00"; // Randomized MAC address
    std::string mac2 = "AC:DE:48:00:11:22"; // Non-randomized MAC address

    if (isRandomizedMAC(mac1)) {
        std::cout << mac1 << " is a randomized MAC address" << std::endl;
    } else {
        std::cout << mac1 << " is not a randomized MAC address" << std::endl;
    }

    if (isRandomizedMAC(mac2)) {
        std::cout << mac2 << " is a randomized MAC address" << std::endl;
    } else {
        std::cout << mac2 << " is not a randomized MAC address" << std::endl;
    }

    return 0;
}
