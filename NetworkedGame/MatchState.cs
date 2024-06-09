using Riptide;
using SuperMineBombersTogether.Bombs;
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
    internal class MatchState
    {
        public List<Player> players;
        public List<AbstractBomb> bombs;
        public Playfield playfield;

        public MatchState()
        {
            players = new List<Player>();
            playfield = new Playfield();
        }

        public void Update(bool isClient, float deltaTime, List<PlayerInputStateC2S> inputs)
        {
            foreach (Player player in players)
            {
                // Update with input matching id
                bool found = false;
                foreach (PlayerInputStateC2S input in inputs)
                {
                    if (player.id == input.clientId)
                    {
                        found = true;
                        player.Update(deltaTime, input);
                        // If you push against a solid cell you start mining it
                        const float miningSpeed = 300f; // Health per second
                        playfield.GetCellAtPos(player.pos)?.Damage(deltaTime * miningSpeed);
                        break;
                    }
                }
                if (!found)
                {
                    player.Update(deltaTime, new PlayerInputStateC2S());
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

        internal void Explode(Vector2 pos)
        {
            throw new NotImplementedException();
        }
    }
}
