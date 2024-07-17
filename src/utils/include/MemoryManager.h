/**
 * @file MemoryManager.h
 * @author Mujdei Ruben
 * @brief 
 * @version 0.1
 * @date 2024-07-16
 * 
 * @copyright Copyright (c) 2024
 *  * !!!!Run wtih sudo when run programm
 * 
 * Commands to create sdcard
 * truncate -s +300M /home/projectx/sdcard.img
 * sudo losetup -fP sdcard.img
 * sudo losetup -a
 * sudo fdisk /dev/loop21
 * sudo mkfs.fat -F 32 /dev/loop21p1
 * sudo mkfs.fat -F 16 /dev/loop21p2
 * sudo mount -o rw,uid=1000,gid=1000 /dev/loop21 /mnt/sdcard
 * sudo xxd -l 17168 -s 118006272 /dev/loop21
 * sudo dd if=/dev/zero of=/dev/loop21 bs=1 seek=118006272 count=17168 conv=notrunc(delete memory)
 * MAIN:
    int main()
    {
        Logger l("ddd","log.log");
        off_t address = 230481 * 512; // Offset for /dev/loop21p2, dummy

        // This lines are called in the request download service to set the address and the path 
        MemoryManager* install = MemoryManager::getInstance(address, "/dev/loop21",&l);
    
        // data from an executable. Simulate data from transfer data frame
        std::vector<uint8_t> data = MemoryManager::readBinary("/home/projectx/Desktop/PoC/src/mcu/main");

        // This lines are called in the transfer data service to save the data 
        MemoryManager* install2 = MemoryManager::getInstance();
        install2->writeToAddress(data);

        // line called to move from adress to the executable(Can be implemented in a routine) 
        std::vector<uint8_t> data2 = MemoryManager::readFromAddress("/dev/loop21", install2->getAddress(), data.size()); //Read from the address
        MemoryManager::writeToFile(data2, "/mnt/sdcard/test"); //Write in bin the data
    }
    */
#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <iomanip>
#include <stdexcept>

#include <unistd.h>
#include <fcntl.h>

#include "Logger.h"

class MemoryManager
{
    private:
        std::string path;
        off_t address = -1;
        /*  Address to track where the last write was made */
        off_t address_continue_to_write = -1;
        static MemoryManager* instance;
        Logger* logger;

        /**
         * @brief Construct a new Memory Manager object
         * 
         * @param address 
         * @param path 
         * @param logger 
         */
        MemoryManager(off_t address, std::string path, Logger* logger);

        /**
         * @brief Method to transform a string number to type integer
         * 
         * @param number 
         * @return int 
         */
        int to_int(std::string number);

        /**
         * @brief Method to check if the address is available
         * 
         * @param address 
         * @return true or false
         */
        bool availableAddress(off_t address);

        /**
         * @brief Method to check if the amount of memory is available
         * 
         * @param size_of_data 
         * @return true or false
         */
        bool availableMemory(off_t size_of_data);

        /**
         * @brief Method to run a command in the bash terminal
         * 
         * @param command 
         * @return std::string 
         */
        std::string runCommand(char command[]);

    public:
        /**
         * @brief Get the Instance object
         * 
         * @return MemoryManager*, the only instance
         */
        static MemoryManager* getInstance();

        /**
         * @brief Get the Instance object
         * 
         * @param address 
         * @param path 
         * @param logger 
         * @return MemoryManager*, the only instance
         */
        static MemoryManager* getInstance(off_t address, std::string path, Logger* logger);

        /**
         * @brief Singletons should not be cloneable.
         * 
         * @param object MemoryManager object
         */
        MemoryManager(MemoryManager &object) = delete;

        /**
         * @brief Set the Address object
         * 
         * @param address 
         */
        void setAddress(off_t address);

        /**
         * @brief Get the Address object
         * 
         * @return off_t 
         */
        off_t getAddress();

        /**
         * @brief Set the Path object
         * 
         * @param path 
         */
        void setPath(std::string path);

        /**
         * @brief Get the Path object
         * 
         * @return std::string 
         */
        std::string getPath();

        /**
         * @brief Method to read a binary file. This is a static method.
         * 
         * @param path_to_binary 
         * @return std::vector<uint8_t> 
         */
        static std::vector<uint8_t> readBinary(std::string path_to_binary);

        /**
         * @brief Method to read from an address. This is a static method.
         * 
         * @param path Path to a sd, usb, etc
         * @param address_start Start address
         * @param size Amount of size to read
         * @return std::vector<uint8_t> 
         */
        static std::vector<uint8_t> readFromAddress(std::string path, off_t address_start, off_t size);

        /**
         * @brief Method to write data in a specific address. This method uses the address specified in the constructor.
         * 
         * @param data Data to be written
         * @return true -if data was written successfully or false -if data wasn't written 
         */
        bool writeToAddress(std::vector<uint8_t> &data);

        /**
         * @brief Method to write data in a specific file. This is a static method.
         * 
         * @param data 
         * @param path_file 
         * @return true or false
         */
        static bool writeToFile(std::vector<uint8_t> &data, std::string path_file);
};

#endif