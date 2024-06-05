using Raylib_CSharp.Colors;
using System.Numerics;

namespace SuperMineBombersTogether
{
    internal class Player
    {
        public static List<Color> colorList = new List<Color> { Color.Blue, Color.Red, Color.Green, Color.Yellow, Color.Purple, Color.Orange, Color.Brown, Color.Pink, Color.Gray, Color.Black };
        public Vector2 Pos = new Vector2(0, 0);
        public Vector2 Vel = new Vector2(0, 0);
        public Color Color { get; private set; }

        public Player(float x, float y, Color c)
        {
            Pos = new Vector2(x, y);
            Color = c;
        }

        public void SetPosition(float x, float y)
        {
            Pos = new Vector2(x, y);
        }

        public void Update(float deltaTime)
        {
            Pos += Vel * deltaTime;
        }
    }
}
