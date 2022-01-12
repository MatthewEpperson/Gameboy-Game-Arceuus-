#include "myLib.h"
#include "spritesheet.h"
#include "game.h"
#include "sound.h"
#include "bowSound.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int hOff;
int vOff;
OBJ_ATTR shadowOAM[128];
ANISPRITE player;
ANISPRITE boss;
ANISPRITE regEnemy[3];
ANISPRITE arrow[10];
ANISPRITE spell[8];
u16 bossSpellCol;
int timer = 0;
enemysRemaining;

int floorheight = 14;

#define GRAVITY -40
#define BOSSGRAVITYFACTOR 0.03
#define GRAVITYFACTOR 0.08
#define JUMPVELOCITY 70
#define BOSSJUMPVELOCITY 20
#define JUMPHOLDVELOCITY 30
#define BOSSMAPWIDTH 1024

enum {LEFT, RIGHT, UP, DOWN}; //This is for all other enemies
enum {LEVEL1, LEVEL2, LEVEL3}; // I will use these to know which state I'm on. Maybe useless, idk yet.
int state;
int screenBlock;
int playerhOff;
speedCheat = 1;
enum {PLAYERRIGHT, PLAYERSWORD, PLAYERUP, PLAYERLEFT, PLAYERIDLE}; // PLAYERUP and PLAYERDOWN prob pointless, but just in case
enum {ARROW1, ARROW2, ARROW3};
enum {SPELL1, SPELL2};
enum {REGENEMY1, REGENEMY2, REGENEMY3, REGENEMYIDLE};
//Initializes game
void initGame() {
    playerhOff = 0;
    screenBlock = 28;
    initPlayer();
    initArrow();
    initRegEnemys();
    initBoss();
    initEnemySpell();
    drawArrow();
    drawRegEnemy();
}

//Updates game
void updateGame() {
    if (state == GAME3 && screenBlock == 31) {
        updateBossSpellRain();
        updateBoss();
    } else if (state != GAME3) {
        updateSpell();
        updateRegEnemy();
    }
    updatePlayer();
    updateArrow();
    bossSpellCol = rand() % 240;
}

/* ========== START OF PLAYER CODE ========== */

