using Raylib_CSharp;
using Riptide;
using SuperMineBombersTogether.Bombs;
using SuperMineBombersTogether.PacketTypes;
using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
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
            bombs = new List<AbstractBomb>();
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
                        if (input.attackPressed)
                        {
                            Console.WriteLine("BOMB!");
                            TinyBomb newBomb = new(bombs.Count, player.pos, new Vector2(10, 0));
                            SpawnBomb(newBomb);
                        }
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
            var bombsToCleanUp = new List<int>();
            for (int i = 0; i < bombs.Count; i++)
            {
                bombs[i].Update(deltaTime, this);
                if (bombs[i].Exploded)
                {
                    bombsToCleanUp.Add(i);
                }
            }
            foreach (int i in bombsToCleanUp)
            {
                bombs.RemoveAt(i);
            }
        }

        public void SpawnBomb(AbstractBomb b)
        {
            bombs.Add(b);
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
            playfield.Draw();
            foreach (var player in players.ToList())
            {
                player.Draw();
            }
            foreach (var b in bombs.ToList())
            {
                b.Draw();
            }
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

        internal void Explode(Vector2 pos, float radius, float damage)
        {
            for (int i = 0; i < 360; i += 10)
            {
                float rads = RayMath.Deg2Rad * i;
                CastExplosionRay(pos, rads, radius, damage);
            }
        }

        private void CastExplosionRay(Vector2 pos, float angle, float length, float damage)
        {
            const float stepSize = 0.3f;
            int stepCount = (int)(length / stepSize);
            Vector2 step = RayMath.Vector2Rotate(Vector2.UnitX, angle);
            Vector2 testPt = pos;
            for (int i = 0; i < stepCount; i++)
            {
                testPt += step * stepSize;
                Cell? cell = playfield.GetCellAtPos(testPt);
                if (cell is null) { continue; }
                if (cell.type  == Cell.CellType.Air) { continue; }
                float cellHp = cell.health;
                cell.Damage(damage);
                // Let the ray be stopped if the cell was not destroyed.
                if (cell.type != Cell.CellType.Air)
                {
                    break;
                }
                damage -= cellHp;
            }
        }
    }
}
