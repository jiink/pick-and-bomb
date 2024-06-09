using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SuperMineBombersTogether.Bombs
{
    internal class Bomb : AbstractBomb
    {
        float startingFuse = 2.0f;
        int damage = 150;
        int radius = 6;
        int price = 0;

        protected override bool Detonate(ref MatchState m)
        {
            m.Explode(pos);
            return true;
        }

        protected override void Update(float deltaTime, MatchState m)
        {
            RollAndCollide(deltaTime, m);
        }
    }
}
