using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Text.Json;
using System.Text.Json.Serialization;

class Communication
{
    public class Base
    {
        public string type { get; set; }
        public string json { get; set; }
    }
    public class Hello
    {
        public string type { get; set; }
        public string msg { get; set; }
    }
}
