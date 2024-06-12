using Raylib_CSharp.Rendering;
using Riptide;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using static SuperMineBombersTogether.Cell;

namespace SuperMineBombersTogether.Bombs
{
    internal abstract class AbstractBomb : IMessageSerializable
    {
        public int id = 0;
        public Vector2 pos = new Vector2(0);
        public Vector2 vel = new Vector2(0);
        float fuseTimer = 0;
        float startingFuse = 0;
        float friction = 0.7f;
        int damage = 0;
        int radius = 0;
        int price = 0;
        int colorIndex = 0;
        public bool exploded = false;

        public AbstractBomb() { }
        public AbstractBomb(int id, Vector2 pos, Vector2 vel)
        {
            this.id = id;
            this.pos = pos;
            this.vel = vel;
            fuseTimer = 1f;
        }

        protected abstract bool Detonate(MatchState m);

        public abstract void Update(float deltaTime, MatchState m);

        protected void RollAndCollide(float deltaTime, MatchState m)
        {
            if (vel == Vector2.Zero) return;
            vel *= friction;
            var prevPos = pos;
            pos += vel * deltaTime;
            if (m.playfield.GetCellAtPos(pos)?.type != CellType.Air)
            {
                vel = Vector2.Zero;
                pos = prevPos;
            }
        }

        protected void FuseTick(float deltaTime, MatchState m)
        {
            fuseTimer -= deltaTime;
            if (fuseTimer <= 0) Detonate(m);
        }

        public void Draw()
        {
            Graphics.DrawCircleV(pos, 0.3f, Player.colorList[colorIndex]);
        }

        public void Deserialize(Message message)
        {
            id = message.GetInt();
            pos = new Vector2(message.GetFloat(), message.GetFloat());
            vel = new Vector2(message.GetFloat(), message.GetFloat());
            colorIndex = message.GetInt();
        }

        public void Serialize(Message message)
        {
            message.AddInt(id);
            message.AddFloat(pos.X);
            message.AddFloat(pos.Y);
            message.AddFloat(vel.X);
            message.AddFloat(vel.Y);
            message.AddInt(colorIndex);
        }

        public static int GetClassId(Type type)
        {
            if (!typeof(AbstractBomb).IsAssignableFrom(type))
            {
                throw new ArgumentException("Type must be a subclass of AbstractBomb", nameof(type));
            }

            return type.FullName.GetHashCode();
        }

        public static Type GetTypeFromId(int id)
        {
            var types = GetSubclassesOfAbstractBomb();
            return types.FirstOrDefault(t => t.FullName.GetHashCode() == id);
        }

        public static IEnumerable<Type> GetSubclassesOfAbstractBomb()
        {
            var assembly = Assembly.GetExecutingAssembly();
            foreach (Type type in assembly.GetTypes())
            {
                if (type.IsSubclassOf(typeof(AbstractBomb)))
                {
                    yield return type;
                }
            }
        }
    }
}
