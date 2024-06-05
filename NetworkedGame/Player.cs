using Raylib_CSharp.Colors;
using Raylib_CSharp.Rendering;
using Riptide;
using SuperMineBombersTogether.PacketTypes;
using System.Numerics;

namespace SuperMineBombersTogether
{
    internal class Player : IMessageSerializable
    {
        public static List<Color> colorList = new List<Color> { Color.Blue, Color.Red, Color.Green, Color.Yellow, Color.Purple, Color.Orange, Color.Brown, Color.Pink, Color.Gray, Color.Black };
        public int Id = 0;
        public Vector2 Pos = new Vector2(0, 0);
        public Vector2 Vel = new Vector2(0, 0);
        float defSpeed = 50; //How fast you walk by default
        int colorIndex = 0;
        
        public Player() { }

        public Player(float x, float y, int colorIndex, int id)
        {
            Pos = new Vector2(x, y);
            this.colorIndex = colorIndex;
            Id = id;
        }

        public void SetPosition(float x, float y)
        {
            Pos = new Vector2(x, y);
        }

        public void Update(float deltaTime, PlayerInputState input)
        {
            Vel = input.direction * defSpeed;
            Pos += Vel * deltaTime;
        }

        public void Draw()
        {
            Graphics.DrawRectangle((int)Pos.X, (int)Pos.Y, 50, 50, colorList[colorIndex]);
        }

        public void Serialize(Message message)
        {
            message.AddFloat(Pos.X);
            message.AddFloat(Pos.Y);
            message.AddFloat(Vel.X);
            message.AddFloat(Vel.Y);
            message.AddInt(colorIndex);
        }

        public void Deserialize(Message message)
        {
            Pos = new Vector2(message.GetFloat(), message.GetFloat());
            Vel = new Vector2(message.GetFloat(), message.GetFloat());
            colorIndex = message.GetInt();
        }
    }
}
