using Riptide;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static SuperMineBombersTogether.Common;

namespace SuperMineBombersTogether.PacketTypes
{
    internal class PlayerAssignS2C : IMessageSerializable
    {
        public const MessageId Id = MessageId.PlayerAssign;
        public int PlayerNumber { get; private set; }

        public PlayerAssignS2C(int playerNumber)
        {
            PlayerNumber = playerNumber;
        }

        public PlayerAssignS2C()
        {
            PlayerNumber = 0;
        }

        public void Serialize(Message message)
        {
            message.AddInt(PlayerNumber);
        }

        public void Deserialize(Message message)
        {
            PlayerNumber = message.GetInt();
        }
    }
}
