using Raylib_CSharp.Windowing;
using Raylib_CSharp;
using Riptide.Utils;
using System.Diagnostics;
using Raylib_CSharp.Interact;
using Raylib_CSharp.Colors;
using Raylib_CSharp.Rendering;

namespace SuperMineBombersTogether
{
    internal class Program
    {
        static void Main(string[] args)
        {
            RiptideLogger.Initialize(Console.WriteLine, false);

            const int screenWidth = 640;
            const int screenHeight = 480;

            Raylib.SetConfigFlags(ConfigFlags.VSyncHint);
            Time.SetTargetFps(Window.GetMonitorRefreshRate(Window.GetCurrentMonitor()));
            Window.Init(screenWidth, screenHeight, "Game");
            string choice = string.Empty;
            while (!Window.ShouldClose() && string.IsNullOrEmpty(choice))
            {
                if (Input.IsKeyPressed(KeyboardKey.H))
                {
                    choice = "host";
                } else if (Input.IsKeyPressed(KeyboardKey.J))
                {
                    choice = "join";
                }

                Graphics.BeginDrawing();
                Graphics.ClearBackground(Color.Pink);
                Graphics.DrawText("Host", 40, 100, 30, Color.Black);
                Graphics.DrawText("Join 127.0.0.1:7777", 40, 200, 30, Color.Black);
                Graphics.EndDrawing();
            }

            if (choice == "host")   
            {
                ServerSide.Start();
                ClientSide.Start();
            }
            else if (choice == "join")
            {
                ClientSide.Start();
            }
        }
    }
}
