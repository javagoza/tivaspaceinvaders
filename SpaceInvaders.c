// SpaceInvaders.c
// Enrique Albertos
// Runs on LM4F120/TM4C123
// Based in Jonathan Valvano and Daniel Valvano starter project for edX Lab 15

// April 10, 2014
// March 2015
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// ******* Required Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PE2/AIN1
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Blue Nokia 5110
// ---------------
// Signal        (Nokia 5110) LaunchPad pin
// Reset         (RST, pin 1) connected to PA7
// SSI0Fss       (CE,  pin 2) connected to PA3
// Data/Command  (DC,  pin 3) connected to PA6
// SSI0Tx        (Din, pin 4) connected to PA5
// SSI0Clk       (Clk, pin 5) connected to PA2
// 3.3V          (Vcc, pin 6) power
// back light    (BL,  pin 7) not connected, consists of 4 white LEDs which draw ~80mA total
// Ground        (Gnd, pin 8) ground

// Red SparkFun Nokia 5110 (LCD-10168)
// -----------------------------------
// Signal        (Nokia 5110) LaunchPad pin
// 3.3V          (VCC, pin 1) power
// Ground        (GND, pin 2) ground
// SSI0Fss       (SCE, pin 3) connected to PA3
// Reset         (RST, pin 4) connected to PA7
// Data/Command  (D/C, pin 5) connected to PA6
// SSI0Tx        (DN,  pin 6) connected to PA5
// SSI0Clk       (SCLK, pin 7) connected to PA2
// back light    (LED, pin 8) not connected, consists of 4 white LEDs which draw ~80mA total

#include "tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "Random.h"
#include "TExaS.h"
#include "RefreshTimer.c"
#include "SoundsManager.c"
#include "SwitchesManager.c"
#include "LedsManager.c"
#include "SlideJoyStickManager.c"
#include "Images.inc.c"
#include "SpritesStateManager.c"


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void); // Enable interrupts


#define MAX_ENEMIES 12
#define MAX_BUNKERS 3
#define MAX_PLAYER 1
#define MAX_ENEMY_BONUS 1
#define MAX_MISSILES 5
#define MAX_LASERS 5

#define MAX_SPRITES MAX_ENEMIES + MAX_PLAYER + MAX_BUNKERS + MAX_ENEMY_BONUS + MAX_MISSILES + MAX_LASERS

#define LASERS_OFFSET  0
#define MISSILES_OFFSET LASERS_OFFSET + MAX_LASERS
#define ENEMIES_OFFSET MISSILES_OFFSET + MAX_MISSILES 
#define PLAYER_OFFSET ENEMIES_OFFSET + MAX_ENEMIES 
#define BUNKERS_OFFSET PLAYER_OFFSET + MAX_PLAYER
#define ENEMY_BONUS_OFFSET BUNKERS_OFFSET + MAX_BUNKERS


#define ENEMIES_PER_ROW 4
#define MAX_ENEMIES_ROWS 3

#define SCREEN_WIDTH 84
#define SCREEN_HEIGHT 48

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif


GameStateType game;


const unsigned char *player[1] = {PlayerShip0};
struct Animation playerAnimation = {1, 2, 0, player, 0, 0, 1};

const unsigned char *enemy10[2] = {SmallEnemy10PointA, SmallEnemy10PointB};
AnimationType enemy10Animation = {2, 2, 1, enemy10, 0, 0, 3};

const unsigned char *enemy20[2] = {SmallEnemy20PointA, SmallEnemy20PointB};
AnimationType enemy20Animation = {2, 1, 1, enemy20, 0, 0, 3};

const unsigned char *enemy30[2] = {SmallEnemy30PointA, SmallEnemy30PointB};
AnimationType enemy30Animation = {2, 3, 1, enemy30, 0, 0, 3};

const unsigned char *smallExplosion[1] = {SmallExplosion0};
AnimationType smallExplosionAnimation = {1, 0, 0, smallExplosion, 0, 0, 1};

const unsigned char *bigExplosion[1] = {BigExplosion0};
AnimationType bigExplosionAnimation = {1, 0, 0, bigExplosion, 0, 0, 1};

