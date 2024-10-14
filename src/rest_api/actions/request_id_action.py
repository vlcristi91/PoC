from actions.base_actions import *
import time
from config import Config
import datetime


class IDsToJson():
    def _to_json(self, data):
        response = {
            "status": data.get("status"),
            "mcu_id": data.get("mcu_id"),
            "ecu_ids": data.get("ecu_ids", []),
            "time_stamp": datetime.datetime.now().isoformat()
        }
        if "reason" in data:
            response["reason"] = data["reason"]
        return response


class RequestIdAction(Action):
    def __init__(self, my_id, id_ecu=None):
        if id_ecu is None:
            id_ecu = []
        super().__init__(my_id, id_ecu)

    def read_ids(self):
        self.id = self.my_id
        try:
            self._send_request_frame()
            response_data = self._read_response_frames()
            response_json = IDsToJson()._to_json(response_data)
            self.bus.shutdown()
            return response_json

        except CustomError as e:
            self.bus.shutdown()
            return {"status": "Error", "message": str(e)}

    def _send_request_frame(self):
        log_info_message(logger, "Sending request frame for ECU IDs")
        self.send(self.id, [0x01, 0x99])
        log_info_message(logger, "Request frame sent")

    def _read_response_frames(self, timeout=10):
        end_time = time.time() + timeout
        while time.time() < end_time:
            response = self.bus.recv(Config.BUS_RECEIVE_TIMEOUT)
            if response:
                data = response.data
                if len(data) < 3:
                    return {"status": "Invalid response length"}

                if data[1] == 0x7F:
                    nrc_msg = data[3]
                    sid_msg = data[2]
                    negative_response = self.handle_negative_response(nrc_msg, sid_msg)
                    return {
                        "status": "error",
                        "message": f"Negative response received while resetting device {id}",
                        "negative_response": negative_response
                    }

                if data[1] == 0xD9:
                    # Positive response
                    mcu_id = int(f"{data[2]:02X}", 16)
                    ecu_ids = [int(f"{byte:02X}", 16) for byte in data[3:]]
                    data_dict = {
                        "status": "Success",
                        "mcu_id": f"{mcu_id:02X}",
                        "ecu_ids": [f"{ecu_id:02X}" for ecu_id in ecu_ids]
                    }
                return data_dict
        return {"status": "No response received within timeout"}
