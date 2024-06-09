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
    internal class PlayerSetPosS2C : IMessageSerializable
    {
        public const MessageId Id = MessageId.PlayerSetPos;

        public int playerId = 0;
        public Vector2 pos = new Vector2(0, 0);

        public PlayerSetPosS2C() { }

        public PlayerSetPosS2C(int playerId, Vector2 pos)
        {
            this.playerId = playerId;
            this.pos = pos;
        }

        public override string ToString()
        {
            return $"Who: {playerId}, Where: {pos}";
        }

        public void Serialize(Message message)
        {
            message.AddInt(playerId);
            message.AddFloat(pos.X);
            message.AddFloat(pos.Y);
        }

        public void Deserialize(Message message)
        {
            playerId = message.GetInt();
            pos = new Vector2(message.GetFloat(), message.GetFloat());
        }
    }
}
