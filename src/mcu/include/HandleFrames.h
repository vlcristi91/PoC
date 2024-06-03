
/**
 * @brief This class is used to call the service needed by analyzing the SID
 *        if frame-type is a request or to send the response to API if 
 *        frame-type is a response.
 *        The method handleFrame takes a parameter of type can_frame,
 *        parses the SID and calls the appropiate function within a switch-case.
 * @file HandleFrames.h
 * @author Tanasevici Mihnea
 * @date 2024-05-20
 * 
*/

#ifndef HANDLE_FRAMES_H
#define HANDLE_FRAMES_H

#include <linux/can.h>
#include <iostream>
#include <vector>

class HandleFrames 
{
public:
    /**
     * @brief Method used to handle a can frame received from the ReceiveFrame class.
     * Takes a can_frame as parameter, checks if the frame is complete and then calls
     * processFrameData() with either a single or multi frame.
     * 
     * @param[in] frame 
    */
    void handleFrame(const struct can_frame &frame);
    /**
     * @brief Method used to call a service or handle a response.
     * It takes frame_id, service id(sid) and frame_data and calls the right service or
     * handles the response received based on the given parameters.
     * 
     * @param[in] frame_id 
     * @param[in] sid
     * @param[in] frame_data
     * @param[in] is_multi_frame
    */
    void processFrameData(canid_t frame_id, int sid, std::vector<uint8_t> frame_data, bool is_multi_frame);
    /**
     * @brief Method used to send a frame based on the nrc(negative response code) received.
     * It takes as parameters frame_id, sid to identify the service, and nrc to send the correct
     * negative response code back to who made the request.
     * @param[in] frame_id 
     * @param[in] sid 
     * @param[in] nrc 
     */
    void processNrc(canid_t frame_id, int sid, int nrc);
};

#endif // HANDLE_FRAMES_H