const unsigned char *missile[2] = {Missile0, Missile1};
AnimationType missileAnimation = {2, 1, 1, missile, 0, 0, 1};


const unsigned char *laser[2] = {Laser0};
AnimationType laserAnimation = {1, 1, 1, laser, 0, 0, 1};

const unsigned char *bunker[3] = {Bunker0, Bunker1, Bunker2};
AnimationType bunkerAnimation = {3, 0, 0, bunker, 0, 0, 1};

const unsigned char *enemyBonus[1] = {SmallEnemyBonus0};
AnimationType enemyBonusAnimation = {1, 2, 0, enemyBonus, 0, 0, 1};


void Suspend(SpriteStateType *self) {
    self->active = FALSE;
    self->visible = FALSE;
}

void Suspend_Enemy(SpriteStateType *self) {
    Suspend(self);
    self->game->score += self->rank;
    self->game->enemiesState.enemiesCount--;
}

void Suspend_Bonus(SpriteStateType *self) {
    Suspend(self);
    self->game->score += self->rank;
}

void Hit(SpriteStateType *self) {
    self->actualAnimationPtr = &self->hitAnimation;
    self->hitAnimation.framesCounter = 0;
    self->hitAnimation.nextImage = 0;
    self->state = EXPLODING;
}

void Hit_Bunker(SpriteStateType *self) {
    if (self->actualAnimationPtr->nextImage >= self->actualAnimationPtr->maxImages) {
        // bunker destroyed
        Hit(self);
        self->suspend(self);
    } else {
        // bunker damaged
        self->actualAnimationPtr->nextImage += 1;
    }
}

void Hit_Player(SpriteStateType *self) {
    // player hit
    Hit(self);
    self->state = HIT;
    self->game->playerState.lives--;
}

void Hit_Enemy(SpriteStateType *self) {
    // player hit
    Hit(self);
}

void Reset(SpriteStateType *self) {
    self->actualAnimationPtr = &self->animation;
    self->hitAnimation.framesCounter = 0;
    self->hitAnimation.nextImage = 0;
    self->state = ACTION;
}

void Update_With_Velocity(SpriteStateType *self) {
    self->x += self->velocityX;
    self->y += self->velocityY;
}

void Update_Missiles(SpriteStateType *self) {
    Update_With_Velocity(self);
}

void Update_Lasers(SpriteStateType *self) {
    Update_With_Velocity(self);
}

void Update_Player_Position(SpriteStateType *self) {
    if (self->state == HIT) {
        (self->reset)(self);
        ;
    }
    self->x = getSlidePosition();
}

void Draw_Static(SpriteStateType *self) {
    AnimationType *animation = &(*self->actualAnimationPtr);
    // sprite outside screen
    if (self->x > SCREEN_WIDTH || self->x < 0 || self->y > SCREEN_HEIGHT || self->y < 0 || !animation->images[animation->nextImage]) {
        self->suspend(self);
        return;
    }
    Nokia5110_PrintBMP(
            self->x,
            self->y,
            animation->images[animation->nextImage], 0);
}

void Draw_Animation(SpriteStateType *self) {
    AnimationType *animation = &(*self->actualAnimationPtr);
    Draw_Static(self);
    animation->framesCounter++;
    if (animation->framesCounter >= animation->framesPerImage) {
        animation->framesCounter = 0;
        animation->nextImage = (animation->nextImage + 1) % animation->maxImages;
    }
}

void Null_Update(SpriteStateType *self) {
}

void Update_Enemy_Bonus_Position(SpriteStateType *self) {
    Update_With_Velocity(self);
    if (self->x > (SCREEN_WIDTH - ((unsigned char) self->width))
            || (self->y > (SCREEN_HEIGHT - ((unsigned char) self->height)))) {
        (self->suspend)(self);
    }
}

