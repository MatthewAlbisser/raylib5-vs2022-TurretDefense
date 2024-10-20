#include <raylib.h>
#include "Math.h"

#include <cassert>
#include <array>
#include <vector>
#include <algorithm>

const float SCREEN_SIZE = 800;

const int TILE_COUNT = 20;
const float TILE_SIZE = SCREEN_SIZE / TILE_COUNT;

enum TileType : int
{
    GRASS,      // Marks unoccupied space, can be overwritten 
    DIRT,       // Marks the path, cannot be overwritten
    WAYPOINT,   // Marks where the path turns, cannot be overwritten
    TURRET,         // [HW3] New turret tiletype.
    COUNT
};

struct Cell
{
    int row;
    int col;
};

constexpr std::array<Cell, 4> DIRECTIONS{ Cell{ -1, 0 }, Cell{ 1, 0 }, Cell{ 0, -1 }, Cell{ 0, 1 } };

inline bool InBounds(Cell cell, int rows = TILE_COUNT, int cols = TILE_COUNT)
{
    return cell.col >= 0 && cell.col < cols && cell.row >= 0 && cell.row < rows;
}

void DrawTile(int row, int col, Color color)
{
    DrawRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE, color);
}

void DrawTile(int row, int col, int type)
{
    Color color = type > 0 ? BEIGE : GREEN;
    DrawTile(row, col, color);
}

Vector2 TileCenter(int row, int col)
{
    float x = col * TILE_SIZE + TILE_SIZE * 0.5f;
    float y = row * TILE_SIZE + TILE_SIZE * 0.5f;
    return { x, y };
}

Vector2 TileCorner(int row, int col)
{
    float x = col * TILE_SIZE;
    float y = row * TILE_SIZE;
    return { x, y };
}

// Returns a collection of adjacent cells that match the search value.
std::vector<Cell> FloodFill(Cell start, int tiles[TILE_COUNT][TILE_COUNT], TileType searchValue)
{
    // "open" = "places we want to search", "closed" = "places we've already searched".
    std::vector<Cell> result;
    std::vector<Cell> open;
    bool closed[TILE_COUNT][TILE_COUNT];
    for (int row = 0; row < TILE_COUNT; row++)
    {
        for (int col = 0; col < TILE_COUNT; col++)
        {
            // We don't want to search zero-tiles, so add them to closed!
            closed[row][col] = tiles[row][col] == 0;
        }
    }

    // Add the starting cell to the exploration queue & search till there's nothing left!
    open.push_back(start);
    while (!open.empty())
    {
        // Remove from queue and prevent revisiting
        Cell cell = open.back();
        open.pop_back();
        closed[cell.row][cell.col] = true;

        // Add to result if explored cell has the desired value
        if (tiles[cell.row][cell.col] == searchValue)
            result.push_back(cell);

        // Search neighbours
        for (Cell dir : DIRECTIONS)
        {
            Cell adj = { cell.row + dir.row, cell.col + dir.col };
            if (InBounds(adj) && !closed[adj.row][adj.col] && tiles[adj.row][adj.col] > 0)
                open.push_back(adj);
        }
    }

    return result;
}

struct Enemy        // [HW3]
{
    size_t curr = 0;
    size_t next = curr + 1;
    Vector2 position{};
    int health = 10;
    bool atEnd = false;
};

struct Turret       // [HW3]
{
    Vector2 position{};
    float range = 250.0f;
    float rateOfFire = 1.0f;
    int damage = 10;
    float currentCDT = 0.0f;
};

struct Bullet
{
    Vector2 position{};
    Vector2 direction{};
    float time = 0.0f;
    bool enabled = true;
};

