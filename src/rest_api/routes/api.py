import sys
import os
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
sys.path.append(PROJECT_ROOT)
import json

from flask import request, jsonify, Blueprint
from actions.request_id_action import RequestIdAction
from actions.update_action import Updates
from actions.read_info_action import *
from utils.logger import log_memory
from actions.manual_send_frame import manual_send_frame
from actions.write_info_action import WriteToDoors, WriteToBattery
from src.ota.google_drive_api.GoogleDriveApi import GDriveAPI

api_bp = Blueprint('api', __name__)
gDrive = GDriveAPI.getInstance()

@api_bp.route('/request_ids', methods=['GET'])
def request_ids():
    requester = RequestIdAction(my_id=0xFA99)
    response = requester.read_ids()
    return jsonify(response)


@api_bp.route('/update_to_version', methods=['POST'])
def update_to_version():
    data = request.get_json()
    ecu_id = data.get('ecu_id')
    version = data.get('version')
    updater = Updates(my_id=0xFA, id_ecu=[0x10, 0x11, 0x12])
    response = updater.update_to(ecu_id=ecu_id,
                                 version=version)
    return jsonify(response)


@api_bp.route('/read_info_battery', methods=['GET'])
def read_info_bat():
    """
    Read information from the battery.
    ---
    responses:
      200:
        description: Information retrieved successfully
        schema:
          type: object
          properties:
            battery_level:
              type: integer
            voltage:
              type: integer
            battery_state_of_charge:
              type: integer
            temperature:
              type: integer
            life_cycle:
              type: integer
            fully_charged:
              type: boolean
            serial_number:
              type: string
            range_battery:
              type: integer
            charging_time:
              type: integer
            device_consumption:
              type: integer
            time_stamp:
              type: string
              format: date-time
    """
    reader = ReadInfo(0xFA, [0x10, 0x11, 0x12])
    response = reader.read_from_battery()
    return jsonify(response)


@api_bp.route('/read_info_engine', methods=['GET'])
def read_info_eng():
    reader = ReadInfo(0xFA, [0x10, 0x11, 0x12])
    response = reader.read_from_engine()
    return jsonify(response)


@api_bp.route('/read_info_doors', methods=['GET'])
def read_info_doors():
    reader = ReadInfo(0xFA, [0x10, 0x11, 0x12])
    response = reader.read_from_doors()
    return jsonify(response)


@api_bp.route('/send_frame', methods=['POST'])
def send_frame():
    data = request.get_json()
    can_id = data.get('can_id')
    can_data = data.get('can_data')
    return jsonify(manual_send_frame(can_id, can_data))


@api_bp.route('/write_info_doors', methods=['POST'])
def write_info_doors():
    data = request.get_json()

    writer = WriteToDoors(0xFA, [0x10, 0x11, 0x12], data)
    response = writer.run()
    return jsonify(response)


@api_bp.route('/write_info_battery', methods=['POST'])
def write_info_battery():
    data = request.get_json()
    writer = WriteToBattery(0xFA, [0x10, 0x11, 0x12], data)
    response = writer.run()
    return jsonify(response)


@api_bp.route('/logs')
def get_logs():
    return jsonify({'logs': log_memory})

# Google Drive API Endpoints
@api_bp.route('/drive_update_data', methods=['GET'])
def update_drive_data():
    try:
        drive_data_str = gDrive.getDriveData()
        # drive_data = json.loads(drive_data_str)
        return jsonify(drive_data_str)
    except Exception as e:
        return jsonify({"error": str(e)}), 500
