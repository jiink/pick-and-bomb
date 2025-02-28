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
using Raylib_CSharp.Interact;
using Raylib_CSharp.Rendering;
using Raylib_CSharp.Windowing;
using Raylib_CSharp;
using Raylib_CSharp.Camera.Cam2D;
using Raylib_CSharp.Colors;

namespace SuperMineBombersTogether
{
    class SMBT
    {
        static MatchState matchState = new();
        static Camera2D camera = new();
        static PlayerInputState inputs = new();
        static Server? server;
        static Client? client;
        private static double snapshotTimer;

        static public void Start(bool hosting)
        {
            Common.isHosting = hosting;
            if (Common.isHosting)
            {
                server = new();
                server.Start(7777, maxClientCount: 4);
                server.ClientConnected += OnClientConnect;
                server.ClientDisconnected += OnClientDisconnect;
            }
            else
            {
                client = new();
                client.ConnectionFailed += OnConnectionFail;
                client.Disconnected += OnDisconnect;
                client.Connect("127.0.0.1:7777");
            }
            matchState.playfield.Fill();
            matchState.AddPlayer(new Player(), true);
            InitCamera(ref camera);

            while (!Window.ShouldClose())
            {
                /////////// Update ////////////////////////////////////////////
                if (Common.isHosting)
                {
                    server!.Update();
                }
                else
                {
                    client!.Update();
                }
                // --------- inputs ------------------
                if (!inputs.attackPressed && Input.IsKeyPressed(KeyboardKey.Space))
                {
                    inputs.attackPressed = true;
                } else
                {
                    inputs.attackPressed = false;
                }
                if (Input.IsKeyDown(KeyboardKey.Right))
                {
                    inputs.direction.X += 1;
                }
                if (Input.IsKeyDown(KeyboardKey.Left))
                {
                    inputs.direction.X -= 1;
                }
                if (Input.IsKeyDown(KeyboardKey.Up))
                {
                    inputs.direction.Y -= 1;
                }
                if (Input.IsKeyDown(KeyboardKey.Down))
                {
                    inputs.direction.Y += 1;
                }
                if (inputs.direction.Length() > 0)
                {
                    inputs.direction = Vector2.Normalize(inputs.direction);
                }
                // --------- game update ------------------
                float deltaTime = Time.GetFrameTime();
                matchState.Update(false, deltaTime, inputs);
                // --------- networking ---------------
                const double snapshotInterval = 0.25f; // seconds between every update
                if (Time.GetTime() - snapshotTimer > snapshotInterval)
                {
                    snapshotTimer = Time.GetTime();
                    SnapshotUpdate();
                }
                ///////////////////////// Draw /////////////////////////////
                Graphics.BeginDrawing();
                Graphics.BeginMode2D(camera);
                Graphics.ClearBackground(Color.RayWhite);
                matchState.Draw();
                Graphics.EndMode2D();
                Graphics.DrawCircle(0, 0, (float)Math.Cos(Time.GetTime() * 10) * 20 + 40, Color.Black);
                Graphics.EndDrawing();
            }

            Window.Close();
        }

        private static void InitCamera(ref Camera2D camera)
        {
            camera.Target = new Vector2(16, 16);
            camera.Offset = new Vector2(Window.GetScreenWidth() / 2, Window.GetScreenHeight() / 2);
            camera.Rotation = 0.0f;
            camera.Zoom = 10.0f;
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
            Player newP = new(0, 0, server.ClientCount, e.Client.Id);
            matchState.AddPlayer(newP, false);
            matchState.playfield.MakeDirty();
        }

        private static void SnapshotUpdate()
        {

            Console.WriteLine("-- Ding.");
            //    // Update clients with all entities (Players and Bombs)
            //    Message entUpdate = Message.Create(MessageSendMode.Unreliable, (ushort)MessageId.EntityUpdate);
            //    entUpdate.AddInt(matchState.players.Count);
            //    foreach (Player p in matchState.players)
            //    {                
            //        entUpdate.AddSerializable(p);
            //    }
            //    entUpdate.AddInt(matchState.bombs.Count);
            //    foreach (var b in matchState.bombs)
            //    {
            //        entUpdate.AddInt((int)b.EntDictId);
            //        entUpdate.AddSerializable(b);
            //    }
            //    server.SendToAll(entUpdate);

            //    if (matchState.playfield.IsDirty())
            //    {
            //        Message pfieldUpdate = Message.Create(MessageSendMode.Reliable, (ushort)MessageId.PlayfieldUpdate);
            //        pfieldUpdate.AddSerializable(matchState.playfield);
            //        server.SendToAll(pfieldUpdate);
            //    }
        }

            private static void OnDisconnect(object? sender, Riptide.DisconnectedEventArgs e)
        {
            Console.WriteLine($"Disconnected from server: {e.Reason} -- {e.Message}");
        }

        private static void OnConnectionFail(object? sender, ConnectionFailedEventArgs e)
        {
            Console.WriteLine($"Failed to connect: {e.Reason}");
        }

        [MessageHandler((ushort)MessageId.Heartbeat)]
        private static void HandleHeartbeat(Message message)
        {
            int someInt = message.GetInt();
        }

        [MessageHandler((ushort)MessageId.EntityUpdate)]
        private static void HandleEntityUpdate(Message message)
        {
            int numPlayers = message.GetInt();
            for (int i = 0; i < numPlayers; i++)
            {
                Player p = message.GetSerializable<Player>();
                int index = -1;
                for (int j = 0; j < matchState.players.Count; j++)
                {
                    if (matchState.players[j].id == p.id)
                    {
                        index = j;
                        break;
                    }
                }
                if (index != -1)
                {
                    matchState.players[index] = p;
                }
                else
                {
                    matchState.AddPlayer(p, false);
                }
            }
            int numBombs = message.GetInt();
            for (int i = 0; i < numBombs; i++)
            {
                EntDict entDictId = (EntDict)message.GetInt();
                AbstractBomb b;
                switch (entDictId)
                {
                    case EntDict.BOMB:
                        b = message.GetSerializable<Bomb>();
                        break;
                    case EntDict.TINY_BOMB:
                        b = message.GetSerializable<TinyBomb>();
                        break;
                    default:
                        Console.WriteLine("NOT FOUND IN ENTDICT! SKIPPING.");
                        continue;
                        break;
                }
                int bIdx = FindBombIndexFromId(matchState.bombs, b.Id);

                if (bIdx != -1)
                {
                    matchState.bombs[bIdx] = b;
                }
                else
                {
                    matchState.SpawnBomb(b);
                }
            }
        }

        private static int FindBombIndexFromId(List<AbstractBomb> bombs, int id)
        {
            int index = -1;
            for (int j = 0; j < bombs.Count; j++)
            {
                if (bombs[j].Id == id)
                {
                    index = j;
                    break;
                }
            }
            return index;
        }

        [MessageHandler((ushort)MessageId.PlayfieldUpdate)]
        private static void HandlePlayfieldUpdate(Message message)
        {
            matchState.playfield.Deserialize(message);
        }
    }
}
