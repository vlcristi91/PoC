/*Author:Stoisor Miruna, 2024*/

#include <sstream>
#include <iomanip>
#include "../include/ReceiveFrames.h"
#include "../include/HandleFrames.h"

ReceiveFrames::ReceiveFrames(int socket, int moduleID) {
    // Print the moduleID for debugging
    std::cout << "Module ID: " << this->moduleID << std::endl;
    if (socket >= 0)
    {
        this->socket = socket;
    } else{
        std::cout<<"Error: Pass a valid Socket\n";
        exit(EXIT_FAILURE);
    }

    //Convert moduleID to hexadecimal before comparison
    std::stringstream ss;
    ss << std::hex << moduleID;
    int moduleID_hex;
    ss >> moduleID_hex;
    const int MIN_VALID_ID = 0x00000000;
    const int MAX_VALID_ID = 0x7FFFFFFF;

    if (moduleID >= MIN_VALID_ID && moduleID <= MAX_VALID_ID) {
        this->moduleID = moduleID;
    } else{
        std::cout << "Error: Pass a valid Module ID\n";
        exit(EXIT_FAILURE);
    }
}

bool ReceiveFrames::Receive(HandleFrames &handleFrame) {
    struct can_frame frame;
    int nbytes = read(this->socket, &frame, sizeof(struct can_frame));

    // Check if the received frame is valid and complete
    if (nbytes < 0) {
        std::cerr << "Read error\n";
        return false;
    } else if (nbytes < sizeof(struct can_frame)) {
        std::cerr << "Incomplete frame read\n";
        return false;
    }

    // Check if the received frame is for your module
    if (frame.can_id != this->moduleID) {
        std::cerr << "Received frame is not for this module\n";
        return false;
    }

    // Check if the received frame is empty
    if (frame.can_dlc == 0) {
        std::cerr << "Received empty frame\n";
        return false;
    }

    // Print the frame for debugging
    PrintFrame(frame);
    
    // Process the received frame
    handleFrame.ProcessReceivedFrame(frame);

    return true;
}

void ReceiveFrames::PrintFrame(const struct can_frame &frame) {
    std::cout << "Received CAN frame:" << std::endl;
    std::cout << "CAN ID: 0x" << std::hex << int(frame.can_id) << std::endl;
    std::cout << "Data Length: " << int(frame.can_dlc) << std::endl;
    std::cout << "Data:";
    for (int i = 0; i < frame.can_dlc; ++i) {
        std::cout << " 0x" << std::hex << int(frame.data[i]);
    }
}

//modul de loging generic pt 3 tipuri debug: info, warning(std, received, empty frame), error
