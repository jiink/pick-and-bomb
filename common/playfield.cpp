#include "common/pabStructs.h"
#include "common/playfield.h"
#include <vector>
#include "playfield.h"

// worldWidth: how many world units wide (e.g. meters) the playfield is
// worldHeight: how many world units tall the playfield is
// bucketSize: how many world units wide and tall each optimization grid cell is
Playfield::Playfield(float worldWidth, float worldHeight, float bucketSize) {
    Reset(worldWidth, worldHeight, bucketSize);
}

Playfield::Playfield() {
    
}

bool Playfield::AddCell(Vector2 worldPos, CellType cType)
{
    Cell cell{};
    cell.id = _cells.size();
    cell.pos = worldPos;
    cell.type = cType;
    return AddRawCell(cell);
}

bool Playfield::AddRawCell(Cell &cell)
{
    cell.health = 100.0f;
    _cells.push_back(cell);
    int bX = (int)(cell.pos.x / _bucketSize);
    int bY = (int)(cell.pos.y / _bucketSize);
    if (bX >= 0 && bX < _gridW &&
        bY >= 0 && bY < _gridH)
    {
        _gridBuckets[bY * _gridW + bX].push_back(cell.id);
        return true;
    } else {
        return false;
    }
}

void Playfield::PopulateFromLevelData(LevelData lvlDat)
{
    Reset(lvlDat.width, lvlDat.height, lvlDat.bucketSize);
    for (LevelData::Point& p : lvlDat.points) {
        CellType cType = CellType::AIR;
        if (p.type < (uint8_t)CellType::MAX_CELL_TYPES) {
            cType = (CellType)p.type;
        }
        AddCell(Vector2{p.x, p.y}, cType);
    }
}

std::vector<Cell *> Playfield::GetAndCleanDirtyCells()
{
    std::vector<Cell*> dirtyCells;
    dirtyCells.reserve(_dirtyCellIndices.size());
    for (int idx : _dirtyCellIndices) {
        Cell& c = _cells[idx];
        c.isDirty = false;
        dirtyCells.push_back(&c);
    }
    _dirtyCellIndices.clear();
    return dirtyCells;
}

bool Playfield::UpdateCell(uint16_t idx, float hp)
{
    if (idx >= _cells.size()) {
        return false;
    }
    _cells[idx].health = hp;
    if (_cells[idx].health <= 0.0f) {
        _cells[idx].type = CellType::AIR;
    }
    return true;
}

int Playfield::GetNumCells()
{
    return _cells.size();
}

Cell* Playfield::GetCellAtWorldPos(Vector2 pos)
{
    int bX = (int)(pos.x / _bucketSize);
    int bY = (int)(pos.y / _bucketSize);
    float minDist = 99999;
    uint16_t closestIdx = 0;
    bool foundClosestIdx = false;
    for (int y = bY - 1; y <= bY + 1; y++) {
        for (int x = bX - 1; x <= bX + 1; x++) {
            bool inBounds = x >= 0 && x < _gridW &&
                y >= 0 && y < _gridH;
            if (!inBounds) {
                continue;
            }
            const std::vector<int>& bucketIndices = _gridBuckets[y * _gridW + x];
            for (int idx : bucketIndices) {
                const Cell& c = _cells[idx];
                float dist = Vector2DistanceSqr(pos, c.pos);
                if (dist < minDist) {
                    minDist = dist;
                    closestIdx = idx;
                    foundClosestIdx = true;
                }
            }
        }
    }
    if (foundClosestIdx) {
        return &_cells[closestIdx];
    } else {
        return nullptr;
    }
}

void Playfield::DamageCell(Vector2 worldPos, float damage)
{
    Cell* cell = GetCellAtWorldPos(worldPos);
    if (cell && cell->type != CellType::AIR) {
        cell->health -= damage;
        if (cell->health <= 0) {
            cell->type = CellType::AIR;
        }
        if (!cell->isDirty) {
            cell->isDirty = true;
            _dirtyCellIndices.push_back(cell->id);
        }
    }
}

bool Playfield::IsInitialized() const
{
    return _gridW > 0 && 
        _gridH > 0 && 
        _cells.size() > 0 && 
        _gridBuckets.size() > 0;
}

void Playfield::Clear()
{
    _cells.clear();
    for (auto& bucket : _gridBuckets) {
        bucket.clear();
    }
    _dirtyCellIndices.clear();
}

void Playfield::Reset(float worldWidth, float worldHeight, float bucketSize) {
    Clear();
    _worldWidth = worldWidth;
    _worldHeight = worldHeight;
    _bucketSize = bucketSize;
    _gridW = std::ceilf(worldWidth / _bucketSize);
    _gridH = std::ceilf(worldHeight / _bucketSize);
    _gridBuckets.resize(_gridW * _gridH);    
}
