#pragma once
#include "common/pabStructs.h"
#include "server/playfield.h"
#include <vector>

class Playfield {
private:
    std::vector<Cell> _cells;
    std::vector<std::vector<int>> _gridBuckets;
    int _gridW = 0;
    int _gridH = 0;
    float _bucketSize;
public:
    Playfield(float worldWidth, float worldHeight, float bucketSize);
    bool AddCell(Vector2 worldPos, CellType cType);
    Cell* GetCellAtWorldPos(Vector2 pos);
    void DamageCell(Vector2 worldPos, float damage);
    bool IsInitialized();
};
