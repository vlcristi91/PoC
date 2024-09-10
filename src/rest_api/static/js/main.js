// Utility function to display JSON response in the output container
function displayResponse(data) {
    const responseContainer = document.getElementById('response-output');
    responseContainer.textContent = JSON.stringify(data, null, 2);
}

// Utility function to fetch logs and update the log table
function fetchLogs() {
    fetch('/api/logs')
        .then(response => response.json())
        .then(data => {
            const logBody = document.getElementById('log-body');
            logBody.innerHTML = '';

            data.logs.reverse().forEach((log, index) => {
                const row = document.createElement('tr');
                row.innerHTML = `<td>${index + 1}</td><td>${log}</td>`;
                logBody.appendChild(row);
            });
        })
        .catch(error => {
            console.error('Error fetching logs:', error);
        });
}

/**
 * Performs an API request using the Fetch API.
 * This method is a refactored version designed to avoid code repetition
 * by consolidating the fetch logic into a single function.
 * @param {string} url - The URL endpoint for the API request.
 * @param {string} method - The HTTP method for the request (e.g., 'GET', 'POST').
 * @param {Object|null} body - The body of the request, to be sent as JSON. Defaults to null if no body is provided.
 * @returns {Promise} - A promise that resolves to the JSON response from the server.
 */
function performApiRequest(url, method, body = null) {
    return fetch(url, {
        method: method,
        headers: {
            'Content-Type': 'application/json',
        },
        body: body ? JSON.stringify(body) : null,
    }).then(response => response.json())
      .then(data => {
          displayResponse(data);
          fetchLogs();
      })
      .catch(error => {
          console.error('Error:', error);
      });
}

function sendFrame() {
    const canId = document.getElementById('can-id').value;
    const canData = document.getElementById('can-data').value;
    if (!canId || !canData) {
        alert('CAN ID and CAN Data cannot be empty.');
        return;
    }

    performApiRequest('/api/send_frame', 'POST', { can_id: canId, can_data: canData });
}

function requestIds() {
    performApiRequest('/api/request_ids', 'GET');
}

function updateToVersion() {
    let type, version;

    // Regular expression for validating file type (letters only)
    const typeRegex = /^[a-zA-Z]+$/;

    // Regular expression for validating version (numbers and dots only)
    const versionRegex = /^[0-9.]+$/;

    // Loop until valid file type is provided
    do {
        type = prompt('Enter file type (letters only):');
        if (type === null) {
            alert('Operation cancelled.');
            return;
        }
        if (!typeRegex.test(type)) {
            alert('Invalid file type. Please enter letters only.');
        }
    } while (!typeRegex.test(type));

    // Loop until valid version is provided
    do {
        version = prompt('Enter software version (numbers and dots only):');
        if (version === null) {
            alert('Operation cancelled.');
            return;
        }
        if (!versionRegex.test(version)) {
            alert('Invalid version. Please enter numbers and dots only.');
        }
    } while (!versionRegex.test(version));

    // Loop until valid version is provided
    do {
        ecu_id = prompt('Enter ECU id (numbers only):');
        if (ecu_id === null) {
            alert('Operation cancelled.');
            return;
        }
        if (!versionRegex.test(ecu_id)) {
            alert('Invalid version. Please enter numbers only.');
        }
    } while (!versionRegex.test(ecu_id));

    // If validation passes, perform the API request
    performApiRequest('/api/update_to_version', 'POST', { update_file_type: type, update_file_version: version, ecu_id: ecu_id });
}

function readInfoBattery() {
    performApiRequest('/api/read_info_battery', 'GET');

}

function readInfoEngine() {
    performApiRequest('/api/read_info_engine', 'GET');
}


function gDriveReadData() {
    fetch('/api/drive_update_data', {
        method: 'GET',
    }).then(response => response.json())
      .then(data => {
          document.getElementById('response-output').textContent = JSON.stringify(data);
          fetchLogs();
      });
}

function readInfoDoors() {
    performApiRequest('/api/read_info_doors', 'GET');
}

function writeInfoDoors() {
    const data = {
        door: prompt('Enter Door Parameter:') || null,
        serial_number: prompt('Enter Serial Number:') || null,
        lighter_voltage: prompt('Enter Cigarette Lighter Voltage:') || null,
        light_state: prompt('Enter Light State:') || null,
        belt: prompt('Enter Belt Card State:') || null,
        windows_closed: prompt('Enter Window Status:') || null,
    };
    performApiRequest('/api/write_info_doors', 'POST', data);
}

function writeInfoBattery() {
    const data = {
        battery_level: prompt('Enter Battery Energy Level:') || null,
        voltage: prompt('Enter Battery Voltage:') || null,
        battery_state_of_charge: prompt('Enter Battery State of Charge:') || null,
        percentage: prompt('Enter Battery Percentage:') || null,
        // temperature: prompt('Enter Battery Temperature:') || null,
        // life_cycle: prompt('Enter Battery Life Cycle:') || null,
        // fully_charged: prompt('Enter Battery Fully Charged Status:') || null,
        // range_battery: prompt('Enter Battery Range:') || null,
        // charging_time: prompt('Enter Battery Charging Time:') || null,
        // device_consumption: prompt('Enter Device Consumption:') || null
    };
    performApiRequest('/api/write_info_battery', 'POST', data);
}

function changeSession() {
    const input = prompt('Enter sub-function code (1 for default session, 2 for programming session):');
    if (input === null) {
        alert('Operation cancelled.');
        return;
    };

    const sub_funct = parseInt(input, 10);
    if (sub_funct !== 1 && sub_funct !== 2) {
        alert('Invalid input. Please enter 1 or 2.');
        return;
    };

    performApiRequest('/api/change_session', 'POST', { sub_funct: sub_funct });
}

function authenticate() {
    performApiRequest('/api/authenticate', 'GET');
}
function read_dtc_info() {
    performApiRequest('/api/read_dtc_info', 'GET');
}

function clear_dtc_info() {
    performApiRequest('/api/clear_dtc_info', 'GET');
}

function get_tester_pres() {
    performApiRequest('/api/tester_present', 'GET');
}

function get_data_ids() {
    performApiRequest('/api/get_identifiers', 'GET');
}