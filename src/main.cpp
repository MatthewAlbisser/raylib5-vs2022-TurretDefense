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
    TURRET,         // [HW3] New tiletype named TURRET, will be called with 3.
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

struct Enemy        // [HW3] Struct for the enemies
{
    size_t curr = 0;
    size_t next = curr + 1;

    Vector2 position{};
    int health = 10;
    bool atEnd = false;
};

struct Turret       // [HW3] Struct for the turrets
{
    Vector2 position{};
    float range = 250.0f;
    float rateOfFire = 1.0f;
    int damage = 10;            // -!!- Applying damage to enemy was crashing the program.
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
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;     // [HW3] Creates vector to hold number of enemies.
    std::vector<Turret> turrets;    // [HW3] Creates vector to hold number of turrets.

    // -- ENEMY VARIABLES ------------
    const float enemySpeed = 250.0f;
    const float enemyRadius = 20.0f;

    float enemyCDT = 0.0f;      // [HW3]    Enemy spawn cooldown timer.
    float spawnStall = 1.0f;    // [HW3]    Time between enemy spawns.
    int enemySpawned = 0;       // [HW3]    Current count of enemies spawned.
    int enemyTotal = 10;        // [HW3]    Enemy count limit to enemy spawning.

    // -- BULLET VARIABLES ------------
    const float bulletTime = 1.0f;
    const float bulletSpeed = 500.0f;
    const float bulletRadius = 15.0f;

    // -- TURRET VARIABLES ------------
    float shootCurrent = 0.0f;
    float shootTotal = 0.25f;

    for (int row = 0; row < TILE_COUNT; ++row)      // [HW3] for each row...
    {
        for (int col = 0; col < TILE_COUNT; ++col)  // [HW3] and for each column...
        {
            if (tiles[row][col] == 3)               // [HW3] If the tile is equal to 3...
            {
                Turret turret;                              // [HW3] Apply struct data to turret variable.
                turret.position = TileCenter(row, col);     // [HW3] Place turret in the center of pre-determined tile position. 
                turrets.push_back(turret);                  // [HW3] Creates a space and adds turret value to the end of the vector.  
            }
        }
    }

    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Tower Defense");
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime(); // Delta time variable.

        Vector2 mouse = GetMousePosition();     // [A1]    Mouse varible set from GetMousePosition.
        Cell mouseCell;                         // [A1]    Struct variable set to mouseCell.
        mouseCell.col = mouse.x / TILE_SIZE;    // [A1]    Column equals X-axis pixel position divided by tilesize to set tile X-coord. 
        mouseCell.row = mouse.y / TILE_SIZE;    // [A1]    Row equals Y-axis pixel position divided by tilesize to set tile Y-coord. 

        // -- ENEMY SPAWNING ---------------------------------
        enemyCDT += dt;     // [HW3] Enemy spawn cool down

        if (enemyCDT >= spawnStall && enemySpawned < enemyTotal)    // [HW3] If enemy cool down is greater or equal to spawn stall time (1 second)
        {                                                           // and total spawned enemies is less then or equal to total enemies (10 units)...
            Enemy enemy;                                            // [HW3] Create enemy from struct,
            enemySpawned++;                                         // [HW3] Adds 1 to total enemy variable,
            enemy.position = TileCenter(waypoints[enemy.curr].row,  // [HW3] Set row position,
                waypoints[enemy.curr].col);                         // [HW3] Set column position,
            enemy.next = 1;                                         // [HW3] Sets next waypoint,
            enemies.push_back(enemy);                               // [HW3] Places enemy at end of vector,
            enemyCDT = 0.0f;                                        // [HW3] Reset cool down.
        }

