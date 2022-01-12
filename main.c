/*
Hi! I hope you enjoy my game that I made titled "Arceuus". The objective of the game is to defeat
the almighty wizard, Arceuus. However, to reach him you must defeat the wizards that come before him.
Please read the how to play and controls screen within the game to understand how to play.

Also, since you are reading the code for the game, the cheat isn't explicitly stated inside the game,
in order to activate the cheat you must press BUTTON_UP and BUTTON_LEFT_SHOULDER at the same time. Upon
doing so, the player will achieve double movement speed and also the ability to fire two arrows at once!

There is no lose state, if a player loses all their health they will simply restart the level they are 
currently on. 

Once you reach Arceuus at the end of stage 3, please be aware that the spells he casts down get faster and faster as
his health lowers, and his animated sprite will also match the speed of his spells.

For whoever reads this, if it's Professor Hansen or a TA, thank you for teaching me/helping me learn the GBA and other CS topics.
*/


#include <stdlib.h>
#include <stdio.h>
#include "myLib.h"
#include "game.h"
#include "bgPause.h"
#include "bgLvl1.h"
#include "bgLvl2.h"
#include "bgLvl3.h"
#include "bgStart.h"
#include "bgXL.h"
#include "spritesheet.h"
#include "bgAbilityScreen.h"
#include "bgHTP.h"
#include "bgCONTROLS.h"
#include "bgCredits.h"

//sound
#include "osrsLegion.h"
#include "osrsForest.h"
#include "osrsUnderground.h"
#include "osrsAttack2.h"
#include "osrsSOTE.h"
#include "bowSound.h"
#include "sound.h"

// Prototypes
void initialize();
// State Prototypes
void goToStart();
void start();
void goToGame();
void game();
void goToPause();
void pause();
void goToWin();
void win();
void goToLose();
void lose();
void goToGame2();
void game2();
void goToGame3();
void game3();
void goToHTP();
void HTPScreen();
void goToCredits();
void credits();

// States

int state;
int lastGameState;
int lastState;
int seed;
int checkSound = 0;

SOUND soundA;
SOUND soundB;

// Button Variables
unsigned short buttons;
unsigned short oldButtons;

// Shadow OAM
OBJ_ATTR shadowOAM[128];
int main()
{
    initialize();

    while (1)
    {
        // Update button variables
        oldButtons = buttons;
        buttons = BUTTONS;

        // State Machine
        switch (state)
        {
        case START:
            start();
            break;
        case GAME:
            game();
            break;
        case PAUSE:
            pause();
            break;
        case WIN:
            win();
            break;
        case LOSE:
            lose();
            break;
        case GAME2:
            game2();
            break;
        case GAME3:
            game3();
            break;
        case ABILITY:
            AbilityScreen();
            break;
        case HTP:
            HTPScreen();
            break;
        case CONTROLS:
            controlsScreen();
            break;
        case CREDITS:
            credits();
        }
    }
}


// Sets up GBA
void initialize()
{
    REG_DISPCTL = MODE0 | SPRITE_ENABLE | BG0_ENABLE | BG1_ENABLE;
    REG_BG1CNT = BG_8BPP | BG_SIZE_SMALL | BG_CHARBLOCK(2) | BG_SCREENBLOCK(27);
    REG_BG0CNT = BG_8BPP | BG_SIZE_SMALL | BG_CHARBLOCK(0) | BG_SCREENBLOCK(31) | SPRITE_ENABLE;

    hOff = 0;
    vOff = 0;
    buttons = BUTTONS;
    oldButtons = 0;
    screenBlock = 28;

    setupSounds();
    setupInterrupts();

    goToStart();
    initGame();
}

// Sets up the start state
int startPointerRow[3] = {62, 83, 105};
int startPointerCol[3] = {89, 67, 77};

int startPointerIndex = 0;
void goToStart() {
    speedCheat = 1;
    REG_DISPCTL = MODE0 | SPRITE_ENABLE | BG0_ENABLE;
    REG_BG0CNT = BG_8BPP | BG_SIZE_SMALL | BG_CHARBLOCK(0) | BG_SCREENBLOCK(31) | SPRITE_ENABLE;
    REG_BG1CNT = BG_8BPP | BG_SIZE_SMALL | BG_CHARBLOCK(2) | BG_SCREENBLOCK(27);

    // Loads start screen palette
    DMANow(3, bgStartPal, PALETTE, bgStartPalLen / 2);
    DMANow(3, bgStartTiles, &CHARBLOCK[0], bgStartTilesLen / 2);
    DMANow(3, bgStartMap, &SCREENBLOCK[31], bgStartMapLen / 2);
    hideSprites();

    DMANow(3, spritesheetTiles, &CHARBLOCK[4], spritesheetTilesLen / 2);
    DMANow(3, spritesheetPal, SPRITEPALETTE, spritesheetPalLen / 2);

    if (soundA.isPlaying == 0) {
        playSoundA(osrsLegion_data, osrsLegion_length, 1);
    }
    
    shadowOAM[116].attr0 = (ROWMASK & (startPointerRow[startPointerIndex])) | ATTR0_SQUARE;
    shadowOAM[116].attr1 = (COLMASK & (startPointerCol[startPointerIndex])) | ATTR1_SMALL;
    shadowOAM[116].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(30, 30);
    
    state = START;
    lastState = START;

}