//Updates the player dependent on buttons pressed
void updatePlayer() {
    int canJump = 1;
    if (player.health <= 0 && state == GAME3) {
        hOff = 0;
        playerhOff = 0;
        player.worldCol = 0;
        screenBlock = 28;
        boss.health = 2000;
        player.health = 100;
        shadowOAM[70].attr0 = ATTR0_HIDE;
    }
    if (BUTTON_HELD(BUTTON_UP) && BUTTON_PRESSED(BUTTON_L) && speedCheat == 1) {
        speedCheat = 0;
        player.cdel = 2;
    } else if (BUTTON_HELD(BUTTON_UP) && BUTTON_PRESSED(BUTTON_L) && speedCheat == 0) {
        speedCheat = 1;
        player.cdel = 1;
    }

    for (int i = 0; i < 8; i++) {
        if (spell[i].active == 1) {
            if (state != GAME3 && screenBlock != 31) {
                if (collision(spell[i].worldCol, spell[i].worldRow, spell[i].width, spell[i].height,
                        player.worldCol, player.worldRow, player.width, player.height)) {
                    player.health -= 3;
                    spell[i].active = 0;
                    shadowOAM[i + 40].attr0 = ATTR0_HIDE;
                }
            } else {
                if (collision(spell[i].worldCol, spell[i].worldRow, spell[i].width, spell[i].height,
                        player.worldCol - playerhOff, player.worldRow, player.width, player.height)) {
                    player.health -= 5;
                    spell[i].active = 0;
                    shadowOAM[i + 40].attr0 = ATTR0_HIDE;
                }
            }
        }
    }

    if (BUTTON_HELD(BUTTON_RIGHT)) {
        player.direction = PLAYERRIGHT;
        player.aniState = PLAYERRIGHT;
        if (state == GAME3) {
            if (player.worldCol + player.width < BOSSMAPWIDTH - 15) {
                player.worldCol += player.cdel;
            }
            if (screenBlock < 31 && (hOff < (BOSSMAPWIDTH - SCREENWIDTH)) && (player.worldCol - playerhOff) > SCREENWIDTH / 3) {
                hOff += player.cdel;
                playerhOff += player.cdel;
            }
        } else if (player.worldCol + player.width < 240) {
            player.worldCol += player.cdel;
        }
    }

    if (BUTTON_HELD(BUTTON_LEFT)) { // MOVE LEFT
        player.direction = PLAYERLEFT;
        player.aniState = PLAYERLEFT;
        if (state == GAME3) { // IF ON THE BOSS STAGE/XL BACKGROUND
            if (screenBlock == 31) {
                if (player.worldCol > 768) {
                    player.worldCol -= player.cdel;
                }
            } else {
                if (player.worldCol > 0) {
                    player.worldCol -= player.cdel;
                }
                if ((hOff - 1) >= 0 && (player.worldCol - playerhOff) <= SCREENWIDTH / 2) {
                    hOff -= player.cdel;
                    playerhOff -= player.cdel;
                }
            }
        } 
        if (player.worldCol > 0 && state != GAME3) {
            player.worldCol -= player.cdel;
        }
    }

    if (state == GAME) { // CHANGE FLOORHEIGHT DEPENDING ON STAGE TO MAKE JUMPING LOOK NICER
        floorheight = 14;
    } else if (state == GAME2) {
        floorheight = 28;
    } else if (state == GAME3) {
        floorheight = 4;
    }
    if (player.worldRow + player.height - (int) round(player.velocity * GRAVITYFACTOR) < SCREENHEIGHT - floorheight) {
        player.velocity = (int) player.velocity + (GRAVITY * GRAVITYFACTOR);
        
        canJump = 0;
        player.worldRow = (int) player.worldRow - round(player.velocity * GRAVITYFACTOR);
    } else {
        player.velocity = 0;
        canJump = 1;
    }
    if (BUTTON_PRESSED(BUTTON_R) && canJump) {
        player.worldRow -= 1;
        player.velocity += JUMPVELOCITY;
        player.jumpTimer = 0;
    }

    if (BUTTON_PRESSED(BUTTON_B) && (player.arrowTimer * player.arrowFireRate) >= 35 && canJump) {
        playSoundB(bowSound_data, bowSound_length, 0);
        fireArrow(speedCheat);
        shadowOAM[0].attr0 = (ROWMASK & (player.worldRow)) | ATTR0_SQUARE;
        shadowOAM[0].attr1 = (COLMASK & (player.worldCol - playerhOff)) | ATTR1_SMALL;
        shadowOAM[0].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(player.aniState, player.curFrame * 2);
        player.arrowTimer = 0;
    }
    player.arrowTimer++;
    if (state != PAUSE) {
        if (hOff >= 256 && state != PAUSE) {
            screenBlock++;
            hOff -= 256;
            REG_BG0CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(screenBlock) | BG_SIZE_WIDE | BG_4BPP;
        }
        if ((hOff - 1) < 0 && screenBlock > 28) {
            screenBlock--;
            hOff += 256;
            REG_BG0CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(screenBlock) | BG_SIZE_WIDE | BG_4BPP;
        }
        REG_BG0HOFF = hOff;
    } else {
        REG_BG0CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(28) | BG_SIZE_SMALL | BG_4BPP;
    }
    animatePlayer();
}

// Animates the player
void animatePlayer() {
    player.prevAniState = player.aniState;
    player.aniState = PLAYERIDLE;
    
    if (player.aniCounter % 8 == 0) {
        player.curFrame = (player.curFrame + 1) % player.numFrames;
    }

    if (BUTTON_HELD(BUTTON_RIGHT)) {
        player.aniState = PLAYERRIGHT;
    }
    if (BUTTON_HELD(BUTTON_LEFT)) {
        player.aniState = PLAYERLEFT;
    }
    if (BUTTON_PRESSED(BUTTON_A)) {
        player.aniState = PLAYERSWORD;
    }

    if (player.aniState == PLAYERIDLE) {
        player.curFrame = 0;
        player.aniCounter = 0;
        player.aniState = player.prevAniState;
    } else {
        player.aniCounter++;
    }
}

