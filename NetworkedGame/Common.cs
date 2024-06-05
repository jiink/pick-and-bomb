using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SuperMineBombersTogether
{
    internal class Common
    {
        public const int tickRate = 60;
        public enum MessageId
        {
            Heartbeat = 0,
            PlayerInput,
            PlayerAssign,
            MatchState,
        }
    }
}