// Runs every frame of the start state
void start() {
    seed++;
    shadowOAM[116].attr0 = (ROWMASK & (startPointerRow[startPointerIndex])) | ATTR0_SQUARE;
    shadowOAM[116].attr1 = (COLMASK & (startPointerCol[startPointerIndex])) | ATTR1_SMALL;
    shadowOAM[116].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(30, 30);

    if (BUTTON_PRESSED(BUTTON_SELECT)) { // Switch case for start screen
        switch (startPointerIndex) {
            case 0:
                stopSound();
                goToGame();
                break;
            case 1:
                goToHTP();
                break;
            case 2:
                goToControls();
                break;
        }
    }
    if (BUTTON_PRESSED(BUTTON_DOWN)) {
        startPointerIndex++;
        if (startPointerIndex > 2) {
            startPointerIndex = 0;
        }
    }
    if (BUTTON_PRESSED(BUTTON_UP)) {
        startPointerIndex--;
        if (startPointerIndex < 0) {
            startPointerIndex = 2;
        }
    }

    DMANow(3, shadowOAM, OAM, 128 * 4);

}

void goToHTP() {
    REG_DISPCTL = MODE0 | BG0_ENABLE;
    DMANow(3, bgHTPPal, PALETTE, bgHTPPalLen / 2);
    REG_BG0CNT = BG_4BPP | BG_SIZE_SMALL | BG_CHARBLOCK(0) | BG_SCREENBLOCK(31);
    DMANow(3, bgHTPTiles, &CHARBLOCK[0], bgHTPTilesLen / 2);
    DMANow(3, bgHTPMap, &SCREENBLOCK[31], bgHTPMapLen / 2);
    state = HTP;
}

void HTPScreen() {
    if (BUTTON_PRESSED(BUTTON_SELECT)) {
        if (lastState == START) {
            goToStart();
        } else if (lastState == PAUSE) {
            goToPause();
        }
    }
}

void controlsScreen() {
    if (BUTTON_PRESSED(BUTTON_SELECT)) {
        if (lastState == START) {
            goToStart();
        } else if (lastState == PAUSE) {
            goToPause();
        }
    }
}

void goToControls() {
    REG_DISPCTL = MODE0 | BG0_ENABLE;
    DMANow(3, bgCONTROLSPal, PALETTE, bgCONTROLSPalLen / 2);
    REG_BG0CNT = BG_4BPP | BG_SIZE_SMALL | BG_CHARBLOCK(0) | BG_SCREENBLOCK(31);
    DMANow(3, bgCONTROLSTiles, &CHARBLOCK[0], bgCONTROLSTilesLen / 2);
    DMANow(3, bgCONTROLSMap, &SCREENBLOCK[31], bgCONTROLSMapLen / 2);
    state = CONTROLS;
}

// Sets up the game state
void goToGame() {
    REG_DISPCTL = MODE0 | BG1_ENABLE | SPRITE_ENABLE | BG0_ENABLE;
    REG_BG0CNT = BG_4BPP | BG_SIZE_SMALL | BG_CHARBLOCK(0) | BG_SCREENBLOCK(31);
    REG_BG1CNT = BG_4BPP | BG_SIZE_SMALL | BG_CHARBLOCK(0) | BG_SCREENBLOCK(31);
    hideSprites();
    waitForVBlank();

    if (soundA.isPlaying == 0) {
        playSoundA(osrsForest_data, osrsForest_length, 1);
    }

    DMANow(3, bgLvl1Pal, PALETTE, 256);
    DMANow(3, bgLvl1Tiles, &CHARBLOCK[0], bgLvl1TilesLen / 2);
    DMANow(3, bgLvl1Map, &SCREENBLOCK[31], bgLvl1MapLen / 2);

    DMANow(3, spritesheetTiles, &CHARBLOCK[4], spritesheetTilesLen / 2);
    DMANow(3, spritesheetPal, SPRITEPALETTE, spritesheetPalLen / 2);

    state = GAME;
    lastGameState = GAME;

}

