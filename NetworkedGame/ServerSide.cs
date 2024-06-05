using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System;
using System.Net;
using System.Net.Sockets;
using Riptide;
using Riptide.Transports;
using static SuperMineBombersTogether.Common;
using SuperMineBombersTogether.PacketTypes;
using System.Numerics;

namespace SuperMineBombersTogether
{
    class ServerSide
    {
        static GameState gameState = new GameState();
        static public void Start()
        {
            Server server = new Server();
            server.Start(7777, maxClientCount: 4);
            server.ClientConnected += OnClientConnect;
            server.ClientDisconnected += OnClientDisconnect;
            Timer timer = new Timer(FixedUpdate, server, 0, 1000 / Common.tickRate);
            Timer heartbeatTimer = new Timer(Heartbeat, server, 0, 500);
        }

        private static void OnClientDisconnect(object? sender, ServerDisconnectedEventArgs e)
        {
            Console.WriteLine($"Client {e.Client.ToString} disconnected...");
        }

        private static void OnClientConnect(object? sender, ServerConnectedEventArgs e)
        {
            if (sender == null) return;
            var server = (Server)sender;
            Console.WriteLine($"Client {e.Client.ToString} connected!");
            gameState.AddPlayer(new Player(100, 100, Player.colorList[server.ClientCount]));
            // Assign player number to client
            Message message = Message.Create(MessageSendMode.Reliable, (ushort)PlayerAssignS2C.Id);
            PlayerAssignS2C playerAssign = new PlayerAssignS2C(gameState.Players.Count - 1);
            message.AddSerializable(playerAssign);
            server.Send(message, e.Client);
        }

        private static void Heartbeat(object? state)
        {
            if (state == null)
            {
                return;
            }
            var server = (Server)state;
            Message message = Message.Create(MessageSendMode.Reliable, 0);
            message.AddInt(99);
            server.SendToAll(message);
        }

        [MessageHandler((ushort)MessageId.PlayerPosition)]
        private static void HandlePlayerPos(ushort u, Message message)
        {
            PlayerPosC2S playerPos = message.GetSerializable<PlayerPosC2S>();
            //Console.WriteLine($"I got {playerPos.X:0.0}, {playerPos.Y:0.0}");
            if (playerPos.PlayerNumber >= gameState.Players.Count)
            {
                return;
            }
            gameState.UpdatePlayerPosition(playerPos.PlayerNumber, playerPos.X, playerPos.Y);
        }

        [MessageHandler((ushort)MessageId.PlayerMove)]
        private static void HandlePlayerMove(ushort u, Message message)
        {
            PlayerMoveC2S playerMove = message.GetSerializable<PlayerMoveC2S>();
            Console.WriteLine($"{playerMove.PlayerNumber} Move {playerMove.X:0.0}, {playerMove.Y:0.0}");
            if (playerMove.PlayerNumber >= gameState.Players.Count)
            {
                return;
            }
            gameState.Players[playerMove.PlayerNumber].Vel = new Vector2(playerMove.X, playerMove.Y);
        }

        private static void FixedUpdate(object? state)
        {
            if (state == null)
            {
                return;
            }
            var server = (Server)state;
            server.Update();

            gameState.Update(1f / tickRate);

            Message message = Message.Create(MessageSendMode.Unreliable, (ushort)MessageId.GameState);
            gameState.Serialize(message);
            server.SendToAll(message);
        }
    }
}
