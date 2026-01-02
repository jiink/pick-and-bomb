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

struct LevelData {
    float width, height, bucketSize;
    struct Point { float x, y; uint8_t type; };
    std::vector<Point> points;
};
const uint8_t LEVEL_FORMAT_VER = 1;

// Struct to represent a cell in the playfield
struct Cell {
    uint16_t id;
    Vector2 pos;
    CellType type;
    float health;
    bool isDirty;
};

struct VoronoiInfo {
    Cell* cell;      // The closest cell (can be nullptr)
    Cell* cell2;      // The second closest cell (can be nullptr)
    float dist1Sqr;  // Squared distance to closest
    float dist2Sqr;  // Squared distance to 2nd closest
};

// TODO look into using PhysFS to publish the game as a single exe

class Playfield {
private:
    std::vector<Cell> _cells;
    std::vector<std::vector<int>> _gridBuckets;
    int _gridW = 0;
    int _gridH = 0;
    std::vector<int> _dirtyCellIndices;
public:
    float _worldWidth = 0.0f;
    float _worldHeight = 0.0f;
    float _bucketSize = 0.0f;
    Playfield(float worldWidth, float worldHeight, float bucketSize);
    Playfield();
    bool AddCell(Vector2 worldPos, CellType cType);
    Cell* GetCellAtWorldPos(Vector2 pos, float* outDistSqr = nullptr);
    void DamageCell(Vector2 worldPos, float damage);
    bool IsInitialized() const;
    const std::vector<Cell>& GetAllCells() const { return _cells; }
    void Clear();
    void Reset(float worldWidth, float worldHeight, float bucketSize);
    bool AddRawCell(Cell& cell);
    void PopulateFromLevelData(LevelData lvlDat);
    std::vector<Cell*> GetAndCleanDirtyCells();
    bool UpdateCell(uint16_t idx, float hp);
    int GetNumCells();
    VoronoiInfo GetVoronoiInfo(Vector2 pos);
};