void Sprite_create(SpriteStateType* sprite,
        double positionX,
        double positionY,
        double velocityX,
        double velocityY,
        short visible,
        short active,
        void (*updatePosition)(SpriteStateType* self),
        void (*draw)(SpriteStateType* self),
        void (*suspend)(SpriteStateType* self),
        void (*hit)(SpriteStateType* self),
        void (*reset)(SpriteStateType* self),
        unsigned char (*intersect)(SpriteStateType* self, SpriteStateType* target),
        AnimationType animation,
        AnimationType hitAnimation,
        GameStateType* game,
        unsigned long rank) {
    sprite->x = positionX;
    sprite->y = positionY;
    sprite->xMargin = (unsigned char) animation.xMargin;
    sprite->yMargin = (unsigned char) animation.yMargin;
    sprite->width = ((unsigned char) animation.images[0][18]);
    sprite->height = ((unsigned char) animation.images[0][22]);
    sprite->active = active;
    sprite->visible = visible;
    sprite->velocityX = velocityX;
    sprite->velocityY = velocityY;
    sprite->updatePosition = updatePosition;
    sprite->draw = draw;
    sprite->animation = animation;
    sprite->hitAnimation = hitAnimation;
    sprite->actualAnimationPtr = &sprite->animation;
    sprite->suspend = suspend;
    sprite->reset = reset;
    sprite->hit = hit;
    sprite->intersect = intersect;
    sprite->state = ACTION;
    sprite->game = game;
    sprite->rank = rank;
}

void Create_Enemies(GameStateType* game) {
    short i;
    short row;
    unsigned long rank;
    AnimationType animation;
    game->enemiesState.enemiesCount = 0;
    for (row = 0; row < MAX_ENEMIES_ROWS; row++) {
        switch (row) {
            case 0: animation = enemy30Animation;
                rank = 30;
                break;
            case 1: animation = enemy20Animation;
                rank = 20;
                break;
            case 2: animation = enemy10Animation;
                rank = 10;
                break;
        }

        for (i = ENEMIES_OFFSET + (row * ENEMIES_PER_ROW); i < (ENEMIES_OFFSET + ENEMIES_PER_ROW * (row + 1)); i++) {
            game->enemiesState.enemiesCount++;
            Sprite_create(&game->sprites[i],
                    ENEMY30W * (i % ENEMIES_PER_ROW) + 1,
                    ENEMY30H * (row + 1),
                    0.5,
                    0,
                    TRUE,
                    TRUE,
                    &Update_With_Velocity,
                    &Draw_Animation,
                    &Suspend_Enemy,
                    &Hit_Enemy,
                    &Reset,
                    &Intersect,
                    animation,
                    smallExplosionAnimation,
                    game,
                    rank);
        }
    }
}

void Create_Player(GameStateType* game) {
    Sprite_create(&game->sprites[PLAYER_OFFSET],
            getSlidePosition(),
            47,
            0,
            0,
            TRUE,
            TRUE,
            &Update_Player_Position,
            &Draw_Animation,
            &Suspend,
            &Hit_Player,
            &Reset,
            &Intersect,
            playerAnimation,
            bigExplosionAnimation,
            game,
            0);
}

void Create_Bunkers(GameStateType* game) {
    int i;
    int j = 0;
    for (i = BUNKERS_OFFSET; i < BUNKERS_OFFSET + MAX_BUNKERS; i++) {
        Sprite_create(&game->sprites[i],
                11 + 22 * j++,
                47 - PLAYERH,
                0,
                0,
                TRUE,
                TRUE,
                &Null_Update,
                &Draw_Static,
                &Suspend,
                &Hit_Bunker,
                &Reset,
                &Intersect,
                bunkerAnimation,
                bigExplosionAnimation,
                game,
                0);
    }

}

void Create_EnemyBonus(GameStateType* game) {
    Sprite_create(&game->sprites[ENEMY_BONUS_OFFSET],
            0,
            ENEMYBONUSH,
            3,
            0,
            FALSE,
            FALSE,
            &Update_Enemy_Bonus_Position,
            &Draw_Animation,
            &Suspend_Bonus,
            &Hit,
            &Reset,
            &Intersect,
            enemyBonusAnimation,
            smallExplosionAnimation,
            game,
            50);
}

