#include <Arduino.h>

#define COMMUNICATION_SERIAL_PORT Serial2
#define LOG_SERIAL_PORT Serial



// Commands
#define EXECUTE_CMD \
  "RUN"  // Execute command manually. Should be followed by action.
#define STORE_CMD \
  "STORE"  // Store command to EEPROM. Should be followed by action.
#define START_CMD "START"    // Start scene execution
#define STOP_CMD "STOP"      // Stop scene execution
#define PAUSE_CMD "PAUSE"    // Pause scene execution
#define RESUME_CMD "RESUME"  // Resume scene execution
#define CLEAR_CMD "CLEAR"    // Clear all stored commands
#define STATUS_CMD \
  "STATUS"  // Get status. Returns "IDLE", "PAUSED", "RUNNING <number>"

// Actions
#define CLOSE_KEY "C"  // Should be followed by port number (1-16)
#define OPEN_KEY "O"   // Should be followed by port number (1-16)
#define WAIT_KEY "W"   // Should be followed by waiting time (0-5999)

// For example "RUN O 1" will open port 1
// For example "STORE W 300" will store command "W 300" in storage

enum err_t {
  OK_CODE = 0,
  ERR_ANY_CODE = 1,
  ERR_CMD_NOT_FOUND_CODE = 2,
  ERR_CMD_INVALID_CODE = 3,
  ERR_CMD_ACTION_INVALID_CODE = 4,
  ERR_CMD_PARAM_INVALID_CODE = 5,
  ERR_CMD_PARAM_OUT_OF_RANGE_CODE = 6,
  ERR_ACTION_FAILED_CODE = 7,
};

enum action_key_t {
  CLOSE_KEY_CODE = 0,
  OPEN_KEY_CODE = 1,
  WAIT_KEY_CODE = 2,
  UNDEFINED_KEY_CODE = 3,
};

String errorCode(err_t err){
    //get number from err_t enum and convert to string
    String errorCode = String(err);
    return errorCode;
}

void sendError(err_t err)
{
    COMMUNICATION_SERIAL_PORT.println("ERR " + errorCode(err)+";");
}

void throwError(err_t err)
{
    sendError(err);
}

void sendLog(String message) {
  // LOG_SERIAL_PORT.println(message);
}

int mapPort(int port)
{
  return port + 21;
}

void open(int port)
{
  digitalWrite(mapPort(port), LOW);
}
void close(int port)
{
  digitalWrite(mapPort(port), HIGH);
}
void testPorts()
{
  for (int i = 0; i < 16; i++)
  {
    open(i);
    delay(100);
    close(i);
  }
}
void setupPorts(){
    for (int i = 1; i < 17; i++)
  {
    pinMode(mapPort(i), OUTPUT);
    close(i);
  }
}

uint16_t commandToStorage(action_key_t action, int param) {
  // First two its indicate the command: 0b01 - open, 0b10 - close, 0b11 - wait
  // next 14 bits indicate the port number or time in seconds
  uint16_t storage = 0;
  if (action == CLOSE_KEY_CODE) {
    storage = 0b01;
  } else if (action == OPEN_KEY_CODE) {
    storage = 0b10;
  } else if (action == WAIT_KEY_CODE) {
    storage = 0b11;
  } else {
    sendLog("Invalid command");
    storage = 0;
    return 0;
  }
  storage = storage << 14;
  storage = storage | (param & 0b1111111111111111);
  return storage;
}
action_key_t keyFromStorage(uint16_t compressedAction) {
  // First two its indicate the command: 0b00 - open, 0b01 - close, 0b10 - wait
  // next 14 bits indicate the port number or time in seconds
  if ((compressedAction >> 14) == 0b01) {
    return CLOSE_KEY_CODE;
  } else if ((compressedAction >> 14) == 0b10) {
    return OPEN_KEY_CODE;
  } else if ((compressedAction >> 14) == 0b11) {
    return WAIT_KEY_CODE;
  } else {
    sendLog("Invalid command");
    return UNDEFINED_KEY_CODE;
  }
}
int paramFromStorage(uint16_t compressedAction) {
  // First two its indicate the command: 0b00 - open, 0b01 - close, 0b10 - wait
  // next 14 bits indicate the port number or time in seconds
  return compressedAction & 0b0011111111111111;
}