        // -- ENEMY PATH FOLLOWING ---------------------------------
        for (Enemy& enemy : enemies)        // [HW3]  For each enemy in enemies vector...
        {
            if (!enemy.atEnd)               // [HW3] If an enemy has not reached the end...
            {
                Vector2 from = TileCenter(waypoints[enemy.curr].row, waypoints[enemy.curr].col);    // [HW3] Set variable for previous waypoint
                Vector2 to = TileCenter(waypoints[enemy.next].row, waypoints[enemy.next].col);      // [HW3] Set varibable for next waypoint
                Vector2 direction = Normalize(to - from);                                           // [HW3] Set variable for direction using both "To" and "From" variable vectors. 
                enemy.position = enemy.position + direction * enemySpeed * dt;                  // [HW3] Calculates enemy speed.
                if (CheckCollisionPointCircle(enemy.position, to, enemyRadius))                 // [HW3] If enemy circle is touching or overlapping with the next waypoint...
                {
                    enemy.curr++;                                       // [HW3] Add 1 to enemies current waypoint, making its previous destination the start.
                    enemy.next++;                                       // [HW3] 
                    enemy.atEnd = enemy.next == waypoints.size();
                    enemy.position = TileCenter(waypoints[enemy.curr].row, waypoints[enemy.curr].col);
                }
            }
        }
        // -- TURRET TARGETING --------------------------------------------
        for (Turret& turret : turrets)      // [HW3] For every turret in the vector spawned...
        {
            turret.currentCDT += dt;            // [HW3] Increase its current cool down float in real time. 
            Enemy* targets = nullptr;           // [HW3] Creates targets pointer for Enemies. points to null on start, preventing issues.
            for (Enemy& enemy : enemies)        // [HW3] For every enemy in the vector spawned...
            {
                float distance = Distance(turret.position, enemy.position);         // [HW3] Create variable for distance between a turret and an enemy.
                if (distance < turret.range)                                        // [HW3] If current distance is shorter then turrets max range...
                {
                    targets = &enemy;                                               // [HW3] Enemies become targeted. 
                }
            }
            if (targets && turret.currentCDT >= turret.rateOfFire)                  // [HW3] If target enemies exist AND turret cool down passes the rate of fire.
            {
                turret.currentCDT = 0.0f;                                           // [HW3] Reset turret cool down.
                Bullet bullet;                                                      // [HW3] Creates bullet from bullet struct. (Moved this chunk from existing code)
                bullet.position = turret.position;                                  // [HW3] Bullet position starts on the active turret position.
                bullet.direction = Normalize(targets->position - bullet.position);  // [HW3] Aims bullet at target using the target pointer direction and distance.
                bullets.push_back(bullet);                                          // [HW3] Place new bullet on the end of bullets vector.
            }
        }

        // -- BULLET MOVEMENT ---------------------------------------
        for (Bullet& bullet : bullets)                                                  
        {
            bullet.position = bullet.position + bullet.direction * bulletSpeed * dt;    
            bullet.time += dt;                                                          
            bool expired = bullet.time >= bulletTime;                                   

            for (int i = 0; i < enemies.size();)         // [HW3] For one enemy in the vector starting at 0.                                                                   
            {
                Enemy& enemy = enemies[i];                                  // [HW3] Creating enemies variable changes.
                bool collision = CheckCollisionCircles(enemy.position,      // [HW3] Check for collision for bullets and enemies.
                    enemyRadius, bullet.position, bulletRadius);            
                if (collision)                                              // [HW3] If collision is true... 
                {
                    enemy.health--;                                         // [HW3] Enemy health will drop by its own amount.
                    if (enemy.health <= 0)                                  // [HW3] When health equals zero ...
                    {
                        enemies.erase(enemies.begin() + i);                 // [HW3] Deletes enemy, and decreases vector size.
                        bullet.enabled = false;                             // [HW3] removes bullet.
                        break;                                              // [HW3] Stops the loop here.
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

        // -- RENDERING ---------------------------------

        BeginDrawing();
        ClearBackground(BLACK);
        for (int row = 0; row < TILE_COUNT; row++)
        {
            for (int col = 0; col < TILE_COUNT; col++)
            {
                DrawTile(row, col, tiles[row][col]);
            }
        }

        for (const Enemy& enemy : enemies)                          // [HW3] Draw enemies when spawned from vector.
            DrawCircleV(enemy.position, enemyRadius, RED);

        for (const Turret& turret : turrets)                        // [HW3] Draw turrets, not simple to change them to squares so they're staying as circles.
            DrawCircleV(turret.position, enemyRadius, DARKPURPLE);

        for (const Bullet& bullet : bullets)
            DrawCircleV(bullet.position, bulletRadius, BLUE);

        DrawText(TextFormat("Total bullets: %i", bullets.size()), 10, 10, 20, BLUE);

        DrawTile(mouseCell.row, mouseCell.col, SKYBLUE);            // [A1] Draw mouse position tile with sky blue colour.
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