void Create_Missiles(GameStateType* game) {
    int i;
    for (i = MISSILES_OFFSET; i < MISSILES_OFFSET + MAX_MISSILES; i++) {
        Sprite_create(&game->sprites[i],
                Random() % SCREEN_WIDTH,
                ENEMYBONUSH,
                0,
                3,
                FALSE,
                FALSE,
                &Update_Missiles,
                &Draw_Animation,
                &Suspend,
                &Hit,
                &Reset,
                &Intersect,
                missileAnimation,
                smallExplosionAnimation,
                game,
                0);
    }

}

void Create_Lasers(GameStateType* game) {
    int i;
    for (i = LASERS_OFFSET; i < LASERS_OFFSET + MAX_LASERS; i++) {
        Sprite_create(&game->sprites[i],
                Random32() % SCREEN_WIDTH,
                47 - PLAYERH,
                0,
                -3,
                FALSE,
                FALSE,
                &Update_Lasers,
                &Draw_Animation,
                &Suspend,
                &Hit,
                &Reset,
                &Intersect,
                laserAnimation,
                smallExplosionAnimation,
                game,
                0);
    }
}

void Init_Sprites(GameStateType* game) {
    Create_Enemies(game);
    Create_Player(game);
    Create_Bunkers(game);
    Create_EnemyBonus(game);
    Create_Missiles(game);
    Create_Lasers(game);
}

unsigned long Check_Upmost_Enemy(SpriteStateType *sprites) {
    int i;
    int found = 0;
    unsigned long lowerY = 999;
    for (i = ENEMIES_OFFSET; i < (ENEMIES_OFFSET + MAX_ENEMIES); i++) {
        if (sprites[i].state == ACTION
                && sprites[i].active
                && sprites[i].y <= lowerY) {
            found = 1;
            lowerY = sprites[i].y;
        }
    }
    return found ? lowerY : 0;
}

/**
 * Do enemies classic coreography, moves all the enemies in block
 * left to right and right to left descending towards the player
 * checks if the active left most enemy or the active right most enemy
 * are touching left or rigth bounds, if so change sprites direction
 * and break
 */
void Update_Enemies_Coreography(SpriteStateType *sprites) {
    int leftmostOffset = -1;
    int rightmostOffset = -1;
    int lowerX = -1;
    int higherX = -1;
    unsigned char firstActive = TRUE;

    int i;
    // check for leftmost sprite and rightmost sprite
    for (i = ENEMIES_OFFSET; i < (ENEMIES_OFFSET + MAX_ENEMIES); i++) {
        if (sprites[i].state == ACTION
                && sprites[i].active) {
            if (firstActive) {
                firstActive = FALSE;
                lowerX = sprites[i].x;
                higherX = sprites[i].x;
                leftmostOffset = i;
                rightmostOffset = i;
            } else {
                if (sprites[i].x < lowerX) {
                    lowerX = sprites[i].x;
                    leftmostOffset = i;
                }
                if (sprites[i].x > higherX) {
                    higherX = sprites[i].x;
                    rightmostOffset = i;
                }
            }
        }
        Sound_Fastinvader1();
    }

    if (leftmostOffset >= 0 && rightmostOffset >= 0) {
        if (((sprites[leftmostOffset].x + sprites[leftmostOffset].velocityX) < 0)
                || ((sprites[rightmostOffset].active && ((sprites[rightmostOffset].x
                + sprites[rightmostOffset].velocityX) > (SCREEN_WIDTH - ENEMY30W))))) {
            for (i = ENEMIES_OFFSET; i < (ENEMIES_OFFSET + MAX_ENEMIES); i++) {
                if (sprites[i].active && sprites[i].visible && sprites[i].state == ACTION) {
                    // change direction
                    sprites[i].velocityX *= -1.2;
                    // change to the next row
                    sprites[i].y += 3;
                }
            }

        }
    }
}

/**
 *Check if an active laser has reached the top of the screen
 * disable the laser and reload it in the player laser gun
 */
void Update_Missed_Lasers(GameStateType *game) {
    unsigned long i;
    LasersStateType* state = &game->lasersState;
    if (!state->lasersLeft) {
        for (i = 0; i < state->maxLasers; i++) {
            if (!state->isLoaded[i]
                    && game->sprites[LASERS_OFFSET + i].active
                    && game->sprites[LASERS_OFFSET + i].visible
                    && game->sprites[LASERS_OFFSET + i].y <= game->sprites[LASERS_OFFSET + i].height) {
                state->isLoaded[i] = TRUE;
                state->lasersLeft += 1;
                game->sprites[LASERS_OFFSET + i].active = FALSE;
                game->sprites[LASERS_OFFSET + i].visible = FALSE;
            }
        }
    }
}

