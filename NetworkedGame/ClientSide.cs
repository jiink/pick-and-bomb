
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


namespace SuperMineBombersTogether
{
    
    class ClientSide
    {
        static int playerNum = 0;
        static GameState gameState = new GameState();
        static Player myPlayer = null;
        static float deltaX = 0;
        static float deltaY = 0;
        static public void Start()
        {
            Client client = new Client();
            client.ConnectionFailed += OnConnectionFail;
            client.Disconnected += OnDisconnect;
            client.Connect("127.0.0.1:7777");
            Timer timer = new Timer(FixedUpdate, client, 0, 1000 / Common.tickRate);

            while (!Window.ShouldClose())
            {
                if (gameState.Players.Count > playerNum)
                {
                    myPlayer = gameState.Players[playerNum];
                }
                if (myPlayer == null)
                {
                    continue;
                }
                const float moveSpeed = 300.0f;
                deltaX = 0;
                deltaY = 0;
                if (Input.IsKeyDown(KeyboardKey.Right)) { 
                    myPlayer.Vel.X = moveSpeed;
                    deltaX += moveSpeed;
                }
                if (Input.IsKeyDown(KeyboardKey.Left)){
                    myPlayer.Vel.X = -moveSpeed;
                    deltaX -= moveSpeed;
                }
                if (Input.IsKeyDown(KeyboardKey.Up)){
                    myPlayer.Vel.Y = -moveSpeed;
                    deltaY -= moveSpeed;
                }
                if (Input.IsKeyDown(KeyboardKey.Down)){
                    myPlayer.Vel.Y = moveSpeed;
                    deltaY += moveSpeed;
                }

                gameState.Update(Time.GetFrameTime());

                Graphics.BeginDrawing();
                Graphics.ClearBackground(Color.RayWhite);
                DrawGameState(gameState);
                Graphics.EndDrawing();
            }

            Window.Close();
        }

        private static void DrawGameState(GameState gameState)
        {
            foreach (Player player in gameState.Players)
            {
                Graphics.DrawRectangle((int)player.Pos.X, (int)player.Pos.Y, 50, 50, player.Color);
            }
        }

        [MessageHandler((ushort)MessageId.Heartbeat)]
        private static void HandleHeartbeat(Message message)
        {
            int someInt = message.GetInt();
            //Console.WriteLine($"I got {someInt}");
        }

        [MessageHandler((ushort)MessageId.PlayerAssign)]
        private static void HandlePlayerAssign(Message message)
        {
            PlayerAssignS2C playerAssign = message.GetSerializable<PlayerAssignS2C>();
            playerNum = playerAssign.PlayerNumber;
            //player.Color = Player.colorList[playerNum];
            Console.WriteLine($"I am player {playerNum}");
        }

        [MessageHandler((ushort)MessageId.GameState)]
        private static void HandleGameState(Message message)
        {
            gameState.Deserialize(message);
        }

        private static void OnDisconnect(object? sender, Riptide.DisconnectedEventArgs e)
        {
            Console.WriteLine($"Disconnected from server: {e.Reason}");
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

            if (client.IsConnected && gameState.Players.Count > playerNum)
            {
                PlayerMoveC2S playerMove = new PlayerMoveC2S(playerNum, deltaX, deltaY);
                Message message2 = Message.Create(MessageSendMode.Unreliable, (ushort)PlayerMoveC2S.Id);
                message2.AddSerializable(playerMove);
                client.Send(message2);
            }
        }

    }
}
