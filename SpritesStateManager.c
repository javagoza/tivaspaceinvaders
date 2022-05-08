typedef struct Animation AnimationType;

struct Animation {
    unsigned short maxImages; // number of images for the sprite
    unsigned short xMargin; // inbound margin of the sprite
    unsigned short yMargin;
    const unsigned char **images; // images of the sprite
    unsigned short nextImage; // next image to be drawed
    unsigned long framesCounter;
    unsigned long framesPerImage;
};


enum SpriteStates {
    ACTION, HIT, EXPLODING, EXPLODED
};

typedef struct MissilesState MissilesStateType;

struct MissilesState {
    unsigned char isLoaded[MAX_MISSILES];
    short maxMissiles;
    short missilesLeft;
};

typedef struct LasersState LasersStateType;

struct LasersState {
    unsigned char isLoaded[MAX_LASERS];
    unsigned short maxLasers;
    unsigned short lasersLeft;
};

typedef struct PlayerState PlayerStateType;

struct PlayerState {
    unsigned short maxLives;
    unsigned short lives;
    unsigned long score;
};

typedef struct BunkersState BunkersStateType;

struct BunkersState {
    unsigned short maxBunkers;
    unsigned short destroyed;
    unsigned char bunkerState[MAX_BUNKERS]; // from 0 to 3
};

typedef struct BonusState BonusStateType;

struct BonusState {
    unsigned short maxBonuses;
    unsigned short used;
};

typedef struct EnemiesState EnemiesStateType;

struct EnemiesState {
    unsigned long enemiesCount;
};

typedef struct GameState GameStateType;
typedef struct SpriteState SpriteStateType;

struct SpriteState {
    enum SpriteStates state;
    double x; // x coordinate
    double y; // y coordinate
    unsigned short xMargin; // x margin protection thick in pixels
    unsigned short yMargin; // y margin protection thick in pixels
    double velocityX; // velocity and direction in frames per second
    double velocityY; // velocity and direction in frames per second
    unsigned short width;
    unsigned short height;
    unsigned char active; // 0=not updatable, 1=updatable
    unsigned char visible; // 0=not visible, 1=visible
    void (*updatePosition)(SpriteStateType *self);
    void (*draw)(SpriteStateType *self);
    void (*hit)(SpriteStateType *self);
    void (*suspend)(SpriteStateType *self);
    void (*reset)(SpriteStateType *self);
    unsigned char (*intersect)(SpriteStateType *self, SpriteStateType *target);
    AnimationType animation;
    AnimationType hitAnimation;
    AnimationType *actualAnimationPtr;
    GameStateType *game;
    unsigned long rank;
};

struct GameState {
    SpriteStateType sprites[MAX_SPRITES];
    BonusStateType bonusState;
    MissilesStateType missilesState;
    LasersStateType lasersState;
    PlayerStateType playerState;
    BunkersStateType bunkersState;
    EnemiesStateType enemiesState;
    unsigned long score;
    unsigned long level;
    unsigned long totalScore;
};


unsigned char Intersect_Rectangles(long ax1, long ax2, long ay1, long ay2, long bx1, long bx2, long by1, long by2) {
    return bx2 >= ax1
            && ax2 >= bx1
            && by2 >= ay1
            && ay2 >= by1;
}

/**
 * compare sprites bounding boxes
 * Bounding boxes are 2 pixeles thickless
 */
unsigned char Intersect(SpriteStateType *self, SpriteStateType *target) {
    return self->active
            && self->visible
            && target->active
            && target->visible
            && Intersect_Rectangles(self->x + self->xMargin, self->x + self->width - self->xMargin,
            self->y + self->yMargin, self->y + self->height - self->yMargin,
            target->x + target->xMargin, target->x + target->width - target->xMargin,
            target->y + target->yMargin, target->y + target->height - target->yMargin);
}