void goToGame2() {
    REG_DISPCTL = MODE0 | SPRITE_ENABLE | BG0_ENABLE | BG1_ENABLE;
    REG_BG0CNT = BG_4BPP | BG_SIZE_SMALL | BG_CHARBLOCK(0) | BG_SCREENBLOCK(31);
    
    waitForVBlank();

    DMANow(3, bgLvl2Pal, PALETTE, 256);
    DMANow(3, bgLvl2Tiles, &CHARBLOCK[0], bgLvl2TilesLen / 2);
    DMANow(3, bgLvl2Map, &SCREENBLOCK[31], bgLvl2MapLen / 2);

    DMANow(3, spritesheetTiles, &CHARBLOCK[4], spritesheetTilesLen / 2);
    DMANow(3, spritesheetPal, SPRITEPALETTE, spritesheetPalLen / 2);

    state = GAME2;
    lastGameState = GAME2;

}

// Runs every frame of the game state
void game() {
    updateGame();
    drawGame();

    if (BUTTON_PRESSED(BUTTON_SELECT)) {
        goToPause();
    }

    if (player.health <= 0) {
        goToGame();
        initGame();
    }

    if (player.worldCol + player.width >= 240 && enemysRemaining <= 0) {
        player.worldRow = 115;
        player.worldCol = 0;
        player.abilityPoints = 2;
        initRegEnemys();
        goToGame2();
    }
    DMANow(3, shadowOAM, OAM, 128 * 4);

}

// Level 2 state
void game2() {
    updateGame();
    drawGame();

    if (BUTTON_PRESSED(BUTTON_SELECT)) {
        goToPause();
    }

    if (player.health <= 0) {
        goToGame2();
        initPlayer();
        player.worldRow = 115;
        initRegEnemys();
    }
    
    if (player.worldCol + player.width >= 240 && enemysRemaining <= 0) {
        player.worldRow = 140;
        player.worldCol = 0;
        player.abilityPoints += 2;
        stopSound();
        goToGame3();
    }
    DMANow(3, shadowOAM, OAM, 128 * 4);

}

void goToGame3() {
    REG_DISPCTL = MODE0 | SPRITE_ENABLE | BG0_ENABLE;
    REG_BG0CNT = BG_4BPP | BG_SIZE_WIDE | BG_CHARBLOCK(0) | BG_SCREENBLOCK(28);
    enemysRemaining = 1;
    DMANow(3, bgXLPal, PALETTE, 256);
    DMANow(3, bgXLTiles, &CHARBLOCK[0], bgXLTilesLen / 2);
    DMANow(3, bgXLMap, &SCREENBLOCK[28], bgXLMapLen / 2);

    REG_BG0VOFF = 0;
    REG_BG0HOFF = 0;

    if (soundA.isPlaying == 0) {
        playSoundA(osrsUnderground_data, osrsUnderground_length, 1);
    }

    DMANow(3, spritesheetTiles, &CHARBLOCK[4], spritesheetTilesLen / 2);
    DMANow(3, spritesheetPal, SPRITEPALETTE, spritesheetPalLen / 2);

    waitForVBlank();
    DMANow(3, shadowOAM, OAM, 128 * 4);

    state = GAME3;
    lastGameState = GAME3;
}

void game3() {
    updateGame();
    drawGame();
    
    if (BUTTON_PRESSED(BUTTON_SELECT)) {
        REG_DISPCTL = MODE0 | SPRITE_ENABLE | BG0_ENABLE | BG1_ENABLE;
        REG_BG0CNT = BG_4BPP | BG_SIZE_SMALL | BG_CHARBLOCK(0) | BG_SCREENBLOCK(31);
        REG_BG1CNT = BG_8BPP | BG_SIZE_SMALL | BG_CHARBLOCK(2) | BG_SCREENBLOCK(31);
        REG_BG0HOFF = 0;
        goToPause();
    }
    if (screenBlock == 31 && checkSound == 0) {
        stopSound();
        playSoundA(osrsAttack2_data, osrsAttack2_length, 1);
        checkSound = 1;
    }

    if (player.worldCol + player.width > 1005 && enemysRemaining <= 0) {
        stopSound();
        goToCredits();
    }
    if (player.health <= 0) {
        stopSound();
        hideSprites();
        checkSound = 0;
        goToGame3();
    }
    
    DMANow(3, shadowOAM, OAM, 128 * 4);
}

// Sets up the pause state
int pausePointerRow[4] = {30, 52, 76, 100};
int pausePointerCol[4] = {62, 62, 62, 62};

