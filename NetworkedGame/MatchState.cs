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
        public List<Player> players;
        public Playfield playfield;
        public Playfield prevPlayfield; // For calculating deltas

        public MatchState()
        {
            players = new List<Player>();
            playfield = new Playfield();
            //playfield.Fill();
            prevPlayfield = new Playfield();
        }

        public void Update(float deltaTime, List<PlayerInputState> inputs)
        {
            foreach (Player player in players)
            {
                // Update with input matching id
                bool found = false;
                foreach (PlayerInputState input in inputs)
                {
                    if (player.Id == input.clientId)
                    {
                        found = true;
                        player.Update(deltaTime, input);
                        // If you push against a solid cell you start mining it
                        const float miningSpeed = 300f; // Health per second
                        playfield.GetCellAtPos(player.Pos)?.Damage(deltaTime * miningSpeed);
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
            players.Add(player);
        }

        public void RemovePlayer(Player player)
        {
            players.Remove(player);
        }

        public void Draw()
        {
            foreach (Player player in players.ToList())
            {
                player.Draw();
            }
            playfield.Draw();
        }

        public void UpdatePlayerPosition(int playerNum, float x, float y)
        {
            if (players.Count < playerNum + 1)
            {
                return;
            }
            Player player = players[playerNum];
            player.SetPosition(x, y);
        }

        public void Serialize(Message message)
        {
            message.AddInt(players.Count);
            foreach (Player player in players)
            {
                message.AddSerializable(player);
            }
            // Calculate delta
            //Playfield pfdelta = playfield.CalculateDelta(prevPlayfield);
            message.AddSerializable(playfield);
        }

        public void Deserialize(Message message)
        {
            int playerCount = message.GetInt();
            for (int i = 0; i < playerCount; i++)
            {
                Player p = message.GetSerializable<Player>();
                players.Add(p);
            }
            //Playfield pfdelta = message.GetSerializable<Playfield>();
            //playfield.ApplyDelta(pfdelta);
            playfield = message.GetSerializable<Playfield>();
            //prevPlayfield = playfield.DeepCopy();
        }
    }
}
