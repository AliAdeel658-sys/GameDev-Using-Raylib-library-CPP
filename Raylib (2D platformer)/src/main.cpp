// platformer.cpp
#include "raylib.h"
#include <array>
#include <cmath>
#include <algorithm>

//----------------------------------------------------------------------------------
// Some Defines -> converted to constexpr
//----------------------------------------------------------------------------------
// Tile collision types
constexpr int EMPTY = -1;
constexpr int BLOCK = 0;     // Start from zero, slopes can be added

// Defined map size
constexpr int TILE_MAP_WIDTH  = 20;
constexpr int TILE_MAP_HEIGHT = 12;

constexpr int MAX_COINS = 10;

// Tile size constants
constexpr int TILE_SIZE  = 16;
constexpr int TILE_ROUND = TILE_SIZE - 1;  // Used in bitwise operation | TILE_SIZE - 1

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
struct Input {
    float right = 0.0f;
    float left  = 0.0f;
    float up    = 0.0f;
    float down  = 0.0f;
    bool  jump  = false;
};

struct Entity {
    int width = 0;
    int height = 0;

    Vector2 position{0.0f, 0.0f};
    float direction = 0.0f;
    float maxSpd = 0.0f;
    float acc = 0.0f;
    float dcc = 0.0f;
    float gravity = 0.0f;
    float jumpImpulse = 0.0f;
    float jumpRelease = 0.0f;
    Vector2 velocity{0.0f, 0.0f};
    // Carry stored subpixel values
    float hsp = 0.0f;
    float vsp = 0.0f;

    bool isGrounded = false;
    bool isJumping = false;
    // Flags for detecting collision
    bool hitOnFloor = false;
    bool hitOnCeiling = false;
    bool hitOnWall = false;

    Input *control = nullptr;
};

struct Coin {
    Vector2 position{0.0f, 0.0f};
    bool visible = true;
};

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
float screenScale = 2.0f;
int screenWidth = 0;
static int screenHeight = 0;

static float deltaTimeFrame = 0.0f;
static bool win = false;
static int score = 0;

static std::array<int, TILE_MAP_WIDTH * TILE_MAP_HEIGHT> tiles{};
static Entity player{};
static Input inputInstance{};
static Camera2D camera{};

// Create coin instances
static std::array<Coin, MAX_COINS> coins = {{
    {{1*16+6, 7*16+6}, true},
    {{3*16+6, 5*16+6}, true},
    {{4*16+6, 5*16+6}, true},
    {{5*16+6, 5*16+6}, true},
    {{8*16+6, 3*16+6}, true},
    {{9*16+6, 3*16+6}, true},
    {{10*16+6, 3*16+6}, true},
    {{13*16+6, 4*16+6}, true},
    {{14*16+6, 4*16+6}, true},
    {{15*16+6, 4*16+6}, true},
}};

//------------------------------------------------------------------------------------
// Function declarations
//------------------------------------------------------------------------------------
static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void UpdateDrawFrame(void);

static void EntityMoveUpdate(Entity *instance);
static void GetDirection(Entity *instance);
static void GroundCheck(Entity *instance);
static void MoveCalc(Entity *instance);
static void GravityCalc(Entity *instance);
static void CollisionCheck(Entity *instance);
static void CollisionHorizontalBlocks(Entity *instance);
static void CollisionVerticalBlocks(Entity *instance);

static int MapGetTileWorld(int x, int y);
static int TileHeight(int x, int y, int tile);

static void MapInit(void);
static void MapDraw(void);
static void PlayerInit(void);
static void InputUpdate(void);
static void PlayerUpdate(void);
static void PlayerDraw(void);
static void CoinInit(void);
static void CoinUpdate(void);
static void CoinDraw(void);

// Utility helpers
int ttc_sign(float x);
float ttc_abs(float x);
float ttc_clamp(float value, float min, float max);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    screenScale = 2.0f;
    screenWidth = TILE_SIZE * TILE_MAP_WIDTH * static_cast<int>(screenScale);
    screenHeight = TILE_SIZE * TILE_MAP_HEIGHT * static_cast<int>(screenScale);

    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "classic game: platformer");

    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    UnloadGame();
    CloseWindow();

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definitions
//------------------------------------------------------------------------------------
void InitGame(void)
{
    win = false;
    score = 0;

    camera.offset = (Vector2){0.0f, 0.0f};
    camera.target = (Vector2){0.0f, 0.0f};
    camera.rotation = 0.0f;
    camera.zoom = screenScale;

    MapInit();
    PlayerInit();
    CoinInit();
}

