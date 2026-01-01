#pragma once
#include <vector>

enum class CellType {
    AIR,
    DIRT,
    STONE,
    TREASURE,
    WALL,
    MAX_CELL_TYPES
};

// Struct to represent a cell in the playfield
struct Cell {
    uint16_t id;
    Vector2 pos;
    CellType type;
    float health;
};

class Playfield {
private:
    std::vector<Cell> _cells;
    std::vector<std::vector<int>> _gridBuckets;
    int _gridW = 0;
    int _gridH = 0;
    float _bucketSize = 0.0f;
public:
    Playfield(float worldWidth, float worldHeight, float bucketSize);
    Playfield();
    bool AddCell(Vector2 worldPos, CellType cType);
    Cell* GetCellAtWorldPos(Vector2 pos);
    void DamageCell(Vector2 worldPos, float damage);
    bool IsInitialized();
};
