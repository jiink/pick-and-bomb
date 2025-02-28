using Riptide;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using static SuperMineBombersTogether.Common;

namespace SuperMineBombersTogether.PacketTypes
{
    internal class PlayerInputState
    {
        public Vector2 direction = new(0, 0);
        public bool attack = false;
        public bool attackPressed = false;
        public bool attackReleased = false;

        internal PlayerInputState() { }

        internal PlayerInputState(Vector2 direction, bool attack, bool attackPressed, bool attackReleased)
        {
            this.direction = direction;
            this.attack = attack;
            this.attackPressed = attackPressed;
            this.attackReleased = attackReleased;
        }

        public override string ToString()
        {
            return $"Dir: {direction}, Atk: {attack}, AtkP: {attackPressed}, AtkR: {attackReleased}";
        }
    }
}