int pausePointerIndex = 0;
void goToPause() {
    pausePointerIndex = 0;
    REG_DISPCTL = MODE0 | SPRITE_ENABLE | BG0_ENABLE;
    REG_BG0CNT = BG_4BPP | BG_SIZE_SMALL | BG_CHARBLOCK(0) | BG_SCREENBLOCK(31) | SPRITE_ENABLE;

    DMANow(3, bgPausePal, PALETTE, bgPausePalLen / 2);
    DMANow(3, bgPauseTiles, &CHARBLOCK[0], bgPauseTilesLen / 2);
    DMANow(3, bgPauseMap, &SCREENBLOCK[31], bgPauseMapLen / 2);
    hideSprites();

    shadowOAM[116].attr0 = (ROWMASK & (pausePointerRow[pausePointerIndex])) | ATTR0_SQUARE;
    shadowOAM[116].attr1 = (COLMASK & (pausePointerCol[pausePointerIndex])) | ATTR1_SMALL;
    shadowOAM[116].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(30, 30);

    state = PAUSE;
    lastState = PAUSE;
}

void goToCredits() {
    REG_DISPCTL = MODE0 | SPRITE_ENABLE | BG0_ENABLE | BG1_ENABLE;
    REG_BG1CNT = BG_8BPP | BG_SIZE_SMALL | BG_CHARBLOCK(2) | BG_SCREENBLOCK(27);
    REG_BG0CNT = BG_8BPP | BG_SIZE_SMALL | BG_CHARBLOCK(0) | BG_SCREENBLOCK(31) | SPRITE_ENABLE;

    playSoundA(osrsSOTE_data, osrsSOTE_length, 1);
    checkSound = 0;

    DMANow(3, bgCreditsPal, PALETTE, bgCreditsPalLen / 2);
    DMANow(3, bgCreditsTiles, &CHARBLOCK[0], bgCreditsTilesLen / 2);
    DMANow(3, bgCreditsMap, &SCREENBLOCK[31], bgCreditsMapLen / 2);
    hideSprites();

    state = CREDITS;
    lastState = CREDITS;
}

void credits() {
    if (BUTTON_PRESSED(BUTTON_START)) {
        stopSound();
        hideSprites();
        main();
    }
}

// Runs every frame of the pause state
void pause() {
    shadowOAM[116].attr0 = (ROWMASK & (pausePointerRow[pausePointerIndex])) | ATTR0_SQUARE;
    shadowOAM[116].attr1 = (COLMASK & (pausePointerCol[pausePointerIndex])) | ATTR1_SMALL;
    shadowOAM[116].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(30, 30);
    
    if (BUTTON_PRESSED(BUTTON_SELECT)) {
        switch (pausePointerIndex)
        {
            case 0:
                if (lastGameState == GAME) {
                    goToGame();
                } else if (lastGameState == GAME2) {
                    goToGame2();
                } else if (lastGameState == GAME3) {
                    goToGame3();
                }
                hideSprites();
                break;
            case 1:
                goToAbilityScreen();
                break;
            case 2:
                goToHTP();
                break;
            case 3:
                goToControls();
                break;
        }
    }
    if (BUTTON_PRESSED(BUTTON_DOWN)) {
        pausePointerIndex++;
        if (pausePointerIndex > 3) {
            pausePointerIndex = 0;
        }
    }
    if (BUTTON_PRESSED(BUTTON_UP)) {
        pausePointerIndex--;
        if (pausePointerIndex < 0) {
            pausePointerIndex = 3;
        }
    }

    DMANow(3, shadowOAM, OAM, 128 * 4);

    // if (BUTTON_PRESSED(BUTTON_DOWN)) {
    //     hideSprites();
    //     goToAbilityScreen();
    // }
}

