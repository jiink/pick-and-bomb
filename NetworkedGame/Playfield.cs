using Riptide;
using Riptide.Utils;
using System.Data;
using System.Diagnostics;
using System.Numerics;

namespace SuperMineBombersTogether
{
    public class Playfield : List<Cell>, IMessageSerializable
    {
        const int MAX = 64;
        const int width = 8;
        const int height = 8;

        public Playfield()
        {
            for (int i = 0; i < MAX; i++)
            {
                Add(new Cell(Cell.CellType.Dirt));
            }
        }

        public void Fill()
        {
            for (int i = 0; i < MAX; i++)
            {
                this[i] = new Cell(Cell.CellType.Dirt);
            }
            RiptideLogger.Log(LogType.Debug, $">>>>> {this[0].isDirty}");
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

        //public void Serialize(Message message)
        //{
        //    int dirtyCellCount = 0;
        //    for (int i = 0; i < Count; i++)
        //    {
        //        if (this[i].isDirty)
        //        {
        //            dirtyCellCount++;
        //        }
        //    }
        //    message.AddInt(dirtyCellCount);
        //    for (int i = 0; i < Count; i++)
        //    {
        //        if (this[i].isDirty)
        //        {
        //            message.AddInt(i);
        //            message.AddSerializable(this[i]);
        //            Cell c = this[i];
        //            c.isDirty = false;
        //            this[i] = c;
        //        }
        //    }
        //    RiptideLogger.Log(LogType.Debug, $"Serialized {dirtyCellCount} dirty cells");
        //}

        //public void Deserialize(Message message)
        //{
        //    int dirtyCellCount = message.GetInt();
        //    for (int i = 0; i < dirtyCellCount; i++)
        //    {
        //        int index = message.GetInt();
        //        this[index] = message.GetSerializable<Cell>();
        //    }
        //}

        public void Serialize(Message message)
        {
            for (int i = 0; i < Count; i++)
            {
                message.AddSerializable(this[i]);
            }
        }

        public void Deserialize(Message message)
        {
            for (int i = 0; i < Count; i++)
            {
                this[i] = message.GetSerializable<Cell>();
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

        public Playfield CalculateDelta(Playfield oldPlayfield)
        {
            int numMarkedDirty = 0;
            Playfield delta = new Playfield();
            for (int i = 0; i < Count; i++)
            {
                if (this[i] != oldPlayfield[i])
                {
                    Cell c = this[i];
                    c.isDirty = true;
                    delta[i] = c;
                    numMarkedDirty++;
                }
            }
            Console.WriteLine($"{this[MAX - 1]} == {oldPlayfield[MAX - 1]}? {this[MAX - 1] == oldPlayfield[MAX - 1]}.");
            //Console.WriteLine($"Marked {numMarkedDirty} cells dirty");
            return delta;
        }

        internal void ApplyDelta(Playfield delta)
        {
            Trace.Assert(delta.Count == Count && Count == MAX);
            for (int i = 0; i < Count; i++)
            {
                if (delta[i].type != Cell.CellType.None)
                {
                    Cell cell = this[i];
                    cell.type = delta[i].type;
                    this[i] = cell;
                }
                if (delta[i].health != 0)
                {
                    Cell cell = this[i];
                    cell.health = delta[i].health;
                    this[i] = cell;
                }
            }
        }

        //public void ReportDelta()
        //{
        //    int diffCells = 0;
        //    for (int i = 0; i < Count; i++)
        //    {
        //        if (this[i].type != Cell.CellType.None || this[i].health !=)
        //        {
        //            diffCells++;
        //        }
        //    }
        //}

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