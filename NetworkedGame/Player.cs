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
        public int id = 0;
        public Vector2 pos = new Vector2(0, 0);
        public Vector2 vel = new Vector2(0, 0);
        float defSpeed = 5; //How fast you walk by default
        int colorIndex = 0;
        
        public Player() { }

        public Player(float x, float y, int colorIndex, int id)
        {
            pos = new Vector2(x, y);
            this.colorIndex = colorIndex;
            this.id = id;
        }

        public void SetPosition(float x, float y)
        {
            pos = new Vector2(x, y);
        }

        public void Update(float deltaTime, PlayerInputStateC2S input)
        {
            vel = input.direction * defSpeed;
            pos += vel * deltaTime;
            
        }

        public void Draw()
        {
            Graphics.DrawCircleV(pos, 0.5f, colorList[colorIndex]);
        }

        public void Serialize(Message message)
        {
            message.AddInt(id);
            message.AddFloat(pos.X);
            message.AddFloat(pos.Y);
            message.AddFloat(vel.X);
            message.AddFloat(vel.Y);
            message.AddInt(colorIndex);
        }

        public void Deserialize(Message message)
        {
            id = message.GetInt();
            pos = new Vector2(message.GetFloat(), message.GetFloat());
            vel = new Vector2(message.GetFloat(), message.GetFloat());
            colorIndex = message.GetInt();
        }
    }
}