void UpdateGame(void)
{
    deltaTimeFrame = GetFrameTime();

    PlayerUpdate();
    CoinUpdate();

    if (win)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
        }
    }
}

void DrawGame(void)
{
    BeginDrawing();

        BeginMode2D(camera);
            ClearBackground(RAYWHITE);

            MapDraw();
            CoinDraw();
            PlayerDraw();

        EndMode2D();

        DrawText(TextFormat("SCORE: %i", score), GetScreenWidth()/2 - MeasureText(TextFormat("SCORE: %i", score), 40)/2, 50, 40, BLACK);

        if (win) DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);

    EndDrawing();
}

void UnloadGame(void)
{
    // No external assets to unload in this sample
}

void MapInit(void)
{
    for (int y = 0; y < TILE_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < TILE_MAP_WIDTH; x++)
        {
            if (y == 0 || x == 0 || y == TILE_MAP_HEIGHT-1 || x == TILE_MAP_WIDTH-1)
            {
                tiles[x + y * TILE_MAP_WIDTH] = BLOCK;
            }
            else
            {
                tiles[x + y * TILE_MAP_WIDTH] = EMPTY;
            }
        }
    }

    // Manual platforms
    tiles[3 + 8*TILE_MAP_WIDTH]  = BLOCK;
    tiles[4 + 8*TILE_MAP_WIDTH]  = BLOCK;
    tiles[5 + 8*TILE_MAP_WIDTH]  = BLOCK;

    tiles[8 + 6*TILE_MAP_WIDTH]  = BLOCK;
    tiles[9 + 6*TILE_MAP_WIDTH]  = BLOCK;
    tiles[10 + 6*TILE_MAP_WIDTH] = BLOCK;

    tiles[13 + 7*TILE_MAP_WIDTH] = BLOCK;
    tiles[14 + 7*TILE_MAP_WIDTH] = BLOCK;
    tiles[15 + 7*TILE_MAP_WIDTH] = BLOCK;

    tiles[1 + 10*TILE_MAP_WIDTH] = BLOCK;
}

void MapDraw(void)
{
    for (int y = 0; y < TILE_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < TILE_MAP_WIDTH; x++)
        {
            if (tiles[x + y * TILE_MAP_WIDTH] > EMPTY)
            {
                DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, GRAY);
            }
        }
    }
}

int MapGetTileWorld(int x, int y)
{
    if (x < 0 || y < 0) return EMPTY;

    x /= TILE_SIZE;
    y /= TILE_SIZE;

    if (x > TILE_MAP_WIDTH || y > TILE_MAP_HEIGHT) return EMPTY;

    return tiles[x + y * TILE_MAP_WIDTH];
}

int TileHeight(int x, int y, int tile)
{
    switch(tile)
    {
        case EMPTY: break;
        case BLOCK: y = (y & ~TILE_ROUND) - 1; break;
    }

    return y;
}

void InputUpdate(void)
{
    inputInstance.right = static_cast<float>(IsKeyDown('D') || IsKeyDown(KEY_RIGHT));
    inputInstance.left  = static_cast<float>(IsKeyDown('A') || IsKeyDown(KEY_LEFT));
    inputInstance.up    = static_cast<float>(IsKeyDown('W') || IsKeyDown(KEY_UP));
    inputInstance.down  = static_cast<float>(IsKeyDown('S') || IsKeyDown(KEY_DOWN));

    if (IsKeyPressed(KEY_SPACE)) inputInstance.jump = true;
    else if (IsKeyReleased(KEY_SPACE)) inputInstance.jump = false;
}

void PlayerInit(void)
{
    player.position.x = (float)(TILE_SIZE * TILE_MAP_WIDTH) * 0.5f;
    player.position.y = TILE_MAP_HEIGHT * TILE_SIZE - 16.0f - 1;
    player.direction = 1.0f;

    player.maxSpd = 1.5625f * 60;
    player.acc = 0.118164f * 60 * 60;
    player.dcc = 0.113281f * 60 * 60;
    player.gravity = 0.363281f * 60 * 60;
    player.jumpImpulse = -6.5625f * 60;
    player.jumpRelease = player.jumpImpulse * 0.2f;
    player.velocity = {0.0f, 0.0f};
    player.hsp = 0.0f;
    player.vsp = 0.0f;

    player.width = 8;
    player.height = 16;

    player.isGrounded = false;
    player.isJumping = false;

    player.control = &inputInstance;
}

