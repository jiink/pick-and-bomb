using Riptide;
using SuperMineBombersTogether.PacketTypes;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace SuperMineBombersTogether
{
    internal class MatchState : IMessageSerializable
    {
        public List<Player> Players { get; private set; }

        public MatchState()
        {
            Players = new List<Player>();
        }

        public void Update(float deltaTime, List<PlayerInputState> inputs)
        {
            foreach (Player player in Players)
            {
                // Update with input matching id
                bool found = false;
                foreach (PlayerInputState input in inputs)
                {
                    if (player.Id == input.clientId)
                    {
                        found = true;
                        player.Update(deltaTime, input);
                        break;
                    }
                }
                if (!found)
                {
                    player.Update(deltaTime, new PlayerInputState());
                }
            }
        }

        public void AddPlayer(Player player)
        {
            Players.Add(player);
        }

        public void RemovePlayer(Player player)
        {
            Players.Remove(player);
        }

        public void Draw()
        {
            foreach (Player player in Players)
            {
                player.Draw();
            }
        }

        public void UpdatePlayerPosition(int playerNum, float x, float y)
        {
            if (Players.Count < playerNum + 1)
            {
                return;
            }
            Player player = Players[playerNum];
            player.SetPosition(x, y);
        }

        public void Serialize(Message message)
        {
            message.AddInt(Players.Count);
            foreach (Player player in Players)
            {
                message.AddSerializable(player);
            }
        }

        public void Deserialize(Message message)
        {
            int playerCount = message.GetInt();
            for (int i = 0; i < playerCount; i++)
            {
                Player p = message.GetSerializable<Player>();
                Players.Add(p);
            }
        }
    }
}
