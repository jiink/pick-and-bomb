using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SuperMineBombersTogether
{
    internal class Common
    {
        public const int tickRate = 20; // updates per second
        public enum MessageId
        {
            Heartbeat = 0,
            PlayerInput,
            PlayerAssign,
            PlayerSetPos,
            PlayerAdd,
            EntityUpdate,
            PlayfieldUpdate
        }
        public enum EntDict
        {
            NONE = 0,
            BOMB,
            TINY_BOMB,
        }
        static public bool isHosting = false;
    }
}