//Initializes player
void initPlayer() {
    player.width = 12;
    player.height = 15;
    player.rdel = 1;
    player.cdel = 1;
    player.lives = 1;
    player.worldRow = 130;
    player.health = 100;
    player.abilityPoints = 0;
    //player.worldRow = 140;
    player.worldCol = 0;
    player.aniCounter = 0;
    player.curFrame = 0;
    player.numFrames = 3;
    player.arrowTimer = 10;
    player.arrowFireRate = 1;
    player.arrowDamage = 1;
    player.score = 0;
    player.aniState = PLAYERRIGHT;
}

//Draws the player sprite
void drawPlayer() {
    if (player.hide) {
        shadowOAM[0].attr0 |= ATTR0_HIDE;
    } else {
        if (player.aniState == PLAYERLEFT) {
            shadowOAM[0].attr0 = (ROWMASK & (player.worldRow)) | ATTR0_SQUARE;
            shadowOAM[0].attr1 = (COLMASK & (player.worldCol - playerhOff)) | ATTR1_SMALL | ATTR1_HFLIP;
            shadowOAM[0].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(0, player.curFrame * 2);
        } else if (player.aniState == PLAYERRIGHT) {
            shadowOAM[0].attr0 = (ROWMASK & (player.worldRow)) | ATTR0_SQUARE;
            shadowOAM[0].attr1 = (COLMASK & (player.worldCol - playerhOff)) | ATTR1_SMALL;
            shadowOAM[0].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(player.aniState, player.curFrame * 2);
        }
    }
}

//Draws player health to screen
void drawPlayerHealth() {
    int ones = player.health % 10;
    int tens = (player.health / 10) % 10;
    int hundreds = (player.health / 100) % 10;

    shadowOAM[36].attr0 = (ROWMASK & (15)) | ATTR0_SQUARE;
    shadowOAM[36].attr1 = (COLMASK & (5)) | ATTR1_SMALL;
    shadowOAM[36].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(0, 28);

    shadowOAM[30].attr0 = (ROWMASK & (15)) | ATTR0_SQUARE;
    shadowOAM[30].attr1 = (COLMASK & (20)) | ATTR1_SMALL;
    shadowOAM[30].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(hundreds * 2, 30);

    shadowOAM[31].attr0 = (ROWMASK & (15)) | ATTR0_SQUARE;
    shadowOAM[31].attr1 = (COLMASK & (30)) | ATTR1_SMALL;
    shadowOAM[31].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(tens * 2, 30);

    shadowOAM[32].attr0 = (ROWMASK & (15)) | ATTR0_SQUARE;
    shadowOAM[32].attr1 = (COLMASK & (40)) | ATTR1_SMALL;
    shadowOAM[32].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(ones * 2, 30);
}

/* ======== END OF PLAYER CODE ======== */

void animateArrow() {
    for (int i = 0; i < 10; i++) {
        arrow[i].prevAniState = arrow[i].aniState;
        arrow[i].aniState = ARROW1;
        if (arrow[i].aniCounter % 2 == 0) {
            arrow[i].curFrame = (arrow[i].curFrame + 1) % arrow[i].numFrames;
        }
        arrow[i].aniCounter++;
    }
}

void animateSpell() {
    for (int i = 0; i < 8; i++) {
        spell[i].prevAniState = spell[i].aniState;
        spell[i].aniState = SPELL1;
        if (spell[i].aniCounter % 10 == 0) {
            spell[i].curFrame = (spell[i].curFrame + 1) % spell[i].numFrames;
        }
        spell[i].aniCounter++;
    }
}