int main()
{
    int tiles[TILE_COUNT][TILE_COUNT]
    {
        //col:0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19    row:
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0 }, // 0
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 1
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 2
            { 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 3
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 4
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 5
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 6
            { 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0 }, // 7
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 8
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 9
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0 }, // 10
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 11
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 12
            { 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0 }, // 13
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 14
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 15
            { 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 16
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0 }, // 17
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 18
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }  // 19
    };

    std::vector<Cell> waypoints = FloodFill({ 0, 12 }, tiles, WAYPOINT);
    std::vector<Enemy> enemies;     // [HW3]
    std::vector<Turret> turrets;    // [HW3]

    for (int row = 0; row < TILE_COUNT; ++row)      // [HW3] for each row.
    {
        for (int col = 0; col < TILE_COUNT; ++col)  // [HW3] and for each column.
        {
            if (tiles[row][col] == 3)   // [HW3] If the tile is set to 3.
            {
                Turret turret;
                turret.position = TileCenter(row, col);
                turrets.push_back(turret);
            }
        }
    }

    //Vector2 enemyPosition = TileCenter(waypoints[curr].row, waypoints[curr].col); // [HW3] Commented out
    const float enemySpeed = 250.0f;
    const float enemyRadius = 20.0f;

    const float bulletTime = 1.0f;
    const float bulletSpeed = 500.0f;
    const float bulletRadius = 15.0f;

    std::vector<Bullet> bullets;
    float shootCurrent = 0.0f;
    float shootTotal = 0.25f;

    float enemyCDT = 0.0f;      // [HW3]    Enemy spawn cooldown timer.
    float spawnStall = 1.0f;    // [HW3]    Time between enemy spawns.
    int enemySpawned = 0;       // [HW3]    Current number of enemies spawned.
    int enemyTotal = 10;        // [HW3]    Enemy count limit to stop enemy spawning.

    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Tower Defense");
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();  // [HW3]   

        Vector2 mouse = GetMousePosition();     // [A1]    Mouse varible set from GetMousePosition.
        Cell mouseCell;                         // [A1]    Struct variable set to mouseCell.
        mouseCell.col = mouse.x / TILE_SIZE;    // [A1]    Column equals X-axis pixel position divided by tilesize to set tile X-coord. 
        mouseCell.row = mouse.y / TILE_SIZE;    // [A1]    Row equals Y-axis pixel position divided by tilesize to set tile Y-coord. 
 
        enemyCDT += dt;             // [HW3]

        // [hw3] Enemy Spawning
        if (enemyCDT >= spawnStall && enemySpawned < enemyTotal)    // [HW3] If enemy cool down is greater or equal to spawn stall time (1 second) and total spawned enemies is less then or equal to total enemies (10 units)...
        {                                                           // [HW3] Create enemy, set position, set direction, set health, reset cool down, add 1 to total enemies, places enemy into vector.
            Enemy enemy;
            enemySpawned++;
            enemy.position = TileCenter(waypoints[enemy.curr].row, waypoints[enemy.curr].col);
            enemy.next = 1;
            enemy.health = 1;
            enemy.atEnd = false;
            enemies.push_back(enemy);
            enemyCDT = 0.0f;
        }

        // Path following
        for (Enemy& enemy : enemies)    // [HW3] Referencing the enemy struct with
        {
            if (!enemy.atEnd)
            {
                Vector2 from = TileCenter(waypoints[enemy.curr].row, waypoints[enemy.curr].col);
                Vector2 to = TileCenter(waypoints[enemy.next].row, waypoints[enemy.next].col);
                Vector2 direction = Normalize(to - from);
                enemy.position = enemy.position + direction * enemySpeed * dt;
                if (CheckCollisionPointCircle(enemy.position, to, enemyRadius))
                {
                    enemy.curr++;
                    enemy.next++;
                    enemy.atEnd = enemy.next == waypoints.size();
                    enemy.position = TileCenter(waypoints[enemy.curr].row, waypoints[enemy.curr].col);
                }
            }
        }

        for (Turret& turret : turrets)
        {
            turret.currentCDT += dt;
            Enemy* targets = nullptr;
            for (Enemy& enemy : enemies)
            {
                float distance = Distance(turret.position, enemy.position);
                if (distance < turret.range)
                {
                    targets = &enemy;
                }
            }
            if (targets && turret.currentCDT >= turret.rateOfFire)
            {
                turret.currentCDT = 0.0f;
                Bullet bullet;                                                      // [hw3] Creates bullet from bullet struct. Moved this chunk from if shooting statement bellow
                bullet.position = turret.position;                                  // [HW3] Changed to Turret position
                bullet.direction = Normalize(targets->position - bullet.position);
                bullets.push_back(bullet);
            }
        }

        // Bullet update
        for (Bullet& bullet : bullets)
        {
            bullet.position = bullet.position + bullet.direction * bulletSpeed * dt;    // aiming
            bullet.time += dt;                                                          // time simulation 

            bool expired = bullet.time >= bulletTime;                                   // remove bullet
            for (int i = 0; i < enemies.size();)
            {
                Enemy& enemy = enemies[i];
                bool collision = CheckCollisionCircles(enemy.position, enemyRadius, bullet.position, bulletRadius);
                if (collision)
                {
                    enemy.health--;
                    if (enemy.health <= 0)
                    {
                        enemies.erase(enemies.begin() + i);
                        bullet.enabled = false;
                        break;
                    }
                }
                else
                {
                    i++;
                }
            }
            bullet.enabled = !expired && bullet.enabled;
        }

        // Bullet removal
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
            [&bullets](Bullet bullet) {
                return !bullet.enabled;
            }), bullets.end());

        BeginDrawing();
        ClearBackground(BLACK);
        for (int row = 0; row < TILE_COUNT; row++)
        {
            for (int col = 0; col < TILE_COUNT; col++)
            {
                DrawTile(row, col, tiles[row][col]);
            }
        }

        // Render
        for (const Enemy& enemy : enemies)
            DrawCircleV(enemy.position, enemyRadius, RED);
        for (const Turret& turret : turrets)
            DrawCircleV(turret.position, enemyRadius, DARKPURPLE);
        for (const Bullet& bullet : bullets)
            DrawCircleV(bullet.position, bulletRadius, BLUE);
        DrawText(TextFormat("Total bullets: %i", bullets.size()), 10, 10, 20, BLUE);
        DrawTile(mouseCell.row, mouseCell.col, PURPLE);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
