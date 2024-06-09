using Riptide;
using Riptide.Utils;
using System.Data;
using System.Diagnostics;
using System.Numerics;

namespace SuperMineBombersTogether
{
    public class Playfield : List<Cell>, IMessageSerializable
    {
        const int width = 32;
        const int height = 32;
        const int MAX = width * height;
        const int maxCellsToSend = 32;

        public Playfield()
        {
            for (int i = 0; i < MAX; i++)
            {
                Add(new Cell(Cell.CellType.Air));
            }
        }

        public void Fill()
        {
            for (int i = 0; i < MAX; i++)
            {
                this[i] = new Cell(Cell.CellType.Dirt);
            }
        }

        public Cell this[int x, int y]
        {
            get
            {
                return this[y * width + x];
            }
            set
            {
                this[y * width + x] = value;
            }
        }

        public Cell? GetCellAtPos(Vector2 worldPos)
        {
            const float cellSize = 1;
            int x = (int)(worldPos.X / cellSize);
            int y = (int)(worldPos.Y / cellSize);
            return GetCellAtRowCol(y, x);
        }

        public Cell? GetCellAtRowCol(int row, int col)
        {
            if (col < 0 || col >= width || row < 0 || row >= height)
            {
                return null;
            }
            return this[col, row];
        }

        public void Serialize(Message message)
        {
            int dirtyCellCount = 0;
            for (int i = 0; i < Count; i++)
            {
                if (this[i].isDirty)
                {
                    dirtyCellCount++;
                }
            }
            if (dirtyCellCount > maxCellsToSend)
            {
                dirtyCellCount = maxCellsToSend;
            }
            message.AddInt(dirtyCellCount);
            Console.WriteLine($"Gonna send {dirtyCellCount} cells");
            int idx = 0;
            while (dirtyCellCount > 0)
            {
                Trace.Assert(idx >= 0 && idx <= Count - 1);
                if (this[idx].isDirty)
                {
                    message.AddInt(idx);
                    message.AddSerializable(this[idx]);
                    this[idx].isDirty = false;
                    dirtyCellCount--;
                }
                idx++;
            }
        }

        public void Deserialize(Message message)
        {
            int dirtyCellCount = message.GetInt();
            Console.WriteLine($"Deseruializing playfield {dirtyCellCount}c");
            for (int i = 0; i < dirtyCellCount; i++)
            {
                int index = message.GetInt();
                this[index] = message.GetSerializable<Cell>();
            }
        }

        public bool IsDirty()
        {
            foreach (Cell c in this)
            { 
                if (c.isDirty)
                {
                    return true;
                }
            }
            return false;
        }

        public void MakeDirty()
        {
            foreach(Cell c in this)
            {
                c.isDirty = true;
            }
        }

        public Playfield DeepCopy()
        {
            Playfield copy = new Playfield();
            for (int i = 0; i < Count; i++)
            {
                copy[i] = this[i];
            }
            return copy;
        }

        public void Draw()
        {
            for (int i = 0; i < Count; i++)
            {
                int x = i % width;
                int y = i / width;
                this[x, y].Draw(x, y);
            }
        }

        
    }
}