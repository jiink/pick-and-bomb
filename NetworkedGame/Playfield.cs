using Riptide;
using System.Numerics;

namespace SuperMineBombersTogether
{
    public class Playfield : List<Cell>, IMessageSerializable
    {
        public int width;
        public int height;

        public Playfield()
        {
            width = 0;
            height = 0;
        }
        public Playfield(int width, int height)
        {
            this.width = width;
            this.height = height;
            Clear();
            for (int i = 0; i < width * height; i++)
            {
                Add(new Cell(Cell.CellType.Dirt));
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
            message.AddInt(width);
            message.AddInt(height);
            for (int i = 0; i < Count; i++)
            {
                message.AddSerializable(this[i]);
            }
        }

        public void Deserialize(Message message)
        {
            Clear();
            width = message.GetInt();
            height = message.GetInt();
            for (int i = 0; i < width * height; i++)
            {
                Add(message.GetSerializable<Cell>());
            }
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