SpriteStateType* Shot_Got_Target_Type(SpriteStateType *laser,
        SpriteStateType *sprites,
        unsigned short offset,
        unsigned short maxOffset) {
    int i;
    // check collision in inverse order, lower screen highest priority
    for (i = offset + maxOffset - 1; i >= offset; i--) {
        if (laser->intersect(laser, &sprites[i])) {
            return &sprites[i];
        }
    }
    return 0;
}

SpriteStateType* Laser_Got_Target(SpriteStateType *laser, SpriteStateType *sprites) {
    SpriteStateType *target = 0;
    target = Shot_Got_Target_Type(laser, sprites, BUNKERS_OFFSET, MAX_BUNKERS);
    if (target == 0) {
        target = Shot_Got_Target_Type(laser, sprites, ENEMIES_OFFSET, MAX_ENEMIES);
    }
    if (target == 0) {
        target = Shot_Got_Target_Type(laser, sprites, ENEMY_BONUS_OFFSET, MAX_ENEMY_BONUS);
    }
    return target;
}

/**
 * Check if an active laser has reached a target
 * disable the laser and hit the target
 */
void Check_Lasers_Collisions(GameStateType* game) {
    unsigned long i;
    LasersStateType* state = &game->lasersState;
    for (i = 0; i < state->maxLasers; i++) {
        if (!state->isLoaded[i]
                && game->sprites[LASERS_OFFSET + i].active
                && game->sprites[LASERS_OFFSET + i].visible) {
            // Active laser check collision with enemies
            SpriteStateType *target = Laser_Got_Target(&game->sprites[LASERS_OFFSET + i], game->sprites);
            if (target) {
                state->isLoaded[i] = TRUE;
                state->lasersLeft += 1;
                game->sprites[LASERS_OFFSET + i].active = FALSE;
                game->sprites[LASERS_OFFSET + i].visible = FALSE;
                (target->hit)(target);
                setLed0On();
                Sound_Explosion();
            }
        }
    }
}

SpriteStateType* Missile_Got_Target(SpriteStateType *laser, SpriteStateType *sprites) {
    SpriteStateType *target = 0;
    target = Shot_Got_Target_Type(laser, sprites, BUNKERS_OFFSET, MAX_BUNKERS);
    if (target == 0) {
        target = Shot_Got_Target_Type(laser, sprites, PLAYER_OFFSET, MAX_PLAYER);
    }
    return target;
}

/**
 * Check if an active missile has reached a target
 * disable the missile and hit the target
 */
void Check_Missiles_Collisions(GameStateType* game) {
    unsigned long i;
    MissilesStateType* state = &game->missilesState;
    for (i = 0; i < state->maxMissiles; i++) {
        if (!state->isLoaded[i]
                && game->sprites[MISSILES_OFFSET + i].active
                && game->sprites[MISSILES_OFFSET + i].visible) {
            // Active missile check collision with bunkers and player
            SpriteStateType *target = Missile_Got_Target(&game->sprites[MISSILES_OFFSET + i], game->sprites);
            if (target) {
                state->isLoaded[i] = TRUE;
                state->missilesLeft += 1;
                game->sprites[MISSILES_OFFSET + i].active = FALSE;
                game->sprites[MISSILES_OFFSET + i].visible = FALSE;
                (target->hit)(target);
                setLed1On();
                Sound_Explosion();
            }
        }
    }
}

SpriteStateType* Enemy_Got_Player(SpriteStateType *enemy, SpriteStateType *sprites) {
    SpriteStateType *target = 0;
    target = Shot_Got_Target_Type(enemy, sprites, PLAYER_OFFSET, MAX_PLAYER);
    return target;
}

