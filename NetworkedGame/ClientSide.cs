
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

namespace SuperMineBombersTogether
{
    class ClientSide
    {
        static MatchState matchState = new();
        static Camera2D camera = new();
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
            camera.Target = new Vector2(32, 32);
            camera.Offset = new Vector2(Window.GetScreenWidth() / 2, Window.GetScreenHeight() / 2);
            camera.Rotation = 0.0f;
            camera.Zoom = 6.0f;
        }

        [MessageHandler((ushort)MessageId.Heartbeat)]
        private static void HandleHeartbeat(Message message)
        {
            int someInt = message.GetInt();
        }

        [MessageHandler((ushort)MessageId.EntityUpdate)]
        private static void HandleEntityUpdate(Message message)
        {
            int numEnts = message.GetInt();
            for (int i = 0; i < numEnts; i++)
            {
                var eId = message.GetInt();
                var ePos = new Vector2(message.GetFloat(), message.GetFloat());
                Player? entWithSameId = null;
                foreach (Player p in matchState.players)
                {
                    if (p.id == eId)
                    {
                        entWithSameId = p;
                        break;
                    }
                }
                if (entWithSameId is null)
                {
                    Player newP = new Player(ePos.X, ePos.Y, 0, eId);
                    matchState.AddPlayer(newP);
                }
                else
                {
                    entWithSameId.pos = ePos;
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
                return;
            }
            var client = (Client)state;
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
                    Input.IsKeyPressed(KeyboardKey.Space),
                    Input.IsKeyReleased(KeyboardKey.Space)
                );
                Message message = Message.Create(MessageSendMode.Unreliable, (ushort)PlayerInputStateC2S.Id);
                message.AddSerializable(playerMove);
                client.Send(message);
            }
        }

    }
}
