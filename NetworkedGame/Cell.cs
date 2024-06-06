using Raylib_CSharp.Colors;
using Raylib_CSharp.Rendering;
using Riptide;
using Riptide.Utils;
using System.Diagnostics;
using System.Numerics;

namespace SuperMineBombersTogether
{
    public class Cell : IMessageSerializable
    {
        public enum CellType
        {
            None, // For deltas
            Air,
            Dirt,
            Stone,
            Treasure,
            Wall
        };

        static Dictionary<CellType, CellProperties> cellProperties = new Dictionary<CellType, CellProperties>
        {
            { CellType.Air, new CellProperties { resistance = 0, solid = false } },
            { CellType.Dirt, new CellProperties { resistance = 1, solid = true } },
            { CellType.Stone, new CellProperties { resistance = 2, solid = true } },
            { CellType.Treasure, new CellProperties { resistance = 1, solid = false } },
            { CellType.Wall, new CellProperties { resistance = 0, solid = true } }
        };
        static Dictionary<CellType, Color> cellColors = new Dictionary<CellType, Color>
        {
            { CellType.Air, Color.Magenta },
            { CellType.Dirt, Color.Brown },
            { CellType.Stone, Color.DarkGray },
            { CellType.Treasure, Color.Gold },
            { CellType.Wall, Color.Gray }
        };
        public CellType type;
        public float health = 100;
        public bool isDirty = false;

        public Cell()
        {
            type = CellType.Air;
        }

        public Cell(CellType type)
        {
            this.type = type;
            isDirty = true;
        }

        public void Damage(float damage)
        {
            if (damage < 0 || type == CellType.Air)
            {
                return;
            }
            CellProperties cellProps = cellProperties[type];
            if (!cellProps.solid)
            {
                return;
            }
            float appliedDamage = damage / cellProps.resistance;
            health -= appliedDamage;
            if (health <= 0)
            {
                type = CellType.Air;
            }
            isDirty = true;
            RiptideLogger.Log(LogType.Debug, $"Damaged cell {type} by {damage} to {health}");
        }

        //public void Serialize(Message message)
        //{
        //    message.AddInt((int)type);
        //    message.AddFloat(health);
        //}

        //public void Deserialize(Message message)
        //{
        //    type = (CellType)message.GetInt();
        //    health = message.GetFloat();
        //}

        public void Serialize(Message message)
        {
            byte hp4bit = (byte)(health/100f * 0x0F);
            byte b = (byte)((byte)type << 4 | (hp4bit & 0x0F));
            message.AddByte(b);
        }

        public void Deserialize(Message message)
        {
            byte b = message.GetByte();
            type = (CellType)(b >> 4);
            byte hp4bit = (byte)(b & 0x0F);
            health = hp4bit / 16f * 100;

        }

        public static bool floatEquals(float a, float b)
        {
            return Math.Abs(a - b) < 0.0001;
        }

        public static bool operator !=(Cell a, Cell b)
        {
            return !a.Equals(b);
        }

        public static bool operator ==(Cell a, Cell b)
        {
            return a.Equals(b);
        }

        public override bool Equals(object obj)
        {
            Trace.Assert(obj is Cell);
            Cell other = (Cell)obj;
            return type == other.type && floatEquals(health, other.health);
        }

        public override string ToString()
        {
            return $"{type} {health}";
        }

        public void Draw(float x, float y)
        {
            if (type == CellType.Air)
            {
                return;
            }
            Color color = Color.Brightness(cellColors[type], -(100 - health) / 100f);
            Graphics.DrawRectangleV(new Vector2(x, y), new Vector2(1, 1), color);
        }
    }

    internal struct CellProperties
    {
        public float resistance;
        public bool solid;
    }
}