void animateRegEnemies() {
    for (int i = 0; i < 3; i++) {
        regEnemy[i].prevAniState = regEnemy[i].aniState;
        regEnemy[i].aniState = REGENEMY1;
        if (regEnemy[i].aniCounter % 15 == 0) {
            regEnemy[i].curFrame = (regEnemy[i].curFrame + 1) % regEnemy[i].numFrames;
        }
        if (regEnemy[i].direction == LEFT || regEnemy[i].direction == RIGHT) {
            regEnemy[i].aniState = REGENEMY2;
        }
        if (regEnemy[i].aniState == REGENEMY1) {
            regEnemy[i].curFrame = 0;
            regEnemy[i].aniCounter = 0;
            regEnemy[i].aniState = regEnemy[i].prevAniState;
        } else {
            regEnemy[i].aniCounter++;
        }
    }
}

// Animates the boss, boss animates faster when health is reduced to a specific threshold
void animateBoss() {
    boss.prevAniState = boss.aniState;
    boss.aniState = REGENEMY1;
    int animateSpeed = 0;
    if (boss.health >= 1500 && boss.health <= 2000) {
        animateSpeed = 10;
    }
    if (boss.health >= 1000 && boss.health <= 1499) {
        animateSpeed = 8;
    } 
    if (boss.health >= 500 && boss.health <= 999) {
        animateSpeed = 6;
    }
    if (boss.health >= 0 && boss.health <= 499) {
        animateSpeed = 4;
    }
    if (boss.aniCounter % animateSpeed == 0) {
        boss.curFrame = (boss.curFrame + 1) % boss.numFrames;
    }
    boss.aniCounter++;
}


// Draws the game
void drawGame() {
    if (state == GAME3 && screenBlock == 31) {
        drawBossHealth();
        drawBoss();
    }
    drawPlayer();
    drawArrow();
    drawSpell();
    drawPlayerHealth();
    drawRegEnemy();
    waitForVBlank();
    DMANow(3, shadowOAM, OAM, 128 * 4);

}


void initBoss() {
    boss.width = 32;
    boss.height = 32;
    boss.rdel = 1;
    boss.cdel = 1;
    boss.health = 2000;
    boss.worldRow = player.worldRow - 5;
    boss.worldCol = 968;
    boss.active = 1;
    boss.erased = 0;
    boss.direction = LEFT;
    boss.curFrame = 12;
    boss.aniCounter = 0;
    boss.numFrames = 3;
    boss.aniState = REGENEMY1;
}

//Initializes regular enemies
void initRegEnemys() {
    enemysRemaining = 3;
    for (int i = 0, j = 0; i < 3; i++) {
        regEnemy[i].width = 32;
        regEnemy[i].height = 32;
        regEnemy[i].rdel = 1;
        regEnemy[i].cdel = 1;
        regEnemy[i].health = 30;
        regEnemy[i].worldRow = player.worldRow - 10;
        regEnemy[i].worldCol = 245 + j;
        regEnemy[i].active = 1;
        regEnemy[i].erased = 0;
        regEnemy[i].direction = LEFT;
        regEnemy[i].curFrame = 0;
        regEnemy[i].aniCounter = 0;
        regEnemy[i].numFrames = 3;
        regEnemy[i].aniState = REGENEMY1;
        j += 40;
    }
}


//Initializes arrows
void initArrow() {
    for (int i = 0; i < 10; i++) {
        arrow[i].width = 16;
        arrow[i].height = 16;
        arrow[i].rdel = 3;
        arrow[i].cdel = 3;
        arrow[i].worldRow = player.worldRow;
        arrow[i].worldCol = 0;
        arrow[i].active = 0;
        arrow[i].erased = 0;
        arrow[i].aniCounter = 0;
        arrow[i].curFrame = 0;
        arrow[i].numFrames = 3;
        arrow[i].aniState = ARROW1;
    }
}

