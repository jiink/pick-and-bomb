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
    internal class PlayerInputState : IMessageSerializable
    {
        public const MessageId Id = MessageId.PlayerInput;

        public int clientId = 0;
        public Vector2 direction = new Vector2(0, 0);
        public bool attack = false;
        public bool attackPressed = false;
        public bool attackReleased = false;

        public PlayerInputState() { }

        public PlayerInputState(Vector2 direction, bool attack, bool attackPressed, bool attackReleased)
        {
            this.direction = direction;
            this.attack = attack;
            this.attackPressed = attackPressed;
            this.attackReleased = attackReleased;
        }

        public void Serialize(Message message)
        {
            message.AddFloat(direction.X);
            message.AddFloat(direction.Y);
            message.AddBool(attack);
            message.AddBool(attackPressed);
            message.AddBool(attackReleased);
        }

        public void Deserialize(Message message)
        {
            direction = new Vector2(message.GetFloat(), message.GetFloat());
            attack = message.GetBool();
            attackPressed = message.GetBool();
            attackReleased = message.GetBool();
        }
    }
}
