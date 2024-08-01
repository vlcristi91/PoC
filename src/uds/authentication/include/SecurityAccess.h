/**
 * @file SecurityAccess.h
 * @author Theodor Stoica
 * @brief UDS Service 0x27 Security Access
 * @version 0.1
 * @date 2024-07-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef UDS_SECURITY_ACCESS_H
#define UDS_SECURITY_ACCESS_H

#include <linux/can.h>
#include <vector>
#include <unordered_map>
#include <bitset>
#include <iomanip>
#include <random>
#include <chrono>

#include "../../../utils/include/GenerateFrames.h"
#include "../../utils/include/Logger.h"

class SecurityAccess
{
    public:
    /* SID for SecurityAccess */
    static constexpr uint8_t SECURITY_ACCESS_SID = 0x27;
    /* SubFunctionNotSupported */
    static constexpr uint8_t SFNS = 0x12;
    /* IncorrectMesssageLengthOrInvalidFormat */
    static constexpr uint8_t IMLOIF = 0x13;
    /* RequestSequenceError */
    static constexpr uint8_t RSE = 0x24;
    /* Invalid key */
    static constexpr uint8_t IK = 0x35;
    /* Exceeded nr of attempts */
    static constexpr uint8_t ENOA = 0x36;
    /* Required time delay not expired */
    static constexpr uint8_t RTDNE = 0x37;
    /* Adjust delay timer here. */
    static constexpr uint8_t TIMEOUT_IN_SECONDS = 0x05;
    /* Adjust nr of attempts here. */
    static constexpr uint8_t MAX_NR_OF_ATTEMPTS = 3;

    private:
        GenerateFrames* generate_frames;
        Logger& security_logger;
        int socket = -1;
        static uint8_t nr_of_attempts;
        static bool mcu_state;
        static uint32_t time_left;
        static std::vector<uint8_t> security_access_seed;
    
    public:
        /**
         * @brief Constructs a SecurityAccess object with a specified socket and logger.
         * This constructor initializes the SecurityAccess object using the provided
         * socket for communication and a reference to a Logger object for logging 
         * security-related events.
         *
         * @param socket The socket descriptor used for communication.
         * @param security_logger Reference to a Logger object for logging security events.
        */
        SecurityAccess(int socket, Logger& security_logger);
        /**
         * @brief Main method to the 0x27 Security Access UDS service.
         * Processes security access using the specified CAN ID and data.
         *
         * @param can_id The CAN identifier used for the security access operation.
         * @param data A vector containing the data bytes to be processed.
         * @return void
        */
        void securityAccess(canid_t can_id, const std::vector<uint8_t>& data);
        /**
         * @brief Getter for MCU state access.
         * 
         * @return The current value of MCU state(true or false).
        */
        static bool getMcuState();

    private:
        /**
         * @brief Computes a security key based on the provided seed using bitwise operations.
         * This function generates a security key from the given seed vector by computing 
         * the two's complement of each byte in the seed. The two's complement is obtained 
         * by inverting the bits of each byte and adding one.
         *
         * @param seed A vector of bytes representing the seed from which the key is computed.
         * @return A vector of bytes representing the computed security key.
        */
        std::vector<uint8_t> computeKey(const std::vector<uint8_t>& seed);
        /**
         * @brief Generates a vector of random bytes representing the seed.
         * This method creates a vector of the specified length filled with random bytes. 
         * It uses a random number generator with a uniform distribution to ensure each byte 
         * value is between 0 and 255.
         * 
         * @param length The number of random bytes to generate for seed.
         * @return A vector of random bytes representing the seed.
         */
        std::vector<uint8_t> generateRandomBytes(size_t length);
        /**
         * @brief Converts a time value in seconds to a CAN frame format.
         * This method constructs a CAN frame consisting of a predefined sequence 
         * followed by padding bytes (if necessary) and the time value in seconds, 
         * ensuring the frame is exactly 8 bytes long. The time value is added in 
         * big-endian format.
         * 
         * @param timeInSeconds The time value in seconds to be included in the frame (max 4 bytes).
         * @return A vector of bytes representing the CAN frame.
        */
        std::vector<uint8_t> convertTimeToCANFrame(uint32_t timeInSeconds);
};

#endif