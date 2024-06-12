using Riptide;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace SuperMineBombersTogether.Bombs
{
    internal class Bomb : AbstractBomb
    {
        static float startingFuse = 2.0f;
        int damage = 150;
        int radius = 6;
        int price = 0;

        public Bomb() { }

        public Bomb(int id, Vector2 pos, Vector2 vel) : base(id, pos, vel, startingFuse)
        {
        }

        protected override bool Detonate(MatchState m)
        {
            m.Explode(pos, radius, damage);
            exploded = true;
            return true;
        }

        public override void Update(float deltaTime, MatchState m)
        {
            if (exploded) { return; }
            RollAndCollide(deltaTime, m);
            FuseTick(deltaTime, m);
        }

        public void Deserialize(Message message)
        {
            base.Deserialize(message);
        }

        public void Serialize(Message message)
        {
            base.Serialize(message);
        }
    }
}
