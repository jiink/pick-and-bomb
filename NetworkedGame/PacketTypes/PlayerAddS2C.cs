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
    internal class PlayerAddS2C : IMessageSerializable
    {
        public const MessageId Id = MessageId.PlayerAdd;

        public Player playerToAdd;

        public PlayerAddS2C() { }

        public PlayerAddS2C(Player p)
        {
            this.playerToAdd = p;
        }

        public void Serialize(Message message)
        {
            message.AddSerializable(playerToAdd);
        }

        public void Deserialize(Message message)
        {
            playerToAdd = message.GetSerializable<Player>();
        }
    }
}