//Goes to ability screen from pause menu
int abilityPointer[2] = {68, 95};
void goToAbilityScreen() {
    REG_DISPCTL = MODE0  | SPRITE_ENABLE | BG0_ENABLE;
    REG_BG0CNT = BG_4BPP | BG_SIZE_SMALL | BG_CHARBLOCK(0) | BG_SCREENBLOCK(31);
    REG_BG1CNT = BG_4BPP | BG_SIZE_SMALL | BG_CHARBLOCK(2) | BG_SCREENBLOCK(27);
    int ones = player.abilityPoints % 10;
    shadowOAM[95].attr0 = (ROWMASK & (43)) | ATTR0_SQUARE;
    shadowOAM[95].attr1 = (COLMASK & (143)) | ATTR1_SMALL;
    shadowOAM[95].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(ones * 2, 30);

    for (int i = 0; i < player.arrowDamage; i++) {
        shadowOAM[120 + i].attr0 = (ROWMASK & (70)) | ATTR0_SQUARE;
        shadowOAM[120 + i].attr1 = (COLMASK & (145 + i * 15)) | ATTR1_SMALL;
        shadowOAM[120 + i].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(4, 0);
    }

    for (int i = 0; i < player.arrowFireRate; i++) {
        shadowOAM[123 +i].attr0 = (ROWMASK & (95)) | ATTR0_SQUARE;
        shadowOAM[123 +i].attr1 = (COLMASK & (145 + i * 15)) | ATTR1_SMALL;
        shadowOAM[123 +i].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(4, 0);
    }

    DMANow(3, bgAbilityScreenPal, PALETTE, 256);
    DMANow(3, bgAbilityScreenTiles, &CHARBLOCK[0], bgAbilityScreenTilesLen / 2);
    DMANow(3, bgAbilityScreenMap, &SCREENBLOCK[31], bgAbilityScreenMapLen / 2);

    DMANow(3, spritesheetTiles, &CHARBLOCK[4], spritesheetTilesLen / 2);
    DMANow(3, spritesheetPal, SPRITEPALETTE, spritesheetPalLen / 2);

    state = ABILITY;
}

//Ability screen

int abilityIndex = 0;
void AbilityScreen() {
    //draws ability points
    int ones = player.abilityPoints % 10;
    shadowOAM[95].attr0 = (ROWMASK & (43)) | ATTR0_SQUARE;
    shadowOAM[95].attr1 = (COLMASK & (143)) | ATTR1_SMALL;
    shadowOAM[95].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(ones * 2, 30);

    shadowOAM[116].attr0 = (ROWMASK & (abilityPointer[abilityIndex])) | ATTR0_SQUARE;
    shadowOAM[116].attr1 = (COLMASK & (20)) | ATTR1_SMALL;
    shadowOAM[116].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(30, 30);

    // Fire Rate sprites
    shadowOAM[123].attr0 = (ROWMASK & (95)) | ATTR0_SQUARE;
    shadowOAM[123].attr1 = (COLMASK & (145)) | ATTR1_SMALL;
    shadowOAM[123].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(4, 0);

    // Arrow damage sprites
    shadowOAM[120].attr0 = (ROWMASK & (70)) | ATTR0_SQUARE;
    shadowOAM[120].attr1 = (COLMASK & (145)) | ATTR1_SMALL;
    shadowOAM[120].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(4, 0);

    if (BUTTON_PRESSED(BUTTON_START)) {
        for (int i = 0; i < 10; i++) {
            shadowOAM[117 + i].attr0 = ATTR0_HIDE;
        }
        goToPause();
    }
    if (BUTTON_PRESSED(BUTTON_DOWN)) {
        if (++abilityIndex > 1) {
            abilityIndex = 0;
        }
    }
    if (BUTTON_PRESSED(BUTTON_UP)) {
        if (--abilityIndex < 0) {
            abilityIndex = 1;
        }
    }
    if (BUTTON_PRESSED(BUTTON_SELECT)) {
        if (abilityIndex == 1 && player.arrowFireRate != 3 && player.abilityPoints > 0) {
            shadowOAM[123 + player.arrowFireRate].attr0 = (ROWMASK & (95)) | ATTR0_SQUARE;
            shadowOAM[123 + player.arrowFireRate].attr1 = (COLMASK & (145 + player.arrowFireRate * 15)) | ATTR1_SMALL;
            shadowOAM[123 + player.arrowFireRate].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(4, 0);
            player.arrowFireRate++;
            player.abilityPoints--;
        } else if (abilityIndex == 0 && player.arrowDamage != 3 && player.abilityPoints > 0) {
            shadowOAM[120 + player.arrowDamage].attr0 = (ROWMASK & (70)) | ATTR0_SQUARE;
            shadowOAM[120 + player.arrowDamage].attr1 = (COLMASK & (145 + player.arrowDamage * 15)) | ATTR1_SMALL;
            shadowOAM[120 + player.arrowDamage].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(4, 0);
            player.arrowDamage++;
            player.abilityPoints--;
        }
    }
    DMANow(3, shadowOAM, OAM, 128 * 4);
}

// Sets up the win state
void goToWin() {}

// Runs every frame of the win state
void win() {}

// Sets up the lose state
void goToLose() {

    state = LOSE;
}

// Runs every frame of the lose state
void lose() {
    if (BUTTON_PRESSED(BUTTON_START)) {
        initialize();
    }
}