void initEnemySpell() {
    int j = 3;
    for (int i = 0; i < 8; i++) {
        spell[i].width = 11;
        spell[i].height = 6;
        spell[i].rdel = 3;
        spell[i].cdel = 3;
        spell[i].worldRow = regEnemy[j--].worldRow;
        spell[i].worldCol = 260;
        spell[i].active = 0;
        spell[i].erased = 0;
        spell[i].aniCounter = 0;
        spell[i].score = 0;
        spell[i].curFrame = 0;
        spell[i].numFrames = 2;
        spell[i].aniState = SPELL1;
        if (j < 0) {
            j = 3;
        }
    }
}

void fireEnemySpell(int row, int col, int direction) {
    for (int i = 0; i < 8; i++) {
        if (spell[i].active == 0) {
            spell[i].worldRow = row + 10;
            spell[i].worldCol = col;
            spell[i].active = 1;
            spell[i].direction = direction;
            spell[i].erased = 0;
            break;
        }
    }
}

// Draws the boss's attack of raining down spells
void updateBossSpellRain() {
    for (int i = 0; i < 8; i++) {
        if (spell[i].active == 1 && screenBlock == 31) {
            if (spell[i].worldRow < 160) {
                if (boss.health >= 1500 && boss.health <= 2000) {
                     spell[i].worldRow += 1;
                } else if (boss.health >= 1000 && boss.health <= 1499) {
                     spell[i].worldRow += 2;
                } else if (boss.health >= 500 && boss.health <= 999) {
                     spell[i].worldRow += 3;
                } else if (boss.health >= 0 && boss.health <= 499) {
                     spell[i].worldRow += 4;
                }

            } else {
                spell[i].active = 0;
                shadowOAM[i + 40].attr0 = ATTR0_HIDE;
            }
            if (collision(spell[i].worldCol, spell[i].worldRow, spell[i].width, spell[i].height,
                    player.worldCol, player.worldRow, player.width, player.height)) {
                    player.health -= 1;
                    spell[i].active = 0;
                    shadowOAM[i + 40].attr0 = ATTR0_HIDE;
            }
        }
    }
    animateSpell();
}

//Fires (shoots) the arrow
void fireArrow(int cheatActive) {
    if (cheatActive == 0) {
        for (int i = 0; i < 10; i++) {
            if (arrow[i].active == 0) {
                arrow[i].worldRow = player.worldRow - 5;
                if (player.direction == PLAYERRIGHT) {
                    arrow[i].worldCol = player.worldCol + 5;
                } else {
                    arrow[i].worldCol = player.worldCol - 5;
                }
                arrow[i].active = 1;
                arrow[i].direction = player.direction;
                arrow[i].erased = 0;
                break;
            }
        }
    }
    for (int i = 0; i < 10; i++) {
        if (arrow[i].active == 0) {
            arrow[i].worldRow = player.worldRow;
            if (player.direction == PLAYERRIGHT) {
                arrow[i].worldCol = player.worldCol + 5;
            } else {
                arrow[i].worldCol = player.worldCol - 5;
            }
            arrow[i].active = 1;
            arrow[i].direction = player.direction;
            arrow[i].erased = 0;
            break;
        }
    }
}

void updateBoss() {
    int canJump = 1;
    for (int i = 0; i < 10; i++) {
        if (collision(arrow[i].worldCol, arrow[i].worldRow, arrow[i].width, arrow[i].height,
                        boss.worldCol, boss.worldRow, boss.width, boss.height)
                        && arrow[i].active == 1 && boss.active == 1) {
            arrow[i].active = 0;
            boss.health -= player.arrowDamage * (rand() % (10 + 1 - 1) + 1);
            shadowOAM[i + 1].attr0 = ATTR0_HIDE;
        }
    }
    if (boss.worldRow + boss.height - (int) round(boss.velocity * BOSSGRAVITYFACTOR) < SCREENHEIGHT - floorheight) {
        boss.velocity = (int) boss.velocity + (GRAVITY * BOSSGRAVITYFACTOR);
        
        canJump = 0;
        boss.worldRow = (int) boss.worldRow - round(boss.velocity * BOSSGRAVITYFACTOR);
    } else {
        boss.velocity = 0;
        canJump = 1;
    }
    if (canJump) {
        boss.worldRow -= 1;
        boss.velocity += BOSSJUMPVELOCITY;
        boss.jumpTimer = 0;
    }
    if (boss.health <= 0) {
        boss.health = 0;
        enemysRemaining = 0;
        boss.active = 0;
        shadowOAM[70].attr0 = ATTR0_HIDE;
    }
    if (boss.active == 1) {
        for (int i = 0; i < 8; i++) {
            if (spell[i].active == 0 && spell[i].worldCol != bossSpellCol) {
                spell[i].worldRow = 0;
                spell[i].worldCol = bossSpellCol;
                spell[i].active = 1;
                spell[i].erased = 0;
                break;
            }
        }
    }
    animateBoss();
}