void Check_Enemies_Collisions(GameStateType* game) {
    unsigned long i;
    for (i = ENEMIES_OFFSET; i < (ENEMIES_OFFSET + MAX_ENEMIES); i++) {
        if (game->sprites[i].active && game->sprites[i].visible && game->sprites[i].state == ACTION) {
            SpriteStateType* target = Enemy_Got_Player(&game->sprites[i], game->sprites);
            if (target) {
                (target->hit)(target);
                (game->sprites[i].hit)(&game->sprites[i]);
                setLed1On();
                Sound_Explosion();
            }
        }
    }
}

/**
 * Check if an active missile has reached the bottom of the screen
 * disable the laser and reload it in the enemy shutter gun
 */
void Update_Missed_Missiles(GameStateType* game) {
    unsigned long i;
    MissilesStateType* state = &game->missilesState;
    if (!state->missilesLeft) {
        for (i = 0; i < state->maxMissiles; i++) {
            if (!state->isLoaded[i]
                    && game->sprites[MISSILES_OFFSET + i].active
                    && game->sprites[MISSILES_OFFSET + i].visible
                    && game->sprites[MISSILES_OFFSET + i].y >= SCREENH) {
                state->isLoaded[i] = TRUE;
                state->missilesLeft += 1;
                game->sprites[MISSILES_OFFSET + i].active = FALSE;
                game->sprites[MISSILES_OFFSET + i].visible = FALSE;
            }
        }
    }
}

/**
 * Call update for all the active sprites
 */
void Update_Sprites(GameStateType* game) {
    int i;
    for (i = 0; i < MAX_SPRITES; i++) {
        if (game->sprites[i].active) {
            (game->sprites[i].updatePosition)(&game->sprites[i]);
            if (game->sprites[i].state == EXPLODING) {
                game->sprites[i].state = EXPLODED;
                game->sprites[i].visible = FALSE;
                game->sprites[i].active = FALSE;
                (*game->sprites[i].suspend)(&game->sprites[i]);
                Sound_Killed();
            }
        }
    }
}

void Start_Laser(GameStateType* game, int i, int x, int y) {
    LasersStateType* state = &game->lasersState;
    state->isLoaded[i] = FALSE;
    state->lasersLeft -= 1;
    game->sprites[LASERS_OFFSET + i].active = TRUE;
    game->sprites[LASERS_OFFSET + i].visible = TRUE;
    game->sprites[LASERS_OFFSET + i].x = x;
    game->sprites[LASERS_OFFSET + i].y = y;
    Sound_Shoot();

}

void Shot_Laser(GameStateType* game) {
    unsigned long i;
    LasersStateType* state = &game->lasersState;
    if (state->lasersLeft) {
        for (i = 0; i < state->maxLasers; i++) {
            if (state->isLoaded[i]) {
                Start_Laser(game, i,
                        game->sprites[PLAYER_OFFSET].x + (game->sprites[PLAYER_OFFSET].width / 2) - 1,
                        game->sprites[PLAYER_OFFSET].y - game->sprites[PLAYER_OFFSET].height);
                break;
            }
        }
    }
}

void Shot_Triple_Laser(GameStateType* game) {
    unsigned long i;
    unsigned long j = 0;
    LasersStateType* state = &game->lasersState;
    if (state->lasersLeft) {
        for (i = 0; i < state->maxLasers; i++) {
            if (state->isLoaded[i]) {
                Start_Laser(game, i,
                        game->sprites[PLAYER_OFFSET].x + (game->sprites[PLAYER_OFFSET].width / 2) - 4 + j * 4,
                        game->sprites[PLAYER_OFFSET].y - game->sprites[PLAYER_OFFSET].height);
                j++;
                if (j > 2) {
                    break;
                };
            }
        }
    }

}

void Shot_Missile_From_Enemy(GameStateType* game, SpriteStateType* enemy) {
    unsigned long i;
    MissilesStateType* state = &game->missilesState;
    if (state->missilesLeft) {
        for (i = 0; i < state->maxMissiles; i++) {
            if (state->isLoaded[i]) {
                SpriteStateType* missile;
                state->isLoaded[i] = FALSE;
                state->missilesLeft -= 1;
                missile = &game->sprites[MISSILES_OFFSET + i];
                missile->active = TRUE;
                missile->visible = TRUE;
                missile->x = enemy->x + (enemy->width / 2) - 1;
                missile->y = enemy->y + (enemy->height / 2) - 1;
                Sound_Shoot();
                break;
            }
        }
    }

}

