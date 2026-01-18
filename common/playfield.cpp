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

Playfield::Playfield() {}

bool Playfield::AddCell(Vector2 worldPos, CellType cType) {
  Cell cell{};
  cell.id = _cells.size();
  cell.pos = worldPos;
  cell.type = cType;
  return AddRawCell(cell);
}

bool Playfield::AddRawCell(Cell& cell) {
  cell.health = 100.0f;
  _cells.push_back(cell);
  int bX = (int)(cell.pos.x / _bucketSize);
  int bY = (int)(cell.pos.y / _bucketSize);
  if (bX >= 0 && bX < _gridW && bY >= 0 && bY < _gridH) {
    _gridBuckets[bY * _gridW + bX].push_back(cell.id);
    return true;
  } else {
    return false;
  }
}

void Playfield::PopulateFromLevelData(LevelData lvlDat) {
  Reset(lvlDat.width, lvlDat.height, lvlDat.bucketSize);
  for (LevelData::Point& p : lvlDat.points) {
    CellType cType = CellType::AIR;
    if (p.type < (uint8_t)CellType::MAX_CELL_TYPES) {
      cType = (CellType)p.type;
    }
    AddCell(Vector2{p.x, p.y}, cType);
  }
  GenerateApproxMap(4);
}

std::vector<Cell*> Playfield::GetAndCleanDirtyCells() {
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

bool Playfield::UpdateCell(uint16_t idx, float hp) {
  if (idx >= _cells.size()) {
    return false;
  }
  _cells[idx].health = hp;
  if (_cells[idx].health <= 0.0f) {
    _cells[idx].type = CellType::AIR;
  }
  return true;
}

int Playfield::GetNumCells() { return _cells.size(); }

Cell* Playfield::GetCell(Vector2 pos, float* outDistSqr) {
  int bX = (int)(pos.x / _bucketSize);
  int bY = (int)(pos.y / _bucketSize);
  float minDistSqr = std::numeric_limits<float>::max();
  uint16_t closestIdx = 0;
  bool foundClosestIdx = false;
  for (int y = bY - 1; y <= bY + 1; y++) {
    for (int x = bX - 1; x <= bX + 1; x++) {
      bool inBounds = x >= 0 && x < _gridW && y >= 0 && y < _gridH;
      if (!inBounds) {
        continue;
      }
      const std::vector<int>& bucketIndices = _gridBuckets[y * _gridW + x];
      for (int idx : bucketIndices) {
        const Cell& c = _cells[idx];
        float dist = Vector2DistanceSqr(pos, c.pos);
        if (dist < minDistSqr) {
          minDistSqr = dist;
          closestIdx = idx;
          foundClosestIdx = true;
        }
      }
    }
  }
  if (foundClosestIdx) {
    if (outDistSqr) {
      *outDistSqr = minDistSqr;
    }
    return &_cells[closestIdx];
  } else {
    return nullptr;
  }
}

Cell* Playfield::GetCellApprox(Vector2 pos) {
  if (_approxMap.empty() || _approxMapWidth == 0) {
    return nullptr;
  }
  int pixPerWorldUnit = _approxMapWidth / (int)std::floorf(_worldWidth);
  int x = (int)(pos.x * pixPerWorldUnit);
  int y = (int)(pos.y * pixPerWorldUnit);
  int mapH = _approxMap.size() / _approxMapWidth;
  if (x < 0 || x >= _approxMapWidth || y < 0 || y >= mapH) {
    return nullptr;
  }
  uint16_t cellId = _approxMap[y * _approxMapWidth + x];
  return (cellId < _cells.size()) ? &_cells[cellId] : nullptr;
}

void Playfield::GenerateApproxMap(int pixPerWorldUnit) {
  int mapW = std::floorf(_worldWidth) * pixPerWorldUnit;
  int mapH = std::floorf(_worldHeight) * pixPerWorldUnit;
  _approxMapWidth = mapW;
  _approxMap.clear();
  _approxMap.resize(mapW * mapH);

  for (int y = 0; y < mapH; y++) {
    for (int x = 0; x < mapW; x++) {
      Vector2 samplePos = Vector2{(x + 0.5f) / (float)pixPerWorldUnit,
                                  (y + 0.5f) / (float)pixPerWorldUnit};
      Cell* cell = GetCell(samplePos);
      int index = y * mapW + x;
      _approxMap[index] = (cell) ? cell->id : 0;
    }
  }
}

void Playfield::DamageCell(Vector2 worldPos, float damage) {
  Cell* cell = GetCell(worldPos);
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

bool Playfield::IsInitialized() const {
  return _gridW > 0 && _gridH > 0 && _cells.size() > 0 &&
         _gridBuckets.size() > 0;
}

void Playfield::Clear() {
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

VoronoiInfo Playfield::GetVoronoiInfo(Vector2 pos) {
  VoronoiInfo result;
  result.cell = nullptr;
  result.cell2 = nullptr;
  // Initialize with max float values
  result.dist1Sqr = std::numeric_limits<float>::max();
  result.dist2Sqr = std::numeric_limits<float>::max();

  int bX = (int)(pos.x / _bucketSize);
  int bY = (int)(pos.y / _bucketSize);

  // Iterate 3x3 buckets around the position
  for (int y = bY - 1; y <= bY + 1; y++) {
    for (int x = bX - 1; x <= bX + 1; x++) {
      // Check bounds
      if (x < 0 || x >= _gridW || y < 0 || y >= _gridH) {
        continue;
      }

      const std::vector<int>& bucketIndices = _gridBuckets[y * _gridW + x];

      // Check every cell in this bucket
      for (int idx : bucketIndices) {
        const Cell& c = _cells[idx];
        float dSqr = Vector2DistanceSqr(pos, c.pos);

        // Insertion logic for top 2
        if (dSqr < result.dist1Sqr) {
          // Push current #1 down to #2
          result.dist2Sqr = result.dist1Sqr;
          result.cell2 = result.cell;

          // Set new #1
          result.dist1Sqr = dSqr;
          result.cell =
              (Cell*)&c; // Cast away const if needed or change return type
        } else if (dSqr < result.dist2Sqr) {
          // It's not closer than #1, but it is closer than #2
          result.dist2Sqr = dSqr;
          result.cell2 = (Cell*)&c;
        }
      }
    }
  }

  return result;
}

Vector2 Playfield::GetBoundaryNormal(Vector2 pos) {
  VoronoiInfo info = GetVoronoiInfo(pos);
  if (!info.cell || !info.cell2) {
    return {0, 0};
  }
  Vector2 p1 = info.cell->pos;
  Vector2 p2 = info.cell2->pos;
  Vector2 norm = Vector2Subtract(p2, p1);
  return Vector2Normalize(norm);
}

void Playfield::Explode(Vector2 center, float radius, float damage) {
  if (_approxMapWidth == 0 || _worldWidth <= 0 || _approxMap.empty()) {
    return;
  }
  int pixPerWorldUnit = _approxMapWidth / (int)std::floorf(_worldWidth);
  if (pixPerWorldUnit <= 0) pixPerWorldUnit = 1;
  float stepSize = 1.0f / (float)pixPerWorldUnit;
  float circumference = 2.0f * 3.14f * radius;
  int rayCount = (int)std::ceilf(circumference / stepSize);
  if (rayCount < 8) rayCount = 8;
  float angleStep = (2.0f * 3.14f) / (float)rayCount;
  for (int i = 0; i < rayCount; i++) {
    float angle = i * angleStep;
    float dirX = std::cosf(angle);
    float dirY = std::sinf(angle);
    float currentRayPower = damage;
    int lastHitCellId = -1;
    for (float dist = 0.0f; dist <= radius; dist += stepSize) {
      Vector2 currentPos = { center.x + dirX * dist, center.y + dirY * dist };
      Cell* cell = GetCellApprox(currentPos);
      if (cell && cell->type != CellType::AIR) {
        if (cell->id == lastHitCellId) {
          continue;
        }
        lastHitCellId = cell->id;
        if (GetCellTypeInfo(cell->type).isInvincible) {
          break;
        }
        if (cell->health <= currentRayPower) {
          currentRayPower -= cell->health;
          cell->health = 0.0f;
          cell->type = CellType::AIR;
          if (!cell->isDirty) {
            cell->isDirty = true;
            _dirtyCellIndices.push_back(cell->id);
          }
        } else {
          // cell absorbs ray
          cell->health -= currentRayPower;
          currentRayPower = 0.0f;
          if (!cell->isDirty) {
            cell->isDirty = true;
            _dirtyCellIndices.push_back(cell->id);
          }
          break; 
        }
        if (currentRayPower <= 0.01f) {
          break;
        }
      }
    }
  }
}
