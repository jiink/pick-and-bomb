using Raylib_CSharp.Colors;
using Raylib_CSharp.Rendering;
using Riptide;
using System.Numerics;

namespace SuperMineBombersTogether
{
    public class Cell : IMessageSerializable
    {
        public enum CellType
        {
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
        CellType type;
        float health = 100;

        public Cell()
        {
            type = CellType.Air;
        }

        public Cell(CellType type)
        {
            this.type = type;
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
        }

        public void Serialize(Message message)
        {
            message.AddInt((int)type);
            message.AddFloat(health);
        }

        public void Deserialize(Message message)
        {
            type = (CellType)message.GetInt();
            health = message.GetFloat();
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