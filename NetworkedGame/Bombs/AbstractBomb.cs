using Raylib_CSharp.Rendering;
using Riptide;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
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

        public AbstractBomb() { }
        public AbstractBomb(int id, Vector2 pos, Vector2 vel)
        {
            this.id = id;
            this.pos = pos;
            this.vel = vel;
        }

        protected abstract bool Detonate(ref MatchState m);

        protected abstract void Update(float deltaTime, MatchState m);

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

        protected void Draw()
        {
            Graphics.DrawCircleV(pos, 0.3f, Player.colorList[colorIndex]);
        }

        public void Deserialize(Message message)
        {
            throw new NotImplementedException();
        }

        public void Serialize(Message message)
        {
            throw new NotImplementedException();
        }
    }
}
