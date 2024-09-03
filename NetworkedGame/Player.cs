using Raylib_CSharp.Colors;
using Raylib_CSharp.Rendering;
using Riptide;
using SuperMineBombersTogether.PacketTypes;
using System.Numerics;
using System.Runtime.InteropServices.Marshalling;

namespace SuperMineBombersTogether
{
    internal class Player : IMessageSerializable
    {
        public static List<Color> colorList = new List<Color> { Color.Blue, Color.Red, Color.Green, Color.Yellow, Color.Purple, Color.Orange, Color.Brown, Color.Pink, Color.Gray, Color.Black };
        public int id = 0;
        public Vector2 pos = new Vector2(0, 0);
        public Vector2 vel = new Vector2(0, 0);
        float defSpeed = 5; //How fast you walk by default
        public bool isDead = false;
        public int health = 100;
        int colorIndex = 0;
        Color Color => colorList[colorIndex];

        public Player() { }

        public Player(float x, float y, int colorIndex, int id)
        {
            pos = new Vector2(x, y);
            this.colorIndex = colorIndex;
            this.id = id;
        }

        public void SetPosition(float x, float y)
        {
            pos = new Vector2(x, y);
        }

        public void Update(float deltaTime, PlayerInputStateC2S input, Playfield pfield)
        {
            vel = input.direction * defSpeed;
            var desiredPos = pos + vel * deltaTime;
            var destination = new Vector2(desiredPos.X, desiredPos.Y);
            if (pfield.IsSolidCellAtPoint(desiredPos))
            {
                vel = Vector2.Zero;
            }
            if (pfield.IsSolidCellAtPoint(new Vector2(desiredPos.X, pos.Y)))
            {
                destination.X = pos.X;
            }
            if (pfield.IsSolidCellAtPoint(new Vector2(pos.X, desiredPos.Y)))
            {
                destination.Y = pos.Y;
            }
            if (pfield.IsSolidCellAtPoint(destination))
            {
                destination = pos;
            }
            pos = destination;

            // If you push against a solid cell you start mining it
            const float miningSpeed = 300f; // Health per second
            pfield.GetCellAtPos(desiredPos)?.Damage(deltaTime * miningSpeed);
        }

        public void Draw()
        {
            if (!isDead)
            {
                Graphics.DrawCircleV(pos, 0.5f, colorList[colorIndex]);
                const float hpRingDiameter = 1.0f;
                Graphics.DrawRing(pos, hpRingDiameter, hpRingDiameter + 0.2f, 0, health * 3.60f, 20, Color);
            }
            else
            {
                Graphics.DrawCircleV(pos, 0.5f, Color.Black);
            }
        }

        public void Serialize(Message message)
        {
            message.AddInt(id);
            message.AddFloat(pos.X);
            message.AddFloat(pos.Y);
            message.AddInt(colorIndex);
            message.AddInt(health);
            message.AddBool(isDead);
        }

        public void Deserialize(Message message)
        {
            id = message.GetInt();
            pos = new Vector2(message.GetFloat(), message.GetFloat());
            colorIndex = message.GetInt();
            health = message.GetInt();
            isDead = message.GetBool();
        }

        public void Damage(int damage)
        {
            Console.WriteLine($"OUCH! {health} - {damage}");
            if (isDead) { return; }
            health -= damage;
            if (health <= 0)
            {
                Console.WriteLine($">>>> PLAYER {id} IS DEAD!");
                isDead = true;
            }
        }
    }
}