void Shot_Random_Missile(GameStateType* game) {
    unsigned long randomColumn;
    int i;
    randomColumn = Random32() >> 21 & 0x0F;
    if (randomColumn < ENEMIES_PER_ROW) {
        //find first active enemy in column
        for (i = ENEMIES_OFFSET + MAX_ENEMIES - ENEMIES_PER_ROW; i >= ENEMIES_OFFSET; i = i - ENEMIES_PER_ROW) {
            if (game->sprites[i + randomColumn].active) {
                Shot_Missile_From_Enemy(game, &game->sprites[i + randomColumn]);
                break;
            }
        }
    }

}

void Launch_Bonus(GameStateType* game) {
    unsigned long randomBonus;
    if (!game->sprites[ENEMY_BONUS_OFFSET].active && Check_Upmost_Enemy(game->sprites) > ENEMYBONUSH * 2) {
        randomBonus = Random32() >> 21 & 0x0F; // 1 over 16 
        if (randomBonus == 1) {
            game->sprites[ENEMY_BONUS_OFFSET].x = 0;
            (*game->sprites[ENEMY_BONUS_OFFSET].reset)(&game->sprites[ENEMY_BONUS_OFFSET]);
            game->sprites[ENEMY_BONUS_OFFSET].active = 1;
            game->sprites[ENEMY_BONUS_OFFSET].state = ACTION;
            game->sprites[ENEMY_BONUS_OFFSET].visible = 1;
        }
    }
}

/**
 * Update main loop
 */
void Update(GameStateType* game) {
    setLed0Off();
    setLed1Off();
    Update_Enemies_Coreography(game->sprites);
    Update_Sprites(game);
    Update_Missed_Lasers(game);
    Update_Missed_Missiles(game);
    Check_Lasers_Collisions(game);
    Check_Missiles_Collisions(game);
    Check_Enemies_Collisions(game);
    Shot_Random_Missile(game);
    Launch_Bonus(game);
    if (isRightPressed()) {
        Shot_Laser(game);
    }
    if (isLeftPressed()) {
        Shot_Triple_Laser(game);
    }
}

/**
 * Draw main loop
 */
void Draw(GameStateType* game) {
    int i;
    Nokia5110_ClearBuffer();
    for (i = 0; i < MAX_SPRITES; i++) {
        if (game->sprites[i].visible) {
            (game->sprites[i].draw)(&game->sprites[i]);
        }
    }
    Nokia5110_DisplayBuffer(); // draw buffer
}

/**
 * Init hardware drivers
 */
void Init_Hardware(void) {
    TExaS_Init(SSI0_Real_Nokia5110_Scope); // set system clock to 80 MHz	
    Nokia5110_Init();
    ADC0_Init(SCREEN_WIDTH - PLAYERW); // initialize ADC0, channel 1, sequencer 3
    PE1_PE0_Switches_Init(); // Initialize PE0 & PE1 fire buttons
    PB5_PB4_Leds_Init(); // Initialize PB5 & PB4 leds
    Sound_Init();
    Random_Init(3);
    Clear_Switches();
}

/**
 * reset lasers state, all lasers in loaded state
 */
void Init_Lasers_State(LasersStateType* state, unsigned short maxLasers) {
    unsigned long i;
    state->maxLasers = maxLasers;
    for (i = 0; i < maxLasers; i++) {
        state->isLoaded[i] = TRUE;
    }
    state->lasersLeft = maxLasers;
}

/**
 * reset missiles state, all missiles in loaded state
 */
void Init_Missiles_State(MissilesStateType* state, unsigned short maxMissiles) {
    unsigned long i;
    state->maxMissiles = maxMissiles;
    for (i = 0; i < maxMissiles; i++) {
        state->isLoaded[i] = TRUE;
    }
    state->missilesLeft = maxMissiles;
}

