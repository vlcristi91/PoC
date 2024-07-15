#include "../include/MCUModule.h"

#ifndef UNIT_TESTING_MODE
Logger MCULogger("MCULogger", "logs/MCULogs.log");
#else
Logger MCULogger;
#endif /* UNIT_TESTING_MODE */
namespace MCU
{
    MCUModule mcu(0x01);
    /* Constructor */
    MCUModule::MCUModule(uint8_t interfaces_number) : 
                    is_running(false),
                    create_interface(CreateInterface::getInstance(interfaces_number, MCULogger)),
                    receive_frames(nullptr),
                    mcu_api_socket(create_interface->createSocket(interfaces_number)),
                    mcu_ecu_socket(create_interface->createSocket(interfaces_number >> 4))
                    {

        receive_frames = new ReceiveFrames(mcu_ecu_socket, mcu_api_socket);
        WriteDataByIdentifier WDBI(0x1111FA10, {PCI_L, WRITE_DATA_BY_IDENTIFIER_SID, OTA_UPDATE_STATUS_DID_MSB, OTA_UPDATE_STATUS_DID_LSB, IDLE}, MCULogger, mcu_api_socket);
    }

    /* Default constructor */
    MCUModule::MCUModule() : is_running(false),
                         create_interface(CreateInterface::getInstance(0x01, MCULogger)),
                         receive_frames(nullptr) {}

    /* Destructor */
    MCUModule::~MCUModule() 
    {
        create_interface->stopInterface();
        delete receive_frames;
    }

    /* Start the module */
    void MCUModule::StartModule() { is_running = true; }

    /* Getter for securityAccess_seed */
    std::vector<uint8_t> MCUModule::getSecurityAccessSeed()
    {
        return securityAccess_seed;
    }

    /* Setter for securityAccess_seed */
    void MCUModule::setSecurityAccessSeed(const std::vector<uint8_t>& seed)
    {
        securityAccess_seed = seed;
    }

    /* Getter for MCU access state */
    bool MCUModule::getMCUState() const
    {
        return mcu_state;
    }

    /* Setter for MCU access state */
    void MCUModule::setMCUState(bool state)
    {
        mcu_state = state;
    }
    int MCUModule::getMcuApiSocket() const 
    {
    return mcu_api_socket;
    }
    int MCUModule::getMcuEcuSocket() const 
    {
    return mcu_ecu_socket;
    }


    /* Stop the module */
    void MCUModule::StopModule() { is_running = false; }

    /* Receive frames */
    void MCUModule::recvFrames() 
    {
        while (is_running)
        {
            receive_frames->startListenAPI();
            receive_frames->startListenCANBus();
            /* Start a thread to process the queue */
            std::thread queue_thread_process(&ReceiveFrames::processQueue, receive_frames);

            /* Start a thread to listen on API socket */
            std::thread queue_thread_listen(&ReceiveFrames::receiveFramesFromAPI, receive_frames);

            /* Receive frames from the CAN bus */
            receive_frames->receiveFramesFromCANBus();

            receive_frames->stopListenAPI();
            receive_frames->stopListenCANBus();

            /* Wait for the threads to finish */
            queue_thread_process.join();
            queue_thread_listen.join();
        }
    }
}