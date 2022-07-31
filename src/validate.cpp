// LEGACY!


// bool validateCommand(String command)
// {
//   // Command must be in a format of: <command> <number>
//   // <command> must be either O or C or W
//   // <number> must be in range of 1-16 for O and C commands
//   // <number> must be in range of 0-5999 for W command
//   if (command.length() < 3)
//   {
//     // log("Command is too short");
//     return false;
//   }
//   if (command.charAt(0) != 'O' && command.charAt(0) != 'C' && command.charAt(0) != 'W')
//   {
//     // log("Command code is invalid");
//     return false;
//   }
//   // Check if first position is a space
//   if (command.charAt(1) != ' ')
//   {
//     // log("Command code is invalid (no separator)");
//     return false;
//   }
//   // Check if command starting from second character is a number
//   for (uint16_t i = 2; i < command.length() - 1; i++)
//   {
//     if (command.charAt(i) < '0' || command.charAt(i) > '9')
//     {
//       return false;
//     }
//   }
//   // Check if command starting from second character is in range of 1-16 for O and C commands
//   if (command.charAt(0) == 'O' || command.charAt(0) == 'C')
//   {
//     int port = command.substring(2).toInt();
//     if (port < 1 || port > 16)
//     {
//       // log("Command code is invalid (port is out of range)");
//       return false;
//     }
//   }
//   // Check if command starting from second character is in range of 0-5999 for W command
//   if (command.charAt(0) == 'W')
//   {
//     int port = command.substring(2).toInt();
//     if (port < 0 || port > 5999)
//     {
//       // log("Command code is invalid (waiting time is out of range)");
//       return false;
//     }
//   }
//   return true;
// }