void Init_Player(PlayerStateType* state) {
    state->lives = 3;
    state->maxLives = 3;
    state->score = 0;
}

void Init_Bunkers(BunkersStateType* state) {
    int i;
    state->destroyed = 0;
    state->maxBunkers = MAX_BUNKERS;
    for (i = 0; i < state->maxBunkers; i++) {
        state->bunkerState[i] = 0;
    }
}

void Init_Level(GameStateType* game) {
    Init_Player(&game->playerState);
    Init_Bunkers(&game->bunkersState);
    Init_Sprites(game);
    Init_Lasers_State(&game->lasersState, MAX_LASERS);
    Init_Missiles_State(&game->missilesState, MAX_MISSILES);
}

void Delay100ms(unsigned long count) {
    unsigned long volatile time;
    while (count > 0) {
        time = 727240; // 0.1sec at 80 MHz
        while (time) {
            time--;
        }
        count--;
    }
}

void Display_Start_Game(void) {
    char toggle = 1;
    int i = 0;
    while (!isRightPressed()) {
        Nokia5110_Clear();
        Nokia5110_SetCursor(0, 0);
        Nokia5110_OutString(" == SPACE ==");
        Nokia5110_SetCursor(i, 1);
        i = (i + 1) % 4;
        Nokia5110_OutString("INVADERS");
        Nokia5110_SetCursor(0, 3);
        if (toggle) {
            setLed0On();
            setLed1Off();
            Nokia5110_OutString("  <<FIRE>>");
        } else {
            setLed1On();
            setLed0Off();
            Nokia5110_OutString("          ");
        }
        toggle = !toggle;
        Nokia5110_SetCursor(0, 4);
        Nokia5110_OutString("  to start");
        Delay100ms(5);
    }
    setLed0Off();
    setLed1Off();
    Clear_Switches();
}

char No_More_Enemies(GameStateType* game) {
    return (game->enemiesState.enemiesCount <= 0);
}

char End_Of_Game(GameStateType* game) {
    return (game->playerState.lives <= 0);
}

char End_Of_Level(GameStateType* game) {
    return End_Of_Game(game) || No_More_Enemies(game);
}

void Display_Score(char* message, GameStateType* game) {
    Nokia5110_Clear();
    Nokia5110_SetCursor(1, 0);
    Nokia5110_OutString(message);
    Nokia5110_SetCursor(0, 2);
    Nokia5110_OutString("LEVEL: ");
    Nokia5110_SetCursor(6, 2);
    Nokia5110_OutUDec(game->level);
    Nokia5110_SetCursor(0, 3);
    Nokia5110_OutString("SCORE:");
    Nokia5110_SetCursor(6, 3);
    Nokia5110_OutUDec(game->score);
    Nokia5110_SetCursor(0, 4);
    Nokia5110_OutString("TOTAL:");
    Nokia5110_SetCursor(6, 4);
    Nokia5110_OutUDec(game->totalScore);
    Clear_Switches();
}

void Display_End_Game(GameStateType* game) {
    Display_Score("GAME OVER!", game);
    while (!isRightPressed());

}

void Display_Next_Level(GameStateType* game) {
    Display_Score("EXCELLENT!", game);
    Delay100ms(20);

}

void Reset_Score(GameStateType* game) {
    game->level = 1;
    game->totalScore = 0;
    game->score = 0;
}

void Set_Next_Level(GameStateType* game) {
    game->level++;
    game->score = 0;
}

int main(void) {
    Init_Hardware();
    EnableInterrupts();
    Display_Start_Game();
    Reset_Score(&game);
    while (1) {
        Clear_Switches();
        Init_Level(&game);
        Timer2_Init((9 - game.level)*2666666);
        //Timer2_Init(1333333/game.level);
        while (!End_Of_Level(&game)) {
            Draw(&game);
            Update(&game);
            while (!Semaphore);
            Semaphore = 0;
        }
        game.totalScore += game.score;
        if (End_Of_Game(&game)) {
            Display_End_Game(&game);
            Reset_Score(&game);
            Display_Start_Game();
        } else if (No_More_Enemies(&game)) {
            Display_Next_Level(&game);
            Set_Next_Level(&game);
        }
    }
}



