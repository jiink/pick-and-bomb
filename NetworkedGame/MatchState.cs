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
        public Player ownedPlayer; // you
        public List<Player> players;
        public List<AbstractBomb> bombs;
        public Playfield playfield;

        public MatchState()
        {
            players = new();
            bombs = new();
            playfield = new();
        }

        public void Update(bool isHosting, float deltaTime, PlayerInputState inputs)
        {
            foreach (Player player in players)
            {
                player.Update(deltaTime, inputs, playfield);
            }
            if (inputs.attackPressed)
            {
                Console.WriteLine("BOMB!");
                Bomb newBomb = new(bombs.Count, ownedPlayer.pos, new Vector2(10, 0));
                SpawnBomb(newBomb);
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

        public void AddPlayer(Player player, bool isHosting)
        {
            players.Add(player);
            player.pos = playfield.spawnPoints[players.Count - 1];
            if (isHosting)
            {
                ownedPlayer = player;
                ownedPlayer.isOwned = true;
            }
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
                // If there are any players in this cell, damage them too
                for (int j = 0; j < players.Count; j++)
                {
                    var dist = (players[j].pos - testPt).Length();
                    Console.WriteLine($"Dist: {dist}");
                    if (dist < stepSize)
                    {
                        players[j].Damage((int)damage);
                    }
                }
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
