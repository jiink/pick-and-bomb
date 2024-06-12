
using Raylib_CSharp;
using Raylib_CSharp.Windowing;
using Raylib_CSharp.Colors;
using Raylib_CSharp.Interact;
using Raylib_CSharp.Rendering;
using Riptide;
using Riptide.Transports;
using static SuperMineBombersTogether.Common;
using SuperMineBombersTogether.PacketTypes;
using System.Reflection.Metadata;
using System.Numerics;
using Raylib_CSharp.Camera.Cam2D;
using SuperMineBombersTogether.Bombs;
using System.Diagnostics;
using System.Reflection;

namespace SuperMineBombersTogether
{
    class ClientSide
    {
        static MatchState matchState = new();
        static Camera2D camera = new();
        static bool attackPressed = false;
        static public void Start()
        {
            Client client = new Client();
            client.ConnectionFailed += OnConnectionFail;
            client.Disconnected += OnDisconnect;
            client.Connect("127.0.0.1:7777");
            Timer timer = new Timer(FixedUpdate, client, 0, 1000 / Common.tickRate);

            InitCamera(ref camera);

            while (!Window.ShouldClose())
            {
                if (!attackPressed && Input.IsKeyPressed(KeyboardKey.Space))
                {
                    attackPressed = true;
                }
                Graphics.BeginDrawing();
                Graphics.BeginMode2D(camera);
                Graphics.ClearBackground(Color.RayWhite);
                matchState.Draw();
                Graphics.EndMode2D();
                Graphics.DrawCircle(0, 0, (float)Math.Cos(Time.GetTime()*10)*20+40, Color.Black);
                Graphics.EndDrawing();
            }

            Window.Close();
        }

        private static void InitCamera(ref Camera2D camera)
        {
            camera.Target = new Vector2(0, 0);
            camera.Offset = new Vector2(Window.GetScreenWidth() / 2, Window.GetScreenHeight() / 2);
            camera.Rotation = 0.0f;
            camera.Zoom = 20.0f;
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
                    matchState.AddPlayer(p);
                }
            }
            int numBombs = message.GetInt();
            for (int i = 0; i < numBombs; i++)
            {
                Type bType = AbstractBomb.GetTypeFromId(message.GetInt());
                Trace.Assert(bType != null);
                MethodInfo method = typeof(Message).GetMethod("GetSerializable");
                MethodInfo generic = method.MakeGenericMethod(bType);
                object bomb = generic.Invoke(message, null);
                AbstractBomb b = (AbstractBomb)bomb;
                //Bomb b = message.GetSerializable<Bomb>();
                int index = -1;
                for (int j = 0; j < matchState.bombs.Count; j++)
                {
                    if (matchState.bombs[j].id == b.id)
                    {
                        index = j;
                        break;
                    }
                }
                if (index != -1)
                {
                    matchState.bombs[index] = b;
                }
                else
                {
                    matchState.SpawnBomb(b);
                }
            }
        }

        [MessageHandler((ushort)MessageId.PlayfieldUpdate)]
        private static void HandlePlayfieldUpdate(Message message)
        {
            matchState.playfield.Deserialize(message);
        }

        private static void OnDisconnect(object? sender, Riptide.DisconnectedEventArgs e)
        {
            Console.WriteLine($"Disconnected from server: {e.Reason} -- {e.Message}");
        }

        private static void OnConnectionFail(object? sender, ConnectionFailedEventArgs e)
        {
            Console.WriteLine($"Failed to connect: {e.Reason}");
        }

        private static void FixedUpdate(object? state)
        {
            if (state == null)
            {
                Console.WriteLine("State is null in FixedUpdate");
                return;
            }
            var client = (Client)state;
            if (client == null)
            {
                Console.WriteLine("Client is null in FixedUpdate");
                return;
            }
            client.Update();

            if (client.IsConnected)
            {
                Vector2 moveDir = new Vector2(0, 0);
                if (Input.IsKeyDown(KeyboardKey.Right))
                {
                    moveDir.X += 1;
                }
                if (Input.IsKeyDown(KeyboardKey.Left))
                {
                    moveDir.X -= 1;
                }
                if (Input.IsKeyDown(KeyboardKey.Up))
                {
                    moveDir.Y -= 1;
                }
                if (Input.IsKeyDown(KeyboardKey.Down))
                {
                    moveDir.Y += 1;
                }
                if (moveDir.Length() > 0)
                {
                    moveDir = Vector2.Normalize(moveDir);
                }
                PlayerInputStateC2S playerMove = new PlayerInputStateC2S(
                    moveDir,
                    Input.IsKeyDown(KeyboardKey.Space),
                    attackPressed,
                    Input.IsKeyReleased(KeyboardKey.Space)
                );
                attackPressed = false;
                Message message = Message.Create(MessageSendMode.Unreliable, (ushort)PlayerInputStateC2S.Id);
                message.AddSerializable(playerMove);
                client.Send(message);
            }
        }

    }
}