bool isSceneRunning = false;
bool isPaused = false;

uint16_t scene[2048];
uint32_t scene_length = 0;

int currentCommandIndex = 0;
uint16_t currentCommand = 0;
uint32_t nextEventTime = 0;

void clearScene() {
  for (uint16_t i = 0; i < 2048; i++) {
    scene[i] = 0;
  }
  scene_length = 0;
}

void addCommandToScene(uint16_t command) {
  scene[scene_length] = command;
  scene_length++;
}

void sendSceneAck(int index) {
  COMMUNICATION_SERIAL_PORT.println("RUNNING " + String(index)+";");
}

void sendStorageAck(int index) {
  COMMUNICATION_SERIAL_PORT.println("STORED " + String(index)+";");
}

void sendAck() { COMMUNICATION_SERIAL_PORT.println("ACK;"); }

void sendNack() { COMMUNICATION_SERIAL_PORT.println("NACK;"); }

void resolveAction(action_key_t key, int param = -1) {
  switch (key) {
    case CLOSE_KEY_CODE:
      if (param < 0 || param > 16) {
        throwError(err_t::ERR_CMD_PARAM_OUT_OF_RANGE_CODE);
        return;
      }
      close(param);
      sendLog("Closing port " + String(param));
      break;
    case OPEN_KEY_CODE:
      if (param < 0 || param > 16) {
        throwError(err_t::ERR_CMD_PARAM_OUT_OF_RANGE_CODE);
        return;
      }
      open(param);
      sendLog("Opening port " + String(param));
      break;
    case WAIT_KEY_CODE:
      if (param < 0 || param > 5999) {
        throwError(err_t::ERR_CMD_PARAM_OUT_OF_RANGE_CODE);
        return;
      }
      if (isSceneRunning) {
        nextEventTime = millis() + param * 1000;

      } else {
        sendLog("No sense in waiting. Scene is not running");
      }
      break;
    default:
      throwError(err_t::ERR_CMD_ACTION_INVALID_CODE);
  }
}

void resolveInput() {
  char rawCommand[16] = "";
  int meaningfulLength =
      COMMUNICATION_SERIAL_PORT.readBytesUntil('\n', rawCommand, 16);
  rawCommand[meaningfulLength] = '\0';

  sendLog("Received command: " + String(rawCommand));

  // Parse command
  char* command = strtok(rawCommand, " ");
  if (command != NULL) {
    if (strstr(command, EXECUTE_CMD) != NULL) {
      // Execute command
      char* action = strtok(NULL, " ");
      char* port = strtok(NULL, " ");
      if (action == NULL) {
        throwError(err_t::ERR_CMD_ACTION_INVALID_CODE);
        return;
      }
      if (port == NULL) {
        throwError(err_t::ERR_CMD_PARAM_INVALID_CODE);
        return;
      }
      String actionStr = String(action);
      int param =  actionStr.toInt();
      
      if (strcmp(action, "O") == 0) {
        resolveAction(OPEN_KEY_CODE, param);
        sendAck();
      } else if (strcmp(action, "C") == 0) {
        resolveAction(CLOSE_KEY_CODE, param);
        sendAck();
      } else if (strcmp(action, "W") == 0) {
        if (isSceneRunning) {
          sendNack();
        } else {
          resolveAction(WAIT_KEY_CODE, param);
          sendAck();
        }
      } else {
        throwError(err_t::ERR_CMD_ACTION_INVALID_CODE);
      }
    } else if (strstr(command, STORE_CMD) != NULL) {
      char* action = strtok(NULL, " ");
      char* port = strtok(NULL, " ");
      if (action == NULL) {
        throwError(err_t::ERR_CMD_ACTION_INVALID_CODE);
        return;
      }
      if (port == NULL) {
        throwError(err_t::ERR_CMD_PARAM_INVALID_CODE);
        return;
      }

      String actionStr = String(action);
      int param =  actionStr.toInt();

      action_key_t key = UNDEFINED_KEY_CODE;
      if (strcmp(action, "O") == 0) {
        key = OPEN_KEY_CODE;
      } else if (strcmp(action, "C") == 0) {
        key = CLOSE_KEY_CODE;
      } else if (strcmp(action, "W") == 0) {
        key = WAIT_KEY_CODE;
      } else {
        throwError(err_t::ERR_CMD_ACTION_INVALID_CODE);
        return;
      }

      uint16_t compressedAction = commandToStorage(key, param);

      addCommandToScene(compressedAction);
      sendStorageAck(scene_length);
    } else if (strstr(command, START_CMD) != NULL) {
      // Start scene
      if (!isSceneRunning) {
        isSceneRunning = true;
        currentCommandIndex = 0;
        currentCommand = scene[currentCommandIndex];
        nextEventTime = millis();
      } else {
        sendLog("Scene is already running");
      }
    } else if (strstr(command, STOP_CMD) != NULL) {
      // Stop scene
      if (isSceneRunning) {
        isSceneRunning = false;
        currentCommandIndex = 0;
        currentCommand = 0;
        nextEventTime = 0;
        sendAck();
      } else {
        sendLog("Scene is not running");
        sendNack();
      }
    } else if (strstr(command, PAUSE_CMD) != NULL) {
      // Pause scene
      if (isSceneRunning) {
        isPaused = true;
        sendAck();
      } else {
        sendLog("Scene is not running");
        sendNack();
      }
    } else if (strstr(command, RESUME_CMD) != NULL) {
      // Resume scene
      if (isPaused) {
        isPaused = false;
        sendAck();
      } else {
        sendLog("Scene is not paused");
        sendNack();
      }
    } else if (strstr(command, CLEAR_CMD) != NULL) {
      // Clear commands
      clearScene();
      sendAck();
      sendLog("Scene cleared");
    } else if (strstr(command, STATUS_CMD) != NULL) {
      // Get status
      if (isPaused && isSceneRunning) {
        COMMUNICATION_SERIAL_PORT.println("STATUS PAUSED " +
                                          String(currentCommandIndex)+";");
      } else if (isSceneRunning) {
        COMMUNICATION_SERIAL_PORT.println("STATUS RUNNING " +
                                          String(currentCommandIndex)+";");
      } else {
        COMMUNICATION_SERIAL_PORT.println("STATUS IDLE;");
      }
    } else {
      throwError(err_t::ERR_CMD_INVALID_CODE);
    }
  } else {
    // Command is invalid
    throwError(err_t::ERR_CMD_NOT_FOUND_CODE);
  }
}

