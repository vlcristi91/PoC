#include "../include/TransferData.h"
#include "../../../src/mcu/include/MCUModule.h"

TransferData::TransferData(int socket, Logger transfer_data_logger)
                : transfer_data_logger(transfer_data_logger), 
                  generate_frames(socket, transfer_data_logger)

{
    this->socket = socket;
}

/* Definition and initialization of the static member variable */
uint8_t TransferData::expected_block_sequence_number = 0x01;  /* Start from 1 */
bool TransferData::is_first_transfer = false;
size_t TransferData::chunk_size = 0;
uint8_t TransferData::expected_transfer_data_requests = 0;
/* Static vector to store the checksums */
std::vector<uint8_t>TransferData::checksums;

/* Method to compute a simple XOR checksum for a block of data */
uint8_t TransferData::computeChecksum(const uint8_t* data, size_t block_size)
{
    uint8_t checksum = 0;
    for (size_t i = 0; i < block_size; ++i)
    {
        checksum ^= data[i];
    }
    /* Return the checksum */
    return checksum;
}

/* Retrieve the checksum vector */
const std::vector<uint8_t>&TransferData::getChecksums()
{
    return checksums;
}

/* method used to transfer the data */
/* frame format = {PCI_L, SID(0x36), block_sequence_counter, transfer_request_parameter_record}*/
void TransferData::transferData(canid_t can_id, std::vector<uint8_t>& transfer_request)
{
    /* Declaration of data as static in order to retain its value between calls */
    static std::vector<uint8_t> data;
    /* Total size of data */
    static size_t total_size = 0;
    /* Keep track of how many bytes have been tracked */
    static size_t bytes_sent = 0;
    NegativeResponse nrc(socket, transfer_data_logger);
    uint8_t block_sequence_counter = transfer_request[2];
    std::vector<uint8_t> response;
    /* Extract and switch sender and receiver */
    uint8_t receiver_id = can_id  & 0xFF;
    uint8_t sender_id = (can_id >> 8) & 0xFF;
    /* target id represents the ecu needed to be updated, if 0 then target will be mcu */
    /* it is 1 byte and can be found between bit 16 and 23 in canid_t */
    uint8_t target_id = (can_id >> 16) & 0xFF;
    /* Reverse IDs, target id will be in same position but receiver will be switched with sender */
    can_id = (target_id << 16) | (receiver_id << 8) | sender_id;
    if (transfer_request.size() < 3)
    {
        /* Incorrect message length or invalid format - prepare a negative response */
        nrc.sendNRC(can_id, TD_SID, NegativeResponse::IMLOIF);
        MCU::mcu->setDidValue(OTA_UPDATE_STATUS_DID, {PROCESSING_TRANSFER_FAILED});
        return;
    }
    else if (expected_block_sequence_number != block_sequence_counter)
    {
        /* Wrong block sequence counter - prepare a negative response */
        nrc.sendNRC(can_id, TD_SID, NegativeResponse::WBSC);
        return;
    }
    else
    {   
        OtaUpdateStatesEnum ota_state = static_cast<OtaUpdateStatesEnum>(MCU::mcu->getDidValue(OTA_UPDATE_STATUS_DID)[0]);
        if(ota_state == WAIT_DOWNLOAD_COMPLETED)
        {
            is_first_transfer = true;
            /* Get chunk_size from request download */
            chunk_size = static_cast<size_t>(RequestDownloadService::getMaxNumberBlock());
            /* set expected transfer data requests that is defined in request download */
            expected_transfer_data_requests = MAX_TRANSFER_DATA_REQUESTS;
            expected_block_sequence_number = 1;
            MCU::mcu->setDidValue(OTA_UPDATE_STATUS_DID, {PROCESSING});
            ota_state = static_cast<OtaUpdateStatesEnum>(MCU::mcu->getDidValue(OTA_UPDATE_STATUS_DID)[0]);
        }

        if(ota_state != PROCESSING)
        {
            LOG_WARN(transfer_data_logger.GET_LOGGER(), "Data transfer is not initialized. Use Request Download in order to initialize a data transfer. Current OTA state:{}", ota_state);
            nrc.sendNRC(can_id, TD_SID, NegativeResponse::CNC);
            return;
        }

        /* Handle the transfer of parameter record (data after PCI, SID and block sequence countre in the frame format of transfer data)*/
        if (transfer_request.size() > 3)
        {
            std::vector<uint8_t> parameter_data(transfer_request.begin() + 3, transfer_request.end());
            
            /* Check if it is the first transfer */
            if (is_first_transfer)
            {
                /* Set the total size of the paramater data */
                total_size = parameter_data.size();
                bytes_sent = 0;
                /* Mark that the first transfer has been done */
                is_first_transfer = false;
            }

            /* Handle the 1MB chunk transfer */
            if (bytes_sent < total_size)
            {
                size_t remaining_bytes = total_size - bytes_sent;
                size_t current_chunk_size = std::min(chunk_size,remaining_bytes);
                /* Extract the current chunk from the parameter data */                 
                std::vector<uint8_t> chunk_data(parameter_data.begin() + bytes_sent, parameter_data.begin() + bytes_sent + current_chunk_size);                

                /* Write the chunk to the memory */
                MemoryManager* memory_manager = MemoryManager::getInstance(transfer_data_logger);                
                bool write_success = memory_manager->writeToAddress(chunk_data);

                if(write_success == false)
                {
                    nrc.sendNRC(can_id, TD_SID, NegativeResponse::TDS);
                    MCU::mcu->setDidValue(OTA_UPDATE_STATUS_DID, {PROCESSING_TRANSFER_FAILED});
                    return;
                }
                else
                {
                    /* Update bytes sent */
                    bytes_sent += current_chunk_size;

                    /* clear vector after writing to adress */
                    response.clear();
                    /* prepare positive response */
                    response.push_back(0x02); /* PCI */
                    response.push_back(0x76); /* Service ID */
                    response.push_back(block_sequence_counter); /* block_sequence_counter */
                    response.insert(response.end(), chunk_data.begin(), chunk_data.end()); /* transfer parameter record */

                    /* Send the postive response frame */
                    generate_frames.sendFrame(can_id, response);
                    
                    /* Increment expected_block_sequence_number only if it matches the current block_sequence_counter */
                    expected_block_sequence_number++;
                }
                /* Check if all data has been sent */
                if (bytes_sent >= total_size)
                {
                    MCU::mcu->setDidValue(OTA_UPDATE_STATUS_DID, {PROCESSING_TRANSFER_COMPLETE});
                    /* Reset the flag for the next transfer */
                    is_first_transfer = true;
                }
            }

            /* Check if all data has been sent */
            if (bytes_sent >= total_size)
            {
                MCU::mcu->setDidValue(OTA_UPDATE_STATUS_DID, {PROCESSING_TRANSFER_COMPLETE});
                /* Reset the flag for the next transfer */
                is_first_transfer = true;
            }
        }
        else
        {
            /* Load data if this is the first transfer */
            if (is_first_transfer)
            {
                /* use memory manager class to transfer the data */
                std::string path_to_main;
                if(FileManager::getEcuPath(target_id, path_to_main, 1, transfer_data_logger) == 0)
                {
                    nrc.sendNRC(can_id, TD_SID, NegativeResponse::TDS);
                    MCU::mcu->setDidValue(OTA_UPDATE_STATUS_DID, {PROCESSING_TRANSFER_FAILED});
                    return;
                }

                /* Read data from the extracted binary */
                data = MemoryManager::readBinary(path_to_main, transfer_data_logger);
                
                /* Set the total size of the binary data */
                total_size = data.size();

                /* Initialize the bytes sent for the first transfer */
                bytes_sent = 0;
                /* Set the flag as false after the first transfer */
                is_first_transfer = false;                  
            }

            /* Check whether there is still data to transfer */
            if (bytes_sent < total_size)
            {        
                /* Check the remaining bytes */
                size_t remaining_bytes = total_size - bytes_sent;
                /* Compute the current chunk size */
                size_t current_chunk_size = std::min(chunk_size, remaining_bytes);
                /* Extract the current chunk from the binary data */
                std::vector<uint8_t> chunk_data(data.begin() + bytes_sent, data.begin() + bytes_sent + current_chunk_size);                

                MemoryManager* memory_manager = MemoryManager::getInstance(transfer_data_logger);   
                /* Write the read data to the partition created, at a specifig address */
                bool write_success = memory_manager->writeToAddress(chunk_data);

                if(write_success == false)
                {
                    nrc.sendNRC(can_id, TD_SID, NegativeResponse::TDS);
                    MCU::mcu->setDidValue(OTA_UPDATE_STATUS_DID, {PROCESSING_TRANSFER_FAILED});
                    return;
                }
                else
                {
                    /* Compute and store checksum for this chunk */
                    uint8_t checksum = computeChecksum(chunk_data.data(), chunk_data.size());
                    
                    /* Store the checksum in a static vector */
                    checksums.push_back(checksum);
                    /* Update bytes sent */
                    bytes_sent += current_chunk_size;

                    /* clear vector after writing to adress */
                    response.clear();
                    /* prepare positive response */
                    response.push_back(0x02); /* PCI */
                    response.push_back(0x76); /* Service ID */
                    response.push_back(block_sequence_counter); /* block_sequence_counter */

                    /* Send the postive response frame */
                    generate_frames.sendFrame(can_id, response);
                    
                    /* Increment expected_block_sequence_number only if it matches the current block_sequence_counter */
                    expected_block_sequence_number++;
                }
            }

            /* Check if all data has been sent */
            if (bytes_sent >= total_size)
            {
                MCU::mcu->setDidValue(OTA_UPDATE_STATUS_DID, {PROCESSING_TRANSFER_COMPLETE});
                /* Reset the flag for the next transfer */
                is_first_transfer = true;
            }
        }

        /* reset it to 0x01 after it reaches 0xFF and resets to 0*/
        if (expected_block_sequence_number == 0x00)
        {
            expected_block_sequence_number = 0x01;
        }
    }
}
