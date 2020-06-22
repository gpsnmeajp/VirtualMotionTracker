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
        public string msg { get; set; }
    }
    public class Pos
    {
        public double idx { get; set; }
        public bool en { get; set; }
        public double x { get; set; }
        public double y { get; set; }
        public double z { get; set; }
        public double qx { get; set; }
        public double qy { get; set; }
        public double qz { get; set; }
        public double qw { get; set; }
    }
}
