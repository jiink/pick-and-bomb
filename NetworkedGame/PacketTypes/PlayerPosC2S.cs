using Riptide;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static SuperMineBombersTogether.Common;

namespace SuperMineBombersTogether.PacketTypes
{
    internal class PlayerPosC2S : IMessageSerializable
    {
        public const MessageId Id = MessageId.PlayerPosition;
        public int PlayerNumber { get; private set; }
        public float X { get; private set; }
        public float Y { get; private set; }

        public PlayerPosC2S(int playerNum, float x, float y)
        {
            PlayerNumber = playerNum;
            X = x;
            Y = y;
        }

        public PlayerPosC2S()
        {
            PlayerNumber = 0;
            X = 0;
            Y = 0;
        }

        public void Serialize(Message message)
        {
            message.AddInt(PlayerNumber);
            message.AddFloat(X);
            message.AddFloat(Y);
        }

        public void Deserialize(Message message)
        {
            PlayerNumber = message.GetInt();
            X = message.GetFloat();
            Y = message.GetFloat();
        }
    }
}
