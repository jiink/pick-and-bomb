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
using static SuperMineBombersTogether.Common;

namespace SuperMineBombersTogether.Bombs
{
    internal abstract class AbstractBomb : IMessageSerializable
    {
        abstract public EntDict EntDictId { get; }
        public int Id { get; set; }
        public Vector2 Pos { get; set; }
        public Vector2 Vel { get; set; }
        private float fuseTimer;
        protected abstract float StartingFuse { get; }
        protected abstract float Friction { get; }
        protected abstract int Damage { get; }
        protected abstract int Radius { get; }
        protected abstract int Price { get; }
        protected virtual int ColorIndex { get; set; }
        public bool Exploded { get; protected set; } = false;

        public AbstractBomb() { }
        public AbstractBomb(int id, Vector2 pos, Vector2 vel)
        {
            Id = id;
            Pos = pos;
            Vel = vel;
            fuseTimer = StartingFuse;
        }

        protected abstract bool Detonate(MatchState m);

        public abstract void Update(float deltaTime, MatchState m);

        protected void RollAndCollide(float deltaTime, MatchState m)
        {
            if (Vel == Vector2.Zero) return;
            Vel *= Friction;
            var prevPos = Pos;
            Pos += Vel * deltaTime;
            if (m.playfield.GetCellAtPos(Pos)?.type != CellType.Air)
            {
                Vel = Vector2.Zero;
                Pos = prevPos;
            }
        }

        protected void FuseTick(float deltaTime, MatchState m)
        {
            fuseTimer -= deltaTime;
            if (fuseTimer <= 0) Detonate(m);
        }

        public void Draw()
        {
            Graphics.DrawCircleV(Pos, 0.3f, Player.colorList[ColorIndex]);
        }

        public void Deserialize(Message message)
        {
            Id = message.GetInt();
            Pos = new Vector2(message.GetFloat(), message.GetFloat());
            Vel = new Vector2(message.GetFloat(), message.GetFloat());
            ColorIndex = message.GetInt();
        }

        public void Serialize(Message message)
        {
            message.AddInt(Id);
            message.AddFloat(Pos.X);
            message.AddFloat(Pos.Y);
            message.AddFloat(Vel.X);
            message.AddFloat(Vel.Y);
            message.AddInt(ColorIndex);
        }
    }
}
