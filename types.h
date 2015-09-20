/* Unfortunately, the Arduino IDE "helpfully" automatically adds
   function prototypes, but screws it up for things like structs. So
   we have to keep our declarations here, in s separate header file.
*/

enum Channel {
  BackLeft = 0,
  BackRight,
  SeatLeft,
  SeatRight
};

enum CommandType {
  None = 0,
  MoveTo
};

struct MoveToCommandData
{
  Channel channel;
  float target;
};

typedef struct s_command {
  CommandType type;
  union {
    MoveToCommandData moveTo;
  } data;
} Command;