//Updates the position of regular enemies. 
// NEED TO SET GLOBAL VARIABLE COUNT INSTEAD OF USING 3 ****
void updateRegEnemy() {

    regEnemy[0].score++;
    regEnemy[1].score++;
    regEnemy[2].score++;
    checkEnemyMovement(0, 40);
    checkEnemyMovement(1, 70);
    checkEnemyMovement(2, 120);

    //Checks for arrow colliding with enemy
    int i = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            if (collision(regEnemy[i].worldCol, regEnemy[i].worldRow, regEnemy[i].width, regEnemy[i].height,
                arrow[j].worldCol, arrow[j].worldRow, arrow[j].width, arrow[j].height)
                && arrow[j].active == 1 && regEnemy[i].active == 1) {
                    regEnemy[i].health -= (player.arrowDamage * 5);
                    arrow[j].active = 0;
                    shadowOAM[j + 1].attr0 = ATTR0_HIDE;
                    if (regEnemy[i].health <= 0) {
                        shadowOAM[i + 10].attr0 = ATTR0_HIDE;
                        regEnemy[i].active = 0;
                        enemysRemaining--;
                    }
            }
        }
    }
    animateRegEnemies();
}

void checkEnemyMovement(int enemyIndex, int distance) {
    if (regEnemy[enemyIndex].score % 2 == 0) {
        if (regEnemy[enemyIndex].active == 1) {
            if (regEnemy[enemyIndex].score % 150 == 0) {
                fireEnemySpell(regEnemy[enemyIndex].worldRow, regEnemy[enemyIndex].worldCol, regEnemy[enemyIndex].direction);
            }
            if (abs(regEnemy[enemyIndex].worldCol - player.worldCol) > distance) {
                if (regEnemy[enemyIndex].direction == LEFT) {
                    regEnemy[enemyIndex].worldCol -= regEnemy[enemyIndex].cdel;
                }
            }
            if (abs(regEnemy[enemyIndex].worldCol + regEnemy[enemyIndex].width - player.worldCol) > distance) {
                if (regEnemy[enemyIndex].direction == RIGHT) {
                    regEnemy[enemyIndex].worldCol += regEnemy[enemyIndex].cdel;
                }
            }
            if (regEnemy[enemyIndex].direction == LEFT && 
                (regEnemy[enemyIndex].worldCol + regEnemy[enemyIndex].width - player.worldCol) < 0) {
                regEnemy[enemyIndex].direction = RIGHT;
            } else if (regEnemy[enemyIndex].direction == RIGHT && (regEnemy[enemyIndex].worldCol - player.worldCol) > 0) {
                regEnemy[enemyIndex].direction = LEFT;
            }
        }
    }
}

//Updates spell location
void updateSpell() {
    for (int i = 0; i < 8; i++) {
        if (spell[i].active == 1) { 
            if (spell[i].score % 3 == 0) {
                if (spell[i].direction == RIGHT) {
                    if (spell[i].worldCol + spell[i].width + 5 < 240) {
                        spell[i].worldCol += spell[i].cdel;
                    } else {
                        spell[i].active = 0;
                        shadowOAM[i + 40].attr0 = ATTR0_HIDE;
                    }
                } else if (spell[i].direction == LEFT) {
                    if (spell[i].worldCol > 0) {
                        spell[i].worldCol -= spell[i].cdel;
                    } else {
                        spell[i].active = 0;
                        shadowOAM[i + 40].attr0 = ATTR0_HIDE;
                    }
                }
            }
            spell[i].score++;
        }
    }
    animateSpell();
}

