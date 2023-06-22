#include <stdbool.h>

#define MAX_EVENTS_IN_QUEUE 1

// Define event types
enum EventType
{
    EVENT_NONE,
    EVENT_PLAY_PAUSE,
    EVENT_VOLUME_UP,
    EVENT_VOLUME_DOWN,
    EVENT_NEXT,
    EVENT_PREV,
    EVENT_QUIT,
    EVENT_TOGGLECOVERS,
    EVENT_TOGGLEREPEAT,
    EVENT_TOGGLEEQUALIZER,
    EVENT_TOGGLEBLOCKS,
    EVENT_ADDTOMAINPLAYLIST,
    EVENT_DELETEFROMMAINPLAYLIST,
    EVENT_EXPORTPLAYLIST,
    EVENT_SHUFFLE,
    EVENT_KEY_PRESS
};

// Structure to store events
struct Event
{
    enum EventType type;
    char key;
};