void PlayerDraw(void)
{
    DrawRectangle((int)(player.position.x - player.width * 0.5f), (int)(player.position.y - player.height + 1), player.width, player.height, RED);
}

void PlayerUpdate(void)
{
    InputUpdate();
    EntityMoveUpdate(&player);
}

void CoinInit(void)
{
    for (int i = 0; i < MAX_COINS; i++) coins[i].visible = true;
}

void CoinDraw(void)
{
    for (int i = 0; i < MAX_COINS; i++)
    {
        if (coins[i].visible)
        {
            DrawRectangle((int)coins[i].position.x, (int)coins[i].position.y, 4, 4, GOLD);
        }
    }
}

void CoinUpdate(void)
{
    Rectangle playerRect = { player.position.x - player.width*0.5f, player.position.y - player.height + 1, (float)player.width, (float)player.height };

    for (int i = 0; i < MAX_COINS; i++)
    {
        if (coins[i].visible)
        {
            Rectangle coinRect = { coins[i].position.x, coins[i].position.y, 4.0f, 4.0f };
            if (CheckCollisionRecs(playerRect, coinRect))
            {
                coins[i].visible = false;
                score += 1;
            }
        }
    }

    win = (score == MAX_COINS);
}

//------------------------------------------------
// Physics functions
//------------------------------------------------
void EntityMoveUpdate(Entity *instance)
{
    GroundCheck(instance);
    GetDirection(instance);
    MoveCalc(instance);
    GravityCalc(instance);
    CollisionCheck(instance);

    // Horizontal velocity together including last frame sub-pixel value
    float xVel = instance->velocity.x * deltaTimeFrame + instance->hsp;
    int xsp = (int)ttc_abs(xVel) * ttc_sign(xVel);
    instance->hsp = instance->velocity.x * deltaTimeFrame - xsp;

    // Vertical velocity together including last frame sub-pixel value
    float yVel = instance->velocity.y * deltaTimeFrame + instance->vsp;
    int ysp = (int)ttc_abs(yVel) * ttc_sign(yVel);
    instance->vsp = instance->velocity.y * deltaTimeFrame - ysp;

    instance->position.x += xsp;
    instance->position.y += ysp;

    instance->position.x = ttc_clamp(instance->position.x, 0.0f, TILE_MAP_WIDTH * (float)TILE_SIZE);
    instance->position.y = ttc_clamp(instance->position.y, 0.0f, TILE_MAP_HEIGHT * (float)TILE_SIZE);
}

void GetDirection(Entity *instance)
{
    instance->direction = (instance->control->right - instance->control->left);
}

void GroundCheck(Entity *instance)
{
    int x = (int)instance->position.x;
    int y = (int)instance->position.y + 1;
    instance->isGrounded = false;

    int c = MapGetTileWorld(x, y);

    if (c != EMPTY)
    {
        int h = TileHeight(x, y, c);
        instance->isGrounded = (y >= h);
    }

    if (!instance->isGrounded)
    {
        int xl = (x - instance->width / 2);
        int l = MapGetTileWorld(xl, y);

        if (l != EMPTY)
        {
            int h = TileHeight(xl, y, l);
            instance->isGrounded = (y >= h);
        }

        if (!instance->isGrounded)
        {
            int xr = (x + instance->width / 2 - 1);
            int r = MapGetTileWorld(xr, y);
            if (r != EMPTY)
            {
                int h = TileHeight(xr, y, r);
                instance->isGrounded = (y >= h);
            }
        }
    }
}

void MoveCalc(Entity *instance)
{
    float deadZone = 0.0f;
    if (ttc_abs(instance->direction) > deadZone)
    {
        instance->velocity.x += instance->direction * instance->acc * deltaTimeFrame;
        instance->velocity.x = ttc_clamp(instance->velocity.x, -instance->maxSpd, instance->maxSpd);
    }
    else
    {
        float xsp = instance->velocity.x;
        if (ttc_abs(0 - xsp) < instance->dcc * deltaTimeFrame) instance->velocity.x = 0;
        else if (xsp > 0) instance->velocity.x -= instance->dcc * deltaTimeFrame;
        else instance->velocity.x += instance->dcc * deltaTimeFrame;
    }
}