//Updates arrow position when fired
int col = 256;
void updateArrow() {
    for (int i = 0; i < 10; i++) {
        if (arrow[i].active == 1) { 
            if (arrow[i].direction == PLAYERRIGHT) {
                if (arrow[i].worldCol + arrow[i].width + 5 < col + playerhOff) {
                    arrow[i].worldCol += arrow[i].cdel;
                } else {
                    arrow[i].active = 0;
                    shadowOAM[i + 1].attr0 = ATTR0_HIDE;
                }
            } else if (arrow[i].direction == PLAYERLEFT) {
                if (arrow[i].worldCol > playerhOff) {
                    arrow[i].worldCol -= arrow[i].cdel;
                } else {
                    arrow[i].active = 0;
                    shadowOAM[i + 1].attr0 = ATTR0_HIDE;
                }
            }
        }
    }
    animateArrow();
}

//Draws the regular enemies
void drawRegEnemy() {
    for (int i = 0; i < 3; i++) {
        if (regEnemy[i].active == 1) {
            if (regEnemy[i].direction == LEFT) {
                shadowOAM[i + 10].attr0 = (ROWMASK & (regEnemy[i].worldRow)) | ATTR0_SQUARE;
                shadowOAM[i + 10].attr1 = (COLMASK & (regEnemy[i].worldCol)) | ATTR1_MEDIUM | ATTR1_HFLIP;
                shadowOAM[i + 10].attr2 = ATTR2_PALROW(1) | ATTR2_TILEID(28, regEnemy[i].curFrame * 4);
            } else {
               shadowOAM[i + 10].attr0 = (ROWMASK & (regEnemy[i].worldRow)) | ATTR0_SQUARE;
                shadowOAM[i + 10].attr1 = (COLMASK & (regEnemy[i].worldCol)) | ATTR1_MEDIUM;
                shadowOAM[i + 10].attr2 = ATTR2_PALROW(1) | ATTR2_TILEID(28, regEnemy[i].curFrame * 4); 
            }
        } else if (regEnemy[i].erased == 0) {
            regEnemy[i].erased = 1;
        }
    }
}

//Draws the arrows the player fires
void drawArrow() {
    for (int i = 0; i < 10; i++) {
        if (arrow[i].active == 1) {
            if (arrow[i].direction == PLAYERLEFT) {
                shadowOAM[i + 1].attr0 = (ROWMASK & (arrow[i].worldRow)) | ATTR0_SQUARE;
                shadowOAM[i + 1].attr1 = (COLMASK & (arrow[i].worldCol - playerhOff)) | ATTR1_SMALL | ATTR1_HFLIP;
                shadowOAM[i + 1].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(4, arrow[i].curFrame * 2);
            } else {
                shadowOAM[i + 1].attr0 = (ROWMASK & (arrow[i].worldRow)) | ATTR0_SQUARE;
                shadowOAM[i + 1].attr1 = (COLMASK & (arrow[i].worldCol - playerhOff))    | ATTR1_SMALL;
                shadowOAM[i + 1].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(4, arrow[i].curFrame * 2);
            }
        } else if (arrow[i].erased == 0) {
            arrow[i].erased = 1;
        }
    }
}


