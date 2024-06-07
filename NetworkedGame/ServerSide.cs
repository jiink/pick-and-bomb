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

namespace SuperMineBombersTogether
{
    class ServerSide
    {
        static MatchState matchState = new MatchState();
        static List<PlayerInputState> playerInputs = new List<PlayerInputState>();
        static int counter = 0;
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
            Console.WriteLine($"Client {e.Client.ToString} (id: {e.Client.Id}) connected!");
            matchState.AddPlayer(new Player(0, 0, server.ClientCount, e.Client.Id));
            // Assign player number to client
            //Message message = Message.Create(MessageSendMode.Reliable, (ushort)PlayerAssignS2C.Id);
            //PlayerAssignS2C playerAssign = new PlayerAssignS2C(matchState.Players.Count - 1);
            //message.AddSerializable(playerAssign);
            //server.Send(message, e.Client);
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

        [MessageHandler((ushort)MessageId.PlayerInput)]
        private static void HandlePlayerInput(ushort u, Message message)
        {
            PlayerInputState playerInput = message.GetSerializable<PlayerInputState>();
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

            matchState.Update(1f / tickRate, playerInputs);
            playerInputs.Clear(); // Consume the inputs

            Message message = Message.Create(MessageSendMode.Unreliable, (ushort)MessageId.MatchState);
            message.AddSerializable(matchState);
            if (counter++ > 10)
            {
                RiptideLogger.Log(LogType.Debug, $"Server sending {(message.WrittenBits / 8) * tickRate} byte/s");
                counter = 0;
            }
            server.SendToAll(message);
        }
    }
}
