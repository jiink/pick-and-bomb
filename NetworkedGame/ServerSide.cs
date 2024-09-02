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
using System.Diagnostics;
using System.Diagnostics.Metrics;
using Riptide.Utils;
using SuperMineBombersTogether.Bombs;

namespace SuperMineBombersTogether
{
    class ServerSide
    {
        static MatchState matchState = new MatchState();
        static List<PlayerInputStateC2S> playerInputs = new List<PlayerInputStateC2S>();
        static int counter = 0;
        static public void Start()
        {
            Server server = new Server();
            server.Start(7777, maxClientCount: 4);
            server.ClientConnected += OnClientConnect;
            server.ClientDisconnected += OnClientDisconnect;
            Timer timer = new Timer(FixedUpdate, server, 0, 1000 / Common.tickRate);
            Timer heartbeatTimer = new Timer(Heartbeat, server, 0, 500);
            matchState.playfield.Fill();
        }

        private static void OnClientDisconnect(object? sender, ServerDisconnectedEventArgs e)
        {
            Console.WriteLine($"Client {e.Client.ToString} disconnected...");
        }

        private static void OnClientConnect(object? sender, ServerConnectedEventArgs e)
        {
            if (sender == null) return;
            var server = (Server)sender;
            Console.WriteLine($"Client {e.Client.ToString} (id: {e.Client.Id}) connected!");
            Player newP = new Player(0, 0, server.ClientCount, e.Client.Id);
            matchState.AddPlayer(newP);
            matchState.playfield.MakeDirty();
        }

        private static void Heartbeat(object? state)
        {
            if (state == null)
            {
                return;
            }
            var server = (Server)state;
            Message message = Message.Create(MessageSendMode.Reliable, (ushort)MessageId.Heartbeat);
            message.AddInt(99);
            server.SendToAll(message);
        }

        [MessageHandler((ushort)MessageId.PlayerInput)]
        private static void HandlePlayerInput(ushort u, Message message)
        {
            PlayerInputStateC2S playerInput = message.GetSerializable<PlayerInputStateC2S>();
            playerInput.clientId = u;
            //Debug.WriteLine($"Got message from {u}: {playerInput}");
            // Check input list to see if we have an input from this client
            bool found = false;
            for (int i = 0; i < playerInputs.Count; i++)
            {
                if (playerInputs[i].clientId == u)
                {
                    playerInputs[i] = playerInput;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                playerInputs.Add(playerInput);
            }
        }

        private static void FixedUpdate(object? state)
        {
            if (state == null)
            {
                return;
            }
            var server = (Server)state;
            server.Update();

            matchState.Update(false, 1f / tickRate, playerInputs);
            playerInputs.Clear(); // Consume the inputs

            // Update clients with all entities (Players and Bombs)
            Message entUpdate = Message.Create(MessageSendMode.Unreliable, (ushort)MessageId.EntityUpdate);
            entUpdate.AddInt(matchState.players.Count);
            foreach (Player p in matchState.players)
            {                
                entUpdate.AddSerializable(p);
            }
            entUpdate.AddInt(matchState.bombs.Count);
            foreach (var b in matchState.bombs)
            {
                entUpdate.AddInt((int)b.EntDictId);
                entUpdate.AddSerializable(b);
            }
            server.SendToAll(entUpdate);

            if (matchState.playfield.IsDirty())
            {
                Message pfieldUpdate = Message.Create(MessageSendMode.Reliable, (ushort)MessageId.PlayfieldUpdate);
                pfieldUpdate.AddSerializable(matchState.playfield);
                server.SendToAll(pfieldUpdate);
            }
        }
    }
}
