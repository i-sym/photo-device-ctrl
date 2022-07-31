# Software for Arduino Mega that controls outputs of the device
The controller recieves simple commands from COMMUNICATION_SERIAL_PORT. You can:
- Do manual commands
- Add scene to cache (in ram)
- Execute scene from cache
- Pause
- Resume
- Stop
- Clear
- Get status

The comand can execute actions

## Standard actions
`O <n>` - Opens port n

`C <n>` - Closes port n

`W <n>` - Waits for n seconds

Store and manual comands must be followed by action

## Commands

### Run command `RUN <action>`
Runs the command `<action>` on the device.
For example `RUN O 15` will open port 15.

The `ACK` will be sent back if the command was executed. `NACK` will be sent if command is irrelevant (wait). The error may be sent if the command is not recognized.

### Store command `STORE <action>`
Stores the command `<action>` in the cache.
For example `STORE O 15` will store the command `O 15` in the cache.

The `STORED <index>` will be sent back if the command was stored. In case of error the errors will be sent.

### START command `START`
Starts scene from cache.
Will return `RUNNING <index>` after each action. And `FINISHED` when the scene is finished.

### PAUSE command `PAUSE`
Pauses scene.
Will return `ACK` if scene was paused and `NACK` if scene has already been paused.


### Resume command `RESUME`
Resumes scene.
Will return `ACK` if scene was resumed and `NACK` if scene is running.


### Stop command `STOP`
Stops scene.
Will return `ACK`.


### Clear command `CLEAR`
Clears cache.
Will return `ACK`.


### Status command `STATUS`
Returns status of the scene:
- `STATUS IDLE` if scene is not running 
- `STATUS RUNNING <index>` if scene is running (and index)
- `STATUS PAUSED <index>` if scene is paused


## Errors
Error messages are sent back if the error occurs. Message is formed as `ERR <number>`.

- `ERR_ANY_CODE` = 1,
- `ERR_CMD_NOT_FOUND_CODE` = 2,
- `ERR_CMD_INVALID_CODE` = 3,
- `ERR_CMD_ACTION_INVALID_CODE` = 4,
- `ERR_CMD_PARAM_INVALID_CODE` = 5,
- `ERR_CMD_PARAM_OUT_OF_RANGE_CODE` = 6,
- `ERR_ACTION_FAILED_CODE` = 7,