//Draws spells to screen that are casted by the wizards
void drawSpell() {
    for (int i = 0; i < 8; i++) {
        if (spell[i].active == 1) {
            if (state == GAME3 && screenBlock == 31) {
                shadowOAM[i + 40].attr0 = (ROWMASK & (spell[i].worldRow)) | ATTR0_SQUARE;
                shadowOAM[i + 40].attr1 = (COLMASK & (spell[i].worldCol)) | ATTR1_SMALL;
                shadowOAM[i + 40].attr2 = ATTR2_PALROW(2)| ATTR2_TILEID(24, spell[i].curFrame * 2);
            }
            if (state != GAME3) {
                if (spell[i].direction == LEFT) {
                    shadowOAM[i + 40].attr0 = (ROWMASK & (spell[i].worldRow)) | ATTR0_SQUARE;
                    shadowOAM[i + 40].attr1 = (COLMASK & (spell[i].worldCol)) | ATTR1_SMALL | ATTR1_HFLIP;
                    shadowOAM[i + 40].attr2 = ATTR2_PALROW(1)| ATTR2_TILEID(26, spell[i].curFrame * 2);
                } else if (spell[i].direction == RIGHT) {
                    shadowOAM[i + 40].attr0 = (ROWMASK & (spell[i].worldRow)) | ATTR0_SQUARE;
                    shadowOAM[i + 40].attr1 = (COLMASK & (spell[i].worldCol)) | ATTR1_SMALL;
                    shadowOAM[i + 40].attr2 = ATTR2_PALROW(1)| ATTR2_TILEID(26, spell[i].curFrame * 2);
                }
            }
        } else if (spell[i].erased == 0) {
            spell[i].erased = 1;
        }
    }
}

// Draws health of Arceuus, the boss, to the screen
void drawBossHealth() {
    int ones = boss.health % 10;
    int tens = (boss.health / 10) % 10;
    int hundreds = (boss.health / 100) % 10;
    int thousands = (boss.health / 1000) % 10;
    if (boss.active == 1) {
        shadowOAM[75].attr0 = (ROWMASK & (15)) | ATTR0_SQUARE;
        shadowOAM[75].attr1 = (COLMASK & (170)) | ATTR1_SMALL;
        shadowOAM[75].attr2 = ATTR2_PALROW(3) | ATTR2_TILEID(0, 28);

        shadowOAM[71].attr0 = (ROWMASK & (15)) | ATTR0_SQUARE;
        shadowOAM[71].attr1 = (COLMASK & (200)) | ATTR1_SMALL;
        shadowOAM[71].attr2 = ATTR2_PALROW(3) | ATTR2_TILEID(hundreds * 2, 30);

        shadowOAM[72].attr0 = (ROWMASK & (15)) | ATTR0_SQUARE;
        shadowOAM[72].attr1 = (COLMASK & (210)) | ATTR1_SMALL;
        shadowOAM[72].attr2 = ATTR2_PALROW(3) | ATTR2_TILEID(tens * 2, 30);

        shadowOAM[73].attr0 = (ROWMASK & (15)) | ATTR0_SQUARE;
        shadowOAM[73].attr1 = (COLMASK & (220)) | ATTR1_SMALL;
        shadowOAM[73].attr2 = ATTR2_PALROW(3) | ATTR2_TILEID(ones * 2, 30);

        shadowOAM[74].attr0 = (ROWMASK & (15)) | ATTR0_SQUARE;
        shadowOAM[74].attr1 = (COLMASK & (190)) | ATTR1_SMALL;
        shadowOAM[74].attr2 = ATTR2_PALROW(3) | ATTR2_TILEID(thousands * 2, 30);
    } else if (boss.active == 0 && state == GAME3) {
        for (int i = 71; i <= 75; i++) {
            shadowOAM[i].attr0 = ATTR0_HIDE;
        }
    }
}

void drawBoss() {
    if (boss.active == 1) {
        if (boss.direction == LEFT) {
            shadowOAM[70].attr0 = (ROWMASK & (boss.worldRow)) | ATTR0_SQUARE;
            shadowOAM[70].attr1 = (COLMASK & (boss.worldCol - 768)) | ATTR1_MEDIUM | ATTR1_HFLIP;
            shadowOAM[70].attr2 = ATTR2_PALROW(2) | ATTR2_TILEID(20, boss.curFrame * 4);
        }
    } 
}