void resolveScene() {
  if (isSceneRunning && !isPaused) {
    sendLog("Scene is running:" + String(currentCommandIndex) + "/" +
            String(scene_length) + ". Next event in " +
            String(nextEventTime - millis()));
    if (millis() > nextEventTime) {
      currentCommand = scene[currentCommandIndex];
      sendLog("Executing command: " + String(currentCommand));
      if (currentCommand == 0) {
        isSceneRunning = false;
        currentCommandIndex = 0;
        currentCommand = 0;
        nextEventTime = 0;
        COMMUNICATION_SERIAL_PORT.println("FINISHED;");
      } else {
        action_key_t currentKeyCode = keyFromStorage(currentCommand);
        int currentParam = paramFromStorage(currentCommand);
        resolveAction(currentKeyCode, currentParam);
        sendSceneAck(currentCommandIndex);
      }

      currentCommandIndex++;
      delay(1);
    }
  }
}

void setup() {
  clearScene();
  setupPorts();
  Serial.begin(9600);
  Serial2.begin(9600);
  // addCommandToScene(commandToStorage(OPEN_KEY_CODE, 1));
  // addCommandToScene(commandToStorage(WAIT_KEY_CODE, 1));
  // addCommandToScene(commandToStorage(CLOSE_KEY_CODE, 1));
  // addCommandToScene(commandToStorage(WAIT_KEY_CODE, 1));
  // addCommandToScene(commandToStorage(OPEN_KEY_CODE, 2));
  // addCommandToScene(commandToStorage(WAIT_KEY_CODE, 2));
  // addCommandToScene(commandToStorage(CLOSE_KEY_CODE, 2));

  // sendLog("Debut Scene loaded");
  // for (int i = 0; i < scene_length; i++) {
  //   sendLog("Scene command: " + String(scene[i]));
  // }
  // sendLog("End Scene loaded");
}

void loop() {
  // Check if there is any data in Serial
  if (COMMUNICATION_SERIAL_PORT.available() > 3) {
    resolveInput();
  }
  resolveScene();
}
