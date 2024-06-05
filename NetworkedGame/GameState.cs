using Riptide;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace SuperMineBombersTogether
{
    internal class GameState : IMessageSerializable
    {
        public List<Player> Players { get; private set; }

        public GameState()
        {
            Players = new List<Player>();
        }

        public void Update(float deltaTime)
        {
            foreach (Player player in Players)
            {
                player.Update(deltaTime);
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

        //public void MovePlayer(int playerNum, float x, float y)
        //{
        //    if (Players.Count < playerNum + 1)
        //    {
        //        return;
        //    }
        //    Player player = Players[playerNum];
        //    UpdatePlayerPosition(playerNum, player.X + x, player.Y + y);
        //}   

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
                message.AddFloat(player.Pos.X);
                message.AddFloat(player.Pos.Y);
                message.AddFloat(player.Vel.X);
                message.AddFloat(player.Vel.Y);
            }
        }

        public void Deserialize(Message message)
        {
            int playerCount = message.GetInt();
            for (int i = 0; i < playerCount; i++)
            {
                float x = message.GetFloat();
                float y = message.GetFloat();

                if (Players.Count < i + 1)
                {
                    Player player = new Player(x, y, Player.colorList[i]);
                    Players.Add(player);
                }
                else
                {
                    Players[i].SetPosition(x, y);
                    Players[i].Vel = new Vector2(message.GetFloat(), message.GetFloat());
                }
            }
        }
    }
}
