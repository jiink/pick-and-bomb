using Riptide;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using static SuperMineBombersTogether.Common;

namespace SuperMineBombersTogether.Bombs
{
    internal class TinyBomb : AbstractBomb
    {
        public override EntDict EntDictId => EntDict.TINY_BOMB;
        protected override float StartingFuse => 2.0f;
        protected override int Damage => 50;
        protected override int Radius => 3;
        protected override float Friction => 0.9f;
        protected override int Price => 1;
        protected override int ColorIndex => 2;

        public TinyBomb() { }

        public TinyBomb(int id, Vector2 pos, Vector2 vel) : base(id, pos, vel)
        {
        }

        protected override bool Detonate(MatchState m)
        {
            m.Explode(Pos, Radius, Damage);
            Exploded = true;
            return true;
        }

        public override void Update(float deltaTime, MatchState m)
        {
            if (Exploded) { return; }
            RollAndCollide(deltaTime, m);
            FuseTick(deltaTime, m);
        }

        public new void Deserialize(Message message)
        {
            base.Deserialize(message);
        }

        public new void Serialize(Message message)
        {
            base.Serialize(message);
        }
    }
}
