#define MAPHEIGHT 256
#define MAPWIDTH 256



// ARROW Struct
typedef struct {
	int row;
	int col;
	int rdel;
	int cdel;
	int height;
	int width;
	int erased;
	int active;
} ARROW;

enum
{
    START,
    GAME,
    GAME2,
    GAME3,
    PAUSE,
    WIN,
    LOSE,
    ABILITY,
    HTP,
    CONTROLS,
    CREDITS
};

// Constants

// Variables
extern int hOff;
extern int vOff;
extern ANISPRITE arrow[10];
extern int enemysRemaining;
extern int lives;
extern int state;
extern int speedCheat;
extern int screenBlock;
extern int playerhOff;
extern OBJ_ATTR shadowOAM[128];
extern ANISPRITE player;


// Prototypes
void initGame();
void updateGame();
void drawGame();
void initPlayer();
void updatePlayer();
void drawPlayer();
void animatePlayer();
void initRegEnemys();