void Jump(Entity *instance)
{
    instance->velocity.y = instance->jumpImpulse;
    instance->isJumping = true;
    instance->isGrounded = false;
}

void GravityCalc(Entity *instance)
{
    if (instance->isGrounded)
    {
        if (instance->isJumping)
        {
            instance->isJumping = false;
            instance->control->jump = false;
        }
        else if (!instance->isJumping && instance->control->jump)
        {
            Jump(instance);
        }
    }
    else
    {
        if (instance->isJumping)
        {
            if (!instance->control->jump)
            {
                instance->isJumping = false;

                if (instance->velocity.y < instance->jumpRelease)
                {
                    instance->velocity.y = instance->jumpRelease;
                }
            }
        }
    }

    instance->velocity.y += instance->gravity * deltaTimeFrame;

    if (instance->velocity.y > -instance->jumpImpulse)
    {
        instance->velocity.y = -instance->jumpImpulse;
    }
}

void CollisionCheck(Entity *instance)
{
    CollisionHorizontalBlocks(instance);
    CollisionVerticalBlocks(instance);
}

void CollisionHorizontalBlocks(Entity *instance)
{
    float xVel = instance->velocity.x * deltaTimeFrame + instance->hsp;
    int xsp = (int)ttc_abs(xVel) * ttc_sign(xVel);

    instance->hitOnWall = false;

    int side;
    if (xsp > 0) side = instance->width / 2 - 1;
    else if (xsp < 0) side = -instance->width / 2;
    else return;

    int x = (int)instance->position.x;
    int y = (int)instance->position.y;
    int mid = -instance->height / 2;
    int top = -instance->height + 1;

    int b = MapGetTileWorld(x + side + xsp, y) > EMPTY;
    int m = MapGetTileWorld(x + side + xsp, y + mid) > EMPTY;
    int t = MapGetTileWorld(x + side + xsp, y + top) > EMPTY;

    if (b || m || t)
    {
        if (xsp > 0) x = ((x + side + xsp) & ~TILE_ROUND) - 1 - side;
        else x = ((x + side + xsp) & ~TILE_ROUND) + TILE_SIZE - side;

        instance->position.x = (float)x;
        instance->velocity.x = 0.0f;
        instance->hsp = 0.0f;

        instance->hitOnWall = true;
    }
}

void CollisionVerticalBlocks(Entity *instance)
{
    float yVel = instance->velocity.y * deltaTimeFrame + instance->vsp;
    int ysp = (int)ttc_abs(yVel) * ttc_sign(yVel);
    instance->hitOnCeiling = false;
    instance->hitOnFloor = false;

    int side = 0;
    if (ysp > 0) side = 0;
    else if (ysp < 0) side = -instance->height + 1;
    else return;

    int x = (int)instance->position.x;
    int y = (int)instance->position.y;
    int xl = -instance->width / 2;
    int xr = instance->width / 2 - 1;

    int c = MapGetTileWorld(x, y + side + ysp) > EMPTY;
    int l = MapGetTileWorld(x + xl, y + side + ysp) > EMPTY;
    int r = MapGetTileWorld(x + xr, y + side + ysp) > EMPTY;

    if (c || l || r)
    {
        if (ysp > 0)
        {
            y = ((y + side + ysp) & ~TILE_ROUND) - 1 - side;
            instance->hitOnFloor = true;
        }
        else
        {
            y = ((y + side + ysp) & ~TILE_ROUND) + TILE_SIZE - side;
            instance->hitOnCeiling = true;
        }

        instance->position.y = (float)y;
        instance->velocity.y = 0.0f;
        instance->vsp = 0.0f;
    }
}

// Return sign of the float as int (-1, 0, 1)
int ttc_sign(float x)
{
    if (x < 0.0f) return -1;
    else if (x < 0.0001f) return 0;
    else return 1;
}

// Return absolute value of float
float ttc_abs(float x)
{
    return (x < 0.0f) ? -x : x;
}

// Clamp value between min and max
float ttc_clamp(float value, float min, float max)
{
    const float res = value < min ? min : value;
    return res > max ? max : res;
}

// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}
